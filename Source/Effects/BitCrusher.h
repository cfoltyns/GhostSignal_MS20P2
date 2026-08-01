/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#pragma once

#include <JuceHeader.h>

namespace fx
{
    struct BitCrusherParams
    {
        double sampleRate { 44100.0 };
        float bitDepth { 8.0f };    // 1-16 bits
        float sampleRateReduction { 0.5f }; // 0-1 (1 = original, 0 = very low)
        float mix { 0.5f };
    };

    class BitCrusher
    {
    public:
        BitCrusher() = default;
        ~BitCrusher() = default;

        void prepare (double sampleRate);
        void reset();
        void setParameters (const BitCrusherParams& p);

        void process (juce::AudioBuffer<float>& buffer, int numSamples);

    private:
        BitCrusherParams params;
        double holdPhase { 0.0 };
        float lastSample[2] { 0.0f, 0.0f };
    };
}