/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Manages active voices, voice allocation rules, and macro controls.
 */

#pragma once

#include <JuceHeader.h>
#include <array>
#include "../Core/VoiceManager.h"
#include "../Core/ModulationMatrix.h"
#include "Voice.h"
#include "AnalogDrift.h"

namespace dsp
{

struct EngineParams
{
    int maxVoices { 32 };
    VoiceMode voiceMode { VoiceMode::polyphonic };
    float masterTune { 440.0f };
    float pitchBendRange { 2.0f };
    float glideTime { 0.0f };
    float unisonDetune { 0.1f };
    int unisonVoices { 3 };
    float unisonSpread { 0.3f };
    float driftAmount { 0.0f };
    float voiceSpread { 0.0f };
};

class SynthEngine
{
public:
    SynthEngine() = default;
    ~SynthEngine() = default;

    void prepare (double sampleRate, int maxBlockSize, int numChannels);
    void reset();
    void setParameters (const EngineParams& p);

    // Voice management
    int noteOn (int midiNote, float velocity, int channel = 1);
    void noteOff (int midiNote);
    void allNotesOff();
    void pitchBend (int channel, int value);
    void aftertouch (int channel, int value);
    void timbre (int channel, int value);

    // Process
    void process (juce::AudioBuffer<float>& buffer,
                  juce::MidiBuffer& midi,
                  const VoiceParams& voiceParams,
                  ModulationMatrix& modMatrix);

    // Access
    VoiceManager& getVoiceManager() { return voiceManager; }
    int getNumActiveVoices() const { return voiceManager.getNumActiveVoices(); }

    // Collect latest LFO output values from active voices (for UI visualization)
    void collectLfoValues (float* lfoValues, int numLfos) const;

private:
    void applyDriftToVoice (VoiceDSP& voice, int voiceIndex);

    VoiceManager voiceManager;
    EngineParams params;

    double sampleRate { 44100.0 };
    int maxBlockSize { 512 };
    int numChannels { 2 };

    // Per-voice drift engines
    std::array<AnalogDrift, 64> driftEngines;
};

} // namespace dsp

