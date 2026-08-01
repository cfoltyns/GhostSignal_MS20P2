/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#pragma once

#include <JuceHeader.h>
#include <array>

namespace fx
{
    struct DelayParams
    {
        double sampleRate { 44100.0 };
        float time { 0.5f };        // seconds
        float feedback { 0.3f };
        float mix { 0.5f };
        float stereoSpread { 0.5f };
        float modulation { 0.0f };  // LFO for chorus/flanger
        bool sync { false };
        int syncFactor { 4 };         // 1/4, 1/8, etc
    };

    class Delay
    {
    public:
        Delay() = default;
        ~Delay() = default;

        void prepare (double sampleRate);
        void reset();
        void setParameters (const DelayParams& p);

        void process (juce::AudioBuffer<float>& buffer, int numSamples);

    private:
        float readInterpolated (int channel, double delayInSamples);
        void writeSample (int channel, int index, float sample);

        DelayParams params;
        juce::AudioBuffer<float> buffer;
        int bufferSamples { 0 };
        int writePosition { 0 };

        // Simple LFO for modulation
        float lfoPhase { 0.0f };
    };
}