#pragma once

// Super 606 XL BassDrum
// Copyright (c) 2026 Matthew Fecher (AnalogMatthew)

#include "SynthDrumCommon.hpp"

namespace SynthDrums606 {

static constexpr float kVoiceOutputTrim = 0.70f;
static constexpr float kXLDecayMin = -1.75f;
static constexpr float kXLDecayMax = 3.00f;

class BassDrum {
public:
    void init(double sampleRate) {
        body_.init(sampleRate);
        clickEnv_.init(sampleRate);
        impulseEnv_.init(sampleRate);
        clickLPF_.init(sampleRate);
        clickLPF_.setCutoffHz(1400.0f);
        bodyLPF_.init(sampleRate);
        bodyLPF_.setCutoffHz(900.0f);
        impulseHPF_.init(sampleRate);
        impulseHPF_.setHighPass(1350.0f, 0.707f);
        impulseLPF_.init(sampleRate);
        impulseLPF_.setCutoffHz(2600.0f);
        dc_.reset();
        lastBodyRaw_ = 0.0f;
    }

    void trigger(float accent) {
        const float a = clampf(accent, 0.0f, 1.0f);
        const float t = tune - 0.5f;
        const float ratio = std::pow(2.0f, tuneSemitones / 12.0f);
        const float startHz = (120.0f + t * 16.0f) * ratio;
        const float endHz = (53.0f + t * 8.0f) * ratio;
        const float transientShape = clampf((attack + clickAmount + impulseAmount) / 3.0f, 0.0f, 1.5f);
        const float thudShape = clampf(transientShape * transientShape, 0.0f, 1.5f);
        const float bodyStartHz = startHz * (1.0f + thudShape * 0.28f);
        const float bodyAmp = 0.92f + thudShape * 0.22f + a * 0.08f;
        const float bodyPitchDecay = lerpf(0.022f, 0.008f, clampf(thudShape, 0.0f, 1.0f));
        body_.trigger(bodyAmp,
                      bodyStartHz,
                      endHz,
                      0.22f + decay * 0.12f,
                      bodyPitchDecay,
                      -0.33f * kPi);
        clickEnv_.setDecaySeconds(0.0020f + attack * 0.0045f + clickAmount * 0.0040f);
        clickEnv_.trigger(0.02f + attack * 0.06f + clickAmount * 0.28f + a * 0.02f);
        impulseEnv_.setDecaySeconds(0.0008f + impulseAmount * 0.0080f);
        impulseEnv_.trigger(0.02f + impulseAmount * 0.55f + thudShape * 0.08f + a * 0.04f);
        bodyLPF_.setCutoffHz(lerpf(620.0f, 1200.0f, tone));
        lastBodyRaw_ = 0.0f;
    }

    float process(float noise) {
        const float bodyRaw = body_.process();
        const float body = bodyLPF_.process(bodyRaw);
        const float click = clickLPF_.process(noise) * clickEnv_.process();
        const float bodyDelta = bodyRaw - lastBodyRaw_;
        lastBodyRaw_ = bodyRaw;
        const float impulseCore = impulseLPF_.process(impulseHPF_.process(bodyDelta));
        const float impulse = impulseCore * impulseEnv_.process();
        const float out = body * (0.92f + drive * 0.10f)
                        + click * (0.06f + clickAmount * 0.90f)
                        + impulse * (0.08f + impulseAmount * 2.20f);

        const float baseDrive = 1.12f + drive * 0.12f;
        const float shaped = soft(out * baseDrive);
        return dc_.process(shaped * level);
    }

    bool isActive() const {
        return body_.isActive() || clickEnv_.isActive() || impulseEnv_.isActive();
    }

    float level = 1.0f;
    float tune = 0.28f;
    float tone = 0.34f;
    float decay = 0.72f;
    float attack = 0.25f;
    float drive = 0.18f;
    float tuneSemitones = 2.0f;
    float clickAmount = 0.25f;
    float impulseAmount = 0.25f;

private:
    SweepSine body_;
    DecayEnvelope clickEnv_;
    DecayEnvelope impulseEnv_;
    OnePoleLPF clickLPF_;
    OnePoleLPF bodyLPF_;
    OnePoleLPF impulseLPF_;
    Biquad impulseHPF_;
    DCBlocker dc_;
    float lastBodyRaw_ = 0.0f;
};

class BassDrumVoice {
public:
    void init(double sampleRate, uint32_t seed = 0x606606u) {
        noise_.init(sampleRate, seed);
        bassDrum_.init(sampleRate);
        stop();
    }

    void trigger(float transientAmount,
                 float decayPercent,
                 float tuneSemitones,
                 float analogPitchJitterSemitones = 0.0f) {
        applyDefaults();
        const float decay = clampf(decayPercent, 0.0f, 1.0f);
        const float transient = clampf(transientAmount, 0.0f, 1.0f);
        // Square the decay knob so most of the XL kick stays short and punchy
        // The last part opens into the long tail
        const float shapedDecay = decay * decay;
        bassDrum_.decay = lerpf(kXLDecayMin, kXLDecayMax, shapedDecay);
        bassDrum_.attack = 0.10f + transient * 0.50f;
        bassDrum_.tuneSemitones = tuneSemitones + analogPitchJitterSemitones;
        bassDrum_.clickAmount = 0.10f + transient * 0.50f;
        bassDrum_.impulseAmount = 0.10f + transient * 0.50f;
        bassDrum_.trigger(1.0f);
        active_ = true;
        silenceFrames_ = 0;
    }

    float process() {
        if (!active_) {
            return 0.0f;
        }

        const float out = bassDrum_.process(noise_.process()) * kVoiceOutputTrim;
        if (bassDrum_.isActive()) {
            silenceFrames_ = 0;
        } else if (std::fabs(out) < 1.0e-5f) {
            silenceFrames_ += 1;
            if (silenceFrames_ >= 32) {
                active_ = false;
            }
        } else {
            silenceFrames_ = 0;
        }
        return out;
    }

    bool isActive() const {
        return active_;
    }

    void stop() {
        active_ = false;
        silenceFrames_ = 0;
    }

private:
    void applyDefaults() {
        bassDrum_.level = 1.0f;
        bassDrum_.tune = 0.28f;
        bassDrum_.tone = 0.34f;
        bassDrum_.decay = 0.72f;
        bassDrum_.attack = 0.25f;
        bassDrum_.drive = 0.18f;
        bassDrum_.tuneSemitones = 2.0f;
        bassDrum_.clickAmount = 0.25f;
        bassDrum_.impulseAmount = 0.25f;
    }

    WhiteNoise noise_;
    BassDrum bassDrum_;
    bool active_ = false;
    int silenceFrames_ = 0;
};

} // namespace SynthDrums606
