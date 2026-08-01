/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#pragma once

#include <JuceHeader.h>
#include <array>

// Modulation source IDs
enum class ModSource
{
    none,
    lfo1,
    lfo2,
    lfo3,
    lfo4,
    env1,
    env2,
    env3,
    env4,
    velocity,
    modWheel,
    pitchBend,
    aftertouch,
    keyTracking,
    random,
    mpeX,
    mpeY,
    mpeZ
};

// Modulation destination IDs
enum class ModDestination
{
    osc1Freq,
    osc1PWM,
    osc1Gain,
    osc2Freq,
    osc2PWM,
    osc2Gain,
    cutoff,
    resonance,
    drive,
    pan,
    ampGain,
    lfo1Rate,
    lfo2Rate,
    lfo3Rate,
    lfo4Rate
};

struct ModSlot
{
    ModSource source { ModSource::none };
    ModDestination destination { ModDestination::osc1Freq };
    float amount { 0.0f };
    bool bipolar { true };
    bool enabled { true };
};

class ModulationMatrix
{
public:
    ModulationMatrix() = default;
    ~ModulationMatrix() = default;

    void prepare (double sampleRate);
    void reset();
    void setParameters (const std::array<ModSlot, 16>& slots);

    // Get modulation value for a specific destination at given sample time
    float getModulationValue (ModDestination dest, const std::array<float, 16>& sourceValues);

    // Process all modulations for a voice (real-time)
    void process (juce::AudioBuffer<float>& buffer, int numSamples);

    // Get number of slots
    static constexpr int getNumSlots() { return 16; }

private:
    std::array<ModSlot, 16> slots;
    std::array<float, 16> cachedValues; // Cache last modulation values
};