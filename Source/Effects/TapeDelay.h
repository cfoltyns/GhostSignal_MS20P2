/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Vintage tape echo emulation with wow, flutter, saturation,
 *              and feedback filtering for authentic analog character.
 *
 * Modular design:
 *   - FractionalDelayLine: circular buffer with linear interpolation
 *   - WowFlutter:         slow + fast pitch modulation (wow 0.2-2Hz, flutter 5-10Hz)
 *   - TapeSaturation:     soft magnetic-tape compression / harmonic enhancement
 *   - TapeFilter:         one-pole low-pass in the feedback loop (tape age)
 *   - Parameter management via juce::SmoothedValue for zipper-free updates
 */

#pragma once

#include <JuceHeader.h>

namespace fx
{

// ──────────────────────────────────────────────────────────────────────────────
// FractionalDelayLine — circular buffer with linear interpolation
// ──────────────────────────────────────────────────────────────────────────────

class FractionalDelayLine
{
public:
    void prepare (double sr, float maxDelayMs);
    void clear();

    /// Read a sample at the given fractional delay (in samples).
    float read (float delayInSamples) const;
    /// Write a sample at the current write position and advance.
    void write (float sample);

    int getBufferLength() const noexcept { return bufferLength; }

private:
    juce::AudioBuffer<float> buffer;
    int writeIndex { 0 };
    int bufferLength { 0 };
    double sampleRate { 44100.0 };
};

// ──────────────────────────────────────────────────────────────────────────────
// WowFlutter — dual-rate sinusoidal pitch modulation
// ──────────────────────────────────────────────────────────────────────────────

class WowFlutter
{
public:
    void prepare (double sr);
    void reset();

    /// Returns a normalised modulation factor (typically ±0.005 or less)
    /// that should be multiplied with the current delay time in samples.
    float getModulation (float wowDepth, float flutterDepth);

private:
    double sampleRate { 44100.0 };
    double wowPhase     { 0.0 };
    double flutterPhase { 0.0 };

    static constexpr double wowFreq     = 0.7;   // Hz — centre of 0.2–2 range
    static constexpr double flutterFreq = 7.0;   // Hz — centre of 5–10 range
};

// ──────────────────────────────────────────────────────────────────────────────
// TapeSaturation — gentle soft-clipping / magnetic compression
// ──────────────────────────────────────────────────────────────────────────────

class TapeSaturation
{
public:
    /// Soft saturation using a smooth arctan-like curve.
    /// drive: 0 (clean) → 1 (warm saturation).  Never hard-clips.
    static float process (float sample, float drive);
};

// ──────────────────────────────────────────────────────────────────────────────
// TapeFilter — one-pole low-pass in the feedback loop (tape age)
// ──────────────────────────────────────────────────────────────────────────────

class TapeFilter
{
public:
    void prepare (double sr);
    void reset();

    /// cutoffNorm: 0 (bright) → 1 (dark).
    float process (float input, float cutoffNorm);

private:
    double sampleRate { 44100.0 };
    float z1 { 0.0f };
};

// ──────────────────────────────────────────────────────────────────────────────
// TapeDelay — main effect class
// ──────────────────────────────────────────────────────────────────────────────

class TapeDelay
{
public:
    TapeDelay() = default;
    ~TapeDelay() = default;

    void prepare (double sampleRate, int maxBlockSize, int numChannels);
    void reset();

    // ── Parameter setters (thread-safe: set target, smoothing happens in process) ──

    void setTimeMs     (float ms)    { timeMs = juce::jlimit (30.0f, 1000.0f, ms); }
    void setSyncMode   (int mode)    { syncMode = juce::jlimit (0, 6, mode); }
    void setSlapTimeMs (float ms)    { slapTimeMs = juce::jlimit (50.0f, 120.0f, ms); }
    void setBPM        (float bpm)   { bpmVal = juce::jlimit (20.0f, 300.0f, bpm); }
    void setFeedback   (float f)     { feedback = juce::jlimit (0.0f, 0.95f, f); }
    void setMix        (float m)     { mix = juce::jlimit (0.0f, 1.0f, m); }
    void setTapeAge    (float a)     { tapeAge = juce::jlimit (0.0f, 1.0f, a); }
    void setSaturation (float s)     { saturation = juce::jlimit (0.0f, 1.0f, s); }
    void setWow        (float w)     { wow = juce::jlimit (0.0f, 1.0f, w); }
    void setFlutter    (float f)     { flutter = juce::jlimit (0.0f, 1.0f, f); }
    void setEnabled    (bool e)      { isEnabled = e; }
    void setPlayHead   (juce::AudioPlayHead* ph) { playHead = ph; }

    void process (juce::AudioBuffer<float>& buffer, int numSamples);

private:
    static inline float flushDenormal (float v) noexcept
    {
        return (std::abs (v) < 1.0e-38f) ? 0.0f : v;
    }

    // ── Helpers ──────────────────────────────────────────────
    float calculateTargetDelayMs() const;
    void  updateBPMFromPlayHead();

    // ── Modular DSP components ───────────────────────────────
    FractionalDelayLine delayLines[2];
    WowFlutter          wowFlutter;
    TapeFilter          feedbackFilter[2];

    // ── Play head for tempo sync ─────────────────────────────
    juce::AudioPlayHead* playHead { nullptr };

    // ── Audio context ────────────────────────────────────────
    double sampleRate     { 44100.0 };
    int    numChannels    { 2 };
    int    maxBlockSize   { 512 };

    // ── Raw parameter values (set by host) ───────────────────
    float timeMs          { 300.0f };
    int   syncMode        { 0 };   // 0=1/2, 1=1/4, 2=1/8, 3=1/16, 4=1/32, 5=Slap, 6=MS
    float slapTimeMs      { 80.0f };
    float bpmVal          { 120.0f };
    float feedback        { 0.5f };
    float mix             { 0.5f };
    float tapeAge         { 0.5f };
    float saturation      { 0.3f };
    float wow             { 0.0f };
    float flutter         { 0.0f };
    bool  isEnabled       { false };

    // ── Smoothed parameters (zipper-free) ───────────────────
    juce::SmoothedValue<float> timeMsSmoothed     { 300.0f };
    juce::SmoothedValue<float> feedbackSmoothed   { 0.5f };
    juce::SmoothedValue<float> mixSmoothed        { 0.5f };
    juce::SmoothedValue<float> tapeAgeSmoothed    { 0.5f };
    juce::SmoothedValue<float> saturationSmoothed { 0.3f };
    juce::SmoothedValue<float> wowSmoothed        { 0.0f };
    juce::SmoothedValue<float> flutterSmoothed    { 0.0f };

    // ── Transport state ───────────────────────────────────────
    bool wasPlaying { false };
};

} // namespace fx
