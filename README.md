# 606 Inspired Synth Drums

Here's the secret sauce behind the Super 606 XL Kick Drum and its melodic metallic hi hats.

It's plain, modular C++ with no AudioKit, JUCE, plugin wrapper, sample player, or mixer attached. Drop the headers into your audio project and connect them to whatever engine you already use.

The Hi-hats are the clever part. Most drum synths start with a handful of square waves. These build the metal one partial at a time, which gives the strike a clearer note without losing the rough edges that make hardware hats feel alive.

If you want to fill out the rest of the kit, toms are the easy next stop. For a 606, 808, or 909 style snare, think of it as two sounds. Make a high pitched tom like body, add a separate burst of noise, then blend until it sits right.

## What's Inside?

- `BassDrum.hpp`:  A swept-sine body with filtered noise and impulse transients.
- `HiHats.hpp`:  One metallic source with separate closed and open hat settings.
- `SynthDrumCommon.hpp`:  The filters, envelopes, noise, and other small pieces shared by the voices.

The code is header only and uses the C++14 standard library. Add the `Source` folder to your header search path and include whichever voices you need.

## Hooking it up

```cpp
#include "BassDrum.hpp"
#include "HiHats.hpp"

SynthDrums606::BassDrumVoice bassDrum;
SynthDrums606::MetalHiHatVoice hiHat;

bassDrum.init(sampleRate);
hiHat.init(sampleRate);

// transient, decay, tuning in semitones, optional pitch variation
bassDrum.trigger(0.30f, 0.80f, 2.22f, 0.0f);

// preset, decay, pitch ratio
hiHat.trigger(SynthDrums606::kClosedHatSpec, 0.70f, 1.0f);

for (int frame = 0; frame < frameCount; ++frame) {
    float mono = bassDrum.process() + hiHat.process();
    outputLeft[frame] += mono;
    outputRight[frame] += mono;
}
```

Each call to `process()` gives you one mono sample. Your host can handle gain, velocity, panning, choke behavior, and the rest of the mixing.

Use `kClosedHatSpec` or `kOpenHatSpec` when you trigger the hat. Both voices accept a seed in `init()` if you want the little variations to repeat.

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

Both voices are made entirely in code.

## Legal Stuff

MIT. See `LICENSE`

This is an independent project inspired by classic analog drum-machine circuits for learning and education purposes. It is not affiliated with or endorsed by Roland Corporation.