#include "anomalies.hpp"

struct DualAttenuverter : Module {
    enum ParamIds {
		A_SCALE_PARAM,
		A_OFFSET_PARAM,
		B_SCALE_PARAM,
		B_OFFSET_PARAM,
	    NUM_PARAMS
    };
    enum InputIds {
		A_INPUT,
		B_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
		A_OUTPUT,
		B_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    DualAttenuverter() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(A_SCALE_PARAM, -3.0f, 3.0f, 1.0f, "scale");
		configParam(A_OFFSET_PARAM, -10.0f, 10.0f, 0.0f, "offset", " v");
		configParam(B_SCALE_PARAM, -3.0f, 3.0f, 1.0f, "scale");
		configParam(B_OFFSET_PARAM, -10.0f, 10.0f, 0.0f, "offset", " v");
		configInput(A_INPUT, "a");
		configInput(B_INPUT, "b");
		configOutput(A_OUTPUT, "a");
		configOutput(B_OUTPUT, "b");
    }

    void process(const ProcessArgs &args) override;
};

void DualAttenuverter::process(const ProcessArgs &args) {
	if (outputs[A_OUTPUT].isConnected()) {
		int a_channels = inputs[A_INPUT].getChannels(); // enable polyphony
		for (int c = 0; c < a_channels; c++) {
			float a_out = clamp(inputs[A_INPUT].getPolyVoltage(c) * params[A_SCALE_PARAM].getValue()
						+ params[A_OFFSET_PARAM].getValue(), -12.0f, 12.0f);
			outputs[A_OUTPUT].setVoltage(a_out, c);
		}
    	outputs[A_OUTPUT].setChannels(a_channels);
	}
	if (outputs[B_OUTPUT].isConnected()) {
		int b_channels = inputs[B_INPUT].getChannels(); // enable polyphony
		for (int c = 0; c < b_channels; c++) {
			float b_out = clamp(inputs[B_INPUT].getPolyVoltage(c) * params[B_SCALE_PARAM].getValue()
			+ params[B_OFFSET_PARAM].getValue(), -12.0f, 12.0f);
			outputs[B_OUTPUT].setVoltage(b_out, c);
		}
		outputs[B_OUTPUT].setChannels(b_channels);
	}
}

struct DualAttenuverterWidget : ModuleWidget {
    DualAttenuverterWidget(DualAttenuverter *module) {
        setModule(module);
        box.size = Vec(2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/face/2at.svg")));

		addParam(createParam<KnobS>(Vec(4, 28), module, DualAttenuverter::A_SCALE_PARAM));
		addParam(createParam<KnobS>(Vec(4, 68), module, DualAttenuverter::A_OFFSET_PARAM));
		addInput(createInput<InPort>(Vec(5, 110), module, DualAttenuverter::A_INPUT));
		addOutput(createOutput<OutPort>(Vec(5, 150), module, DualAttenuverter::A_OUTPUT));

		addParam(createParam<KnobS>(Vec(4, 198), module, DualAttenuverter::B_SCALE_PARAM));
		addParam(createParam<KnobS>(Vec(4, 238), module, DualAttenuverter::B_OFFSET_PARAM));
		addInput(createInput<InPort>(Vec(5, 280), module, DualAttenuverter::B_INPUT));
		addOutput(createOutput<OutPort>(Vec(5, 320), module, DualAttenuverter::B_OUTPUT));
	}
};

Model *modelDualAttenuverter = createModel<DualAttenuverter, DualAttenuverterWidget>("2at");
