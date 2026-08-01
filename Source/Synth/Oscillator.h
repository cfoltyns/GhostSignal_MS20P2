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
    enum class Waveform
    {
        triangle,
        saw,
        square,
        pulse,
        sine,
        noise,
        superSaw,
        superSquare
    };

    struct OscillatorParams
    {
        double sampleRate { 44100.0 };
        Waveform waveform { Waveform::saw };
        float frequency { 440.0f };
        float phase { 0.0f };
        float drift { 0.0f };
        float stereoWidth { 0.0f };
        float gain { 1.0f };
        float shape { 0.0f };
        float pwm { 0.5f };
        int   octave { 0 };
        int   semitone { 0 };
        float fineTune { 0.0f };
        bool  hardSyncEnable { false };
        bool  fmReceive { false };
        bool  ringModEnable { false };
    };

    class Oscillator
    {
    public:
        Oscillator() = default;
        ~Oscillator() = default;

        void prepare (double sampleRate);
        void reset();
        void setParameters (const OscillatorParams& p);
        void setFrequency (float freq) { params.frequency = freq; }

        // Render numSamples into buffer. If stereoWidth > 0, renders stereo.
        void process (juce::AudioBuffer<float>& buffer, int numSamples, float fmInput = 0.0f);

    private:
        float renderSample (float phase, float fmInput);
        float noiseSample();

        OscillatorParams params;
        float phaseLeft { 0.0f };
        float phaseRight { 0.0f };
        float phaseInc { 0.0f };
        float phaseIncRight { 0.0f };
        std::mt19937 rng;
        std::uniform_real_distribution<float> dist { -1.0f, 1.0f };
    };
}
