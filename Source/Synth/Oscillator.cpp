/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#include "Oscillator.h"
#include <random>

namespace dsp
{
    void Oscillator::prepare (double newSampleRate)
    {
        params.sampleRate = newSampleRate;
        reset();
    }

    void Oscillator::reset()
    {
        phaseLeft = 0.0f;
        phaseRight = 0.0f;
        phaseInc = 0.0f;
        phaseIncRight = 0.0f;
        std::random_device rd;
        rng.seed (rd());
    }

    void Oscillator::setParameters (const OscillatorParams& p)
    {
        params = p;
        const float freq = params.frequency * std::pow (2.0f, (float) params.octave + params.semitone / 12.0f + params.fineTune / 100.0f);
        const double sr = (params.sampleRate > 0.0) ? params.sampleRate : 44100.0;
        const double norm = freq / sr;
        phaseInc = (float) (norm * juce::MathConstants<double>::twoPi);
        phaseIncRight = phaseInc; // mono for now
    }

    float Oscillator::renderSample (float phase, float fmInput)
    {
        using juce::MathConstants;
        const float p = phase;
        float sample = 0.0f;

        switch (params.waveform)
        {
            case Waveform::sine:
            case Waveform::triangle:
            {
                // naive triangle / sine-ish
                sample = 4.0f * std::abs (p / MathConstants<float>::twoPi - 0.5f) - 1.0f;
                if (params.waveform == Waveform::sine)
                    sample = std::sin (p);
                break;
            }
            case Waveform::saw:
            {
                sample = 2.0f * (p / MathConstants<float>::twoPi) - 1.0f;
                break;
            }
            case Waveform::square:
            case Waveform::pulse:
            {
                // Both Square and Pulse honours the PWM parameter so the pulse
                // width knob is meaningful for either waveform. With pwm == 0.5
                // this reproduces the classic 50% square wave.
                sample = (p < params.pwm * MathConstants<float>::twoPi) ? 1.0f : -1.0f;
                break;
            }
            case Waveform::superSaw:
            {
                float s = 0.0f;
                for (int i = 0; i < 7; ++i)
                {
                    float offset = (float) i * 0.02f;
                    float ph = p + offset;
                    s += 2.0f * (std::fmod (ph, MathConstants<float>::twoPi) / MathConstants<float>::twoPi) - 1.0f;
                }
                sample = s / 7.0f;
                break;
            }
            case Waveform::superSquare:
            {
                float s = 0.0f;
                for (int i = 0; i < 5; ++i)
                {
                    float offset = (float) i * 0.05f;
                    s += ((p + offset) < MathConstants<float>::pi) ? 1.0f : -1.0f;
                }
                sample = s / 5.0f;
                break;
            }
            case Waveform::noise:
            {
                sample = noiseSample();
                break;
            }
        }

        // FM
        if (params.fmReceive && fmInput != 0.0f)
            sample += fmInput * 0.1f;

        return juce::jlimit (-1.0f, 1.0f, sample * params.gain);
    }

    float Oscillator::noiseSample()
    {
        return dist (rng);
    }

    void Oscillator::process (juce::AudioBuffer<float>& buffer, int numSamples, float fmInput)
    {
        const int ch = buffer.getNumChannels();
        if (ch <= 0 || numSamples <= 0)
            return;
            
        auto* left = buffer.getWritePointer (0, 0);
        auto* right = (ch > 1 ? buffer.getWritePointer (1, 0) : nullptr);

        for (int i = 0; i < numSamples; ++i)
        {
            const float s = renderSample (phaseLeft, fmInput);
            left[i] = s;

            if (right != nullptr)
            {
                const float sr = renderSample (phaseRight, fmInput);
                // crude stereo from width: use two detuned phases
                right[i] = juce::jlimit (-1.0f, 1.0f, s * (1.0f - params.stereoWidth) + sr * params.stereoWidth);
            }

            phaseLeft += phaseInc;
            if (phaseLeft > juce::MathConstants<float>::twoPi)
                phaseLeft -= juce::MathConstants<float>::twoPi;

            phaseRight += phaseIncRight * (1.0f + params.drift);
            if (phaseRight > juce::MathConstants<float>::twoPi)
                phaseRight -= juce::MathConstants<float>::twoPi;
        }
    }
}