/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#include "LowPass.h"

namespace dsp::filter
{
    void LowPass::prepare (double sampleRate)
    {
        params.sampleRate = sampleRate;
        reset();
    }

    void LowPass::reset()
    {
        for (int i = 0; i < 2; ++i)
        {
            hp[i] = bp[i] = lp[i] = 0.0f;
            x1[i] = x2[i] = y1[i] = y2[i] = 0.0f;
        }
    }

    void LowPass::setParameters (const LowPassParams& p)
    {
        params = p;
    }

    void LowPass::processK35 (juce::AudioBuffer<float>& buffer, int numSamples)
    {
        const float c = juce::jlimit (20.0f, 20000.0f, params.cutoff);
        const float w = 2.0f * juce::MathConstants<float>::pi * c / (float) params.sampleRate;
        const float q = 0.5f + params.resonance * 10.0f;
        const float cosw = std::cos (w);
        const float sinw = std::sin (w);
        const float alpha = sinw / (2.0f * q);

        const float b0 = (1.0f - cosw) / 2.0f;
        const float b1 = 1.0f - cosw;
        const float b2 = (1.0f - cosw) / 2.0f;
        const float a0 = 1.0f + alpha;
        const float a1 = -2.0f * cosw;
        const float a2 = 1.0f - alpha;

        const int ch = buffer.getNumChannels();
        for (int i = 0; i < numSamples; ++i)
        {
            for (int c = 0; c < ch; ++c)
            {
                const float x = buffer.getSample (c, i) * (1.0f + params.drive * 10.0f);
                const float out = (b0 * x + b1 * x1[c] + b2 * x2[c] - a1 * y1[c] - a2 * y2[c]) / a0;
                x2[c] = x1[c]; x1[c] = x; y2[c] = y1[c]; y1[c] = out;
                buffer.setSample (c, i, std::tanh (out));
            }
        }
    }

    void LowPass::processLadder (juce::AudioBuffer<float>& buffer, int numSamples)
    {
        const float c = juce::jlimit (20.0f, 20000.0f, params.cutoff);
        const float coeff = std::exp (-2.0f * juce::MathConstants<float>::pi * c / (float) params.sampleRate);
        const float drive = 1.0f + params.drive * 10.0f;
        const int ch = buffer.getNumChannels();
        for (int i = 0; i < numSamples; ++i)
        {
            for (int c = 0; c < ch; ++c)
            {
                float x = buffer.getSample (c, i) * drive;
                float out = 0.0f;
                float stage = x;
                for (int s = 0; s < (params.slope / 12); ++s)
                {
                    stage = coeff * (stage + x - lp[c]);
                    lp[c] = std::tanh (stage);
                    out += lp[c];
                }
                buffer.setSample (c, i, out / (params.slope / 12));
            }
        }
    }

    void LowPass::processSVF (juce::AudioBuffer<float>& buffer, int numSamples)
    {
        const float c = juce::jlimit (20.0f, 20000.0f, params.cutoff);
        const float f = 2.0f * std::sin (juce::MathConstants<float>::pi * c / (float) params.sampleRate);
        const float q = 1.0f - params.resonance * 0.9f;
        const int ch = buffer.getNumChannels();
        for (int i = 0; i < numSamples; ++i)
        {
            for (int c = 0; c < ch; ++c)
            {
                const float x = buffer.getSample (c, i) * (1.0f + params.drive);
                lp[c] += f * bp[c];
                hp[c] = x - lp[c] - q * bp[c];
                bp[c] += f * hp[c];
                lp[c] += f * bp[c];
                buffer.setSample (c, i, lp[c]);
            }
        }
    }

    void LowPass::process (juce::AudioBuffer<float>& buffer, int numSamples)
    {
        switch (params.model)
        {
            case FilterModel::ladder:
                processLadder (buffer, numSamples);
                break;
            case FilterModel::stateVariable:
                processSVF (buffer, numSamples);
                break;
            default:
                processK35 (buffer, numSamples);
                break;
        }
    }
}