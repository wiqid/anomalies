#include "anomalies.hpp"

struct Halvorsen : Module {
    enum ParamIds {
		SPEED_PARAM,
		SHAPE_PARAM,
		AMP_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        NUM_INPUTS
    };
    enum OutputIds {
		X_OUTPUT,
		Y_OUTPUT,
		Z_OUTPUT,
		T_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

	HalvorsenAttractor halvorsen;

	static constexpr float SHAPE_PARAM_MIN = 1.23f; // smaller escapes to inf
	static constexpr float SHAPE_PARAM_MAX = 1.63f; // higher is non-chaotic
	static constexpr float SPEED_PARAM_MIN = 0.001f;
	static constexpr float SPEED_PARAM_MAX = 1.0f;
	static constexpr float AMP_PARAM_MIN = 0.1f;
	static constexpr float AMP_PARAM_MAX = 10.0f;
	static constexpr float AMP_PARAM_DEFAULT = 5.0f; // ±5v bipolar CV
	float amplitude = AMP_PARAM_DEFAULT * 0.2f; // default amplification is 1
	float tfactor; // mystery 4th dimension (tempered to within normal amp range)

    Halvorsen() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(SPEED_PARAM, SPEED_PARAM_MIN, SPEED_PARAM_MAX, HalvorsenAttractor::DEFAULT_SPEED, "speed");
		configParam(SHAPE_PARAM, SHAPE_PARAM_MIN, SHAPE_PARAM_MAX, HalvorsenAttractor::DEFAULT_A, "shape");
		configParam(AMP_PARAM, AMP_PARAM_MIN, AMP_PARAM_MAX, AMP_PARAM_DEFAULT, "scale");
		configOutput(X_OUTPUT, "x");
		configOutput(Y_OUTPUT, "y");
		configOutput(Z_OUTPUT, "z");
		configOutput(T_OUTPUT, "t factor");		
    }
    
    void process(const ProcessArgs &args) override;
};

void Halvorsen::process(const ProcessArgs &args) {
	if (outputs[X_OUTPUT].isConnected()
		|| outputs[Y_OUTPUT].isConnected()
		|| outputs[Z_OUTPUT].isConnected()
		|| outputs[T_OUTPUT].isConnected()) {
		halvorsen.a = clamp(params[SHAPE_PARAM].getValue(), SHAPE_PARAM_MIN, SHAPE_PARAM_MAX);
		halvorsen.speed = clamp(params[SPEED_PARAM].getValue(), SPEED_PARAM_MIN, SPEED_PARAM_MAX) * 1.5f;
		amplitude = clamp(params[AMP_PARAM].getValue(), AMP_PARAM_MIN, AMP_PARAM_MAX) * 0.2f;
		halvorsen.process(1.0f / args.sampleRate);
		tfactor = halvorsen.x + halvorsen.y - halvorsen.z;
		outputs[X_OUTPUT].setVoltage((0.5f * halvorsen.x + 1.6f) * amplitude);
		outputs[Y_OUTPUT].setVoltage((0.5f * halvorsen.y + 1.6f) * amplitude);
		outputs[Z_OUTPUT].setVoltage((0.5f * halvorsen.z + 1.6f) * amplitude);
		outputs[T_OUTPUT].setVoltage((0.23f * tfactor + 1.6f) * amplitude);
	}
}

struct HalvorsenWidget : ModuleWidget {
    HalvorsenWidget(Halvorsen *module) {
        setModule(module);
        box.size = Vec(2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/face/hal.svg")));

		addParam(createParam<KnobS>(Vec(4, 35), module, Halvorsen::SPEED_PARAM));
		addParam(createParam<KnobS>(Vec(4, 85), module, Halvorsen::SHAPE_PARAM));
		addParam(createParam<KnobS>(Vec(4, 135), module, Halvorsen::AMP_PARAM));
		addOutput(createOutput<OutPort>(Vec(5, 200), module, Halvorsen::X_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(5, 240), module, Halvorsen::Y_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(5, 280), module, Halvorsen::Z_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(5, 320), module, Halvorsen::T_OUTPUT));
	}
};

Model *modelHalvorsen = createModel<Halvorsen, HalvorsenWidget>("halvorsen");
