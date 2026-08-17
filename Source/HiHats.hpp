#pragma once

// Super 606 Open & Closed Hi Hats
// Copyright (c) 2026 Matthew Fecher (AnalogMatthew)

#include "SynthDrumCommon.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace SynthDrums606 {

struct Partial {
    float frequencyHz;
    float amplitude;
    // Bell lines get the short strike accent
    bool bell = false;
};

// I ran FFTs at a few points in the open hat recording and kept the peaks
// that stayed put through the decay
// The 5978, 6340 and 7686 Hz lines carry most of the audible ting
// I pulled the rest down by ear so the middle did not turn into a harsh chord
static constexpr Partial kOpenHatPartials[] = {
    { 3804.0f, 0.072f },       { 4270.0f, 0.077f },
    { 4500.0f, 0.052f },       { 4632.0f, 0.038f },
    { 4800.0f, 0.033f },       { 4900.0f, 0.099f },
    { 5140.0f, 0.049f },       { 5400.0f, 0.116f },
    { 5500.0f, 0.116f },       { 5610.0f, 0.088f },
    { 5978.0f, 0.742f, true }, { 6340.0f, 1.300f, true },
    { 6500.0f, 0.181f },       { 6600.0f, 0.178f },
    { 6701.0f, 0.050f },       { 7106.0f, 0.126f },
    { 7200.0f, 0.128f },       { 7339.0f, 0.040f },
    { 7500.0f, 0.132f },       { 7686.0f, 0.573f, true },
    { 7854.0f, 0.086f },       { 7938.0f, 0.039f },
    { 8048.0f, 0.062f },       { 8399.0f, 0.078f },
    { 8514.0f, 0.081f },       { 8601.0f, 0.050f },
    { 8700.0f, 0.045f },       { 8876.0f, 0.166f },
    { 9000.0f, 0.051f },       { 9100.0f, 0.081f },
    { 9394.0f, 0.101f },       { 9500.0f, 0.052f },
    { 9600.0f, 0.064f },       { 9756.0f, 0.045f },
    { 9899.0f, 0.042f },       {10143.0f, 0.062f },
    {10300.0f, 0.034f },       {10800.0f, 0.043f },
    {11201.0f, 0.037f },       {11571.0f, 0.036f },
    {11673.0f, 0.041f },       {12318.0f, 0.060f },
    {12680.0f, 0.090f },       {12841.0f, 0.033f },
    {13340.0f, 0.045f },       {13663.0f, 0.034f },
    {16122.0f, 0.030f },
};

static constexpr int kMaxPartialCount = 64;

struct HiHatSpec {
    const Partial *partials;
    int partialCount;
    float noiseHighPassHz;
    float noiseLowPassHz;
    float tonalMix;
    float noiseMix;
    // 0 skips this. Past about 0.6 the metal starts turning into noise
    float saturationDrive;
    float outputTrim;
    float attackTimeConstantSeconds;
    // Short filtered noise burst at the start so the hit gets through a mix
    float clickAmount;
    float clickDecaySeconds;
    // Bell lines start louder, then settle after the hit
    float bellAccentAmount;
    float bellAccentDecaySeconds;
    // Closed hat uses the fast decay. Open hat blends in the slow one
    float envelopeFastWeight;
    float fastDecaySeconds;
    float slowDecaySeconds;
    // True changes the ring itself. False only moves the outer gate
    bool decayScalesTimeConstants;
    float referenceDurationSeconds;
    float minimumDecaySeconds;
    float minimumDurationSeconds;
    // Fade the gate so the open hat does not stop with a click
    float gateFadeMaxSeconds;
    // A few Hz of slow drift keeps the long open hat from turning into a chord
    // Set the depth to 0 when the lines should stay put
    float lineWobbleDepth;
    float lineWobbleCorrelationSeconds;
};

static constexpr HiHatSpec kClosedHatSpec = {
    // One metal source, short envelope for closed and long envelope for open
    kOpenHatPartials,
    47,                  // partialCount
    6800.0f,             // noiseHighPassHz
    12500.0f,            // noiseLowPassHz
    0.110f,              // tonalMix
    0.909f,              // noiseMix, final balance set by ear
    0.60f,               // saturationDrive
    1.75f,               // outputTrim, makeup after the saturation stage
    0.0003f,             // attackTimeConstantSeconds (gate edge, almost instant)
    0.35f,               // clickAmount
    0.003f,              // clickDecaySeconds
    0.70f,               // bellAccentAmount   } stronger, snappier strike so
    0.040f,              // bellAccentDecaySec } the ting cuts in a full mix
    1.0f,                // envelopeFastWeight (single exponential)
    0.03355f,            // fastDecaySeconds (falls about 0.26 dB per ms)
    0.03355f,            // slowDecaySeconds, matches the fast curve here
    true,                // decayScalesTimeConstants, this moves the actual ring
                         // instead of merely closing the gate later
    9700.0f / 44100.0f,  // 9700 frames in the 44.1 kHz closed hat reference
    0.004f,              // minimumDecaySeconds
    0.028f,              // minimumDurationSeconds
    0.045f,              // gateFadeMaxSeconds (short fade at the gate)
    0.0007f,             // lineWobbleDepth
    0.020f,              // lineWobbleCorrelationSeconds
};

static constexpr HiHatSpec kOpenHatSpec = {
    kOpenHatPartials,
    47,                  // partialCount
    6800.0f,             // noiseHighPassHz
    16000.0f,            // noiseLowPassHz, enough room for the shimmer
    0.110f,              // tonalMix
    0.909f,              // noiseMix, final balance set by ear
    0.60f,               // saturationDrive
    1.63f,               // outputTrim, makeup after the saturation stage
    0.0003f,             // attackTimeConstantSeconds (gate edge, almost instant)
    0.35f,               // clickAmount
    0.003f,              // clickDecaySeconds
    0.70f,               // bellAccentAmount   } stronger, snappier strike so
    0.040f,              // bellAccentDecaySec } the ting cuts in a full mix
    0.11f,               // envelopeFastWeight, a little snap over the wash
    0.010f,              // fastDecaySeconds
    0.580f,              // slowDecaySeconds, most of the tail lives here
    true,                // decayScalesTimeConstants, knob shortens the sizzle
                         // while the outer gate cleans up the end
    77864.0f / 44100.0f, // 77864 frames in the 44.1 kHz open hat reference
    0.004f,              // minimumDecaySeconds
    0.050f,              // minimumDurationSeconds
    0.500f,              // gateFadeMaxSeconds, tucks the long tail away cleanly
    0.0007f,             // lineWobbleDepth, a few Hz keeps it breathing
    0.020f,              // lineWobbleCorrelationSeconds, short enough that
                         // the lines never line up into giant crest spikes
};

static constexpr float kFullLevelSampleRateRatio = 0.40f;
static constexpr float kMutedSampleRateRatio = 0.48f;

class LowPassBiquad {
public:
    void init(double sampleRate) {
        sampleRate_ = sampleRate;
        reset();
    }

    void setLowPass(float cutoffHz, float q) {
        const float cutoff = clampf(
            cutoffHz,
            10.0f,
            static_cast<float>(sampleRate_ * 0.45));
        const float safeQ = std::max(0.05f, q);
        const float w0 = kTwoPi * cutoff
                       / static_cast<float>(sampleRate_);
        const float cosW0 = std::cos(w0);
        const float alpha = std::sin(w0) / (2.0f * safeQ);
        const float inverseA0 = 1.0f / (1.0f + alpha);
        b0_ = ((1.0f - cosW0) * 0.5f) * inverseA0;
        b1_ = (1.0f - cosW0) * inverseA0;
        b2_ = b0_;
        a1_ = (-2.0f * cosW0) * inverseA0;
        a2_ = (1.0f - alpha) * inverseA0;
    }

    void reset() {
        z1_ = 0.0f;
        z2_ = 0.0f;
    }

    float process(float input) {
        const float output = b0_ * input + z1_;
        z1_ = flushDenormal(b1_ * input - a1_ * output + z2_);
        z2_ = flushDenormal(b2_ * input - a2_ * output);
        return output;
    }

private:
    double sampleRate_ = 44100.0;
    float b0_ = 1.0f;
    float b1_ = 0.0f;
    float b2_ = 0.0f;
    float a1_ = 0.0f;
    float a2_ = 0.0f;
    float z1_ = 0.0f;
    float z2_ = 0.0f;
};

class MetalHiHatVoice {
public:
    void init(double sampleRate, uint32_t seed = 0x606606u) {
        sampleRate_ = std::isfinite(sampleRate) && sampleRate >= 8000.0
            ? sampleRate
            : 44100.0;
        const uint32_t safeSeed = seed == 0u ? 0x606606u : seed;
        phaseRandom_.seed(safeSeed ^ 0x9E3779B9u);
        wobbleRandom_.seed(safeSeed ^ 0x51D3B7A1u);
        noise_.init(sampleRate_, safeSeed ^ 0xA511E9B3u);
        noiseHighPass_.init(sampleRate_);
        noiseLowPass_.init(sampleRate_);
        stop();
    }

    void trigger(const HiHatSpec &spec, float decayPercent, float pitchRatio) {
        if (!std::isfinite(decayPercent) ||
            !std::isfinite(pitchRatio) ||
            pitchRatio <= 0.0f) {
            stop();
            return;
        }

        const float decay = clampf(decayPercent, 0.0f, 1.0f);
        const float frequencyRatio = clampf(
            pitchRatio,
            1.0f / 16.0f,
            16.0f);

        activePartialCount_ = std::min(spec.partialCount, kMaxPartialCount);
        for (int index = 0; index < activePartialCount_; ++index) {
            const float frequencyHz = spec.partials[index].frequencyHz * frequencyRatio;
            const float sampleRateRatio = frequencyHz / static_cast<float>(sampleRate_);
            float level = 1.0f;
            if (sampleRateRatio >= kMutedSampleRateRatio) {
                level = 0.0f;
            } else if (sampleRateRatio > kFullLevelSampleRateRatio) {
                const float taper = (kMutedSampleRateRatio - sampleRateRatio)
                                  / (kMutedSampleRateRatio - kFullLevelSampleRateRatio);
                level = taper * taper * (3.0f - 2.0f * taper);
            }

            // Start phases close enough to land as one hit
            // Perfect alignment clips, fully random phases lose the attack
            phases_[index] = (phaseRandom_.bipolar() + 1.0f)
                           * 0.3f * kPi;
            increments_[index] = level > 0.0f
                ? kTwoPi * sampleRateRatio
                : 0.0f;
            amplitudes_[index] = spec.partials[index].amplitude * level;
            bellFlags_[index] = spec.partials[index].bell;
        }
        bellAccentAmount_ = std::max(0.0f, spec.bellAccentAmount);
        bellAccentCoefficient_ = decayCoef(
            sampleRate_,
            std::max(0.005f, spec.bellAccentDecaySeconds));
        bellAccentEnvelope_ = 1.0f;
        clickAmount_ = std::max(0.0f, spec.clickAmount);
        clickCoefficient_ = decayCoef(
            sampleRate_,
            std::max(0.0005f, spec.clickDecaySeconds));
        clickEnvelope_ = 1.0f;

        noiseHighPass_.setHighPass(spec.noiseHighPassHz * frequencyRatio, 0.70f);
        noiseLowPass_.setLowPass(spec.noiseLowPassHz * frequencyRatio, 0.707f);
        noiseHighPass_.reset();
        noiseLowPass_.reset();
        dcBlocker_.reset();

        tonalMix_ = spec.tonalMix;
        noiseMix_ = spec.noiseMix;
        saturationDrive_ = std::max(0.0f, spec.saturationDrive);
        outputTrim_ = spec.outputTrim;

        if (spec.lineWobbleDepth > 0.0f) {
            wobbleAlpha_ = 1.0f - decayCoef(
                sampleRate_,
                std::max(0.005f, spec.lineWobbleCorrelationSeconds));
            // Scale the noise so correlation does not change the wobble depth
            wobbleDrive_ = spec.lineWobbleDepth * std::sqrt(6.0f * wobbleAlpha_);
        } else {
            wobbleAlpha_ = 0.0f;
            wobbleDrive_ = 0.0f;
        }
        for (int index = 0; index < kMaxPartialCount; ++index) {
            wobbleStates_[index] = 0.0f;
        }

        const float timeConstantScale = spec.decayScalesTimeConstants ? decay : 1.0f;
        fastDecayCoefficient_ = decayCoef(
            sampleRate_,
            std::max(spec.minimumDecaySeconds, timeConstantScale * spec.fastDecaySeconds));
        slowDecayCoefficient_ = decayCoef(
            sampleRate_,
            std::max(spec.minimumDecaySeconds, timeConstantScale * spec.slowDecaySeconds));
        fastEnvelopeWeight_ = clampf(spec.envelopeFastWeight, 0.0f, 1.0f);
        fastEnvelope_ = 1.0f;
        slowEnvelope_ = 1.0f;
        attackCoefficient_ = 1.0f - decayCoef(
            sampleRate_,
            spec.attackTimeConstantSeconds);
        attackEnvelope_ = 0.0f;

        const float naturalDurationSeconds = std::max(
            spec.minimumDurationSeconds,
            decay * spec.referenceDurationSeconds);
        naturalFrameCount_ = static_cast<uint64_t>(std::max(
            1.0,
            std::floor(naturalDurationSeconds * sampleRate_ + 0.5)));
        uint64_t gateFadeFrames = static_cast<uint64_t>(std::max(
            0.0,
            std::floor(static_cast<double>(spec.gateFadeMaxSeconds) * sampleRate_ + 0.5)));
        gateFadeFrames = std::min(gateFadeFrames, naturalFrameCount_);
        gateFadeFrames_ = gateFadeFrames;
        inverseGateFadeFrames_ = gateFadeFrames > 0
            ? 1.0f / static_cast<float>(gateFadeFrames)
            : 0.0f;
        frameIndex_ = 0;
        active_ = true;
    }

    float process() {
        if (!active_ || frameIndex_ >= naturalFrameCount_) {
            active_ = false;
            return 0.0f;
        }

        if (wobbleDrive_ > 0.0f) {
            for (int index = 0; index < activePartialCount_; ++index) {
                wobbleStates_[index] += wobbleDrive_ * wobbleRandom_.bipolar()
                                      - wobbleAlpha_ * wobbleStates_[index];
            }
        }

        const float bellGain = 1.0f + bellAccentAmount_ * bellAccentEnvelope_;
        bellAccentEnvelope_ = flushDenormal(bellAccentEnvelope_ * bellAccentCoefficient_);

        float tonal = 0.0f;
        for (int index = 0; index < activePartialCount_; ++index) {
            const float lineGain = bellFlags_[index] ? bellGain : 1.0f;
            tonal += std::sin(phases_[index]) * amplitudes_[index] * lineGain;
            phases_[index] += increments_[index] * (1.0f + wobbleStates_[index]);
            if (phases_[index] >= kTwoPi) {
                phases_[index] -= kTwoPi;
            }
        }

        const float noise = noiseLowPass_.process(
            noiseHighPass_.process(noise_.process()));

        attackEnvelope_ += attackCoefficient_ * (1.0f - attackEnvelope_);
        fastEnvelope_ = flushDenormal(fastEnvelope_ * fastDecayCoefficient_);
        slowEnvelope_ = flushDenormal(slowEnvelope_ * slowDecayCoefficient_);
        float envelope = attackEnvelope_
            * (fastEnvelopeWeight_ * fastEnvelope_
               + (1.0f - fastEnvelopeWeight_) * slowEnvelope_);
        if (gateFadeFrames_ > 0) {
            const uint64_t framesRemaining = naturalFrameCount_ - frameIndex_;
            if (framesRemaining < gateFadeFrames_) {
                envelope *= static_cast<float>(framesRemaining) * inverseGateFadeFrames_;
            }
        }

        float source = tonal * tonalMix_ + noise * noiseMix_;
        if (saturationDrive_ > 0.0f) {
            source = std::tanh(source * saturationDrive_);
        }
        float body = source * envelope
                   + noise * clickAmount_ * clickEnvelope_;
        clickEnvelope_ = flushDenormal(clickEnvelope_ * clickCoefficient_);
        float output = body * outputTrim_;
        output = flushDenormal(dcBlocker_.process(output));
        if (!std::isfinite(output)) {
            stop();
            return 0.0f;
        }

        ++frameIndex_;
        if (frameIndex_ >= naturalFrameCount_) {
            active_ = false;
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
        gateFadeFrames_ = 0;
        inverseGateFadeFrames_ = 0.0f;
        attackEnvelope_ = 0.0f;
        fastEnvelope_ = 0.0f;
        slowEnvelope_ = 0.0f;
        noiseHighPass_.reset();
        noiseLowPass_.reset();
        dcBlocker_.reset();
        activePartialCount_ = 0;
        wobbleAlpha_ = 0.0f;
        wobbleDrive_ = 0.0f;
        bellAccentAmount_ = 0.0f;
        bellAccentEnvelope_ = 0.0f;
        clickAmount_ = 0.0f;
        clickEnvelope_ = 0.0f;
        for (int index = 0; index < kMaxPartialCount; ++index) {
            phases_[index] = 0.0f;
            increments_[index] = 0.0f;
            amplitudes_[index] = 0.0f;
            wobbleStates_[index] = 0.0f;
            bellFlags_[index] = false;
        }
    }

private:
    double sampleRate_ = 44100.0;
    Random phaseRandom_;
    Random wobbleRandom_;
    WhiteNoise noise_;
    Biquad noiseHighPass_;
    LowPassBiquad noiseLowPass_;
    DCBlocker dcBlocker_;
    float phases_[kMaxPartialCount] = {};
    float increments_[kMaxPartialCount] = {};
    float amplitudes_[kMaxPartialCount] = {};
    float wobbleStates_[kMaxPartialCount] = {};
    bool bellFlags_[kMaxPartialCount] = {};
    float wobbleAlpha_ = 0.0f;
    float wobbleDrive_ = 0.0f;
    float bellAccentAmount_ = 0.0f;
    float bellAccentCoefficient_ = 0.999f;
    float bellAccentEnvelope_ = 0.0f;
    float clickAmount_ = 0.0f;
    float clickCoefficient_ = 0.999f;
    float clickEnvelope_ = 0.0f;
    int activePartialCount_ = 0;
    float tonalMix_ = 0.0f;
    float noiseMix_ = 0.0f;
    float saturationDrive_ = 0.0f;
    float outputTrim_ = 1.0f;
    float attackCoefficient_ = 0.01f;
    float attackEnvelope_ = 0.0f;
    float fastDecayCoefficient_ = 0.999f;
    float slowDecayCoefficient_ = 0.999f;
    float fastEnvelopeWeight_ = 1.0f;
    float fastEnvelope_ = 0.0f;
    float slowEnvelope_ = 0.0f;
    uint64_t naturalFrameCount_ = 0;
    uint64_t gateFadeFrames_ = 0;
    float inverseGateFadeFrames_ = 0.0f;
    uint64_t frameIndex_ = 0;
    bool active_ = false;
};

} // namespace SynthDrums606
