/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#include "Envelope.h"

namespace dsp
{
    void Envelope::prepare (double newSampleRate)
    {
        params.sampleRate = newSampleRate;
        reset();
    }

    void Envelope::reset()
    {
        stage = EnvStage::idle;
        level = 0.0f;
        samplesRemaining = 0;
    }

    void Envelope::setParameters (const EnvelopeParams& p)
    {
        params = p;
    }

    float Envelope::getStageTime (EnvStage s) const
    {
        switch (s)
        {
            case EnvStage::delay:   return params.delay;
            case EnvStage::attack:  return params.attack;
            case EnvStage::hold:    return params.hold;
            case EnvStage::decay:   return params.decay;
            case EnvStage::sustain: return 0.0f;
            case EnvStage::release: return params.release;
            default: return 0.0f;
        }
    }

    void Envelope::noteOn (float vel)
    {
        velocity = juce::jlimit (0.0f, 1.0f, vel * params.velocityScale);
        stage = EnvStage::delay;
        const float delayTime = juce::jmax (0.0f, getStageTime (EnvStage::delay));
        samplesRemaining = (int) (delayTime * params.sampleRate);
        if (samplesRemaining <= 0)
        {
            // No delay, go straight to attack
            stage = EnvStage::attack;
            const float attackTime = juce::jmax (0.001f, getStageTime (EnvStage::attack));
            samplesRemaining = (int) (attackTime * params.sampleRate);
            if (samplesRemaining <= 0)
                samplesRemaining = 1;
        }
    }

    void Envelope::noteOff()
    {
        if (stage != EnvStage::idle)
        {
            stage = EnvStage::release;
            const float releaseTime = juce::jmax (0.001f, getStageTime (EnvStage::release));
            samplesRemaining = (int) (releaseTime * params.sampleRate);
            if (samplesRemaining <= 0)
                samplesRemaining = 1;
        }
    }

    float Envelope::process (int numSamples)
    {
        if (stage == EnvStage::idle)
            return level;

        float out = 0.0f;
        for (int i = 0; i < numSamples; ++i)
        {
            if (samplesRemaining > 0)
                --samplesRemaining;

            switch (stage)
            {
                case EnvStage::delay:
                {
                    level = 0.0f;
                    if (samplesRemaining <= 0)
                    {
                        stage = EnvStage::attack;
                        const float attackTime = juce::jmax (0.001f, getStageTime (EnvStage::attack));
                        samplesRemaining = (int) (attackTime * params.sampleRate);
                        if (samplesRemaining <= 0)
                            samplesRemaining = 1;
                    }
                    break;
                }
                case EnvStage::attack:
                {
                    const int totalSamples = juce::jmax (1, (int) (juce::jmax (0.001f, params.attack) * params.sampleRate));
                    const int safeRemaining = juce::jmax (1, samplesRemaining);
                    level += (1.0f - level) / (float) safeRemaining;
                    if (samplesRemaining <= 0)
                        stage = EnvStage::hold;
                    break;
                }
                case EnvStage::hold:
                    level = 1.0f;
                    stage = EnvStage::decay;
                    samplesRemaining = (int) (juce::jmax (0.001f, getStageTime (EnvStage::decay)) * params.sampleRate);
                    if (samplesRemaining <= 0)
                        samplesRemaining = 1;
                    break;
                case EnvStage::decay:
                {
                    const int safeRemaining = juce::jmax (1, samplesRemaining);
                    level += (params.sustain - level) / (float) safeRemaining;
                    if (samplesRemaining <= 0)
                        stage = EnvStage::sustain;
                    break;
                }
                case EnvStage::sustain:
                    level = params.sustain;
                    break;
                case EnvStage::release:
                {
                    const int safeRemaining = juce::jmax (1, samplesRemaining);
                    level += (0.0f - level) / (float) safeRemaining;
                    if (samplesRemaining <= 0)
                        stage = EnvStage::idle;
                    break;
                }
                default:
                    break;
            }

            out = level;
        }
        return out;
    }
}
