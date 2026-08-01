/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Aggregate modulator header for DAHDSR envelopes and Chaos LFOs.
 */

#pragma once

#include "Envelope.h"
#include "LFO.h"

namespace dsp
{

// Advanced DAHDSR envelope (Delay-Attack-Hold-Decay-Sustain-Release)
struct DAHDSRParams
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
    float curve { 0.5f }; // 0=linear, 0.5=exponential, 1=logarithmic
    bool  invert { false };
    bool  legato { false };
};

class DAHDSREnvelope
{
public:
    DAHDSREnvelope() = default;
    ~DAHDSREnvelope() = default;

    void prepare (double sampleRate);
    void reset();
    void setParameters (const DAHDSRParams& p);
    void noteOn (float velocity, float noteNum = 69.0f);
    void noteOff();

    // Process returns value 0..1
    float process (int numSamples);
    bool isActive() const;

    // Get current stage for visualization
    int getCurrentStage() const { return static_cast<int>(stage); }

private:
    enum class Stage { idle, delay, attack, hold, decay, sustain, release };

    DAHDSRParams params;
    Stage stage { Stage::idle };
    float level { 0.0f };
    float velocity { 0.0f };
    int samplesRemaining { 0 };
};

// Chaos LFO: adds smoothed random walk mode
struct ChaosLfoParams : public LfoParams
{
    float chaosAmount { 0.0f };    // 0..1 blend of chaos
    float chaosRate { 1.0f };      // how fast chaos evolves
    float chaosSmooth { 0.5f };    // smoothing factor 0..1
};

class ChaosLfo : public Lfo
{
public:
    ChaosLfo() = default;
    ~ChaosLfo() = default;

    void setChaosParameters (const ChaosLfoParams& p);
    void process (int numSamples, float phaseOffset = 0.0f);

    float getChaosOutput() const { return chaosOutput; }

private:
    float generateChaos();
    float chaosOutput { 0.0f };
    float chaosTarget { 0.0f };
    float chaosPhase { 0.0f };
    ChaosLfoParams chaosParams;
};

} // namespace dsp

