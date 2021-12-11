#include "anomalies.hpp"

struct Lorenz : Module {
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

	LorenzAttractor lorenz;

	static constexpr float SHAPE_PARAM_MIN = 0.6f;
	static constexpr float SHAPE_PARAM_MAX = 3.25f;
	static constexpr float SPEED_PARAM_MIN = 0.001f;
	static constexpr float SPEED_PARAM_MAX = 1.0f;
	static constexpr float AMP_PARAM_MIN = 0.1f;
	static constexpr float AMP_PARAM_MAX = 10.0f;
	static constexpr float AMP_PARAM_DEFAULT = 5.0f; // ±5v bipolar CV
	float amplitude = AMP_PARAM_DEFAULT * 0.2f; // default amplification is 1
	float tfactor; // mystery 4th dimension (tempered to within normal amp range)

    Lorenz() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(SPEED_PARAM, SPEED_PARAM_MIN, SPEED_PARAM_MAX, LorenzAttractor::DEFAULT_SPEED, "speed");
		configParam(SHAPE_PARAM, SHAPE_PARAM_MIN, SHAPE_PARAM_MAX, LorenzAttractor::DEFAULT_B, "shape");
		configParam(AMP_PARAM, AMP_PARAM_MIN, AMP_PARAM_MAX, AMP_PARAM_DEFAULT, "scale");
		configOutput(X_OUTPUT, "x");
		configOutput(Y_OUTPUT, "y");
		configOutput(Z_OUTPUT, "z");
		configOutput(T_OUTPUT, "t factor");		
    }

    void process(const ProcessArgs &args) override;
};

void Lorenz::process(const ProcessArgs &args) {
	if (outputs[X_OUTPUT].isConnected()
		|| outputs[Y_OUTPUT].isConnected()
		|| outputs[Z_OUTPUT].isConnected()
		|| outputs[T_OUTPUT].isConnected()) {
		lorenz.beta = clamp(params[SHAPE_PARAM].getValue(), SHAPE_PARAM_MIN, SHAPE_PARAM_MAX);
		lorenz.speed = clamp(params[SPEED_PARAM].getValue(), SPEED_PARAM_MIN, SPEED_PARAM_MAX) * 1.5f;
		amplitude = clamp(params[AMP_PARAM].getValue(), AMP_PARAM_MIN, AMP_PARAM_MAX) * 0.214f;
		lorenz.process(1.0f / args.sampleRate);
		tfactor = lorenz.x + lorenz.y - lorenz.z;
		outputs[X_OUTPUT].setVoltage((0.23f * lorenz.x) * amplitude);
		outputs[Y_OUTPUT].setVoltage((0.17f * lorenz.y) * amplitude);
		outputs[Z_OUTPUT].setVoltage((0.20f * lorenz.z - 5.0f) * amplitude);
		outputs[T_OUTPUT].setVoltage((0.094f * tfactor + 3.0f) * amplitude);
	}
}

struct LorenzWidget : ModuleWidget {
    LorenzWidget(Lorenz *module) {
        setModule(module);
        box.size = Vec(2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/face/lor.svg")));

		addParam(createParam<KnobS>(Vec(4, 35), module, Lorenz::SPEED_PARAM));
		addParam(createParam<KnobS>(Vec(4, 85), module, Lorenz::SHAPE_PARAM));
		addParam(createParam<KnobS>(Vec(4, 135), module, Lorenz::AMP_PARAM));
		addOutput(createOutput<OutPort>(Vec(5, 200), module, Lorenz::X_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(5, 240), module, Lorenz::Y_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(5, 280), module, Lorenz::Z_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(5, 320), module, Lorenz::T_OUTPUT));
	}
};

Model *modelLorenz = createModel<Lorenz, LorenzWidget>("lorenz");
