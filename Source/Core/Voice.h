/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#pragma once

#include <JuceHeader.h>

namespace dsp { struct VoiceParams; }

class Voice
{
public:
    Voice() = default;
    virtual ~Voice() = default;

    virtual void prepare (double sampleRate, int maxBlockSize) = 0;
    virtual void reset() = 0;
    virtual void noteOn (int midiNoteNumber, float velocity) = 0;
    virtual void noteOff() = 0;
    virtual void pitchBend (int pitchBendValue) {}
    virtual void pressure (int pressureValue) {}
    virtual void timbre (int timbreValue) {}
    virtual void process (juce::AudioBuffer<float>& buffer, int startSample, int numSamples) = 0;
    virtual void setParameters (const dsp::VoiceParams&) {}

    bool isActive() const { return active; }
    int getNoteNumber() const { return noteNumber; }

protected:
    bool active { false };
    int noteNumber { -1 };
    float velocity { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Voice)
};
