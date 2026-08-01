/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#include "NoiseGen.h"

namespace dsp
{
    void NoiseGen::prepare (double sampleRate)
    {
        params.sampleRate = sampleRate;
        reset();
    }

    void NoiseGen::reset()
    {
        b0 = b1 = b2 = b3 = b4 = b5 = 0.0f;
        lastWhite = 0.0f;
        lastBlue = 0.0f;
        std::random_device rd;
        rng.seed (rd());
    }

    void NoiseGen::setParameters (const NoiseGenParams& p)
    {
        params = p;
    }

    float NoiseGen::nextWhite()
    {
        return uniform (rng);
    }

    float NoiseGen::nextPink()
    {
        // Paul Kellet's pink noise approximation
        b0 = 0.99886f * b0 + uniform (rng) * 0.0555179f;
        b1 = 0.99332f * b1 + uniform (rng) * 0.0750758f;
        b2 = 0.96900f * b2 + uniform (rng) * 0.1538520f;
        b3 = 0.86650f * b3 + uniform (rng) * 0.3104856f;
        b4 = 0.55000f * b4 + uniform (rng) * 0.5329522f;
        b5 = -0.7616f * b5;
        return (b0 + b1 + b2 + b3 + b4 + b5) * 0.11f;
    }

    float NoiseGen::nextBrown()
    {
        const float w = nextWhite();
        b0 = (b0 + (0.02f * w)) / 1.02f;
        return juce::jlimit (-1.0f, 1.0f, b0 * 3.5f);
    }

    float NoiseGen::nextBlue()
    {
        const float w = nextWhite();
        float out = w - lastWhite;
        lastWhite = w;
        return juce::jlimit (-1.0f, 1.0f, out * 0.707f);
    }

    float NoiseGen::nextViolet()
    {
        const float b = nextBlue();
        float out = b - lastBlue;
        lastBlue = b;
        return juce::jlimit (-1.0f, 1.0f, out * 0.707f);
    }

    void NoiseGen::process (juce::AudioBuffer<float>& buffer, int numSamples)
    {
        const int ch = buffer.getNumChannels();
        for (int i = 0; i < numSamples; ++i)
        {
            float s = 0.0f;
            switch (params.type)
            {
                case NoiseType::white:  s = nextWhite(); break;
                case NoiseType::pink:   s = nextPink(); break;
                case NoiseType::brown:  s = nextBrown(); break;
                case NoiseType::blue:   s = nextBlue(); break;
                case NoiseType::violet: s = nextViolet(); break;
                default:                s = nextWhite(); break;
            }

            // brightness filter: simple one-pole highpass-ish
            if (params.brightness > 0.0f)
            {
                const float a = 0.5f + params.brightness * 0.5f;
                s = a * s + (1.0f - a) * b5;
                b5 = s;
            }

            s *= params.gain;
            for (int c = 0; c < ch; ++c)
                buffer.addFrom (c, i, &s, 1, 1.0f);
        }
    }
}