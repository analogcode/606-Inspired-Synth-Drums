#pragma once

// Super 606 Snare
// Copyright (c) 2026 Matthew Fecher (AnalogMatthew)

#include "SynthDrumCommon.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace SynthDrums606 {

// I kept the tuned shell and filtered wires separate so Tune and Snappy
// can move without pulling the other half around
// The odd-looking decimals below are the values fitted from the recordings
static constexpr float kSnareReferenceSampleRate = 44100.0f;
static constexpr float kSnareBodyDurationSeconds =
    11016.0f / kSnareReferenceSampleRate;
static constexpr float kSnareNoiseDurationSeconds =
    16323.0f / kSnareReferenceSampleRate;

static constexpr float kSnareBodySettledHz = 201.09442f;
static constexpr float kSnareBodyBendHz = 149.23143f;
static constexpr float kSnareBodyBendTimeSeconds = 0.0133309f;
static constexpr float kSnareBodyAttackTimeSeconds = 0.0007633316f;
static constexpr float kSnareBodyAttackShape = 3.2748916f;
static constexpr float kSnareBodyT60Seconds = 0.243f;
static constexpr float kSnareBodyLevel = 0.710f;
static constexpr float kSnareBodyToneLevel = 0.391f;
static constexpr float kSnareBodyTransientLevel = 0.44f;
static constexpr float kSnareBodyTransientT60Seconds = 0.030f;
static constexpr float kSnareBodyStartPhase = 1.1711004f;

static constexpr float kSnareImpactStartSeconds = 0.0002763113f;
static constexpr float kSnareImpactEndSeconds = 0.0013649489f;
static constexpr float kSnareImpactLevel = -0.3000014f;
static constexpr float kSnareImpactDecaySeconds = 0.0025407782f;

static constexpr float kSnareWireBand1Hz = 2998.628f;
static constexpr float kSnareWireBand1Q = 1.2771896f;
static constexpr float kSnareWireBand2Hz = 4596.2905f;
static constexpr float kSnareWireBand2Q = 0.30369505f;
static constexpr float kSnareWireBand2Amount = 1.597958f;
static constexpr float kSnareWireLowPassHz = 19677.396f;
static constexpr float kSnareWireAttackSeconds = 0.0031f;
static constexpr float kSnareWireAttackShape = 1.3f;
static constexpr float kSnareWireT60Seconds = 0.395f;
static constexpr float kSnareWireLevel = 1.28f;
static constexpr float kSnareWireRingLevel = 0.014f;
static constexpr float kSnareWireRingAttackSeconds = 0.012f;
static constexpr float kSnareWireRingStartPhase = -1.1f;

static constexpr float kSnareFullDecayThreshold = 0.999f;
static constexpr float kSnareGateCurve = 11.05f;
static constexpr float kSnareGateTargetRatio = 1.5881701e-5f;

class SnareVoice {
public:
    void init(double sampleRate, uint32_t seed = 0x606606u) {
        sampleRate_ = std::isfinite(sampleRate) && sampleRate >= 8000.0
            ? static_cast<float>(sampleRate)
            : kSnareReferenceSampleRate;
        random_.seed(seed == 0u ? 0x606606u : seed);
        noiseBand1_.init(sampleRate_);
        noiseBand2_.init(sampleRate_);
        noiseLowPass_.init(sampleRate_);
        stop();
    }

    void trigger(float decayPercent,
                 float pitchRatio,
                 float snappyAmount,
                 float noiseColorRatio = 1.0f) {
        if (!std::isfinite(decayPercent) ||
            !std::isfinite(pitchRatio) ||
            !std::isfinite(snappyAmount) ||
            !std::isfinite(noiseColorRatio) ||
            pitchRatio <= 0.0f ||
            noiseColorRatio <= 0.0f) {
            stop();
            return;
        }

        decay_ = clampf(decayPercent, 0.01f, 1.0f);
        pitchRatio_ = clampf(pitchRatio, 0.25f, 4.0f);
        snappyAmount_ = clampf(snappyAmount, 0.0f, 1.0f);
        noiseColorRatio_ = clampf(noiseColorRatio, 0.25f, 4.0f);

        selectedDurationSeconds_ = decay_ >= kSnareFullDecayThreshold
            ? kSnareNoiseDurationSeconds
            : kSnareNoiseDurationSeconds * decay_;

        // Short settings fold the end down instead of chopping it off
        const float removedDuration = std::max(
            0.0f, kSnareNoiseDurationSeconds - selectedDurationSeconds_);
        gateFadeSeconds_ = std::min(selectedDurationSeconds_, removedDuration);
        gateHoldSeconds_ = std::max(
            0.0f, selectedDurationSeconds_ - gateFadeSeconds_);

        naturalFrameCount_ = static_cast<uint64_t>(std::max(
            1.0f, std::ceil(selectedDurationSeconds_ * sampleRate_)));
        frameIndex_ = 0;
        bodyPhase_ = kSnareBodyStartPhase;
        noiseRingPhase_ = kSnareWireRingStartPhase;
        configureNoiseFilters();
        active_ = true;
    }

    float process() {
        if (!active_ || frameIndex_ >= naturalFrameCount_) {
            active_ = false;
            return 0.0f;
        }

        const float time = static_cast<float>(frameIndex_) / sampleRate_;
        const float body = processBody(time);

        // Keep the wire stream moving at zero Snappy so later hits stay repeatable
        const float noise = processNoise(time) * snappyAmount_;
        const float output = flushDenormal(body + noise);

        ++frameIndex_;
        if (frameIndex_ >= naturalFrameCount_) {
            active_ = false;
        }
        if (!std::isfinite(output)) {
            stop();
            return 0.0f;
        }
        return output;
    }

    bool isActive() const {
        return active_;
    }

    void stop() {
        active_ = false;
        frameIndex_ = 0;
        naturalFrameCount_ = 0;
        bodyPhase_ = 0.0f;
        noiseRingPhase_ = 0.0f;
        noiseBand1_.reset();
        noiseBand2_.reset();
        noiseLowPass_.reset();
    }

private:
    void configureNoiseFilters() {
        noiseBand1_.setBandPass(
            kSnareWireBand1Hz * noiseColorRatio_, kSnareWireBand1Q);
        noiseBand2_.setBandPass(
            kSnareWireBand2Hz * noiseColorRatio_, kSnareWireBand2Q);
        noiseBand1_.reset();
        noiseBand2_.reset();
        noiseLowPass_.setCutoffHz(kSnareWireLowPassHz * noiseColorRatio_);
        noiseLowPass_.reset();
        noiseSampleRateGain_ = std::sqrt(
            sampleRate_ / kSnareReferenceSampleRate);
    }

    float decayGate(float time) const {
        if (decay_ >= kSnareFullDecayThreshold || time < gateHoldSeconds_) {
            return 1.0f;
        }
        if (gateFadeSeconds_ <= 0.0f) {
            return 0.0f;
        }
        const float progress = (time - gateHoldSeconds_) / gateFadeSeconds_;
        if (progress >= 1.0f) {
            return 0.0f;
        }
        return std::max(0.0f,
            (1.0f + kSnareGateTargetRatio)
                * std::exp(-kSnareGateCurve * progress)
                - kSnareGateTargetRatio);
    }

    float processBody(float time) {
        const float tunedTime = time * pitchRatio_;
        if (tunedTime >= kSnareBodyDurationSeconds
            || time >= selectedDurationSeconds_) {
            return 0.0f;
        }

        // I pitch the shell note but leave the short bend alone
        // Moving both made the high notes dive too much
        const float frequency = kSnareBodySettledHz * pitchRatio_
            + kSnareBodyBendHz
                * std::exp(-time / kSnareBodyBendTimeSeconds);
        const float attack = std::pow(
            1.0f - std::exp(-time / kSnareBodyAttackTimeSeconds),
            kSnareBodyAttackShape);
        const float envelope = attack * std::exp(
            -kT60ToTau * tunedTime / kSnareBodyT60Seconds);
        const float transientEnvelope = attack * std::exp(
            -kT60ToTau * time / kSnareBodyTransientT60Seconds);

        float impact = 0.0f;
        if (time >= kSnareImpactStartSeconds
            && time < kSnareImpactEndSeconds) {
            impact = kSnareImpactLevel * std::exp(
                -(time - kSnareImpactStartSeconds)
                    / kSnareImpactDecaySeconds);
        }

        const float output = ((kSnareBodyToneLevel * envelope
            + kSnareBodyTransientLevel * transientEnvelope)
                * std::sin(bodyPhase_) + impact)
            * kSnareBodyLevel * decayGate(time);
        bodyPhase_ = wrapPhase(
            bodyPhase_ + kTwoPi * frequency / sampleRate_);
        return flushDenormal(output);
    }

    float processNoise(float time) {
        if (time >= selectedDurationSeconds_) {
            return 0.0f;
        }

        const float white = random_.bipolarLow24();
        const float band1 = noiseBand1_.process(white);
        const float band2 = noiseBand2_.process(white);
        const float colored = noiseLowPass_.process(
            band1 - kSnareWireBand2Amount * band2);

        // Let the wires open over 3.1 ms instead of clicking on all at once
        const float attackProgress = std::min(
            time / kSnareWireAttackSeconds, 1.0f);
        const float attack = 1.0f
            - std::pow(1.0f - attackProgress, kSnareWireAttackShape);
        const float envelope = attack * std::exp(
            -kT60ToTau * time / kSnareWireT60Seconds);
        float output = kSnareWireLevel
            * noiseSampleRateGain_ * envelope * colored;

        // A quiet low ring ties the wire sound back to the shell
        const float ringFrequency = (kSnareBodySettledHz
            + kSnareBodyBendHz
                * std::exp(-time / kSnareBodyBendTimeSeconds))
            * noiseColorRatio_;
        const float ringAttack = 1.0f
            - std::exp(-time / kSnareWireRingAttackSeconds);
        const float ringEnvelope = ringAttack * std::exp(
            -kT60ToTau * time / kSnareWireT60Seconds);
        output += kSnareWireRingLevel
            * ringEnvelope * std::sin(noiseRingPhase_);
        noiseRingPhase_ = wrapPhase(
            noiseRingPhase_ + kTwoPi * ringFrequency / sampleRate_);
        return flushDenormal(output * decayGate(time));
    }

    static float wrapPhase(float phase) {
        while (phase >= kTwoPi) {
            phase -= kTwoPi;
        }
        while (phase < 0.0f) {
            phase += kTwoPi;
        }
        return phase;
    }

    float sampleRate_ = kSnareReferenceSampleRate;
    float decay_ = 1.0f;
    float pitchRatio_ = 1.0f;
    float snappyAmount_ = 1.0f;
    float noiseColorRatio_ = 1.0f;
    float selectedDurationSeconds_ = kSnareNoiseDurationSeconds;
    float gateHoldSeconds_ = kSnareNoiseDurationSeconds;
    float gateFadeSeconds_ = 0.0f;
    float bodyPhase_ = 0.0f;
    float noiseRingPhase_ = 0.0f;
    float noiseSampleRateGain_ = 1.0f;
    uint64_t frameIndex_ = 0;
    uint64_t naturalFrameCount_ = 0;
    bool active_ = false;
    Random random_;
    Biquad noiseBand1_;
    Biquad noiseBand2_;
    OnePoleLPF noiseLowPass_;
};

} // namespace SynthDrums606
