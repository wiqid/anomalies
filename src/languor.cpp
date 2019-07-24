#include "anomalies.hpp"

struct Languor : Module {

	enum ParamIds {
		SPEED_PARAM,
		SHAPE_PARAM,
		AMP_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		SPEED_INPUT,
		SHAPE_INPUT,
		AMP_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		HX_OUTPUT,
		HY_OUTPUT,
		HZ_OUTPUT,
		HT_OUTPUT,
		DX_OUTPUT,
		DY_OUTPUT,
		DZ_OUTPUT,
		DT_OUTPUT,
		LX_OUTPUT,
		LY_OUTPUT,
		LZ_OUTPUT,
		LT_OUTPUT,
		AX_OUTPUT,
		AY_OUTPUT,
		AZ_OUTPUT,
		AT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	HalvorsenAttractor halvorsen;
	DadrasAttractor dadras;
	LorenzAttractor lorenz;

	static constexpr float SHAPE_PARAM_MIN = 0.1f;
	static constexpr float SHAPE_PARAM_MAX = 10.0f;
	static constexpr float SHAPE_PARAM_DEFAULT = 5.0f;
	static constexpr float SPEED_PARAM_MIN = 0.001f;
	static constexpr float SPEED_PARAM_MAX = 1.0f;
	static constexpr float SPEED_PARAM_DEFAULT = 0.5f;
	static constexpr float AMP_PARAM_MIN = 0.1f;
	static constexpr float AMP_PARAM_MAX = 10.0f;
	static constexpr float AMP_PARAM_DEFAULT = 5.0f; // ±5v bipolar CV
	float amplitude = AMP_PARAM_DEFAULT * 0.2f; // default amplification is 1
	float hatfactor;
	float datfactor;
	float lotfactor;

	Languor() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(SPEED_PARAM, SPEED_PARAM_MIN, SPEED_PARAM_MAX, SPEED_PARAM_DEFAULT, "speed");
		configParam(SHAPE_PARAM, SHAPE_PARAM_MIN, SHAPE_PARAM_MAX, SHAPE_PARAM_DEFAULT, "shape");
		configParam(AMP_PARAM, AMP_PARAM_MIN, AMP_PARAM_MAX, AMP_PARAM_DEFAULT, "scale");
	}
	void process(const ProcessArgs &args) override;
};

void Languor::process(const ProcessArgs &args) {
	if (outputs[HX_OUTPUT].isConnected()|| outputs[HY_OUTPUT].isConnected() || outputs[HZ_OUTPUT].isConnected() || outputs[HT_OUTPUT].isConnected()
	|| outputs[DX_OUTPUT].isConnected()	|| outputs[DY_OUTPUT].isConnected() || outputs[DZ_OUTPUT].isConnected() || outputs[DT_OUTPUT].isConnected()
	|| outputs[LX_OUTPUT].isConnected()	|| outputs[LY_OUTPUT].isConnected() || outputs[LZ_OUTPUT].isConnected() || outputs[LT_OUTPUT].isConnected()
	|| outputs[AX_OUTPUT].isConnected()	|| outputs[AY_OUTPUT].isConnected() || outputs[AZ_OUTPUT].isConnected() || outputs[AT_OUTPUT].isConnected())
	{
		float _shape = clamp(params[SHAPE_PARAM].getValue() + inputs[SHAPE_INPUT].getNormalVoltage(0.0f) * 2.0f, SHAPE_PARAM_MIN, SHAPE_PARAM_MAX);
		float _speed = clamp(params[SPEED_PARAM].getValue() + inputs[SPEED_INPUT].getNormalVoltage(0.0f) * 0.2f, SPEED_PARAM_MIN, SPEED_PARAM_MAX);
		amplitude = clamp(params[AMP_PARAM].getValue() + inputs[AMP_INPUT].getNormalVoltage(0.0f) * 2.0f, AMP_PARAM_MIN, AMP_PARAM_MAX) * 0.2f;

		///// halvorsen
		halvorsen.a = (_shape / 25.0f) + 1.23f; // halvorsen shape ok from 1.23 to 1.63
		halvorsen.speed = _speed * 0.75f;
		halvorsen.process(1.0f / args.sampleRate);
		// since chaotic values can escape to infinity, check the output
		if (!std::isfinite(halvorsen.x)) { halvorsen.x = 0.f; };
		if (!std::isfinite(halvorsen.y)) { halvorsen.y = 0.f; };
		if (!std::isfinite(halvorsen.z)) { halvorsen.z = 0.f; };
		hatfactor = halvorsen.x + halvorsen.y - halvorsen.z;

		outputs[HX_OUTPUT].setVoltage((0.5f * halvorsen.x + 1.6f) * amplitude);
		outputs[HY_OUTPUT].setVoltage((0.5f * halvorsen.y + 1.6f) * amplitude);
		outputs[HZ_OUTPUT].setVoltage((0.5f * halvorsen.z + 1.6f) * amplitude);
		outputs[HT_OUTPUT].setVoltage((0.23f * hatfactor + 1.6f) * amplitude);

		///// dadras
		dadras.q = (_shape / 4.0f) + 1.5f; // dadras shape ok from 1.445 to 9.0 (and higher)
		dadras.speed = _speed * 0.5f;
		dadras.process(1.0f / args.sampleRate);
		// since chaotic values can escape to infinity, check the output
		if (!std::isfinite(dadras.x)) { dadras.x = 0.f; };
		if (!std::isfinite(dadras.y)) { dadras.y = 0.f; };
		if (!std::isfinite(dadras.z)) { dadras.z = 0.f; };
		datfactor = dadras.x + dadras.y - dadras.z;

		outputs[DX_OUTPUT].setVoltage(0.37f * dadras.x * amplitude);
		outputs[DY_OUTPUT].setVoltage(0.45f * dadras.y * amplitude);
		outputs[DZ_OUTPUT].setVoltage(0.45f * dadras.z * amplitude);
		outputs[DT_OUTPUT].setVoltage(0.205f * datfactor * amplitude);

		///// lorenz
		lorenz.beta = (_shape / 4.0f) + 0.6f; // lorenz shape ok from 0.6 to 3.25
		lorenz.speed = _speed * 0.03f;
		lorenz.process(1.0f / args.sampleRate);
		// since chaotic values can escape to infinity, check the output
		if (!std::isfinite(lorenz.x)) { lorenz.x = 0.f; };
		if (!std::isfinite(lorenz.y)) { lorenz.y = 0.f; };
		if (!std::isfinite(lorenz.z)) { lorenz.z = 0.f; };
		lotfactor = lorenz.x + lorenz.y - lorenz.z;

		outputs[LX_OUTPUT].setVoltage((0.23f * lorenz.x) * amplitude * 0.214f);
		outputs[LY_OUTPUT].setVoltage((0.17f * lorenz.y) * amplitude * 0.214f);
		outputs[LZ_OUTPUT].setVoltage((0.20f * lorenz.z - 5.0f) * amplitude * 0.214f);
		outputs[LT_OUTPUT].setVoltage((0.094f * lotfactor + 3.0f) * amplitude * 0.214f);

		///// weighted averages
		outputs[AX_OUTPUT].setVoltage(((0.2f * halvorsen.x + 1.6f) + (0.74f * dadras.x) + (0.06f * lorenz.x)) * 0.35f * amplitude);
		outputs[AY_OUTPUT].setVoltage(((0.2f * halvorsen.y + 1.6f) + (0.9f * dadras.y) + (0.043f * lorenz.y)) * 0.35f * amplitude);
		outputs[AZ_OUTPUT].setVoltage(((0.2f * halvorsen.z + 1.6f) + (0.9f * dadras.z) + ((0.20f * lorenz.z - 5.0f) * 0.25f)) * 0.35f * amplitude);
		outputs[AT_OUTPUT].setVoltage(((0.11f * hatfactor + 1.6f) + (0.41f * datfactor) + ((0.094f * lotfactor + 3.0f) * 0.25f)) * 0.35f * amplitude);

	}
}

struct LanguorWidget : ModuleWidget {
	LanguorWidget(Languor *module) {
		setModule(module);
		box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/face/languor.svg")));

		addParam(createParam<KnobM>(Vec(7.5, 53), module, Languor::SPEED_PARAM));
		addParam(createParam<KnobM>(Vec(45, 53), module, Languor::SHAPE_PARAM));
		addParam(createParam<KnobM>(Vec(82.5, 53), module, Languor::AMP_PARAM));
		addInput(createInput<InPort>(Vec(12.5, 102), module, Languor::SPEED_INPUT));
		addInput(createInput<InPort>(Vec(50, 102), module, Languor::SHAPE_INPUT));
		addInput(createInput<InPort>(Vec(87.5, 102), module, Languor::AMP_INPUT));

		addOutput(createOutput<OutPort>(Vec(8, 200), module, Languor::HX_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(8, 240), module, Languor::HY_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(8, 280), module, Languor::HZ_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(8, 320), module, Languor::HT_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(36, 200), module, Languor::DX_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(36, 240), module, Languor::DY_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(36, 280), module, Languor::DZ_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(36, 320), module, Languor::DT_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(64, 200), module, Languor::LX_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(64, 240), module, Languor::LY_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(64, 280), module, Languor::LZ_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(64, 320), module, Languor::LT_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(92, 200), module, Languor::AX_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(92, 240), module, Languor::AY_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(92, 280), module, Languor::AZ_OUTPUT));
		addOutput(createOutput<OutPort>(Vec(92, 320), module, Languor::AT_OUTPUT));
	}
};

Model *modelLanguor = createModel<Languor, LanguorWidget>("languor");
