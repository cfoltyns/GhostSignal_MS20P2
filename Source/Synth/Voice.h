/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#pragma once

#include <JuceHeader.h>
#include "../Core/Voice.h"
#include "Oscillator.h"
#include "SubOsc.h"
#include "NoiseGen.h"
#include "Mixer.h"
#include "Filter/HighPass.h"
#include "Filter/LowPass.h"
#include "Filter/Saturation.h"
#include "Envelope.h"
#include "LFO.h"
#include "AnalogDrift.h"

namespace dsp
{
    struct VoiceParams
    {
        // Osc1
        OscillatorParams osc1;
        // Osc2
        OscillatorParams osc2;
        // Sub
        SubOscParams sub;
        // Noise
        NoiseGenParams noise;
        // Mixer
        MixerParams mixer;
        // HPF
        filter::HighPassParams hpf;
        // LPF
        filter::LowPassParams lpf;
        // Post-filter saturation
        filter::SaturationParams saturation;
        // Envelopes
        EnvelopeParams env1;
        EnvelopeParams env2;
        EnvelopeParams env3;
        EnvelopeParams env4;
        // LFOs
        LfoParams lfo1;
        LfoParams lfo2;
        LfoParams lfo3;
        LfoParams lfo4;
        // LFO destinations (index into lfoDestinationChoices)
        int lfo1Dest { 0 };
        int lfo2Dest { 0 };
        int lfo3Dest { 0 };
        int lfo4Dest { 0 };
        // Amp
        float ampGain { 0.7f };
        float pan { 0.5f };
        float ampDrive { 0.0f };
        // Glide
        float glideTime { 0.0f };
        bool glideEnabled { false };
    };
}

class VoiceDSP : public Voice
{
public:
    VoiceDSP() = default;
    ~VoiceDSP() override = default;

    void prepare (double sampleRate, int maxBlockSize) override;
    void reset() override;
    void noteOn (int midiNoteNumber, float velocity) override;
    void noteOff() override;
    void pitchBend (int pitchBendValue) override;
    void pressure (int pressureValue) override;
    void timbre (int timbreValue) override;

    void setParameters (const dsp::VoiceParams& p) override;
    void process (juce::AudioBuffer<float>& buffer, int startSample, int numSamples) override;

    // Drift support
    void setDrift (float pitchCents, float filterCutoffSemitones, float resOffset,
                   float envAttackScale, float envDecayScale, float envReleaseScale,
                   float satOffset, float thermalPitchCents);

private:
    dsp::VoiceParams params;

    dsp::Oscillator osc1;
    dsp::Oscillator osc2;
    dsp::SubOsc sub;
    dsp::NoiseGen noise;
    dsp::Mixer mixer;
    dsp::filter::HighPass hpf;
    dsp::filter::LowPass lpf;
    dsp::filter::Saturation saturation;
    dsp::Envelope env1;
    dsp::Envelope env2;
    dsp::Envelope env3;
    dsp::Envelope env4;
    dsp::Lfo lfo1;
    dsp::Lfo lfo2;
    dsp::Lfo lfo3;
    dsp::Lfo lfo4;

    juce::AudioBuffer<float> osc1Buffer;
    juce::AudioBuffer<float> osc2Buffer;
    juce::AudioBuffer<float> subBuffer;
    juce::AudioBuffer<float> noiseBuffer;
    juce::AudioBuffer<float> mixBuffer;

    float currentNote { 69.0f };
    float currentPitchBend { 0.0f };
    float currentPressure { 0.0f };
    float currentTimbre { 0.5f };

    // Drift state
    float driftPitchCents { 0.0f };
    float driftFilterCutoff { 0.0f };
    float driftResonance { 0.0f };
    float driftEnvAttackScale { 1.0f };
    float driftEnvDecayScale { 1.0f };
    float driftEnvReleaseScale { 1.0f };
    float driftSatOffset { 0.0f };
    float driftThermalPitch { 0.0f };

    // Glide/portamento slew limiting
    float glideTargetNote { 69.0f };
    float glideCurrentNote { 69.0f };
    float glideTime { 0.0f };
    float glideRate { 0.0f };
    bool glideActive { false };
    bool glideEnabled { false };
};
