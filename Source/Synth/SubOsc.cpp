/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#include "SubOsc.h"

namespace dsp
{
    void SubOsc::prepare (double newSampleRate)
    {
        params.sampleRate = newSampleRate;
        reset();
    }

    void SubOsc::reset()
    {
        phase = 0.0f;
        phaseInc = 0.0f;
    }

    void SubOsc::setParameters (const SubOscParams& p)
    {
        params = p;
        const float freq = params.frequency * std::pow (2.0f, (float) params.octave);
        phaseInc = (float) (freq / params.sampleRate) * juce::MathConstants<float>::twoPi;
        if (phaseInc > juce::MathConstants<float>::pi)
            phaseInc = juce::MathConstants<float>::pi; // naive anti-alias limit
    }

    float SubOsc::renderSample()
    {
        using juce::MathConstants;
        float s = 0.0f;
        switch (params.waveform)
        {
            case SubWaveform::sine:
                s = std::sin (phase);
                break;
            case SubWaveform::square:
                s = (phase < MathConstants<float>::pi) ? 1.0f : -1.0f;
                break;
            case SubWaveform::triangle:
                s = 4.0f * std::abs (phase / MathConstants<float>::twoPi - 0.5f) - 1.0f;
                break;
        }

        // simple soft clip for saturation
        if (params.saturation > 0.0f)
        {
            const float drive = 1.0f + params.saturation * 10.0f;
            s = std::tanh (s * drive);
        }

        return s * params.gain;
    }

    void SubOsc::process (juce::AudioBuffer<float>& buffer, int numSamples)
    {
        const int ch = buffer.getNumChannels();
        for (int i = 0; i < numSamples; ++i)
        {
            const float s = renderSample();
            for (int c = 0; c < ch; ++c)
                buffer.addFrom (c, i, &s, 1, 1.0f);

            phase += phaseInc;
            if (phase > MathConstants<float>::twoPi)
                phase -= MathConstants<float>::twoPi;
        }
    }
}
