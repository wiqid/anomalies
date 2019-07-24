#include "anomalies.hpp"

struct Thomas : Module {
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

	ThomasAttractor thomas;

	static constexpr float SHAPE_PARAM_MIN = 0.08f; // values under 0.10 increasingly go out of range
	static constexpr float SHAPE_PARAM_MAX = 0.23f; // values over 0.208 are stable
	static constexpr float SPEED_PARAM_MIN = 0.001f;
	static constexpr float SPEED_PARAM_MAX = 1.0f;
	static constexpr float AMP_PARAM_MIN = 0.1f;
	static constexpr float AMP_PARAM_MAX = 10.0f;
	static constexpr float AMP_PARAM_DEFAULT = 5.0f; // ±5v bipolar CV
	float amplitude = AMP_PARAM_DEFAULT * 0.2f; // default amplification is 1
	float tfactor; // mystery 4th dimension (tempered to within normal amp range)

    Thomas() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(SPEED_PARAM, SPEED_PARAM_MIN, SPEED_PARAM_MAX, ThomasAttractor::DEFAULT_SPEED, "speed");
		configParam(SHAPE_PARAM, SHAPE_PARAM_MIN, SHAPE_PARAM_MAX, ThomasAttractor::DEFAULT_B, "shape");
		configParam(AMP_PARAM, AMP_PARAM_MIN, AMP_PARAM_MAX, AMP_PARAM_DEFAULT, "scale");
    }
    void process(const ProcessArgs &args) override;
};

void Thomas::process(const ProcessArgs &args) {
	if (outputs[X_OUTPUT].isConnected()
		|| outputs[Y_OUTPUT].isConnected()
		|| outputs[Z_OUTPUT].isConnected()
		|| outputs[T_OUTPUT].isConnected()) {
		thomas.b = clamp(params[SHAPE_PARAM].getValue(), SHAPE_PARAM_MIN, SHAPE_PARAM_MAX);
		thomas.speed = clamp(params[SPEED_PARAM].getValue(), SPEED_PARAM_MIN, SPEED_PARAM_MAX) * 5.0f;
		amplitude = clamp(params[AMP_PARAM].getValue(), AMP_PARAM_MIN, AMP_PARAM_MAX) * 0.2f;

		thomas.process(1.0f / args.sampleRate);
		// since chaotic values can escape to infinity, check the output
		if (!std::isfinite(thomas.x)) { thomas.x = 0.f; };
		if (!std::isfinite(thomas.y)) { thomas.y = 0.f; };
		if (!std::isfinite(thomas.z)) { thomas.z = 0.f; };

		tfactor = thomas.x + thomas.y - thomas.z;
		outputs[X_OUTPUT].setVoltage(thomas.x * amplitude);
		outputs[Y_OUTPUT].setVoltage(thomas.y * amplitude);
		outputs[Z_OUTPUT].setVoltage(thomas.z * amplitude);
		outputs[T_OUTPUT].setVoltage(0.75f * tfactor * amplitude);
	}
}

struct ThomasWidget : ModuleWidget {
    ThomasWidget(Thomas *module) {
        setModule(module);
        box.size = Vec(2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/face/thom.svg")));

		addParam(createParam<KnobS>(Vec(4, 35), module, Thomas::SPEED_PARAM));
		addParam(createParam<KnobS>(Vec(4, 85), module, Thomas::SHAPE_PARAM));
		addParam(createParam<KnobS>(Vec(4, 135), module, Thomas::AMP_PARAM));
		addOutput(createOutput<OutPort>(Vec(5, 200), module, Thomas::X_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(5, 240), module, Thomas::Y_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(5, 280), module, Thomas::Z_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(5, 320), module, Thomas::T_OUTPUT));
	}
};

Model *modelThomas = createModel<Thomas, ThomasWidget>("thomas");
