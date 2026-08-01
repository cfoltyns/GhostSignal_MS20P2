/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Vintage tape echo emulation — full DSP implementation.
 *              Fractional delay line with linear interpolation, dual-rate
 *              wow/flutter modulation, soft magnetic saturation, feedback
 *              low-pass filtering, and tempo-sync support.
 *
 * All processing is real-time safe: no memory allocation inside process(),
 * no locks, no dynamic allocations during audio processing.
 */

#include "TapeDelay.h"
#include <cmath>

namespace fx
{

// ──────────────────────────────────────────────────────────────────────────────
// FractionalDelayLine
// ──────────────────────────────────────────────────────────────────────────────

void FractionalDelayLine::prepare (double sr, float maxDelayMs)
{
    sampleRate = sr;
    bufferLength = static_cast<int> (sr * maxDelayMs / 1000.0) + 4;
    if (bufferLength < 4)
        bufferLength = 4;

    buffer.setSize (1, bufferLength, false, false, true);
    buffer.clear();
    writeIndex = 0;
}

void FractionalDelayLine::clear()
{
    if (buffer.getNumSamples() > 0)
        buffer.clear();
    writeIndex = 0;
}

float FractionalDelayLine::read (float delayInSamples) const
{
    if (bufferLength <= 0)
        return 0.0f;

    // Clamp delay to valid range
    if (delayInSamples < 0.0f)
        delayInSamples = 0.0f;
    if (delayInSamples > (float) (bufferLength - 1))
        delayInSamples = (float) (bufferLength - 1);

    // Fractional read position relative to write index
    double readPos = (double) writeIndex - delayInSamples;

    // Wrap into buffer range
    int intRead = (int) std::floor (readPos);
    float frac = (float) (readPos - intRead);

    int base = intRead % bufferLength;
    if (base < 0)
        base += bufferLength;

    int next = (base + 1) % bufferLength;

    float s1 = buffer.getSample (0, base);
    float s2 = buffer.getSample (0, next);

    // Linear interpolation
    return s1 + (s2 - s1) * frac;
}

void FractionalDelayLine::write (float sample)
{
    if (bufferLength <= 0)
        return;

    buffer.setSample (0, writeIndex, sample);
    writeIndex = (writeIndex + 1) % bufferLength;
}

// ──────────────────────────────────────────────────────────────────────────────
// WowFlutter
// ──────────────────────────────────────────────────────────────────────────────

void WowFlutter::prepare (double sr)
{
    sampleRate = sr;
    reset();
}

void WowFlutter::reset()
{
    wowPhase     = 0.0;
    flutterPhase = 0.0;
}

float WowFlutter::getModulation (float wowDepth, float flutterDepth)
{
    if (sampleRate <= 0.0)
        return 0.0f;

    // Wow: slow modulation (0.2–2 Hz).  Depth scales the rate slightly
    // and the amplitude of the modulation.
    double wowRate = 0.2 + wowDepth * 1.8; // 0.2 → 2.0 Hz
    double wowMod  = std::sin (wowPhase) * wowDepth * 0.008; // ±0.8 % max

    // Flutter: faster modulation (5–10 Hz)
    double flutterRate = 5.0 + flutterDepth * 5.0; // 5 → 10 Hz
    double flutterMod  = std::sin (flutterPhase) * flutterDepth * 0.004; // ±0.4 % max

    // Advance phases
    wowPhase     += wowRate * juce::MathConstants<double>::twoPi / sampleRate;
    flutterPhase += flutterRate * juce::MathConstants<double>::twoPi / sampleRate;

    // Wrap phases (cheap modulo via subtraction)
    if (wowPhase > juce::MathConstants<double>::twoPi)
        wowPhase -= juce::MathConstants<double>::twoPi;
    if (flutterPhase > juce::MathConstants<double>::twoPi)
        flutterPhase -= juce::MathConstants<double>::twoPi;

    return (float) (wowMod + flutterMod);
}

// ──────────────────────────────────────────────────────────────────────────────
// TapeSaturation
// ──────────────────────────────────────────────────────────────────────────────

float TapeSaturation::process (float sample, float drive)
{
    if (drive <= 0.001f)
        return sample;

    // Gentle soft-clipping: x / sqrt(1 + x²) is a smooth, non-hard-clipping curve
    // that adds even-order harmonics and compresses peaks.
    float k   = 1.0f + drive * 2.5f;   // input gain
    float x   = sample * k;
    float y   = x / std::sqrt (1.0f + x * x);

    // Makeup gain to compensate for compression
    float makeup = 1.0f + drive * 0.4f;
    return y * makeup;
}

// ──────────────────────────────────────────────────────────────────────────────
// TapeFilter (one-pole LPF for feedback loop)
// ──────────────────────────────────────────────────────────────────────────────

void TapeFilter::prepare (double sr)
{
    sampleRate = sr;
    reset();
}

void TapeFilter::reset()
{
    z1 = 0.0f;
}

float TapeFilter::process (float input, float cutoffNorm)
{
    // cutoffNorm: 0 (bright / 20 kHz) → 1 (dark / 200 Hz)
    // Map exponentially for musical response
    float cutoffHz = 20000.0f * std::pow (0.01f, cutoffNorm); // 20k → 200 Hz

    float w0 = 2.0f * juce::MathConstants<float>::pi * cutoffHz / (float) sampleRate;
    float a0 = w0 / (1.0f + w0);
    float b1 = 1.0f - a0;

    z1 = input * a0 + z1 * b1;
    return z1;
}

// ──────────────────────────────────────────────────────────────────────────────
// TapeDelay — main class
// ──────────────────────────────────────────────────────────────────────────────

void TapeDelay::prepare (double sr, int /*maxBlockSize*/, int channels)
{
    sampleRate = sr;
    numChannels = juce::jmax (1, channels);

    // Allocate 6 seconds of delay buffer (covers 1/2 note at 20 BPM)
    constexpr float maxDelayMs = 6000.0f;

    for (int ch = 0; ch < 2; ++ch)
    {
        delayLines[ch].prepare (sr, maxDelayMs);
        feedbackFilter[ch].prepare (sr);
    }

    wowFlutter.prepare (sr);

    // Set smoothing rates — 20 ms for most params, 30 ms for delay time
    // to avoid clicks on tempo changes
    const double smoothingSec = 0.02;
    const double delaySmoothingSec = 0.03;

    timeMsSmoothed.reset (sr, delaySmoothingSec);
    feedbackSmoothed.reset (sr, smoothingSec);
    mixSmoothed.reset (sr, smoothingSec);
    tapeAgeSmoothed.reset (sr, smoothingSec);
    saturationSmoothed.reset (sr, smoothingSec);
    wowSmoothed.reset (sr, smoothingSec);
    flutterSmoothed.reset (sr, smoothingSec);

    reset();
}

void TapeDelay::reset()
{
    for (int ch = 0; ch < 2; ++ch)
    {
        delayLines[ch].clear();
        feedbackFilter[ch].reset();
    }
    wowFlutter.reset();

    // Re-sync smoothed values to current raw values
    timeMsSmoothed.setCurrentAndTargetValue (timeMs);
    feedbackSmoothed.setCurrentAndTargetValue (feedback);
    mixSmoothed.setCurrentAndTargetValue (mix);
    tapeAgeSmoothed.setCurrentAndTargetValue (tapeAge);
    saturationSmoothed.setCurrentAndTargetValue (saturation);
    wowSmoothed.setCurrentAndTargetValue (wow);
    flutterSmoothed.setCurrentAndTargetValue (flutter);

    wasPlaying = false;
}

float TapeDelay::calculateTargetDelayMs() const
{
    // Sync modes: 0=1/2, 1=1/4, 2=1/8, 3=1/16, 4=1/32, 5=Slap, 6=MS
    const float quarterNoteMs = 60000.0f / bpmVal;

    switch (syncMode)
    {
        case 0: return quarterNoteMs * 2.0f;  // 1/2 note
        case 1: return quarterNoteMs;          // 1/4 note
        case 2: return quarterNoteMs * 0.5f;   // 1/8 note
        case 3: return quarterNoteMs * 0.25f;  // 1/16 note
        case 4: return quarterNoteMs * 0.125f; // 1/32 note
        case 5: return slapTimeMs;             // Slap (50-120ms fixed)
        case 6: return timeMs;                 // MS mode (use time knob)
        default: return timeMs;
    }
}

void TapeDelay::updateBPMFromPlayHead()
{
    if (playHead == nullptr)
        return;

    // Use the modern getPosition() API (returns Optional<PositionInfo>)
    if (auto info = playHead->getPosition())
    {
        if (auto bpm = info->getBpm())
        {
            bpmVal = juce::jlimit (20.0f, 300.0f, (float) *bpm);
        }
        wasPlaying = info->getIsPlaying();
    }
}

void TapeDelay::process (juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (! isEnabled || numSamples <= 0)
        return;

    const int numCh = juce::jmin (buffer.getNumChannels(), numChannels);
    if (numCh <= 0)
        return;

    // ── Update tempo from play head ────────────────────────────
    updateBPMFromPlayHead();

    // ── Update smoothed delay time target ──────────────────────
    float targetDelayMs = calculateTargetDelayMs();
    timeMsSmoothed.setTargetValue (targetDelayMs);

    // ── Update all other smoothed parameter targets ────────────
    feedbackSmoothed.setTargetValue (feedback);
    mixSmoothed.setTargetValue (mix);
    tapeAgeSmoothed.setTargetValue (tapeAge);
    saturationSmoothed.setTargetValue (saturation);
    wowSmoothed.setTargetValue (wow);
    flutterSmoothed.setTargetValue (flutter);

    // ── Main per-sample processing loop ────────────────────────
    for (int ch = 0; ch < numCh; ++ch)
    {
        auto* data = buffer.getWritePointer (ch);

        for (int i = 0; i < numSamples; ++i)
        {
            const float input = data[i];

            // Get smoothed delay time for this sample
            float delayMs   = timeMsSmoothed.getNextValue();
            float delaySmp  = delayMs * 0.001f * (float) sampleRate;

            // Apply wow/flutter modulation (stereo offset for width)
            float wowDepth     = wowSmoothed.getNextValue();
            float flutterDepth = flutterSmoothed.getNextValue();
            float mod = wowFlutter.getModulation (wowDepth, flutterDepth);
            // Apply a small per-channel phase offset for stereo width
            float modAmt = mod * (1.0f + (ch == 1 ? 0.3f : 0.0f));

            float readDelay = delaySmp * (1.0f + modAmt);

            // Read delayed sample
            float delayed = delayLines[ch].read (readDelay);

            // Apply feedback filter (tape age — progressive HF loss)
            float ageVal = tapeAgeSmoothed.getNextValue();
            float filteredFeedback = feedbackFilter[ch].process (delayed, ageVal);

            // Apply saturation in the feedback loop (magnetic compression)
            float satVal = saturationSmoothed.getNextValue();
            float fbGain = feedbackSmoothed.getNextValue();
            float fbSignal = TapeSaturation::process (filteredFeedback * fbGain, satVal);

            // Write input + feedback into the delay line
            float toWrite = input + fbSignal;
            delayLines[ch].write (toWrite);

            // Output saturation (lighter than feedback saturation)
            float wet = TapeSaturation::process (delayed, satVal * 0.5f);

            // Dry/wet mix
            float mixVal = mixSmoothed.getNextValue();
            data[i] = input * (1.0f - mixVal) + wet * mixVal;
        }
    }
}

} // namespace fx
