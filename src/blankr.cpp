#include "anomalies.hpp"

struct BlankR : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        NUM_INPUTS
    };
    enum OutputIds {
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    BlankR() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }
    void process(const ProcessArgs &args) override { }
};

struct BlankRWidget : ModuleWidget {
    BlankPanel *panel;
    Widget *rightHandle;

    BlankRWidget(BlankR *module) {
        setModule(module);
        box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        panel = new BlankPanel(COLOR_PURPLE_DARK);
        panel->box.size = box.size;
        addChild(panel);

        ModuleResizeHandle *leftHandle = new ModuleResizeHandle;
		ModuleResizeHandle *rightHandle = new ModuleResizeHandle;
		rightHandle->right = true;
		this->rightHandle = rightHandle;
		addChild(leftHandle);
        addChild(rightHandle);
    }

    void step() override {
        panel->box.size = box.size;
        rightHandle->box.pos.x = box.size.x - rightHandle->box.size.x;
        ModuleWidget::step();
    }

    json_t *toJson() override {
		json_t *rootJ = ModuleWidget::toJson();

		// width
		json_object_set_new(rootJ, "width", json_real(box.size.x));

		return rootJ;
	}

	void fromJson(json_t *rootJ) override {
		ModuleWidget::fromJson(rootJ);

		// width
		json_t *widthJ = json_object_get(rootJ, "width");
		if (widthJ)
			box.size.x = json_number_value(widthJ);
    }
};

Model *modelBlankR = createModel<BlankR, BlankRWidget>("expanse");
