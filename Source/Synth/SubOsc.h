/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#pragma once

#include <JuceHeader.h>

namespace dsp
{
    enum class SubWaveform { sine, square, triangle };

    struct SubOscParams
    {
        double sampleRate { 44100.0 };
        SubWaveform waveform { SubWaveform::sine };
        float frequency { 0.0f };
        float gain { 0.0f };
        float saturation { 0.0f };
        int   octave { -1 };
    };

    class SubOsc
    {
    public:
        SubOsc() = default;
        ~SubOsc() = default;

        void prepare (double sampleRate);
        void reset();
        void setParameters (const SubOscParams& p);
        void process (juce::AudioBuffer<float>& buffer, int numSamples);

    private:
        float renderSample();
        SubOscParams params;
        float phase { 0.0f };
        float phaseInc { 0.0f };
    };
}
