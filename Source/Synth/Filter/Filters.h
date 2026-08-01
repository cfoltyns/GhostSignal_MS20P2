/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Aggregate filter header for Rev1, Rev2, and Nova filter models.
 */

#pragma once

#include "FilterTypes.h"
#include "HighPass.h"
#include "LowPass.h"
#include "Saturation.h"

namespace dsp::filter
{

// Rev 1: Sallen-Key HP+LP with aggressive feedback clipping (classic MS-20 dirty)
struct Rev1Params
{
    double sampleRate { 44100.0 };
    float hpfCutoff { 100.0f };
    float hpfResonance { 0.0f };
    float lpfCutoff { 1000.0f };
    float lpfResonance { 0.0f };
    float drive { 0.0f };
    float feedback { 0.0f }; // self-oscillation feedback
};

// Rev 2: OTA-based filter with smoother OTA-limited feedback
struct Rev2Params
{
    double sampleRate { 44100.0 };
    float cutoff { 1000.0f };
    float resonance { 0.0f };
    float drive { 0.0f };
    float feedback { 0.0f };
};

// Nova: Modern 4-pole morphing filter with stereo panning
struct NovaParams
{
    double sampleRate { 44100.0 };
    float cutoff { 1000.0f };
    float resonance { 0.0f };
    float drive { 0.0f };
    float morph { 0.0f };       // 0=LP, 0.5=BP, 1=HP
    float slope { 12.0f };      // 12 or 24 dB/oct
    float stereoOffset { 0.0f }; // stereo panning offset
    float saturation { 0.0f };  // multi-stage saturation drive
};

class Rev1Filter
{
public:
    Rev1Filter() = default;
    ~Rev1Filter() = default;

    void prepare (double sampleRate);
    void reset();
    void setParameters (const Rev1Params& p);
    void process (juce::AudioBuffer<float>& buffer, int numSamples);

private:
    Rev1Params params;
    float hpState[2] { 0.0f, 0.0f };
    float lpState[2] { 0.0f, 0.0f };
    float feedbackState[2] { 0.0f, 0.0f };
};

class Rev2Filter
{
public:
    Rev2Filter() = default;
    ~Rev2Filter() = default;

    void prepare (double sampleRate);
    void reset();
    void setParameters (const Rev2Params& p);
    void process (juce::AudioBuffer<float>& buffer, int numSamples);

private:
    Rev2Params params;
    float state[4] { 0.0f, 0.0f, 0.0f, 0.0f };
};

class NovaFilter
{
public:
    NovaFilter() = default;
    ~NovaFilter() = default;

    void prepare (double sampleRate);
    void reset();
    void setParameters (const NovaParams& p);
    void process (juce::AudioBuffer<float>& buffer, int numSamples);

private:
    float processSample (float input, int channel);

    NovaParams params;
    float lpState[2] { 0.0f, 0.0f };
    float bpState[2] { 0.0f, 0.0f };
    float hpState[2] { 0.0f, 0.0f };
    float driveState[2] { 0.0f, 0.0f };
};

} // namespace dsp::filter

