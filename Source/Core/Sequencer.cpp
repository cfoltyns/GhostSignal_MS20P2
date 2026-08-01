/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: 64-step performance sequencer implementation.
 */

#include "Sequencer.h"

Sequencer::Sequencer()
    : rng (std::random_device{}())
{
    prepare (44100.0);
}

void Sequencer::prepare (double sr)
{
    sampleRate = sr;
    samplesPerStep = (60.0 / tempo) * sampleRate;
    sampleCounter = 0.0;
}

void Sequencer::reset()
{
    sampleCounter = 0.0;
    currentStep = 0;
    playing = false;
    currentNote = -1;
    newNote = false;
}

void Sequencer::start()
{
    playing = true;
    currentStep = 0;
    sampleCounter = 0.0;
}

void Sequencer::stop()
{
    playing = false;
    currentStep = 0;
    currentNote = -1;
}

void Sequencer::setPlaying (bool p)
{
    if (p) start();
    else stop();
}

void Sequencer::setTempo (float bpm)
{
    tempo = juce::jlimit (20.0f, 300.0f, bpm);
    samplesPerStep = (60.0 / tempo) * sampleRate;
}

void Sequencer::setStepLength (int steps)
{
    numSteps = juce::jlimit (1, 64, steps);
}

void Sequencer::setCurrentStep (int step)
{
    currentStep = juce::jlimit (0, numSteps - 1, step);
}

StepData& Sequencer::getStep (int index)
{
    return steps[juce::jlimit (0, 63, index)];
}

void Sequencer::setStep (int index, const StepData& data)
{
    if (index >= 0 && index < 64)
        steps[index] = data;
}

void Sequencer::clearStep (int index)
{
    if (index >= 0 && index < 64)
        steps[index] = StepData{};
}

void Sequencer::clearAll()
{
    for (auto& step : steps)
        step = StepData{};
}

bool Sequencer::process (int numSamples)
{
    newNote = false;

    if (! playing)
        return false;

    sampleCounter += static_cast<double>(numSamples);

    // Apply swing: odd steps get delayed slightly
    double stepLength = samplesPerStep;
    if (swingAmount > 0.0f && (currentStep % 2) == 1)
        stepLength += stepLength * swingAmount * 0.5;

    if (sampleCounter >= stepLength)
    {
        sampleCounter -= stepLength;
        advanceStep();
    }

    return newNote;
}

void Sequencer::advanceStep()
{
    const auto& step = steps[currentStep];

    // Reset ratchet for new step
    ratchetTotal = step.ratchet;
    ratchetCounter = 0;

    // Check probability
    std::uniform_real_distribution<float> dist { 0.0f, 1.0f };
    bool shouldPlay = step.active && step.note >= 0 && (dist(rng) <= step.probability);

    if (shouldPlay)
    {
        currentNote = step.note;
        currentVelocity = step.velocity;
        currentMPEX = step.mpeX;
        currentMPEY = step.mpeY;
        currentMPEZ = step.mpeZ;
        newNote = true;
    }
    else
    {
        currentNote = -1;
    }

    // Advance to next step
    currentStep = (currentStep + 1) % numSteps;
}

