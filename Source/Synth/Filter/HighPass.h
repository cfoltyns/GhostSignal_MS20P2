/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#pragma once

#include <JuceHeader.h>
#include "FilterTypes.h"

namespace dsp::filter
{
    struct HighPassParams
    {
        double sampleRate { 44100.0 };
        FilterModel model { FilterModel::k35Vintage };
        float cutoff { 100.0f };
        float resonance { 0.0f };
        float drive { 0.0f };
        int   slope { 12 }; // 12, 24, 36, 48
    };

    class HighPass
    {
    public:
        HighPass() = default;
        ~HighPass() = default;

        void prepare (double sampleRate);
        void reset();
        void setParameters (const HighPassParams& p);
        void process (juce::AudioBuffer<float>& buffer, int numSamples);

    private:
        void processK35 (juce::AudioBuffer<float>& buffer, int numSamples);
        void processLadder (juce::AudioBuffer<float>& buffer, int numSamples);

        HighPassParams params;
        float x1[2] { 0.0f }, x2[2] { 0.0f }, y1[2] { 0.0f }, y2[2] { 0.0f }; // Per-channel state
    };
}
