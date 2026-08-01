/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#pragma once

#include <JuceHeader.h>
#include <bitset>
#include <array>
#include "../Synth/Voice.h"

enum class VoiceMode
{
    monophonic,
    polyphonic,
    unison,
    legato
};

enum class MonoPriority
{
    last,
    low,
    high
};

struct VoiceManagerParams
{
    VoiceMode mode { VoiceMode::polyphonic };
    MonoPriority monoPriority { MonoPriority::last };
    int maxVoices { 16 };
    float portamento { 0.0f };
    float unisonDetune { 0.1f };
    int unisonVoices { 3 };
    float unisonSpread { 0.3f };
};

class VoiceManager
{
public:
    VoiceManager()
    {
        // Initialize tracking arrays to safe defaults
        std::fill (noteToVoice, noteToVoice + 128, -1);
        std::fill (voiceNote, voiceNote + maxPolyphony, -1);
        std::fill (voiceChannel, voiceChannel + maxPolyphony, -1);
    }
    ~VoiceManager() = default;

    void prepare (double sampleRate, int maxBlockSize, const VoiceManagerParams& p);
    void reset();
    void setParameters (const VoiceManagerParams& p);

    // Handle voice allocation
    int allocateVoice (int noteNumber, float velocity, int channel = 1);
    void releaseVoice (int voiceIndex);
    void releaseVoiceForNote (int noteNumber);
    void releaseAllVoices();

    // MPE routing
    void pitchBendForChannel (int channel, int pitchBendValue);
    void pressureForChannel (int channel, int pressureValue);
    void timbreForChannel (int channel, int timbreValue);

    // Get voice state for processing
    Voice* getVoice (int index) { return voices[index].get(); }
    int getNumActiveVoices() const { return activeVoiceCount; }

    // Get maximum number of voice slots
    static constexpr int getMaxVoiceSlots() { return maxPolyphony; }

    float getPortamentoValue() const { return portamentoValue; }

private:
    int findFreeVoice();
    int findVoiceForNote (int noteNumber);
    int getOldestVoice();
    int getHighestVoice();
    int getLowestVoice();

    VoiceManagerParams params;

    static constexpr int maxPolyphony = 64;
    std::array<std::unique_ptr<Voice>, maxPolyphony> voices;
    std::bitset<maxPolyphony> voiceActive;
    int activeVoiceCount { 0 };

    // Note tracking
    int noteToVoice[128];
    int voiceNote[maxPolyphony];
    int voiceChannel[maxPolyphony];

    // Mono mode state
    int lastNote { -1 };

    // Portamento
    double sampleRate { 44100.0 };
    double portamentoSamples { 0.0 };
    float portamentoValue { 0.0f };
    float portamentoTarget { 0.0f };
    float portamentoIncrement { 0.0f };
    int portamentoVoice { -1 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoiceManager)
};