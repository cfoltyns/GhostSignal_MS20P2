/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Voice-specific analog component drift/tolerance modeling.
 * Generates per-voice parameter offsets to emulate component tolerances.
 */

#pragma once

#include <JuceHeader.h>
#include <random>
#include <array>

namespace dsp
{

struct DriftParams
{
    double sampleRate { 44100.0 };
    float  driftAmount { 0.0f };     // 0..1 global drift depth
    float  pitchDrift { 0.0f };       // cents of pitch wander
    float  filterDrift { 0.0f };      // cutoff offset in semitones
    float  resDrift { 0.0f };         // resonance tolerance offset
    float  envTimeDrift { 0.0f };     // envelope time scaling variation
    float  satDrift { 0.0f };         // saturation curve offset
    int    seed { 0 };                // per-voice seed
};

class AnalogDrift
{
public:
    AnalogDrift();
    ~AnalogDrift() = default;

    AnalogDrift (const AnalogDrift&) = delete;
    AnalogDrift& operator= (const AnalogDrift&) = delete;

    void prepare (const DriftParams& p);
    void reset();

    // Get static offsets (set once per note-on)
    float getPitchOffset() const noexcept       { return pitchOffset; }
    float getFilterCutoffOffset() const noexcept { return filterCutoffOffset; }
    float getResonanceOffset() const noexcept    { return resOffset; }
    float getEnvAttackScale() const noexcept     { return envAttackScale; }
    float getEnvDecayScale() const noexcept      { return envDecayScale; }
    float getEnvReleaseScale() const noexcept    { return envReleaseScale; }
    float getSatOffset() const noexcept          { return satOffset; }

    // Update thermal drift (call per block)
    void updateThermalDrift (int numSamples);

    // Get drifting pitch value (slowly wandering)
    float getThermalPitchCents() const noexcept { return thermalPitchCents; }

private:
    DriftParams params;
    float pitchOffset { 0.0f };
    float filterCutoffOffset { 0.0f };
    float resOffset { 0.0f };
    float envAttackScale { 1.0f };
    float envDecayScale { 1.0f };
    float envReleaseScale { 1.0f };
    float satOffset { 0.0f };

    // Thermal drift state
    float thermalPitchPhase { 0.0f };
    float thermalPitchCents { 0.0f };
    float thermalFilterPhase { 0.0f };
    float thermalFilterOffset { 0.0f };

    std::mt19937 rng;
    std::normal_distribution<float> normalDist { 0.0f, 1.0f };
    std::uniform_real_distribution<float> uniformDist { 0.0f, 1.0f };
};

} // namespace dsp

