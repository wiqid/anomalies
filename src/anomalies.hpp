#include <rack.hpp>
#include "anomalous-math.hpp"

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin *pluginInstance;

////////// color scheme //////////

static const NVGcolor COLOR_BLACK = nvgRGB(0x00, 0x00, 0x00);
static const NVGcolor COLOR_GREY_DARK = nvgRGB(0x20, 0x20, 0x20);
static const NVGcolor COLOR_PURPLE_DARK = nvgRGB(0x21, 0x1e, 0x29);

////////// custom widgets //////////

// drawable blank adapted from rack::core
struct BlankPanel : Widget {
	Widget *panelBorder;
	NVGcolor color;

	BlankPanel(NVGcolor _color) {
		panelBorder = new PanelBorder;
		color = _color;
		addChild(panelBorder);
	}

	void step() override {
		panelBorder->box.size = box.size;
		Widget::step();
	}

	void draw(const DrawArgs &args) override {
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0.0, 0.0, box.size.x, box.size.y);
		nvgFillColor(args.vg, color);
		nvgFill(args.vg);
		Widget::draw(args);
	}
};

// resize handle adapted from rack::core
struct ModuleResizeHandle : OpaqueWidget {
	bool right = false;
	Vec dragPos;
	Rect originalBox;

	ModuleResizeHandle() {
		box.size = Vec(1 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	}

	void onDragStart(const event::DragStart &e) override {
		if (e.button != GLFW_MOUSE_BUTTON_LEFT)
			return;

		dragPos = APP->scene->rack->mousePos;
		ModuleWidget *mw = getAncestorOfType<ModuleWidget>();
		assert(mw);
		originalBox = mw->box;
	}

	void onDragMove(const event::DragMove &e) override {
		ModuleWidget *mw = getAncestorOfType<ModuleWidget>();
		assert(mw);

		Vec newDragPos = APP->scene->rack->mousePos;
		float deltaX = newDragPos.x - dragPos.x;

		Rect newBox = originalBox;
		Rect oldBox = mw->box;
		const float minWidth = 3 * RACK_GRID_WIDTH;
		if (right) {
			newBox.size.x += deltaX;
			newBox.size.x = std::fmax(newBox.size.x, minWidth);
			newBox.size.x = std::round(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
		}
		else {
			newBox.size.x -= deltaX;
			newBox.size.x = std::fmax(newBox.size.x, minWidth);
			newBox.size.x = std::round(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
			newBox.pos.x = originalBox.pos.x + originalBox.size.x - newBox.size.x;
		}

		// Set box and test whether it's valid
		mw->box = newBox;
		if (!APP->scene->rack->requestModulePos(mw, newBox.pos)) {
			mw->box = oldBox;
		}
	}
};


struct KnobS : app::SvgKnob {
    KnobS() {
        minAngle = -0.83 * M_PI;
        maxAngle = 0.83 * M_PI;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/knob_s.svg")));
    }
};

struct KnobM : app::SvgKnob {
    KnobM() {
        minAngle = -0.83 * M_PI;
        maxAngle = 0.83 * M_PI;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/knob_m.svg")));
    }
};

struct InPort : app::SvgPort {
    InPort() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/inport_sc.svg")));
    }
};

struct OutPort : app::SvgPort {
    OutPort() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/component/outport_cu.svg")));
    }
};


// Declare each Model, defined in each module source file
// extern Model *modelMyModule;
extern Model *modelBlankR;
extern Model *modelLanguor;
extern Model *modelHalvorsen;
extern Model *modelLorenz;
extern Model *modelThomas;
extern Model *modelSakarya;
extern Model *modelDadras;
extern Model *modelSprottLinzF;
extern Model *modelDualAttenuverter;
