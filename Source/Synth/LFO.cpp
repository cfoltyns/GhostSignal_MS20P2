/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#include "LFO.h"

namespace dsp
{
    void Lfo::prepare (double sampleRate)
    {
        params.sampleRate = sampleRate;
        reset();
    }

    void Lfo::reset()
    {
        phase = 0.0f;
        currentOutput = 0.0f;
        nextRandomValue = 0.0f;
        std::random_device rd;
        rng.seed (rd());
    }

    void Lfo::setParameters (const LfoParams& p)
    {
        params = p;
    }

    float Lfo::nextRandom()
    {
        nextRandomValue = uniform (rng);
        return nextRandomValue;
    }

    float Lfo::nextNoise()
    {
        return uniform (rng);
    }

    float Lfo::nextChaos()
    {
        float chaosState = std::fmod (phase * 0.1f + 0.1f * std::sin (phase), 1.0f);
        return chaosState * 2.0f - 1.0f;
    }

    float Lfo::renderMorph (float p) const
    {
        const float twoPi = juce::MathConstants<float>::twoPi;
        const float morph = params.shapeMorph; // 0..1

        // Compute the three base shapes:
        // +Saw (sawUp): 0 -> 1
        float sawUp = p / twoPi;
        // -Saw (sawDown): 1 -> 0
        float sawDown = 1.0f - p / twoPi;
        // Triangle: 0 -> 1 -> 0
        float tri = (p < juce::MathConstants<float>::pi)
            ? (p / juce::MathConstants<float>::pi)
            : (2.0f - p / juce::MathConstants<float>::pi);
        // Pulse (square): 1 for first half, -1 for second
        float pulse = (p < juce::MathConstants<float>::pi) ? 1.0f : -1.0f;

        // Morph from +Saw/-Pulse (0.0) -> Tri/Sqr (0.5) -> -Saw/+Pulse (1.0)
        float out;
        if (morph < 0.5f)
        {
            // Blend from +Saw/-Pulse to Tri/Sqr
            float t = morph * 2.0f; // 0..1
            float a = sawUp;        // +Saw
            float b = pulse;        // -Pulse (inverted)
            float c = tri;          // Triangle
            float d = pulse;        // Square (same as pulse)
            // Crossfade: left side uses +Saw blended to Tri, right side uses -Pulse blended to Sqr
            out = (1.0f - t) * a + t * c;
        }
        else
        {
            // Blend from Tri/Sqr to -Saw/+Pulse
            float t = (morph - 0.5f) * 2.0f; // 0..1
            float a = tri;          // Triangle
            float b = pulse;        // Square
            float c = sawDown;      // -Saw
            float d = pulse;        // +Pulse (same as square but we keep it)
            out = (1.0f - t) * a + t * c;
        }

        return out;
    }

    void Lfo::process (int numSamples, float phaseOffset)
    {
        const float rate = params.rate;
        const double twoPi = juce::MathConstants<double>::twoPi;
        
        for (int i = 0; i < numSamples; ++i)
        {
            phase += (float) (rate / params.sampleRate) * (float) twoPi;
            if (phase > (float) twoPi)
                phase -= (float) twoPi;

            float out = 0.0f;
            switch (params.waveform)
            {
                case LfoWaveform::triangle:
                    out = renderMorph (phase);
                    break;
                case LfoWaveform::sine:
                    out = std::sin (phase);
                    break;
                case LfoWaveform::sawUp:
                    out = 2.0f * (phase / (float) twoPi) - 1.0f;
                    break;
                case LfoWaveform::sawDown:
                    out = 1.0f - 2.0f * (phase / (float) twoPi);
                    break;
                case LfoWaveform::square:
                    out = (phase < juce::MathConstants<float>::pi) ? 1.0f : -1.0f;
                    break;
                case LfoWaveform::random:
                    if (i == 0 || (phase < 0.01f))
                        out = nextRandom();
                    else
                        out = nextRandomValue;
                    break;
                case LfoWaveform::sampleHold:
                    if (i == 0 || (phase < 0.05f))
                        out = nextRandom();
                    else
                        out = nextRandomValue;
                    break;
                case LfoWaveform::noise:
                    out = nextNoise();
                    break;
                case LfoWaveform::chaos:
                    out = nextChaos();
                    break;
                default:
                    out = 0.0f;
                    break;
            }

            // apply fade in
            if (params.fade > 0.0f && i < numSamples * params.fade)
                out *= (float) i / (numSamples * params.fade);

            currentOutput = out * params.depth;
        }
    }
}
