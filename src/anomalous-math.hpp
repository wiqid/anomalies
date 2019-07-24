// wiqid math stuff

#pragma once
#include <math.h>

// attractor code based on https://github.com/joelrobichaud/Nohmad/blob/master/src/StrangeAttractors.cpp
// by Joel Robichaud, MIT licensed
// and formulas from Jürgen Meier's website http://www.3d-meier.de/tut19/Seite0.html

struct HalvorsenAttractor {
	float a, speed; // params
	float x, y, z; // outs

	static constexpr float DEFAULT_A = 1.43f;
	static constexpr float DEFAULT_SPEED = 0.5f;

	HalvorsenAttractor() :
		a(DEFAULT_A), speed(DEFAULT_SPEED),
		x(1.0f), y(0.0f), z(0.0f) {}

	void process(float dt) {
		float dx = (-a * x) - (4 * y) - (4 * z) - (y * y);
		float dy = (-a * y) - (4 * z) - (4 * x) - (z * z);
		float dz = (-a * z) - (4 * x) - (4 * y) - (x * x);

		x += dx * dt * speed * speed;
        y += dy * dt * speed * speed;
        z += dz * dt * speed * speed;
	}
};

struct LorenzAttractor {
    float sigma, beta, rho, speed; // params
    float x, y, z; // outs

    static constexpr float DEFAULT_S = 10.0f;
    static constexpr float DEFAULT_B = 8.0f / 3.0f;
    static constexpr float DEFAULT_R = 28.0f;
    static constexpr float DEFAULT_SPEED = 0.5f;

    LorenzAttractor() :
        sigma(DEFAULT_S), beta(DEFAULT_B), rho(DEFAULT_R), speed(DEFAULT_SPEED),
        x(1.0f), y(1.0f), z(1.0f) {}

    void process(float dt) {
        float dx = sigma * (y - x);
        float dy = x * (rho - z) - y;
        float dz = (x * y) - (beta * z);

        x += dx * dt * speed * speed;
        y += dy * dt * speed * speed;
        z += dz * dt * speed * speed;
    }
};

struct ThomasAttractor {
	float b, speed; // params
	float x, y, z; // outs

	static constexpr float DEFAULT_B = 0.188f;
	static constexpr float DEFAULT_SPEED = 0.5f;

	ThomasAttractor() :
		b(DEFAULT_B), speed(DEFAULT_SPEED),
		x(0.1f), y(0.0f), z(0.0f) {}

	void process(float dt) {
		float dx = -b * x + sin(y);
		float dy = -b * y + sin(z);
		float dz = -b * z + sin(x);

		x += dx * dt * speed * speed;
        y += dy * dt * speed * speed;
        z += dz * dt * speed * speed;
	}
};

struct SakaryaAttractor {
	float a, b, speed; // params
	float x, y, z; // outs

	static constexpr float DEFAULT_A = 0.398f;
	static constexpr float DEFAULT_B = 0.3f;
	static constexpr float DEFAULT_SPEED = 0.5f;

	SakaryaAttractor() :
		a(DEFAULT_A), b(DEFAULT_B), speed(DEFAULT_SPEED),
		x(1.0f), y(-1.0f), z(1.0f) {}

	void process(float dt) {
		float dx = -x + y + y * z;
		float dy = -x - y + a * x * z;
		float dz = z - b * x * y;

		x += dx * dt * speed * speed;
        y += dy * dt * speed * speed;
        z += dz * dt * speed * speed;
	}
};

struct DadrasAttractor {
	float p, q, r, s, e, speed; // params
	float x, y, z; // outs

	static constexpr float DEFAULT_P = 3.0f;
	static constexpr float DEFAULT_Q = 2.75f;
	static constexpr float DEFAULT_R = 1.7f;
	static constexpr float DEFAULT_S = 2.0f;
	static constexpr float DEFAULT_E = 9.0f;
	static constexpr float DEFAULT_SPEED = 0.5f;

	DadrasAttractor() :
		p(DEFAULT_P), q(DEFAULT_Q), r(DEFAULT_R), s(DEFAULT_S),
		e(DEFAULT_E), speed(DEFAULT_SPEED),
		x(1.0f), y(1.0f), z(0.0f) {}

	void process(float dt) {
		float dx = y - p * x + q * y * z;
		float dy = r * y - x * z + z;
		float dz = s * x * y - e * z;

		x += dx * dt * speed * speed;
        y += dy * dt * speed * speed;
        z += dz * dt * speed * speed;
	}
};

struct SprottLinzFAttractor {
	float a, speed; // params
	float x, y, z; // outs

	static constexpr float DEFAULT_A = 0.5f;
	static constexpr float DEFAULT_SPEED = 0.5f;

	SprottLinzFAttractor() :
		a(DEFAULT_A), speed(DEFAULT_SPEED),
		x(0.1f), y(0.0f), z(0.0f) {}

	void process(float dt) {
		float dx = y + z;
		float dy = -x + a * y;
		float dz = x * x - z;

		x += dx * dt * speed * speed;
        y += dy * dt * speed * speed;
        z += dz * dt * speed * speed;
	}
};
