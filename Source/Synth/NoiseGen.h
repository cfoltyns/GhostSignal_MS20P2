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
    enum class NoiseType { white, pink, brown, blue, violet, tape, vinyl, dust };

    struct NoiseGenParams
    {
        double sampleRate { 44100.0 };
        NoiseType type { NoiseType::white };
        float gain { 0.0f };
        float brightness { 0.0f };
        bool stereo { false };
    };

    class NoiseGen
    {
    public:
        NoiseGen() = default;
        ~NoiseGen() = default;

        void prepare (double sampleRate);
        void reset();
        void setParameters (const NoiseGenParams& p);
        void process (juce::AudioBuffer<float>& buffer, int numSamples);

    private:
        float nextWhite();
        float nextPink();
        float nextBrown();
        float nextBlue();
        float nextViolet();

        NoiseGenParams params;
        std::mt19937 rng;
        std::uniform_real_distribution<float> uniform { -1.0f, 1.0f };
        float b0 { 0.0f }, b1 { 0.0f }, b2 { 0.0f }, b3 { 0.0f }, b4 { 0.0f }, b5 { 0.0f };
        float lastWhite { 0.0f };
        float lastBlue { 0.0f };
    };
}
