/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#pragma once

#include <JuceHeader.h>

namespace dsp
{
    enum class EnvStage { idle, delay, attack, hold, decay, sustain, release };

    struct EnvelopeParams
    {
        double sampleRate { 44100.0 };
        float delay   { 0.0f };
        float attack  { 0.01f };
        float hold    { 0.0f };
        float decay   { 0.1f };
        float sustain { 0.7f };
        float release { 0.3f };
        float velocityScale { 1.0f };
        float keyScale { 0.0f };
        float retrigger { 0.0f };
    };

    class Envelope
    {
    public:
        Envelope() = default;
        ~Envelope() = default;

        void prepare (double sampleRate);
        void reset();
        void setParameters (const EnvelopeParams& p);
        void noteOn (float velocity);
        void noteOff();

        // Returns current envelope value in range 0..1
        float process (int numSamples);
        bool isActive() const { return stage != EnvStage::idle; }

    private:
        float getStageTime (EnvStage stage) const;

        EnvelopeParams params;
        EnvStage stage { EnvStage::idle };
        float level { 0.0f };
        float velocity { 0.0f };
        int samplesRemaining { 0 };
    };
}
