/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: DAHDSR envelope and Chaos LFO implementations.
 */

#include "Modulators.h"

namespace dsp
{

// === DAHDSR Envelope ===

void DAHDSREnvelope::prepare (double sampleRate)
{
    params.sampleRate = sampleRate;
    reset();
}

void DAHDSREnvelope::reset()
{
    stage = Stage::idle;
    level = 0.0f;
    velocity = 0.0f;
    samplesRemaining = 0;
}

void DAHDSREnvelope::setParameters (const DAHDSRParams& p)
{
    params = p;
}

void DAHDSREnvelope::noteOn (float vel, float noteNum)
{
    velocity = vel * params.velocityScale;
    if (params.keyScale > 0.0f)
    {
        float keyScale = 1.0f + (noteNum - 60.0f) / 60.0f * params.keyScale;
        velocity *= keyScale;
    }
    velocity = juce::jlimit (0.0f, 1.0f, velocity);

    if (params.delay > 0.0f)
    {
        stage = Stage::delay;
        samplesRemaining = (int) (params.delay * params.sampleRate);
    }
    else
    {
        stage = Stage::attack;
        samplesRemaining = (int) (params.attack * params.sampleRate);
    }
    level = 0.0f;
}

void DAHDSREnvelope::noteOff()
{
    if (stage != Stage::idle)
    {
        stage = Stage::release;
        samplesRemaining = (int) (params.release * params.sampleRate);
    }
}

float DAHDSREnvelope::process (int numSamples)
{
    if (stage == Stage::idle)
        return 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        if (samplesRemaining <= 0)
        {
            // Advance to next stage
            switch (stage)
            {
                case Stage::delay:
                    stage = Stage::attack;
                    samplesRemaining = (int) (params.attack * params.sampleRate);
                    break;
                case Stage::attack:
                    stage = Stage::hold;
                    samplesRemaining = (int) (params.hold * params.sampleRate);
                    level = 1.0f;
                    break;
                case Stage::hold:
                    stage = Stage::decay;
                    samplesRemaining = (int) (params.decay * params.sampleRate);
                    break;
                case Stage::decay:
                    stage = Stage::sustain;
                    samplesRemaining = 0;
                    level = params.sustain;
                    break;
                case Stage::release:
                    stage = Stage::idle;
                    level = 0.0f;
                    break;
                default:
                    break;
            }
        }

        if (stage == Stage::idle)
            break;

        --samplesRemaining;

        // Calculate level based on current stage
        float progress = 1.0f;
        if (samplesRemaining > 0)
        {
            int totalSamples = 0;
            switch (stage)
            {
                case Stage::delay:   totalSamples = (int) (params.delay * params.sampleRate); break;
                case Stage::attack:  totalSamples = (int) (params.attack * params.sampleRate); break;
                case Stage::hold:    totalSamples = (int) (params.hold * params.sampleRate); break;
                case Stage::decay:   totalSamples = (int) (params.decay * params.sampleRate); break;
                case Stage::release: totalSamples = (int) (params.release * params.sampleRate); break;
                default: break;
            }
            if (totalSamples > 0)
                progress = 1.0f - (float) samplesRemaining / (float) totalSamples;
        }

        switch (stage)
        {
            case Stage::delay:
                level = 0.0f;
                break;
            case Stage::attack:
                level = progress * velocity;
                break;
            case Stage::hold:
                level = velocity;
                break;
            case Stage::decay:
                level = velocity + (params.sustain - velocity) * progress;
                break;
            case Stage::sustain:
                level = params.sustain * velocity;
                break;
            case Stage::release:
                level = level * (1.0f - progress);
                break;
            default:
                break;
        }
    }

    if (params.invert)
        return 1.0f - level;
    return level;
}

bool DAHDSREnvelope::isActive() const
{
    return stage != Stage::idle;
}

// === Chaos LFO ===

void ChaosLfo::setChaosParameters (const ChaosLfoParams& p)
{
    chaosParams = p;
    Lfo::setParameters (p);
}

void ChaosLfo::process (int numSamples, float phaseOffset)
{
    // Process base LFO
    Lfo::process (numSamples, phaseOffset);

    if (chaosParams.chaosAmount > 0.0f)
    {
        chaosOutput = generateChaos();
        // Blend between LFO and chaos
        float baseOutput = Lfo::getOutput();
        chaosOutput = baseOutput * (1.0f - chaosParams.chaosAmount) + chaosOutput * chaosParams.chaosAmount;
    }
    else
    {
        chaosOutput = Lfo::getOutput();
    }
}

float ChaosLfo::generateChaos()
{
    const float dt = 1.0f / (float) chaosParams.sampleRate;
    chaosPhase += dt * chaosParams.chaosRate * 10.0f;

    if (chaosPhase >= 1.0f)
    {
        chaosPhase -= 1.0f;
        // New random target using member RNG
        std::uniform_real_distribution<float> dist (-1.0f, 1.0f);
        chaosTarget = dist (rng);
    }

    // Smooth interpolation toward target
    float smooth = juce::jlimit (0.0f, 1.0f, chaosParams.chaosSmooth);
    chaosOutput += (chaosTarget - chaosOutput) * (1.0f - smooth) * 0.1f;

    return chaosOutput;
}

} // namespace dsp

