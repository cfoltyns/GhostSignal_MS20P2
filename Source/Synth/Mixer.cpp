/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#include "Mixer.h"

namespace dsp
{
    void Mixer::reset()
    {
        feedbackState = 0.0f;
    }

    void Mixer::setParameters (const MixerParams& p)
    {
        params = p;
    }

    void Mixer::process (juce::AudioBuffer<float>& output,
                         const juce::AudioBuffer<float>* osc1,
                         const juce::AudioBuffer<float>* osc2,
                         const juce::AudioBuffer<float>* sub,
                         const juce::AudioBuffer<float>* noise,
                         int numSamples)
    {
        const int ch = output.getNumChannels();

        for (int i = 0; i < numSamples; ++i)
        {
            float mix = 0.0f;

            if (osc1 != nullptr)
            {
                for (int c = 0; c < ch; ++c)
                    mix += osc1->getSample (c, i) * params.osc1Level;
            }
            if (osc2 != nullptr)
            {
                for (int c = 0; c < ch; ++c)
                    mix += osc2->getSample (c, i) * params.osc2Level;
            }
            if (sub != nullptr)
            {
                for (int c = 0; c < ch; ++c)
                    mix += sub->getSample (c, i) * params.subLevel;
            }
            if (noise != nullptr)
            {
                for (int c = 0; c < ch; ++c)
                    mix += noise->getSample (c, i) * params.noiseLevel;
            }

            // feedback loop
            feedbackState = mix * params.feedback + feedbackState * 0.9f;
            mix += feedbackState;

            // analog gain staging + soft clip
            const float drive = 1.0f + params.drive * 8.0f;
            mix = std::tanh (mix * drive);

            for (int c = 0; c < ch; ++c)
                output.setSample (c, i, mix);
        }
    }
}
