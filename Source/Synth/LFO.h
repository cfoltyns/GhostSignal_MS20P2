/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#pragma once

#include <JuceHeader.h>
#include <random>

namespace dsp
{
    enum class LfoWaveform { triangle, sine, sawUp, sawDown, square, random, sampleHold, noise, chaos, step, envelope, audioRate };

    struct LfoParams
    {
        double sampleRate { 44100.0 };
        LfoWaveform waveform { LfoWaveform::triangle };
        float rate { 1.0f };
        float phase { 0.0f };
        float fade { 0.0f };
        float humanize { 0.0f };
        float stereoOffset { 0.0f };
        bool  oneShot { false };
        bool  loop { true };
        bool  tempoSync { false };
        float depth { 1.0f };
        // MS-20 style shape morph: 0.0 = +Saw/-Pulse, 0.5 = Tri/Sqr, 1.0 = -Saw/+Pulse
        float shapeMorph { 0.5f };
    };

class Lfo
    {
    public:
        Lfo() = default;
        ~Lfo() = default;

        void prepare (double sampleRate);
        void reset();
        void setParameters (const LfoParams& p);
        void process (int numSamples, float phaseOffset = 0.0f);

        float getOutput() const { return currentOutput; }

    protected:
        float nextRandom();
        float nextNoise();
        float nextChaos();
        float renderMorph (float phase) const;

        LfoParams params;
        float phase { 0.0f };
        float currentOutput { 0.0f };
        float nextRandomValue { 0.0f };
        std::mt19937 rng;
        std::uniform_real_distribution<float> uniform { -1.0f, 1.0f };
    };
}
