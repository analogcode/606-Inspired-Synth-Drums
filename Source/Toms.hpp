#pragma once

// Super 606 Low & High Toms
// Copyright (c) 2026 Matthew Fecher (AnalogMatthew)

#include "SynthDrumCommon.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace SynthDrums606 {

static constexpr float kTomSilenceThreshold = 5.0e-6f;

struct TomMode {
    float ratio;
    float level;
    float decayT60Seconds;
    float startPhase;
};

struct TomSpec {
    float mainHz;
    float mainGlideHz;
    float mainGlideTimeSeconds;
    float mainDecayT60Seconds;
    float mainStartPhase;
    float bodyAttackSeconds;

    float lowerModeRatio;
    float lowerModeLevel;
    float lowerModeDecayT60Seconds;
    float lowerModeStartPhase;

    float secondHarmonicLevel;
    float thirdHarmonicLevel;

    float strikeHz;
    float strikeLevel;
    float strikeDecayT60Seconds;
    float strikeStartPhase;
    float strikeRiseSeconds;

    std::array<TomMode, 4> upperModes;

    float snapDecayT60Seconds;
    float burstDecayT60Seconds;
    float tailNoiseDecayT60Seconds;
    float noiseRiseSeconds;

    float lowSnapLevel;
    float lowBurstLevel;
    float lowHighPassHz;
    float lowLowPassHz;

    float highSnapLevel;
    float highBurstLevel;
    float highHighPassHz;
    float highLowPassHz;

    float focusedStrikeLevel;
    float focusedStrikeHighPassHz;
    float focusedStrikeLowPassHz;

    float tailNoiseLevel;
    float tailNoiseHighPassHz;
    float tailNoiseLowPassHz;

    float outputTrim;
};

// I kept the lower ring separate because it hangs on after the main note
// I used the four upper peaks that stayed visible in the tail and stopped there
// because the weaker lines did not change the sound enough to keep
static constexpr TomSpec kHighTomSpec = {
    208.0f,              // mainHz
    0.0f,                // mainGlideHz, high tom stays on one note
    0.050f,              // mainGlideTimeSeconds, unused when glide is zero
    0.218f,              // mainDecayT60Seconds, time to fall 60 dB
    2.674f,              // mainStartPhase
    0.00012f,            // bodyAttackSeconds
    135.616f / 208.0f,   // lowerModeRatio, measured 135.616 Hz ring
    0.090f,              // lowerModeLevel
    0.315f,              // lowerModeDecayT60Seconds
    1.059f,              // lowerModeStartPhase
    0.0095f,             // secondHarmonicLevel
    0.0032f,             // thirdHarmonicLevel
    285.0f,              // strikeHz
    0.71f,               // strikeLevel, backed off so it joins the body
    0.0125f,             // strikeDecayT60Seconds
    0.420f,              // strikeStartPhase
    0.00003f,            // strikeRiseSeconds
    {{
        {305.2f / 208.0f, 0.002617f, 0.800f, -2.987f},
        {345.7f / 208.0f, 0.001954f, 0.800f, 0.618f},
        {367.0f / 208.0f, 0.003381f, 0.400f, 1.002f},
        {386.7f / 208.0f, 0.003546f, 0.360f, 0.597f},
    }},
    0.012f,              // snapDecayT60Seconds
    0.080f,              // burstDecayT60Seconds
    0.300f,              // tailNoiseDecayT60Seconds
    0.0015f,             // noiseRiseSeconds
    0.500f,              // lowSnapLevel
    0.030f,              // lowBurstLevel
    300.0f,              // lowHighPassHz
    3000.0f,             // lowLowPassHz
    0.030f,              // highSnapLevel
    0.009f,              // highBurstLevel
    700.0f,              // highHighPassHz
    12000.0f,            // highLowPassHz
    0.0f,                // focusedStrikeLevel, low tom needs it but high does not
    700.0f,              // focusedStrikeHighPassHz
    1100.0f,             // focusedStrikeLowPassHz
    0.010f,              // tailNoiseLevel
    300.0f,              // tailNoiseHighPassHz
    1500.0f,             // tailNoiseLowPassHz
    0.80f,               // outputTrim
};

// I fit the low tom on its own instead of pitching the high tom down
// The body starts about 40 Hz high, then falls into the 124.435 Hz note
// I added the narrow noise strike after the body was right so the hit had weight
static constexpr TomSpec kLowTomSpec = {
    124.435f,             // mainHz, settled note in the Kit1 recording
    39.921f,              // mainGlideHz, added at the start
    0.050622f,            // mainGlideTimeSeconds
    0.3276f,              // mainDecayT60Seconds, time to fall 60 dB
    2.2826f,              // mainStartPhase
    0.00012f,             // bodyAttackSeconds
    1.0f,                 // lowerModeRatio, unused here
    0.0f,                 // lowerModeLevel
    0.100f,               // lowerModeDecayT60Seconds
    0.0f,                 // lowerModeStartPhase
    0.0095f,              // secondHarmonicLevel
    0.0032f,              // thirdHarmonicLevel
    367.0f,               // strikeHz
    1.05f,                // strikeLevel
    0.005f,               // strikeDecayT60Seconds
    0.200f,               // strikeStartPhase
    0.00015f,             // strikeRiseSeconds
    {{
        {1.0f, 0.0f, 0.100f, 0.0f},
        {1.0f, 0.0f, 0.100f, 0.0f},
        {1.0f, 0.0f, 0.100f, 0.0f},
        {1.0f, 0.0f, 0.100f, 0.0f},
    }},
    0.012f,               // snapDecayT60Seconds
    0.080f,               // burstDecayT60Seconds
    0.300f,               // tailNoiseDecayT60Seconds
    0.0015f,              // noiseRiseSeconds
    0.500f,               // lowSnapLevel
    0.030f,               // lowBurstLevel
    300.0f,               // lowHighPassHz
    3000.0f,              // lowLowPassHz
    0.030f,               // highSnapLevel
    0.009f,               // highBurstLevel
    700.0f,               // highHighPassHz
    12000.0f,             // highLowPassHz
    1.100f,               // focusedStrikeLevel
    700.0f,               // focusedStrikeHighPassHz
    1100.0f,              // focusedStrikeLowPassHz
    0.010f,               // tailNoiseLevel
    300.0f,               // tailNoiseHighPassHz
    1500.0f,              // tailNoiseLowPassHz
    0.80f,                // outputTrim
};

class TomVoice {
public:
    void init(double sampleRate, uint32_t seed = 0x606606u) {
        sampleRate_ = std::isfinite(sampleRate) && sampleRate >= 8000.0
            ? static_cast<float>(sampleRate)
            : 44100.0f;
        const uint32_t safeSeed = seed == 0u ? 0x606606u : seed;
        // The three noise bands get their own stream so changing one does not
        // shuffle the others
        lowNoiseRandom_.seed(safeSeed ^ 0x60001604u);
        highNoiseRandom_.seed(safeSeed ^ 0x60001607u);
        tailNoiseRandom_.seed(safeSeed ^ 0x60001605u);

        lowNoiseHighPass_.init(sampleRate_);
        lowNoiseLowPass_.init(sampleRate_);
        highNoiseHighPass_.init(sampleRate_);
        highNoiseLowPass_.init(sampleRate_);
        focusedStrikeHighPass_.init(sampleRate_);
        focusedStrikeLowPass_.init(sampleRate_);
        tailNoiseHighPass_.init(sampleRate_);
        tailNoiseLowPass_.init(sampleRate_);
        stop();
    }

    void trigger(const TomSpec &spec, float decayPercent, float pitchRatio) {
        if (!std::isfinite(decayPercent) ||
            !std::isfinite(pitchRatio) ||
            pitchRatio <= 0.0f) {
            stop();
            return;
        }

        spec_ = spec;
        const float decay = clampf(decayPercent, 0.05f, 1.0f);
        const float ratio = clampf(pitchRatio, 0.25f, 4.0f);

        mainHz_ = spec_.mainHz * ratio;
        mainGlideHz_ = spec_.mainGlideHz * ratio;
        mainGlidePole_ = onePoleCoef(sampleRate_, spec_.mainGlideTimeSeconds);
        lowerModeHz_ = mainHz_ * spec_.lowerModeRatio;
        strikeHz_ = spec_.strikeHz * ratio;
        mainPhase_ = spec_.mainStartPhase;
        lowerModePhase_ = spec_.lowerModeStartPhase;
        strikePhase_ = spec_.strikeStartPhase;
        mainEnvelope_ = 1.0f;
        lowerModeEnvelope_ = 1.0f;
        strikeEnvelope_ = 1.0f;

        for (std::size_t index = 0; index < spec_.upperModes.size(); ++index) {
            const TomMode &mode = spec_.upperModes[index];
            upperModeHz_[index] = mainHz_ * mode.ratio;
            upperModePhase_[index] = mode.startPhase;
            upperModeEnvelope_[index] = 1.0f;
            upperModePole_[index] = decayCoefT60(
                sampleRate_, mode.decayT60Seconds * decay);
        }

        snapEnvelope_ = 1.0f;
        burstEnvelope_ = 1.0f;
        tailNoiseEnvelope_ = 1.0f;
        mainPole_ = decayCoefT60(
            sampleRate_, spec_.mainDecayT60Seconds * decay);
        lowerModePole_ = decayCoefT60(
            sampleRate_, spec_.lowerModeDecayT60Seconds * decay);
        strikePole_ = decayCoefT60(
            sampleRate_, spec_.strikeDecayT60Seconds * decay);
        snapPole_ = decayCoefT60(
            sampleRate_, spec_.snapDecayT60Seconds * decay);
        burstPole_ = decayCoefT60(
            sampleRate_, spec_.burstDecayT60Seconds * decay);
        tailNoisePole_ = decayCoefT60(
            sampleRate_, spec_.tailNoiseDecayT60Seconds * decay);

        bodyRise_ = 0.0f;
        bodyRiseCoefficient_ = 1.0f - onePoleCoef(
            sampleRate_, spec_.bodyAttackSeconds);
        strikeRise_ = 0.0f;
        strikeRiseCoefficient_ = 1.0f - onePoleCoef(
            sampleRate_, spec_.strikeRiseSeconds);
        snapRise_ = 0.0f;
        snapRiseCoefficient_ = 1.0f - onePoleCoef(sampleRate_, 0.0004f);
        noiseRise_ = 0.0f;
        noiseRiseCoefficient_ = 1.0f - onePoleCoef(
            sampleRate_, spec_.noiseRiseSeconds);

        lowNoiseHighPass_.setHighPass(spec_.lowHighPassHz * ratio, 0.70710678f);
        lowNoiseLowPass_.setLowPass(spec_.lowLowPassHz * ratio, 0.70710678f);
        highNoiseHighPass_.setHighPass(spec_.highHighPassHz * ratio, 0.70710678f);
        highNoiseLowPass_.setLowPass(spec_.highLowPassHz * ratio, 0.70710678f);
        if (spec_.focusedStrikeLevel != 0.0f) {
            focusedStrikeHighPass_.setHighPass(
                spec_.focusedStrikeHighPassHz * ratio, 0.70710678f);
            focusedStrikeLowPass_.setLowPass(
                spec_.focusedStrikeLowPassHz * ratio, 0.70710678f);
        }
        tailNoiseHighPass_.setHighPass(
            spec_.tailNoiseHighPassHz * ratio, 0.70710678f);
        tailNoiseLowPass_.setLowPass(
            spec_.tailNoiseLowPassHz * ratio, 0.70710678f);
        lowNoiseHighPass_.reset();
        lowNoiseLowPass_.reset();
        highNoiseHighPass_.reset();
        highNoiseLowPass_.reset();
        focusedStrikeHighPass_.reset();
        focusedStrikeLowPass_.reset();
        tailNoiseHighPass_.reset();
        tailNoiseLowPass_.reset();
        active_ = true;
    }

    float process() {
        if (!active_) {
            return 0.0f;
        }

        mainPhase_ = wrapPhase(
            mainPhase_ + kTwoPi * (mainHz_ + mainGlideHz_) / sampleRate_);
        if (spec_.lowerModeLevel != 0.0f) {
            lowerModePhase_ = wrapPhase(
                lowerModePhase_ + kTwoPi * lowerModeHz_ / sampleRate_);
        }
        strikePhase_ = wrapPhase(
            strikePhase_ + kTwoPi * strikeHz_ / sampleRate_);
        for (std::size_t index = 0; index < spec_.upperModes.size(); ++index) {
            if (spec_.upperModes[index].level == 0.0f) {
                continue;
            }
            upperModePhase_[index] = wrapPhase(
                upperModePhase_[index] + kTwoPi * upperModeHz_[index] / sampleRate_);
        }

        bodyRise_ += (1.0f - bodyRise_) * bodyRiseCoefficient_;
        strikeRise_ += (1.0f - strikeRise_) * strikeRiseCoefficient_;
        snapRise_ += (1.0f - snapRise_) * snapRiseCoefficient_;
        noiseRise_ += (1.0f - noiseRise_) * noiseRiseCoefficient_;

        const float main = (std::sin(mainPhase_)
            + spec_.secondHarmonicLevel * std::sin(2.0f * mainPhase_)
            + spec_.thirdHarmonicLevel * std::sin(3.0f * mainPhase_))
            * mainEnvelope_;
        const float lowerMode = spec_.lowerModeLevel != 0.0f
            ? spec_.lowerModeLevel * std::sin(lowerModePhase_) * lowerModeEnvelope_
            : 0.0f;
        const float strike = spec_.strikeLevel * std::sin(strikePhase_)
            * strikeEnvelope_ * strikeRise_;

        float upperModes = 0.0f;
        for (std::size_t index = 0; index < spec_.upperModes.size(); ++index) {
            if (spec_.upperModes[index].level == 0.0f) {
                continue;
            }
            upperModes += spec_.upperModes[index].level
                * std::sin(upperModePhase_[index]) * upperModeEnvelope_[index];
        }

        float lowNoise = lowNoiseRandom_.bipolar24();
        lowNoise = lowNoiseLowPass_.process(lowNoiseHighPass_.process(lowNoise));
        const float highNoiseSource = highNoiseRandom_.bipolar24();
        float highNoise = highNoiseLowPass_.process(
            highNoiseHighPass_.process(highNoiseSource));
        float tailNoise = tailNoiseRandom_.bipolar24();
        tailNoise = tailNoiseLowPass_.process(
            tailNoiseHighPass_.process(tailNoise));

        const float snap = snapRise_ * snapEnvelope_;
        const float burst = noiseRise_ * burstEnvelope_;
        float excitation = lowNoise
                * (spec_.lowSnapLevel * snap + spec_.lowBurstLevel * burst)
            + highNoise
                * (spec_.highSnapLevel * snap + spec_.highBurstLevel * burst)
            + tailNoise * spec_.tailNoiseLevel
                * noiseRise_ * tailNoiseEnvelope_;
        if (spec_.focusedStrikeLevel != 0.0f) {
            float focusedStrike = focusedStrikeHighPass_.process(highNoiseSource);
            focusedStrike = focusedStrikeLowPass_.process(focusedStrike);
            excitation += focusedStrike * spec_.focusedStrikeLevel * snap;
        }

        const float output = ((main + lowerMode + upperModes) * bodyRise_
            + strike + excitation) * spec_.outputTrim;

        mainEnvelope_ = flushDenormal(mainEnvelope_ * mainPole_);
        mainGlideHz_ = flushDenormal(mainGlideHz_ * mainGlidePole_);
        lowerModeEnvelope_ = flushDenormal(
            lowerModeEnvelope_ * lowerModePole_);
        strikeEnvelope_ = flushDenormal(strikeEnvelope_ * strikePole_);
        bool upperModesAreSilent = true;
        for (std::size_t index = 0; index < spec_.upperModes.size(); ++index) {
            upperModeEnvelope_[index] = flushDenormal(
                upperModeEnvelope_[index] * upperModePole_[index]);
            if (spec_.upperModes[index].level * upperModeEnvelope_[index]
                > kTomSilenceThreshold) {
                upperModesAreSilent = false;
            }
        }
        snapEnvelope_ = flushDenormal(snapEnvelope_ * snapPole_);
        burstEnvelope_ = flushDenormal(burstEnvelope_ * burstPole_);
        tailNoiseEnvelope_ = flushDenormal(
            tailNoiseEnvelope_ * tailNoisePole_);
        if (mainEnvelope_ <= kTomSilenceThreshold
            && spec_.lowerModeLevel * lowerModeEnvelope_ <= kTomSilenceThreshold
            && upperModesAreSilent) {
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
    }

private:
    static float wrapPhase(float phase) {
        while (phase >= kTwoPi) {
            phase -= kTwoPi;
        }
        while (phase < 0.0f) {
            phase += kTwoPi;
        }
        return phase;
    }

    TomSpec spec_ = kHighTomSpec;
    float sampleRate_ = 44100.0f;
    bool active_ = false;

    float mainHz_ = 208.0f;
    float mainGlideHz_ = 0.0f;
    float lowerModeHz_ = 135.616f;
    float strikeHz_ = 285.0f;
    float mainPhase_ = 0.0f;
    float lowerModePhase_ = 0.0f;
    float strikePhase_ = 0.0f;
    std::array<float, 4> upperModeHz_ = {{0.0f, 0.0f, 0.0f, 0.0f}};
    std::array<float, 4> upperModePhase_ = {{0.0f, 0.0f, 0.0f, 0.0f}};
    std::array<float, 4> upperModeEnvelope_ = {{0.0f, 0.0f, 0.0f, 0.0f}};
    std::array<float, 4> upperModePole_ = {{0.0f, 0.0f, 0.0f, 0.0f}};
    float mainEnvelope_ = 0.0f;
    float lowerModeEnvelope_ = 0.0f;
    float strikeEnvelope_ = 0.0f;
    float snapEnvelope_ = 0.0f;
    float burstEnvelope_ = 0.0f;
    float tailNoiseEnvelope_ = 0.0f;
    float mainPole_ = 0.0f;
    float mainGlidePole_ = 0.0f;
    float lowerModePole_ = 0.0f;
    float strikePole_ = 0.0f;
    float snapPole_ = 0.0f;
    float burstPole_ = 0.0f;
    float tailNoisePole_ = 0.0f;
    float bodyRise_ = 0.0f;
    float bodyRiseCoefficient_ = 0.0f;
    float strikeRise_ = 0.0f;
    float strikeRiseCoefficient_ = 0.0f;
    float snapRise_ = 0.0f;
    float snapRiseCoefficient_ = 0.0f;
    float noiseRise_ = 0.0f;
    float noiseRiseCoefficient_ = 0.0f;

    Random lowNoiseRandom_;
    Random highNoiseRandom_;
    Random tailNoiseRandom_;
    Biquad lowNoiseHighPass_;
    Biquad lowNoiseLowPass_;
    Biquad highNoiseHighPass_;
    Biquad highNoiseLowPass_;
    Biquad focusedStrikeHighPass_;
    Biquad focusedStrikeLowPass_;
    Biquad tailNoiseHighPass_;
    Biquad tailNoiseLowPass_;
};

} // namespace SynthDrums606
