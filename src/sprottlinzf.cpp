#include "anomalies.hpp"

struct SprottLinzF : Module {
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

	SprottLinzFAttractor slf;

	static constexpr float SHAPE_PARAM_MIN = 0.43f;
	static constexpr float SHAPE_PARAM_MAX = 0.51f;
	static constexpr float SPEED_PARAM_MIN = 0.001f;
	static constexpr float SPEED_PARAM_MAX = 1.0f;
	static constexpr float AMP_PARAM_MIN = 0.1f;
	static constexpr float AMP_PARAM_MAX = 10.0f;
	static constexpr float AMP_PARAM_DEFAULT = 5.0f; // ±5v bipolar CV
	float amplitude = AMP_PARAM_DEFAULT * 0.2f; // default amplification is 1
	float tfactor; // mystery 4th dimension (tempered to within normal amp range)

    SprottLinzF() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(SPEED_PARAM, SPEED_PARAM_MIN, SPEED_PARAM_MAX, SprottLinzFAttractor::DEFAULT_SPEED, "speed");
		configParam(SHAPE_PARAM, SHAPE_PARAM_MIN, SHAPE_PARAM_MAX, SprottLinzFAttractor::DEFAULT_A, "shape");
		configParam(AMP_PARAM, AMP_PARAM_MIN, AMP_PARAM_MAX, AMP_PARAM_DEFAULT, "scale");
    }
    void process(const ProcessArgs &args) override;
};

void SprottLinzF::process(const ProcessArgs &args) {
	if (outputs[X_OUTPUT].isConnected()
		|| outputs[Y_OUTPUT].isConnected()
		|| outputs[Z_OUTPUT].isConnected()
		|| outputs[T_OUTPUT].isConnected()) {
		slf.a = clamp(params[SHAPE_PARAM].getValue(), SHAPE_PARAM_MIN, SHAPE_PARAM_MAX);
		slf.speed = clamp(params[SPEED_PARAM].getValue(), SPEED_PARAM_MIN, SPEED_PARAM_MAX) * 4.5f;
		amplitude = clamp(params[AMP_PARAM].getValue(), AMP_PARAM_MIN, AMP_PARAM_MAX) * 0.2f;

		slf.process(1.0f / args.sampleRate);
		// since chaotic values can escape to infinity, check the output
		if (!std::isfinite(slf.x)) { slf.x = 0.f; };
		if (!std::isfinite(slf.y)) { slf.y = 0.f; };
		if (!std::isfinite(slf.z)) { slf.z = 0.f; };

		tfactor = slf.x + slf.y - slf.z;
		outputs[X_OUTPUT].setVoltage((2.2f * slf.x + 1.7f) * amplitude);
		outputs[Y_OUTPUT].setVoltage((1.92f * slf.y + 3.3f) * amplitude);
		outputs[Z_OUTPUT].setVoltage((1.8f * slf.z - 4.4f) * amplitude);
		outputs[T_OUTPUT].setVoltage((0.83f * tfactor + 4.1f) * amplitude);
	}
}

struct SprottLinzFWidget : ModuleWidget {
    SprottLinzFWidget(SprottLinzF *module) {
        setModule(module);
        box.size = Vec(2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/face/slf.svg")));

		addParam(createParam<KnobS>(Vec(4, 35), module, SprottLinzF::SPEED_PARAM));
		addParam(createParam<KnobS>(Vec(4, 85), module, SprottLinzF::SHAPE_PARAM));
		addParam(createParam<KnobS>(Vec(4, 135), module, SprottLinzF::AMP_PARAM));
		addOutput(createOutput<OutPort>(Vec(5, 200), module, SprottLinzF::X_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(5, 240), module, SprottLinzF::Y_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(5, 280), module, SprottLinzF::Z_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(5, 320), module, SprottLinzF::T_OUTPUT));
	}
};

Model *modelSprottLinzF = createModel<SprottLinzF, SprottLinzFWidget>("slf");
