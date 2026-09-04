/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#pragma once

#include <JuceHeader.h>
#include <atomic>
#include "ModulationMatrix.h"
#include "PatchBay.h"
#include "Sequencer.h"
#include "../Synth/SynthEngine.h"
#include "../Effects/EffectsChain.h"
#include "../Effects/TapeDelay.h"

class AudioEngine
{
public:
    AudioEngine() = default;
    ~AudioEngine() = default;

    void prepare (double sampleRate, int maxBlockSize, int numOutputChannels);
    void reset();

    void process (juce::AudioBuffer<float>& buffer,
                  juce::MidiBuffer& midi,
                  juce::AudioProcessorValueTreeState& apvts,
                  juce::AudioPlayHead* playHead = nullptr);

    // Access
    dsp::SynthEngine& getSynthEngine() { return synthEngine; }
    ModulationMatrix& getModMatrix() { return modMatrix; }
    PatchBay& getPatchBay() { return patchBay; }
    Sequencer& getSequencer() { return sequencer; }
    fx::EffectsChain& getEffectsChain() { return fxChain; }
    fx::TapeDelay& getTapeDelay() { return tapeDelay; }

    // Thread-safe LFO output values for UI visualization (normalized -1..1)
    float getLfoOutput (int lfoIndex) const { return lfoOutputs[lfoIndex].load(); }
    void setLfoOutput (int lfoIndex, float value) { lfoOutputs[lfoIndex].store(value); }

private:
    void handleMidi (const juce::MidiBuffer& midi);
    void renderVoices (juce::AudioBuffer<float>& buffer, int numSamples);
    void applyMasterGain (juce::AudioBuffer<float>& buffer);

    double sampleRate { 44100.0 };
    int maxBlockSize { 512 };
    int numOutputChannels { 2 };

    dsp::SynthEngine synthEngine;
    ModulationMatrix modMatrix;
    PatchBay patchBay;
    Sequencer sequencer;
    fx::EffectsChain fxChain;
    fx::TapeDelay tapeDelay;

    // Thread-safe LFO output values for UI visualization (normalized -1..1)
    std::atomic<float> lfoOutputs[4] { {0.0f}, {0.0f}, {0.0f}, {0.0f} };
};
