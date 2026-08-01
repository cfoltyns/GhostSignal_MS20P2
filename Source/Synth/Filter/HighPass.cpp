/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#include "HighPass.h"

namespace dsp::filter
{
    void HighPass::prepare (double sampleRate)
    {
        params.sampleRate = sampleRate;
        reset();
    }

    void HighPass::reset()
    {
        for (int i = 0; i < 2; ++i)
        {
            x1[i] = x2[i] = y1[i] = y2[i] = 0.0f;
        }
    }

    void HighPass::setParameters (const HighPassParams& p)
    {
        params = p;
    }

    void HighPass::processK35 (juce::AudioBuffer<float>& buffer, int numSamples)
    {
        const float c = juce::jlimit (20.0f, 20000.0f, params.cutoff);
        const float w = 2.0f * juce::MathConstants<float>::pi * c / (float) params.sampleRate;
        const float q = 0.707f + params.resonance * 5.0f;
        const float cosw = std::cos (w);
        const float sinw = std::sin (w);
        const float alpha = sinw / (2.0f * q);

        const float b0 = (1.0f + cosw) / 2.0f;
        const float b1 = -(1.0f + cosw);
        const float b2 = (1.0f + cosw) / 2.0f;
        const float a0 = 1.0f + alpha;
        const float a1 = -2.0f * cosw;
        const float a2 = 1.0f - alpha;

        const int ch = buffer.getNumChannels();
        for (int i = 0; i < numSamples; ++i)
        {
            for (int c = 0; c < ch; ++c)
            {
                const float x = buffer.getSample (c, i);
                const float out = (b0 * x + b1 * x1[c] + b2 * x2[c] - a1 * y1[c] - a2 * y2[c]) / a0;
                x2[c] = x1[c]; x1[c] = x; y2[c] = y1[c]; y1[c] = out;
                buffer.setSample (c, i, out);
            }
        }
    }

    void HighPass::processLadder (juce::AudioBuffer<float>& buffer, int numSamples)
    {
        // simplified 12dB ladder HP
        const float c = juce::jlimit (20.0f, 20000.0f, params.cutoff);
        const float coeff = std::exp (-2.0f * juce::MathConstants<float>::pi * c / (float) params.sampleRate);
        const int ch = buffer.getNumChannels();
        for (int i = 0; i < numSamples; ++i)
        {
            for (int c = 0; c < ch; ++c)
            {
                const float x = buffer.getSample (c, i);
                y1[c] = coeff * (y1[c] + x - x1[c]);
                x1[c] = x;
                buffer.setSample (c, i, y1[c]);
            }
        }
    }

    void HighPass::process (juce::AudioBuffer<float>& buffer, int numSamples)
    {
        switch (params.model)
        {
            case FilterModel::k35Vintage:
            case FilterModel::ota:
            case FilterModel::stateVariable:
            case FilterModel::cleanDigital:
                processK35 (buffer, numSamples);
                break;
            case FilterModel::ladder:
                processLadder (buffer, numSamples);
                break;
        }
    }
}