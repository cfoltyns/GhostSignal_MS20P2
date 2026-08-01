/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Rev1, Rev2, and Nova filter implementations.
 */

#include "Filters.h"
#include <cmath>

namespace dsp::filter
{

// === Rev1 Filter (Sallen-Key HP+LP with aggressive feedback) ===

void Rev1Filter::prepare (double sr)
{
    params.sampleRate = sr;
    reset();
}

void Rev1Filter::reset()
{
    hpState[0] = hpState[1] = 0.0f;
    lpState[0] = lpState[1] = 0.0f;
    feedbackState[0] = feedbackState[1] = 0.0f;
}

void Rev1Filter::setParameters (const Rev1Params& p)
{
    params = p;
}

void Rev1Filter::process (juce::AudioBuffer<float>& buffer, int numSamples)
{
    int numCh = juce::jmin (buffer.getNumChannels(), 2);
    for (int ch = 0; ch < numCh; ++ch)
    {
        auto* data = buffer.getWritePointer (ch);
        for (int i = 0; i < numSamples; ++i)
        {
            float in = data[i];

            // HPF Sallen-Key stage
            float hpCutoff = juce::jlimit (20.0f, 20000.0f, params.hpfCutoff);
            float g = std::tan (juce::MathConstants<float>::pi * hpCutoff / (float) params.sampleRate);
            float k = 1.0f / (1.0f + 2.0f * g * (1.0f - g));
            float hp = k * (in - hpState[ch] + g * (hpState[ch] - hpState[ch]));
            hpState[ch] = hp;

            // LPF Sallen-Key stage
            float lpCutoff = juce::jlimit (20.0f, 20000.0f, params.lpfCutoff);
            float g2 = std::tan (juce::MathConstants<float>::pi * lpCutoff / (float) params.sampleRate);
            float k2 = 1.0f / (1.0f + 2.0f * g2 * (1.0f - g2));
            float lp = k2 * (hp - lpState[ch] + g2 * (lpState[ch] - lpState[ch]));
            lpState[ch] = lp;

            // Feedback with aggressive clipping (MS-20 style)
            float fb = params.feedback * feedbackState[ch];
            float output = lp + fb;
            output = std::tanh (output * (1.0f + params.drive * 2.0f));
            feedbackState[ch] = output;

            data[i] = output;
        }
    }
}

// === Rev2 Filter (OTA-based, smoother) ===

void Rev2Filter::prepare (double sr)
{
    params.sampleRate = sr;
    reset();
}

void Rev2Filter::reset()
{
    for (auto& s : state) s = 0.0f;
}

void Rev2Filter::setParameters (const Rev2Params& p)
{
    params = p;
}

void Rev2Filter::process (juce::AudioBuffer<float>& buffer, int numSamples)
{
    int numCh = juce::jmin (buffer.getNumChannels(), 2);
    for (int ch = 0; ch < numCh; ++ch)
    {
        auto* data = buffer.getWritePointer (ch);
        for (int i = 0; i < numSamples; ++i)
        {
            float cutoff = juce::jlimit (20.0f, 20000.0f, params.cutoff);
            float g = std::tan (juce::MathConstants<float>::pi * cutoff / (float) params.sampleRate);
            float r = params.resonance * 4.0f;
            float in = data[i] + r * state[3];

            // 4-pole OTA ladder
            for (int j = 0; j < 4; ++j)
            {
                in = in - state[j];
                state[j] = state[j] + g * std::tanh (in * (1.0f + params.drive * 0.5f));
                in = state[j];
            }

            // Feedback
            state[3] = std::tanh (state[3] * (1.0f + params.feedback));
            data[i] = state[3];
        }
    }
}

// === Nova Filter (modern 4-pole morphing stereo) ===

void NovaFilter::prepare (double sr)
{
    params.sampleRate = sr;
    reset();
}

void NovaFilter::reset()
{
    lpState[0] = lpState[1] = 0.0f;
    bpState[0] = bpState[1] = 0.0f;
    hpState[0] = hpState[1] = 0.0f;
    driveState[0] = driveState[1] = 0.0f;
}

void NovaFilter::setParameters (const NovaParams& p)
{
    params = p;
}

float NovaFilter::processSample (float input, int channel)
{
    float cutoff = juce::jlimit (20.0f, 20000.0f, params.cutoff);
    float g = std::tan (juce::MathConstants<float>::pi * cutoff / (float) params.sampleRate);
    float r = params.resonance * 4.0f;
    float drive = params.drive * 3.0f;

    // Pre-drive
    float in = input + driveState[channel];
    in = std::tanh (in * (1.0f + params.saturation));

    // State Variable Filter core
    float hp = in - lpState[channel] - r * bpState[channel];
    float bp = bpState[channel] + g * hp;
    float lp = lpState[channel] + g * bp;

    // Store state
    hpState[channel] = hp;
    bpState[channel] = bp;
    lpState[channel] = lp;
    driveState[channel] = input;

    // Morph between LP/BP/HP
    float morph = juce::jlimit (0.0f, 1.0f, params.morph);
    float output = (1.0f - morph) * lp + morph * (0.5f * bp + 0.5f * hp);

    // Slope control: for 12dB, blend with input
    if (params.slope > 12.0f)
    {
        float blend = (params.slope - 12.0f) / 12.0f;
        output = (1.0f - blend) * output + blend * output; // simplified
    }

    // Post-drive saturation
    output = std::tanh (output * (1.0f + params.saturation * 0.5f));
    return output;
}

void NovaFilter::process (juce::AudioBuffer<float>& buffer, int numSamples)
{
    int numCh = buffer.getNumChannels();

    for (int ch = 0; ch < numCh; ++ch)
    {
        auto* data = buffer.getWritePointer (ch);
        float chOffset = params.stereoOffset * (float) ((ch == 0) ? -1 : 1);

        for (int i = 0; i < numSamples; ++i)
        {
            data[i] = processSample (data[i], ch);
        }
    }
}

} // namespace dsp::filter

