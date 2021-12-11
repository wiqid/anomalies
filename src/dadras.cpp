#include "anomalies.hpp"

struct Dadras : Module {
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

	DadrasAttractor dadras;

	static constexpr float SHAPE_PARAM_MIN = 1.445f; // smaller is stable
	static constexpr float SHAPE_PARAM_MAX = 9.0f; // higher pretty much stays in similar shape
	static constexpr float SPEED_PARAM_MIN = 0.001f;
	static constexpr float SPEED_PARAM_MAX = 1.0f;
	static constexpr float AMP_PARAM_MIN = 0.1f;
	static constexpr float AMP_PARAM_MAX = 10.0f;
	static constexpr float AMP_PARAM_DEFAULT = 5.0f; // ±5v bipolar CV
	float amplitude = AMP_PARAM_DEFAULT * 0.2f; // default amplification is 1
	float tfactor; // mystery 4th dimension (tempered to within normal amp range)

    Dadras() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(SPEED_PARAM, SPEED_PARAM_MIN, SPEED_PARAM_MAX, DadrasAttractor::DEFAULT_SPEED, "speed");
		configParam(SHAPE_PARAM, SHAPE_PARAM_MIN, SHAPE_PARAM_MAX, DadrasAttractor::DEFAULT_Q, "shape");
		configParam(AMP_PARAM, AMP_PARAM_MIN, AMP_PARAM_MAX, AMP_PARAM_DEFAULT, "scale");
		configOutput(X_OUTPUT, "x");
		configOutput(Y_OUTPUT, "y");
		configOutput(Z_OUTPUT, "z");
		configOutput(T_OUTPUT, "t factor");				
    }
    
    void process(const ProcessArgs &args) override;
};

void Dadras::process(const ProcessArgs &args) {
	if (outputs[X_OUTPUT].isConnected()
		|| outputs[Y_OUTPUT].isConnected()
		|| outputs[Z_OUTPUT].isConnected()
		|| outputs[T_OUTPUT].isConnected()) {
		dadras.q = clamp(params[SHAPE_PARAM].getValue(), SHAPE_PARAM_MIN, SHAPE_PARAM_MAX);
		dadras.speed = clamp(params[SPEED_PARAM].getValue(), SPEED_PARAM_MIN, SPEED_PARAM_MAX) * 2.5f;
		amplitude = clamp(params[AMP_PARAM].getValue(), AMP_PARAM_MIN, AMP_PARAM_MAX) * 0.2f;

		dadras.process(1.0f / args.sampleRate);
		// since chaotic values can escape to infinity, check the output
		if (!std::isfinite(dadras.x)) { dadras.x = 0.f; };
		if (!std::isfinite(dadras.y)) { dadras.y = 0.f; };
		if (!std::isfinite(dadras.z)) { dadras.z = 0.f; };

		tfactor = dadras.x + dadras.y - dadras.z;
		outputs[X_OUTPUT].setVoltage(0.37f * dadras.x * amplitude);
		outputs[Y_OUTPUT].setVoltage(0.45f * dadras.y * amplitude);
		outputs[Z_OUTPUT].setVoltage(0.45f * dadras.z * amplitude);
		outputs[T_OUTPUT].setVoltage(0.205f * tfactor * amplitude);
	}
}

struct DadrasWidget : ModuleWidget {
    DadrasWidget(Dadras *module) {
        setModule(module);
        box.size = Vec(2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/face/dadr.svg")));

		addParam(createParam<KnobS>(Vec(4, 35), module, Dadras::SPEED_PARAM));
		addParam(createParam<KnobS>(Vec(4, 85), module, Dadras::SHAPE_PARAM));
		addParam(createParam<KnobS>(Vec(4, 135), module, Dadras::AMP_PARAM));
		addOutput(createOutput<OutPort>(Vec(5, 200), module, Dadras::X_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(5, 240), module, Dadras::Y_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(5, 280), module, Dadras::Z_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(5, 320), module, Dadras::T_OUTPUT));
	}
};

Model *modelDadras = createModel<Dadras, DadrasWidget>("dadras");
