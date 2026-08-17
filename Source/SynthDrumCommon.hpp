#pragma once

// Little bits shared by the drum voices

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace SynthDrums606 {

static constexpr float kPi = 3.14159265358979323846f;
static constexpr float kTwoPi = 6.28318530717958647692f;
static constexpr float kMinimumStateMagnitude = 1.0e-20f;

inline float flushDenormal(float value) {
    return std::fabs(value) < kMinimumStateMagnitude ? 0.0f : value;
}

inline float clampf(float x, float lo, float hi) {
    return std::max(lo, std::min(x, hi));
}

inline float lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

inline float soft(float x) {
    return std::tanh(x);
}

class Random {
public:
    // Tiny repeatable random numbers. Plenty good enough for drum noise
    void seed(uint32_t s) {
        state_ = (s == 0u) ? 0x12345678u : s;
    }

    float bipolar() {
        const uint32_t x = next();
        return static_cast<float>(x) * (2.0f / 4294967295.0f) - 1.0f;
    }

    // The tom noise was tuned with the top 24 bits of the generator
    // Keep that mapping here so the other drum voices do not change
    float bipolar24() {
        const uint32_t x = next();
        return static_cast<float>(x >> 8) * (2.0f / 16777215.0f) - 1.0f;
    }

private:
    uint32_t next() {
        uint32_t x = state_;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        state_ = x;
        return x;
    }

    uint32_t state_ = 0x12345678u;
};

class DCBlocker {
public:
    void reset() {
        x1_ = 0.0f;
        y1_ = 0.0f;
    }

    float process(float x) {
        const float y = x - x1_ + 0.995f * y1_;
        x1_ = x;
        y1_ = y;
        return y;
    }

private:
    float x1_ = 0.0f;
    float y1_ = 0.0f;
};

class OnePoleLPF {
public:
    void init(double sampleRate) {
        sampleRate_ = sampleRate;
        reset();
        setCutoffHz(1000.0f);
    }

    void reset() {
        z_ = 0.0f;
    }

    void setCutoffHz(float cutoffHz) {
        const float safeCutoff = clampf(cutoffHz, 10.0f, static_cast<float>(sampleRate_ * 0.45));
        a_ = 1.0f - std::exp(-kTwoPi * safeCutoff / static_cast<float>(sampleRate_));
    }

    float process(float x) {
        z_ += a_ * (x - z_);
        return z_;
    }

private:
    double sampleRate_ = 44100.0;
    float a_ = 0.1f;
    float z_ = 0.0f;
};

class Biquad {
public:
    void init(double sampleRate) {
        sampleRate_ = sampleRate;
        reset();
    }

    void reset() {
        z1_ = 0.0f;
        z2_ = 0.0f;
    }

    void setHighPass(float cutoffHz, float q) {
        const float c = clampf(cutoffHz, 10.0f, static_cast<float>(sampleRate_ * 0.45));
        const float safeQ = std::max(0.05f, q);
        const float w0 = kTwoPi * c / static_cast<float>(sampleRate_);
        const float cosW0 = std::cos(w0);
        const float alpha = std::sin(w0) / (2.0f * safeQ);
        setCoefficients((1.0f + cosW0) * 0.5f,
                        -(1.0f + cosW0),
                        (1.0f + cosW0) * 0.5f,
                        1.0f + alpha,
                        -2.0f * cosW0,
                        1.0f - alpha);
    }

    void setLowPass(float cutoffHz, float q) {
        const float c = clampf(cutoffHz, 10.0f, static_cast<float>(sampleRate_ * 0.45));
        const float safeQ = std::max(0.05f, q);
        const float w0 = kTwoPi * c / static_cast<float>(sampleRate_);
        const float cosW0 = std::cos(w0);
        const float alpha = std::sin(w0) / (2.0f * safeQ);
        setCoefficients((1.0f - cosW0) * 0.5f,
                        1.0f - cosW0,
                        (1.0f - cosW0) * 0.5f,
                        1.0f + alpha,
                        -2.0f * cosW0,
                        1.0f - alpha);
    }

    float process(float x) {
        const float y = b0_ * x + z1_;
        z1_ = b1_ * x - a1_ * y + z2_;
        z2_ = b2_ * x - a2_ * y;
        return y;
    }

private:
    void setCoefficients(float b0, float b1, float b2, float a0, float a1, float a2) {
        const float invA0 = 1.0f / a0;
        b0_ = b0 * invA0;
        b1_ = b1 * invA0;
        b2_ = b2 * invA0;
        a1_ = a1 * invA0;
        a2_ = a2 * invA0;
    }

    double sampleRate_ = 44100.0;
    float b0_ = 1.0f;
    float b1_ = 0.0f;
    float b2_ = 0.0f;
    float a1_ = 0.0f;
    float a2_ = 0.0f;
    float z1_ = 0.0f;
    float z2_ = 0.0f;
};

static inline float decayCoef(double sampleRate, float seconds) {
    const float safeSeconds = std::max(0.0005f, seconds);
    return std::exp(-1.0f / static_cast<float>(sampleRate * safeSeconds));
}

static inline float onePoleCoef(double sampleRate, float seconds) {
    const float safeSeconds = std::max(0.00001f, seconds);
    return std::exp(-1.0f / static_cast<float>(sampleRate * safeSeconds));
}

static inline float decayCoefT60(double sampleRate, float seconds) {
    const float safeSeconds = std::max(0.0001f, seconds);
    return std::exp(-6.9077553f / static_cast<float>(sampleRate * safeSeconds));
}

class DecayEnvelope {
public:
    void init(double sampleRate) {
        sampleRate_ = sampleRate;
        clear();
        setDecaySeconds(0.1f);
    }

    void setDecaySeconds(float seconds) {
        coef_ = decayCoef(sampleRate_, seconds);
    }

    void trigger(float level) {
        value_ = std::max(value_, level);
    }

    void clear() {
        value_ = 0.0f;
    }

    float process() {
        value_ *= coef_;
        if (value_ < 1.0e-6f) {
            value_ = 0.0f;
        }
        return value_;
    }

    bool isActive() const {
        return value_ > 1.0e-6f;
    }

private:
    double sampleRate_ = 44100.0;
    float coef_ = 0.999f;
    float value_ = 0.0f;
};

class SweepSine {
public:
    void init(double sampleRate) {
        sampleRate_ = sampleRate;
        clear();
    }

    void trigger(float amplitude,
                 float startHz,
                 float endHz,
                 float ampDecay,
                 float pitchDecay,
                 float phaseOffset = 0.0f) {
        amp_ = amplitude;
        phase_ = phaseOffset;
        currentHz_ = startHz;
        targetHz_ = endHz;
        ampCoef_ = decayCoef(sampleRate_, ampDecay);
        const float safePitch = std::max(0.0005f, pitchDecay);
        pitchCoef_ = 1.0f - std::exp(-1.0f / static_cast<float>(sampleRate_ * safePitch));
    }

    float process() {
        currentHz_ += (targetHz_ - currentHz_) * pitchCoef_;
        phase_ += kTwoPi * currentHz_ / static_cast<float>(sampleRate_);
        if (phase_ >= kTwoPi) {
            phase_ -= kTwoPi;
        }
        const float out = std::sin(phase_) * amp_;
        amp_ *= ampCoef_;
        if (amp_ < 1.0e-6f) {
            amp_ = 0.0f;
        }
        return out;
    }

    void clear() {
        phase_ = 0.0f;
        currentHz_ = 100.0f;
        targetHz_ = 100.0f;
        amp_ = 0.0f;
        ampCoef_ = 0.999f;
        pitchCoef_ = 0.001f;
    }

    bool isActive() const {
        return amp_ > 1.0e-6f;
    }

private:
    double sampleRate_ = 44100.0;
    float phase_ = 0.0f;
    float currentHz_ = 100.0f;
    float targetHz_ = 100.0f;
    float amp_ = 0.0f;
    float ampCoef_ = 0.999f;
    float pitchCoef_ = 0.001f;
};

class WhiteNoise {
public:
    void init(double sampleRate, uint32_t seed = 0x606606u) {
        (void)sampleRate;
        rng_.seed(seed);
        lastIn_ = 0.0f;
        lastOut_ = 0.0f;
    }

    float process() {
        const float x = rng_.bipolar();
        const float y = x - lastIn_ + 0.995f * lastOut_;
        lastIn_ = x;
        lastOut_ = y;
        return y;
    }

private:
    Random rng_;
    float lastIn_ = 0.0f;
    float lastOut_ = 0.0f;
};

} // namespace SynthDrums606
