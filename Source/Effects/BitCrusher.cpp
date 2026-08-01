/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#include "BitCrusher.h"

namespace fx
{
    void BitCrusher::prepare (double newSampleRate)
    {
        params.sampleRate = newSampleRate;
        reset();
    }

    void BitCrusher::reset()
    {
        holdPhase = 0.0;
        lastSample[0] = lastSample[1] = 0.0f;
    }

    void BitCrusher::setParameters (const BitCrusherParams& p)
    {
        params = p;
    }

    void BitCrusher::process (juce::AudioBuffer<float>& buffer, int numSamples)
    {
        const int bits = (int) juce::jlimit (1, 16, (int) params.bitDepth);
        const float quantization = 1.0f / (float) ((1 << bits) - 1);
        const float oversample = 1.0f - params.sampleRateReduction;
        const double decimation = juce::jlimit (1.0, 100.0, 1.0 / (0.01 + oversample * 10.0));

        for (int i = 0; i < numSamples; ++i)
        {
            for (int ch = 0; ch < juce::jmin (2, buffer.getNumChannels()); ++ch)
            {
                const float in = buffer.getSample (ch, i);
                float out = in;

                // Sample rate reduction (decimation)
                holdPhase += decimation;
                if (holdPhase >= 1.0)
                {
                    lastSample[ch] = in;
                    holdPhase -= 1.0;
                }
                out = lastSample[ch];

                // Bit depth reduction (quantization)
                out = std::floor (out * (float) ((1 << bits) - 1)) / (float) ((1 << bits) - 1);

                // Mix dry/wet
                buffer.setSample (ch, i, in * (1.0f - params.mix) + out * params.mix);
            }
        }
    }
}