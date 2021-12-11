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

    float width = RACK_GRID_WIDTH * 6;

    BlankR() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }
    void process(const ProcessArgs &args) override { }

    json_t *dataToJson() override {
        json_t *rootJ = json_object();
        json_object_set_new(rootJ, "width", json_real(width));
        return rootJ;
    }

    void dataFromJson(json_t *rootJ) override {
        json_t *widthJ = json_object_get(rootJ, "width");
        if (widthJ) width = json_number_value(widthJ);
    }
};

struct BlankRWidget : ModuleWidget {
    BlankPanel *panel;
    Widget *rightHandle;

    BlankRWidget(BlankR *module) {
        setModule(module);
        box.size = Vec(module ? module->width : RACK_GRID_WIDTH * 6, RACK_GRID_HEIGHT);
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
        if (box.size.x < RACK_GRID_WIDTH * 6) box.size.x = RACK_GRID_WIDTH * 6;
        rightHandle->box.pos.x = box.size.x - rightHandle->box.size.x;
        BlankR *blankR = dynamic_cast<BlankR*>(module);
        if (blankR) blankR->width = box.size.x;
        ModuleWidget::step();
    }
};

Model *modelBlankR = createModel<BlankR, BlankRWidget>("expanse");
