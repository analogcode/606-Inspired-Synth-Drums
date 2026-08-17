# 606 Inspired Synth Drums

See demos of the [SUPER 606 app here](https://audiokitpro.com/super606). 

Here's the secret sauce behind the Super 606 XL "608"  Kick Drum, Analog Toms, and its melodic metallic Hi-Hats. 


It's original modular C++ with no plugin  wrapper (no JUCE, etc), sample player, or mixer attached. Drop the headers into your audio project and connect them to whatever engine you already use.

The hi-hats are the clever part. Most drum synths start with a handful of square waves. These build the metal one partial at a time, which gives the strike a clearer note without losing the rough edges that make hardware hats feel alive. I haven't seen any other repos make synth hi-hats this way, so thought it was worth adding to the conversation.

## Hear the Low Tom

The toms use a few quieter resonances around the main note instead of a single clean sine. The low tom falls into pitch over its first few cycles, while the high tom leaves a lower ring hanging behind the body.



- 🔊 [OG 1983 Analog 606 Lo Tom](Audio/606_LT_Hardware_48k.wav)
- 🔊 [NEW Open-Source  606 Lo Tom](Audio/606_LT_NEW_OpenSource.wav)


## Get Creative

If you want to fill out the rest of the kit, a 606, 808, or 909 style snare is a good next stop. Think of it as two sounds: make a high pitched tom-like body, add a separate burst of noise, then blend until it sits right. And also check out the excellent [Mutable Instruments Peaks Snare DSP](https://github.com/pichenettes/eurorack/tree/master/peaks/drums).


## What's Inside?

- `BassDrum.hpp`:  A swept-sine body with filtered noise and impulse transients.
- `HiHats.hpp`:  One metallic source with separate closed and open hat settings.
- `Toms.hpp`:  Separate low and high tom settings built around the same voice.
- `SynthDrumCommon.hpp`:  The filters, envelopes, noise, and other small pieces shared by the voices.

The code is header only and uses the C++14 standard library. Add the `Source` folder to your header search path and include whichever voices you need.

## Hooking it up

```cpp
#include "BassDrum.hpp"
#include "HiHats.hpp"
#include "Toms.hpp"

SynthDrums606::BassDrumVoice bassDrum;
SynthDrums606::MetalHiHatVoice hiHat;
SynthDrums606::TomVoice lowTom;
SynthDrums606::TomVoice highTom;

bassDrum.init(sampleRate);
hiHat.init(sampleRate);
lowTom.init(sampleRate, 0x6061u);
highTom.init(sampleRate, 0x6062u);

// transient, decay, tuning in semitones, optional pitch variation
bassDrum.trigger(0.30f, 0.80f, 2.22f, 0.0f);

// preset, decay, pitch ratio
hiHat.trigger(SynthDrums606::kClosedHatSpec, 0.70f, 1.0f);

// low or high spec, decay, pitch ratio
lowTom.trigger(SynthDrums606::kLowTomSpec, 0.80f, 1.0f);
highTom.trigger(SynthDrums606::kHighTomSpec, 0.80f, 1.0f);

for (int frame = 0; frame < frameCount; ++frame) {
    float mono = bassDrum.process()
               + hiHat.process()
               + lowTom.process()
               + highTom.process();
    outputLeft[frame] += mono;
    outputRight[frame] += mono;
}
```

Each call to `process()` gives you one mono sample. Your host can handle gain, velocity, panning, choke behavior, and the rest of the mixing.

Use `kClosedHatSpec` or `kOpenHatSpec` when you trigger the hat. The tom voice works the same way with `kLowTomSpec` and `kHighTomSpec`. Each voice accepts a seed in `init()` if you want the little variations to repeat.

## Knobs

`BassDrumVoice::trigger()` wants:

- transient amount from 0 to 1
- decay from 0 to 1
- tuning in semitones
- optional pitch variation in semitones

`MetalHiHatVoice::trigger()` wants:

- a closed or open hat specification
- decay from 0 to 1
- a pitch ratio, where 1 is the original tuning

`TomVoice::trigger()` wants:

- a low or high tom specification
- decay from 0 to 1
- a pitch ratio, where 1 is the original tuning

All of the voices are made entirely in code.

## Legal Stuff

MIT. See `LICENSE`

This is an independent project inspired by classic analog drum-machine circuits for learning and education purposes. It is not affiliated with or endorsed by Roland Corporation.
