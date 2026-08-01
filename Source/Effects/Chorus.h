/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#pragma once

#include <JuceHeader.h>

namespace fx
{
    struct ChorusParams
    {
        double sampleRate { 44100.0 };
        float rate { 0.5f };      // Hz
        float depth { 0.5f };     // 0-1
        float centreDelay { 0.02f }; // 20ms base
        float feedback { 0.0f };
        float mix { 0.5f };
    };

    class Chorus
    {
    public:
        Chorus() = default;
        ~Chorus() = default;

        void prepare (double sampleRate);
        void reset();
        void setParameters (const ChorusParams& p);

        void process (juce::AudioBuffer<float>& buffer, int numSamples);

    private:
        float readDelay (double phase, double delaySamples);

        ChorusParams params;
        juce::AudioBuffer<float> delayBuffer;
        int bufferSamples { 0 };
        int writePos { 0 };

        float lfoPhase[2] { 0.0f, 0.0f };
    };
}