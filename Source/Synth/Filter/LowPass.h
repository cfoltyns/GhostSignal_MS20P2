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
    struct LowPassParams
    {
        double sampleRate { 44100.0 };
        FilterModel model { FilterModel::k35Vintage };
        float cutoff { 1000.0f };
        float resonance { 0.0f };
        float drive { 0.0f };
        int   slope { 12 };
        float morph { 0.0f };
        float blend { 0.5f }; // 0 serial, 1 parallel
    };

    class LowPass
    {
    public:
        LowPass() = default;
        ~LowPass() = default;

        void prepare (double sampleRate);
        void reset();
        void setParameters (const LowPassParams& p);
        void process (juce::AudioBuffer<float>& buffer, int numSamples);

    private:
        void processK35 (juce::AudioBuffer<float>& buffer, int numSamples);
        void processLadder (juce::AudioBuffer<float>& buffer, int numSamples);
        void processSVF (juce::AudioBuffer<float>& buffer, int numSamples);

        LowPassParams params;
        float hp[2] { 0.0f }, bp[2] { 0.0f }, lp[2] { 0.0f };
        float x1[2] { 0.0f }, x2[2] { 0.0f }, y1[2] { 0.0f }, y2[2] { 0.0f }; // Per-channel state
    };
}
