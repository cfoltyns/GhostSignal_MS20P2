/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#include "Saturation.h"

namespace dsp::filter
{
    float Saturation::applyDrive (float x, float drive)
    {
        const float d = 1.0f + drive * 20.0f;
        return std::tanh (x * d);
    }

    float Saturation::wavefolder (float x, float bias)
    {
        x += bias;
        x = std::abs (x);
        x = 1.0f - x;
        x = std::abs (x);
        return x;
    }

    void Saturation::process (juce::AudioBuffer<float>& buffer, int numSamples)
    {
        if (params.drive <= 0.0f || params.model == 0)
            return;

        const int ch = buffer.getNumChannels();
        for (int i = 0; i < numSamples; ++i)
        {
            for (int c = 0; c < ch; ++c)
            {
                float x = buffer.getSample (c, i);
                const float wet = applyDrive (x, params.drive);

                switch (params.model)
                {
                    case 9: // wavefolder
                        x = wavefolder (x, params.bias);
                        break;
                    case 2: // tube-ish
                        x = std::tanh (x * (1.0f + params.drive * 5.0f)) * 1.2f;
                        break;
                    default:
                        x = wet;
                        break;
                }

                buffer.setSample (c, i, x * params.mix + buffer.getSample (c, i) * (1.0f - params.mix));
            }
        }
    }
}
