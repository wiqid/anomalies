#include "anomalies.hpp"

struct Sakarya : Module {
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

	SakaryaAttractor sakarya;

	static constexpr float SHAPE_PARAM_MIN = 0.125f;
	static constexpr float SHAPE_PARAM_MAX = 0.5f;
	static constexpr float SPEED_PARAM_MIN = 0.001f;
	static constexpr float SPEED_PARAM_MAX = 1.0f;
	static constexpr float AMP_PARAM_MIN = 0.1f;
	static constexpr float AMP_PARAM_MAX = 10.0f;
	static constexpr float AMP_PARAM_DEFAULT = 5.0f; // ±5v bipolar CV
	float amplitude = AMP_PARAM_DEFAULT * 0.2f; // default amplification is 1
	float tfactor; // mystery 4th dimension (tempered to within normal amp range)

    Sakarya() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(SPEED_PARAM, SPEED_PARAM_MIN, SPEED_PARAM_MAX, SakaryaAttractor::DEFAULT_SPEED, "speed");
		configParam(SHAPE_PARAM, SHAPE_PARAM_MIN, SHAPE_PARAM_MAX, SakaryaAttractor::DEFAULT_B, "shape");
		configParam(AMP_PARAM, AMP_PARAM_MIN, AMP_PARAM_MAX, AMP_PARAM_DEFAULT, "scale");
    }
    void process(const ProcessArgs &args) override;
};

void Sakarya::process(const ProcessArgs &args) {
	if (outputs[X_OUTPUT].isConnected()
		|| outputs[Y_OUTPUT].isConnected()
		|| outputs[Z_OUTPUT].isConnected()
		|| outputs[T_OUTPUT].isConnected()) {
		sakarya.b = clamp(params[SHAPE_PARAM].getValue(), SHAPE_PARAM_MIN, SHAPE_PARAM_MAX);
		sakarya.speed = clamp(params[SPEED_PARAM].getValue(), SPEED_PARAM_MIN, SPEED_PARAM_MAX) * 3.0f;
		amplitude = clamp(params[AMP_PARAM].getValue(), AMP_PARAM_MIN, AMP_PARAM_MAX) * 0.2f;

		sakarya.process(1.0f / args.sampleRate);
		// since chaotic values can escape to infinity, check the output
		if (!std::isfinite(sakarya.x)) { sakarya.x = 0.f; };
		if (!std::isfinite(sakarya.y)) { sakarya.y = 0.f; };
		if (!std::isfinite(sakarya.z)) { sakarya.z = 0.f; };

		tfactor = sakarya.x + sakarya.y - sakarya.z;
		outputs[X_OUTPUT].setVoltage(0.2f * sakarya.x * amplitude);
		outputs[Y_OUTPUT].setVoltage(0.35f * sakarya.y * amplitude);
		outputs[Z_OUTPUT].setVoltage((0.35f * sakarya.z - 0.75f) * amplitude);
		outputs[T_OUTPUT].setVoltage(0.11f * tfactor * amplitude);
	}
}

struct SakaryaWidget : ModuleWidget {
    SakaryaWidget(Sakarya *module) {
        setModule(module);
        box.size = Vec(2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/face/sak.svg")));

		addParam(createParam<KnobS>(Vec(4, 35), module, Sakarya::SPEED_PARAM));
		addParam(createParam<KnobS>(Vec(4, 85), module, Sakarya::SHAPE_PARAM));
		addParam(createParam<KnobS>(Vec(4, 135), module, Sakarya::AMP_PARAM));
		addOutput(createOutput<OutPort>(Vec(5, 200), module, Sakarya::X_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(5, 240), module, Sakarya::Y_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(5, 280), module, Sakarya::Z_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(5, 320), module, Sakarya::T_OUTPUT));
	}
};

Model *modelSakarya = createModel<Sakarya, SakaryaWidget>("sakarya");
