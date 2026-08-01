/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Per-voice analog drift modeling.
 */

#include "AnalogDrift.h"

namespace dsp
{

AnalogDrift::AnalogDrift()
    : rng (std::random_device{}())
{
}

void AnalogDrift::prepare (const DriftParams& p)
{
    params = p;
    reset();
}

void AnalogDrift::reset()
{
    // Seed RNG with voice-specific seed
    if (params.seed != 0)
        rng.seed (params.seed);
    else
        rng.seed (static_cast<unsigned int>(std::random_device{}()));

    // Generate static tolerance offsets based on drift amount
    const float da = params.driftAmount;

    pitchOffset = normalDist(rng) * params.pitchDrift * da * 0.5f;
    filterCutoffOffset = normalDist(rng) * params.filterDrift * da * 0.5f;
    resOffset = normalDist(rng) * params.resDrift * da * 0.1f;
    satOffset = normalDist(rng) * params.satDrift * da * 0.3f;

    // Envelope time scaling (log-normal style: mostly 1.0, rarely off)
    envAttackScale = 1.0f + normalDist(rng) * params.envTimeDrift * da * 0.2f;
    envAttackScale = juce::jlimit (0.5f, 2.0f, envAttackScale);

    envDecayScale = 1.0f + normalDist(rng) * params.envTimeDrift * da * 0.2f;
    envDecayScale = juce::jlimit (0.5f, 2.0f, envDecayScale);

    envReleaseScale = 1.0f + normalDist(rng) * params.envTimeDrift * da * 0.2f;
    envReleaseScale = juce::jlimit (0.5f, 2.0f, envReleaseScale);

    thermalPitchPhase = 0.0f;
    thermalPitchCents = 0.0f;
    thermalFilterPhase = 0.0f;
    thermalFilterOffset = 0.0f;
}

void AnalogDrift::updateThermalDrift (int numSamples)
{
    // Slow thermal oscillation (sub-sonic, ~0.05-0.2 Hz)
    const float dt = static_cast<float>(numSamples) / static_cast<float>(params.sampleRate);
    const float da = params.driftAmount;

    // Two very slow oscillators for pitch drift
    thermalPitchPhase += dt * 0.12f; // ~0.12 Hz
    if (thermalPitchPhase > 1.0f)
        thermalPitchPhase -= 1.0f;

    const float angle1 = thermalPitchPhase * juce::MathConstants<float>::twoPi;
    const float angle2 = thermalPitchPhase * 0.47f * juce::MathConstants<float>::twoPi;

    thermalPitchCents = (std::sin (angle1) + std::sin (angle2))
                        * params.pitchDrift * da * 0.3f;

    // Slow filter cutoff wander
    thermalFilterPhase += dt * 0.08f;
    if (thermalFilterPhase > 1.0f)
        thermalFilterPhase -= 1.0f;

    const float filterAngle = thermalFilterPhase * juce::MathConstants<float>::twoPi;
    thermalFilterOffset = std::sin (filterAngle) * params.filterDrift * da * 0.2f;
}

} // namespace dsp

