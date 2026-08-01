/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: 64-step performance sequencer with probability, ratchets, and gate.
 */

#pragma once

#include <JuceHeader.h>
#include <array>
#include <random>

struct StepData
{
    int   note { -1 };          // -1 = rest
    float velocity { 0.8f };
    float gate { 0.5f };        // 0..1 gate length as fraction of step
    float probability { 1.0f }; // 0..1 chance of playing
    int   ratchet { 1 };        // 1-4 subdivisions
    float slide { 0.0f };       // 0..1 portamento between steps
    float mpeX { 0.0f };        // MPE CC74 timbre
    float mpeY { 0.0f };        // MPE pressure
    float mpeZ { 0.0f };        // MPE pitchbend
    bool  active { false };
};

class Sequencer
{
public:
    Sequencer();
    ~Sequencer() = default;

    void prepare (double sampleRate);
    void reset();

    // Playback control
    void start();
    void stop();
    void setPlaying (bool playing);
    bool isPlaying() const { return playing; }

    // Transport
    void setTempo (float bpm);
    void setStepLength (int steps); // 1-64
    int getStepLength() const { return numSteps; }
    void setCurrentStep (int step);
    int getCurrentStep() const { return currentStep; }

    // Step editing
    StepData& getStep (int index);
    void setStep (int index, const StepData& data);
    void clearStep (int index);
    void clearAll();

    // Process: returns true if a note should fire on this block
    bool process (int numSamples);

    // Get current note info for voice allocation
    int getCurrentNote() const { return currentNote; }
    float getCurrentVelocity() const { return currentVelocity; }
    bool hasNewNote() const { return newNote; }

    // MPE output for current step
    float getCurrentMPEX() const { return currentMPEX; }
    float getCurrentMPEY() const { return currentMPEY; }
    float getCurrentMPEZ() const { return currentMPEZ; }

    // Pattern data access
    std::array<StepData, 64>& getSteps() { return steps; }
    const std::array<StepData, 64>& getSteps() const { return steps; }

    // Swing
    void setSwing (float swing) { swingAmount = swing; }
    float getSwing() const { return swingAmount; }

private:
    void advanceStep();

    std::array<StepData, 64> steps;
    int numSteps { 16 };
    int currentStep { 0 };
    bool playing { false };

    // Timing
    double sampleRate { 44100.0 };
    float tempo { 120.0f };
    double samplesPerStep { 0.0 };
    double sampleCounter { 0.0 };
    float swingAmount { 0.0f };

    // Note output
    int currentNote { -1 };
    float currentVelocity { 0.0f };
    float currentMPEX { 0.0f };
    float currentMPEY { 0.0f };
    float currentMPEZ { 0.0f };
    bool newNote { false };

    // Ratchet state
    int ratchetCounter { 0 };
    int ratchetTotal { 1 };

    std::mt19937 rng;
};

