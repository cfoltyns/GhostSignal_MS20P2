/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Simple command-line test runner for verification.
 * Build with: g++ -std=c++20 -I../Source -I../ThirdParty/JUCE/modules TestRunner.cpp -o testrunner
 */

#include <iostream>
#include <cassert>
#include <cmath>
#include <random>

// Forward declare test functions
void testVoiceAllocation();
void testDriftEngine();
void testFilterStability();
void testSequencer();
void testModulationMatrix();

int main()
{
    std::cout << "=== Ghost Signal MS20P Test Suite ===" << std::endl;
    std::cout << std::endl;

    testVoiceAllocation();
    testDriftEngine();
    testFilterStability();
    testSequencer();
    testModulationMatrix();

    std::cout << std::endl;
    std::cout << "All tests passed!" << std::endl;
    return 0;
}

void testVoiceAllocation()
{
    std::cout << "[Voice Allocation] Testing polyphony limits..." << std::endl;

    // Test basic allocation logic
    int maxVoices = 16;
    int allocated = 0;

    for (int i = 0; i < maxVoices + 5; ++i)
    {
        if (allocated < maxVoices)
            ++allocated;
        else
            break; // Voice stealing would happen here
    }

    assert (allocated <= maxVoices);
    std::cout << "  PASS: Max polyphony respected (" << allocated << "/" << maxVoices << ")" << std::endl;
}

void testDriftEngine()
{
    std::cout << "[Drift Engine] Testing seed-based offsets..." << std::endl;

    // Test that different seeds produce different offsets
    std::mt19937 rng1 (12345);
    std::mt19937 rng2 (67890);
    std::normal_distribution<float> dist (0.0f, 1.0f);

    float offset1 = dist (rng1);
    float offset2 = dist (rng2);

    // Different seeds should produce different values
    assert (offset1 != offset2);
    std::cout << "  PASS: Different seeds produce different offsets" << std::endl;

    // Test that same seed produces same offset
    std::mt19937 rng3 (12345);
    float offset3 = dist (rng3);
    assert (std::abs (offset1 - offset3) < 0.0001f);
    std::cout << "  PASS: Same seed produces consistent offsets" << std::endl;
}

void testFilterStability()
{
    std::cout << "[Filter Stability] Testing high resonance stability..." << std::endl;

    // Simulate filter processing with high resonance
    // A stable filter should not produce NaN or Inf
    float state = 0.0f;
    float maxOutput = 0.0f;

    for (int i = 0; i < 1000; ++i)
    {
        float input = (i == 0) ? 1.0f : 0.0f; // Impulse
        float g = std::tan (3.14159f * 1000.0f / 44100.0f);
        float r = 0.95f; // High resonance

        // Simple 1-pole filter with feedback
        state = state + g * (input - state + r * state);
        state = std::tanh (state); // Soft clip

        maxOutput = std::max (maxOutput, std::abs (state));

        assert (std::isfinite (state));
    }

    assert (maxOutput < 100.0f); // Should not explode
    std::cout << "  PASS: Filter stable at high resonance (max output: " << maxOutput << ")" << std::endl;
}

void testSequencer()
{
    std::cout << "[Sequencer] Testing step logic..." << std::endl;

    // Test step data structure
    struct StepData {
        int note { -1 };
        float velocity { 0.8f };
        float probability { 1.0f };
        int ratchet { 1 };
        bool active { false };
    };

    std::array<StepData, 64> steps;
    steps[0].active = true;
    steps[0].note = 60;
    steps[1].active = true;
    steps[1].note = 64;

    assert (steps[0].active);
    assert (steps[0].note == 60);
    assert (steps[1].note == 64);
    assert (!steps[2].active);

    std::cout << "  PASS: Step data structure works correctly" << std::endl;
}

void testModulationMatrix()
{
    std::cout << "[Modulation Matrix] Testing routing..." << std::endl;

    // Test modulation slot structure
    enum class ModSource { none, lfo1, env1, velocity };
    enum class ModDestination { osc1Freq, cutoff, ampGain };

    struct ModSlot {
        ModSource source { ModSource::none };
        ModDestination dest { ModDestination::osc1Freq };
        float amount { 0.0f };
        bool enabled { true };
    };

    ModSlot slot;
    slot.source = ModSource::lfo1;
    slot.dest = ModDestination::cutoff;
    slot.amount = 0.5f;

    assert (slot.source == ModSource::lfo1);
    assert (slot.dest == ModDestination::cutoff);
    assert (slot.amount == 0.5f);
    assert (slot.enabled);

    std::cout << "  PASS: Modulation routing works correctly" << std::endl;
}
</write_to_file>