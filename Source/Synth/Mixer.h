/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#pragma once

#include <JuceHeader.h>

namespace dsp
{
    struct MixerParams
    {
        float osc1Level { 0.8f };
        float osc2Level { 0.8f };
        float subLevel  { 0.0f };
        float noiseLevel { 0.0f };
        float extInput  { 0.0f };
        float feedback  { 0.0f };
        float drive     { 0.0f };
    };

    class Mixer
    {
    public:
        Mixer() = default;
        ~Mixer() = default;

        void prepare (double /*sampleRate*/) {}
        void reset();
        void setParameters (const MixerParams& p);

        // Mixes sources into output. Inputs are optional (can be nullptr).
        void process (juce::AudioBuffer<float>& output,
                      const juce::AudioBuffer<float>* osc1,
                      const juce::AudioBuffer<float>* osc2,
                      const juce::AudioBuffer<float>* sub,
                      const juce::AudioBuffer<float>* noise,
                      int numSamples);

    private:
        MixerParams params;
        float feedbackState { 0.0f };
    };
}
