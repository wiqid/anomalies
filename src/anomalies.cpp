#include "anomalies.hpp"

Plugin *pluginInstance;

void init(Plugin *p) {
	pluginInstance = p;

	// Add modules here
	// p->addModel(modelMyModule);

	p->addModel(modelBlankR);
	p->addModel(modelLanguor);
	p->addModel(modelHalvorsen);
	p->addModel(modelLorenz);
	p->addModel(modelThomas);
	p->addModel(modelSakarya);
	p->addModel(modelDadras);
	p->addModel(modelSprottLinzF);
	p->addModel(modelDualAttenuverter);
	p->addModel(modelFullScope);
	// p->addModel(modelClock);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
