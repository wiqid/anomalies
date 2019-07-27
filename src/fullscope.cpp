#include "anomalies.hpp"

#define BUFFER_SIZE 512

struct FullScope : Module {
	enum ParamIds {
		X_SCALE_PARAM,
		X_POS_PARAM,
		Y_SCALE_PARAM,
		Y_POS_PARAM,
		TIME_PARAM,
		LISSAJOUS_PARAM,
		TRIG_PARAM,
		EXTERNAL_PARAM,
		ROTATION_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		X_INPUT,
		Y_INPUT,
		TRIG_INPUT,
		COLOR_INPUT,
		TIME_INPUT,
		ROTATION_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
    enum LightIds {
        NUM_LIGHTS
    };

	float bufferX[BUFFER_SIZE] = {};
	float bufferY[BUFFER_SIZE] = {};
	int bufferIndex = 0;
	float frameIndex = 0;

	bool lissajous = true;
	bool showstats = false;
	dsp::SchmittTrigger resetTrigger;

	FullScope() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(X_POS_PARAM, -10.0, 10.0, 0.0, "x position", " v");
		configParam(Y_POS_PARAM, -10.0, 10.0, 0.0, "y position", " v");
		configParam(X_SCALE_PARAM, -2.f, 8.f, 0.f, "x scale", " v", 1/2.f, 10);
		configParam(Y_SCALE_PARAM, -2.f, 8.f, 0.f, "y scale", " v", 1/2.f, 10);
		configParam(ROTATION_PARAM, -10.0, 10.0, 0, "rotation");
		configParam(TIME_PARAM, 4.f, 16.f, 10.f, "time");
	}
	void process(const ProcessArgs &args) override;

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "lissajous", json_integer((int) lissajous));
		json_object_set_new(rootJ, "showstats", json_integer((int) showstats));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		json_t *sumJ = json_object_get(rootJ, "lissajous");
		if (sumJ)
			lissajous = json_integer_value(sumJ);

		json_t *statJ = json_object_get(rootJ, "showstats");
		if (statJ)
			showstats = json_integer_value(statJ);
	}

	void onReset() override {
		lissajous = true;
		showstats = false;
	}
};

void FullScope::process(const ProcessArgs &args) {
	// Compute time
	float deltaTime = std::pow(2.f, -params[TIME_PARAM].getValue() + inputs[TIME_INPUT].getVoltage());
	int frameCount = (int) std::ceil(deltaTime * args.sampleRate);

	// Add frame to buffer
	if (bufferIndex < BUFFER_SIZE) {
		if (++frameIndex > frameCount) {
			frameIndex = 0;
			bufferX[bufferIndex] = inputs[X_INPUT].getVoltage();
			bufferY[bufferIndex] = inputs[Y_INPUT].getVoltage();
			bufferIndex++;
		}
	}

	// Are we waiting on the next trigger?
	if (bufferIndex >= BUFFER_SIZE) {
		// Trigger immediately if external but nothing plugged in, or in Lissajous mode
		if (lissajous) {
			bufferIndex = 0;
			frameIndex = 0;
			return;
		}

		// Reset the Schmitt trigger so we don't trigger immediately if the input is high
		if (frameIndex == 0) {
			resetTrigger.reset();
		}
		frameIndex++;

		// Must go below 0.1V to trigger
		// resetTrigger.setThresholds(params[TRIG_PARAM].getValue() - 0.1, params[TRIG_PARAM].getValue());
		float gate = inputs[X_INPUT].getVoltage();

		// Reset if triggered
		float holdTime = 0.1;
		if (resetTrigger.process(gate) || (frameIndex >= args.sampleRate * holdTime)) {
			bufferIndex = 0; frameIndex = 0; return;
		}

		// Reset if we've waited too long
		if (frameIndex >= args.sampleRate * holdTime) {
			bufferIndex = 0; frameIndex = 0; return;
		}
	}
}

struct FullScopeDisplay : TransparentWidget {
	FullScope *module;
	int frame = 0;
	float rot = 0;
	std::shared_ptr<Font> font;

	struct Stats {
		float vmin, vmax;
		void calculate(float *values) {
			// vrms = 0.0;
			vmax = -INFINITY;
			vmin = INFINITY;
			for (int i = 0; i < BUFFER_SIZE; i++) {
				float v = values[i];
				// vrms += v*v;
				vmax = fmaxf(vmax, v);
				vmin = fminf(vmin, v);
			}
			// vrms = sqrtf(vrms / BUFFER_SIZE);
			// vpp = vmax - vmin;
		}
	};
	Stats statsX, statsY;

	FullScopeDisplay() {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/font/OfficeCodePro-Light.ttf"));
	}

	inline float rescalefjw(float x, float xMin, float xMax, float yMin, float yMax) {
		return yMin + (x - xMin) / (xMax - xMin) * (yMax - yMin);
	}

	void drawWaveform(const DrawArgs &args, float *valuesX, float *valuesY) {
		if (!valuesX)
			return;
		nvgSave(args.vg);
		Rect b = Rect(Vec(0, 0), box.size);
		nvgScissor(args.vg, b.pos.x, b.pos.y, b.size.x, b.size.y);

		float rotRate = rescalefjw(module->params[FullScope::ROTATION_PARAM].getValue() + module->inputs[FullScope::ROTATION_INPUT].getVoltage(), 0, 10, 0, 0.5);
		if(rotRate != 0){
			nvgTranslate(args.vg, box.size.x/2.0, box.size.y/2.0);
			nvgRotate(args.vg, rot+=rotRate);
			nvgTranslate(args.vg, -box.size.x/2.0, -box.size.y/2.0);
		} else {
			nvgRotate(args.vg, 0);
		}

		nvgBeginPath(args.vg);
		// Draw maximum display left to right
		for (int i = 0; i < BUFFER_SIZE; i++) {
			float x, y;
			if (valuesY) {
				x = valuesX[i] / 2.f + 0.5f;
				y = valuesY[i] / 2.f + 0.5f;
			}
			else {
				x = (float)i / (BUFFER_SIZE - 1);
				y = valuesX[i] / 2.f + 0.5f;
			}
			Vec p;
			p.x = b.pos.x + b.size.x * x;
			p.y = b.pos.y + b.size.y * (1.0 - y);
			if (i == 0)
				nvgMoveTo(args.vg, p.x, p.y);
			else
				nvgLineTo(args.vg, p.x, p.y);
		}
		nvgLineCap(args.vg, NVG_ROUND);
		nvgMiterLimit(args.vg, 2.f);
		nvgStrokeWidth(args.vg, 1.5f);
		nvgGlobalCompositeOperation(args.vg, NVG_LIGHTER);
		nvgStroke(args.vg);
		nvgResetScissor(args.vg);
		nvgRestore(args.vg);
	}

	void drawStats(const DrawArgs &args, Vec pos, const char *title, Stats *stats) {
		nvgFontSize(args.vg, 11);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, -0.5);

		nvgFillColor(args.vg, nvgRGBA(0xf4, 0xbd, 0x8d, 0xc0));
		nvgText(args.vg, pos.x, pos.y + 11, title, NULL);

		nvgFillColor(args.vg, nvgRGBA(0xf4, 0xbd, 0x8d, 0xc0));
		pos = pos.plus(Vec(20, 11));

		std::string text;
		text = "max";
		text += isNear(stats->vmax, 0.f, 100.f) ? string::f("% 6.2f", stats->vmax) : "  ---";
		nvgText(args.vg, pos.x, pos.y, text.c_str(), NULL);
		text = "min";
		text += isNear(stats->vmin, 0.f, 100.f) ? string::f("% 6.2f", stats->vmin) : "  ---";
		nvgText(args.vg, pos.x + 55, pos.y, text.c_str(), NULL);
	}

	void draw(const DrawArgs &args) override {
		if(!module) return;

		float gainX = powf(2.0, roundf(module->params[FullScope::X_SCALE_PARAM].getValue()));
		float gainY = powf(2.0, roundf(module->params[FullScope::Y_SCALE_PARAM].getValue()));
		float offsetX = module->params[FullScope::X_POS_PARAM].getValue();
		float offsetY = module->params[FullScope::Y_POS_PARAM].getValue();

		float valuesX[BUFFER_SIZE];
		float valuesY[BUFFER_SIZE];
		for (int i = 0; i < BUFFER_SIZE; i++) {
			int j = i;
			// Lock display to buffer if buffer update deltaTime <= 2^-11
			if (module->lissajous)
				j = (i + module->bufferIndex) % BUFFER_SIZE;
			valuesX[i] = (module->bufferX[j] + offsetX) * gainX / 10.0;
			valuesY[i] = (module->bufferY[j] + offsetY) * gainY / 10.0;
		}

		// color
		if(module->inputs[FullScope::COLOR_INPUT].isConnected()){
			float hue = rescalefjw(module->inputs[FullScope::COLOR_INPUT].getVoltage(), 0.0, 6.0, 0, 1.0);
			nvgStrokeColor(args.vg, nvgHSLA(hue, 0.5, 0.5, 0xc0));
		} else {
			nvgStrokeColor(args.vg, nvgRGBA(0xf4, 0xbd, 0x8d, 0xc0)); // mac 'n' cheese
			//nvgStrokeColor(args.vg, nvgRGBA(0x9b, 0xcc, 0x82, 0xc0)); // pistachio
		}

		// draw waveforms
		if (module->lissajous) {
			// X x Y
			if (module->inputs[FullScope::X_INPUT].isConnected() || module->inputs[FullScope::Y_INPUT].isConnected()) {
				drawWaveform(args, valuesX, valuesY);
			}
		}
		else {
			// Y
			if (module->inputs[FullScope::Y_INPUT].isConnected()) {
				drawWaveform(args, valuesY, NULL);
			}

			// X
			if (module->inputs[FullScope::X_INPUT].isConnected()) {
				nvgStrokeColor(args.vg, nvgRGBA(0xb0, 0x8d, 0xf4, 0xc0)); // bright lavender
				drawWaveform(args, valuesX, NULL);
			}
		}

		// if stats enabled, calculate and show them
		if (module->showstats) {
			if (++frame >= 4) {
				frame = 0;
				statsX.calculate(module->bufferX);
				statsY.calculate(module->bufferY);
			}

			drawStats(args, Vec(18, 0), "  x", &statsX);
			drawStats(args, Vec(144, 0), "| y", &statsY);
		}
	}
};

struct FullScopeWidget : ModuleWidget {
    BlankPanel *panel;
    Widget *rightHandle;
	TransparentWidget *display;
	FullScopeWidget(FullScope *module);
	void step() override;
	json_t *toJson() override;
	void fromJson(json_t *rootJ) override;
	void appendContextMenu(Menu *menu) override;
};

FullScopeWidget::FullScopeWidget(FullScope *module) {
	setModule(module);
	box.size = Vec(26 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    panel = new BlankPanel(COLOR_BLACK);
	panel->box.size = box.size;
	addChild(panel);

	ModuleResizeHandle *leftHandle = new ModuleResizeHandle;
	ModuleResizeHandle *rightHandle = new ModuleResizeHandle;
	rightHandle->right = true;
	this->rightHandle = rightHandle;
	addChild(leftHandle);
	addChild(rightHandle);

	FullScopeDisplay *display = new FullScopeDisplay();
	display->module = module;
	display->box.pos = Vec(0, 0);
	display->box.size = Vec(box.size.x, box.size.y);
	addChild(display);
	this->display = display;

	int compX = 5, compY = -17, adder = 22;
	addInput(createInput<InPortMini>(Vec(compX, compY+=adder), module, FullScope::X_INPUT));
	addInput(createInput<InPortMini>(Vec(compX, compY+=adder), module, FullScope::Y_INPUT));
	addInput(createInput<InPortMini>(Vec(compX, compY+=adder), module, FullScope::COLOR_INPUT));
	addInput(createInput<InPortMini>(Vec(compX, compY+=adder), module, FullScope::ROTATION_INPUT));
	addInput(createInput<InPortMini>(Vec(compX, compY+=adder), module, FullScope::TIME_INPUT));

	addParam(createParam<KnobMini>(Vec(compX, compY+=adder), module, FullScope::X_POS_PARAM));
	addParam(createParam<KnobMini>(Vec(compX, compY+=adder), module, FullScope::Y_POS_PARAM));
	addParam(createParam<KnobMiniSnap>(Vec(compX, compY+=adder), module, FullScope::X_SCALE_PARAM));
	addParam(createParam<KnobMiniSnap>(Vec(compX, compY+=adder), module, FullScope::Y_SCALE_PARAM));
	addParam(createParam<KnobMini>(Vec(compX, compY+=adder), module, FullScope::ROTATION_PARAM));
	addParam(createParam<KnobMini>(Vec(compX, compY+=adder), module, FullScope::TIME_PARAM));

	addChild(createWidget<Logo>(Vec(7, 361)));
}

void FullScopeWidget::step() {
	panel->box.size = box.size;
	display->box.size = Vec(box.size.x, box.size.y);
	rightHandle->box.pos.x = box.size.x - rightHandle->box.size.x;
	ModuleWidget::step();
}

json_t *FullScopeWidget::toJson() {
	json_t *rootJ = ModuleWidget::toJson();
	json_object_set_new(rootJ, "width", json_real(box.size.x));
	json_object_set_new(rootJ, "height", json_real(box.size.y));
	return rootJ;
}

void FullScopeWidget::fromJson(json_t *rootJ) {
	ModuleWidget::fromJson(rootJ);
	json_t *widthJ = json_object_get(rootJ, "width");
	if (widthJ)
		box.size.x = json_number_value(widthJ);
	json_t *heightJ = json_object_get(rootJ, "height");
	if (heightJ)
		box.size.y = json_number_value(heightJ);
}

struct FullScopeLissajousModeMenuItem : MenuItem {
	FullScope *fullScope;
	void onAction(const event::Action &e) override {
		fullScope->lissajous ^= true;
	}
};

struct StatsMenuItem : MenuItem {
	FullScope *fullScope;
	void onAction(const event::Action &e) override {
		fullScope->showstats ^= true;
	}
};

void FullScopeWidget::appendContextMenu(Menu *menu) {
	FullScope *fullScope = dynamic_cast<FullScope*>(module);
	assert(fullScope);

	menu->addChild(new MenuSeparator());

	FullScopeLissajousModeMenuItem *lissMenuItem = createMenuItem<FullScopeLissajousModeMenuItem>("lissajous mode", CHECKMARK(fullScope->lissajous));
	lissMenuItem->fullScope = fullScope;
	menu->addChild(lissMenuItem);

	StatsMenuItem *statItem = createMenuItem<StatsMenuItem>("show statistics", CHECKMARK(fullScope->showstats));
	statItem->fullScope = fullScope;
	menu->addChild(statItem);
}

Model *modelFullScope = createModel<FullScope, FullScopeWidget>("fullscope");
