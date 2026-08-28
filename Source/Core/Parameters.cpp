/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#include "Parameters.h"

const juce::StringArray Parameters::oscWaveformChoices = { "Triangle", "Saw", "Square", "Pulse", "Sine", "Noise", "Super Saw" };
const juce::StringArray Parameters::lfoWaveformChoices = { "Sine", "Triangle", "Square", "Sawtooth", "Random" };
const juce::StringArray Parameters::lfoDestinationChoices = { "Off", "VCO1 Pitch", "VCO2 Pitch", "VCO1 PWM", "VCO2 PWM", "VCO1 Tune", "VCO2 Tune", "VCO1 Level", "VCO2 Level", "Filter Cutoff", "Filter Res", "Amp Gain", "Pan" };

const juce::StringArray Parameters::noiseTypeChoices = { "Brown", "Pink", "White", "Blue", "Violet" };

const juce::StringArray Parameters::tapeDelayTimeModeChoices = { "1/2", "1/4", "1/8", "1/16", "1/32", "Slap", "MS" };

juce::AudioProcessorValueTreeState::ParameterLayout Parameters::createParameterLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout params;

    auto addFloat = [&](const String& id, const String& name, float min, float max, float def, float step = 0.001f, float skew = 1.0f)
    {
        params.add (std::make_unique<AudioParameterFloat> (
            ParameterID (id, 1), name,
            NormalisableRange<float> (min, max, step, skew), def));
    };

    auto addChoice = [&](const String& id, const String& name, const StringArray& choices, int def)
    {
        params.add (std::make_unique<AudioParameterChoice> (
            ParameterID (id, 1), name, choices, def));
    };

    // -- Global --
    addFloat  (paramMasterVolume, "Master Volume", 0.0f, 1.0f, masterVolumeDefault, 0.001f, 0.5f);
    addChoice (paramVoiceMode, "Voice Mode", StringArray { "Poly", "Mono", "Unison" }, 0);
    addFloat  (paramGlideTime, "Glide Time", 0.0f, 2.0f, 0.0f, 0.001f, 0.5f);
    addFloat  (paramMasterTune, "Master Tune", -100.0f, 100.0f, 0.0f);

    // -- VCO1 --
    addChoice (paramOsc1Waveform, "VCO1 Waveform",
               StringArray { "Triangle", "Saw", "Square", "Pulse", "Sine", "Noise", "Super Saw" }, 1);
    addFloat  (paramOsc1PulseWidth, "VCO1 Pulse Width", 0.01f, 0.99f, 0.5f);
    addChoice (paramOsc1Scale, "VCO1 Scale", StringArray { "32'", "16'", "8'", "4'" }, 2);
    addFloat  (paramOsc1Octave, "VCO1 Octave", -2.0f, 2.0f, 0.0f, 1.0f);
    addFloat  (paramOsc1Gain, "VCO1 Gain", 0.0f, 1.0f, 0.8f);
    addFloat  (paramOsc1Tune, "VCO1 Tune", -100.0f, 100.0f, 0.0f);

    // -- VCO2 --
    addChoice (paramOsc2Waveform, "VCO2 Waveform",
               StringArray { "Triangle", "Saw", "Square", "Pulse", "Sine", "Noise", "Super Saw" }, 1);
    addFloat  (paramOsc2PulseWidth, "VCO2 Pulse Width", 0.01f, 0.99f, 0.5f);
    addFloat  (paramOsc2Pitch, "VCO2 Pitch", -24.0f, 24.0f, 0.0f);
    addChoice (paramOsc2Scale, "VCO2 Scale", StringArray { "32'", "16'", "8'", "4'" }, 2);
    addFloat  (paramOsc2Octave, "VCO2 Octave", -3.0f, 3.0f, 0.0f, 1.0f);
    addFloat  (paramOsc2Gain, "VCO2 Gain", 0.0f, 1.0f, 0.8f);
    addFloat  (paramOsc2Tune, "VCO2 Tune", -100.0f, 100.0f, 0.0f);

    // -- SUB --
    addFloat (paramSubOctave, "Sub Octave", -3.0f, 3.0f, 0.0f);
    addFloat (paramSubGain, "Sub Gain", 0.0f, 1.0f, 0.5f);

    // -- NOISE --
    addChoice (paramNoiseType, "Noise Type", Parameters::noiseTypeChoices, 2);

    addFloat  (paramNoiseGain, "Noise Gain", 0.0f, 1.0f, 0.5f);

    // -- MIXER --
    addFloat (paramMixerVco1Level,   "VCO1 Level",   0.0f, 1.0f, 0.8f);
    addFloat (paramMixerVco2Level,   "VCO2 Level",   0.0f, 1.0f, 0.8f);
    addFloat (paramMixerSubLevel,    "Sub Level",    0.0f, 1.0f, 0.5f);
    addFloat (paramMixerLfoCvAmount, "LFO/CV Amount", 0.0f, 1.0f, 0.0f);
    addFloat (paramMixerEgCvAmount,  "EG/CV Amount",  0.0f, 1.0f, 0.0f);
    addFloat (paramMixerDrive,       "Mixer Drive",   0.0f, 1.0f, 0.0f);

    // -- FILTER --
    addFloat (paramHPFCutoff, "HPF Cutoff", 20.0f, 1000.0f, 20.0f, 0.001f, 0.4f);
    addFloat (paramHPFRes,    "HPF Resonance", 0.0f, 1.0f, 0.0f);
    addFloat (paramLPFCutoff, "LPF Cutoff", 20.0f, 20000.0f, 20000.0f, 0.001f, 0.25f);
    addFloat (paramLPFRes,    "LPF Resonance", 0.0f, 1.0f, 0.0f);
    addFloat (paramLPFDrive,  "LPF Drive", 0.0f, 1.0f, 0.0f);

    // -- SATURATION --
    addFloat (paramSatDrive, "Saturation Drive", 0.0f, 1.0f, 0.0f);

    // -- LFO 1 --
    addChoice (paramLFO1Waveform, "LFO1 Waveform", Parameters::lfoWaveformChoices, 0);
    addFloat  (paramLFO1Rate,     "LFO1 Rate", 0.0f, 1.0f, 0.25f, 0.001f, 0.5f);
    addFloat  (paramLFO1Depth,    "LFO1 Depth", 0.0f, 1.0f, 0.5f);
    addChoice (paramLFO1Dest,    "LFO1 Destination", Parameters::lfoDestinationChoices, 0);
    addFloat  (paramLFO1Sync,    "LFO1 Sync", 0.0f, 1.0f, 0.0f, 1.0f);

    // -- LFO 2 --
    addChoice (paramLFO2Waveform, "LFO2 Waveform", Parameters::lfoWaveformChoices, 0);
    addFloat  (paramLFO2Rate,     "LFO2 Rate", 0.0f, 1.0f, 0.25f, 0.001f, 0.5f);
    addFloat  (paramLFO2Depth,    "LFO2 Depth", 0.0f, 1.0f, 0.5f);
    addChoice (paramLFO2Dest,    "LFO2 Destination", Parameters::lfoDestinationChoices, 0);
    addFloat  (paramLFO2Sync,    "LFO2 Sync", 0.0f, 1.0f, 0.0f, 1.0f);

    // -- LFO 3 --
    addChoice (paramLFO3Waveform, "LFO3 Waveform", Parameters::lfoWaveformChoices, 0);
    addFloat  (paramLFO3Rate,     "LFO3 Rate", 0.0f, 1.0f, 0.25f, 0.001f, 0.5f);
    addFloat  (paramLFO3Depth,    "LFO3 Depth", 0.0f, 1.0f, 0.5f);
    addChoice (paramLFO3Dest,    "LFO3 Destination", Parameters::lfoDestinationChoices, 0);
    addFloat  (paramLFO3Sync,    "LFO3 Sync", 0.0f, 1.0f, 0.0f, 1.0f);

    // -- LFO 4 --
    addChoice (paramLFO4Waveform, "LFO4 Waveform", Parameters::lfoWaveformChoices, 0);
    addFloat  (paramLFO4Rate,     "LFO4 Rate", 0.0f, 1.0f, 0.25f, 0.001f, 0.5f);
    addFloat  (paramLFO4Depth,    "LFO4 Depth", 0.0f, 1.0f, 0.5f);
    addChoice (paramLFO4Dest,    "LFO4 Destination", Parameters::lfoDestinationChoices, 0);
    addFloat  (paramLFO4Sync,    "LFO4 Sync", 0.0f, 1.0f, 0.0f, 1.0f);

    // -- EG1 (Delay / Attack / Release) --
    addFloat (paramEg1Delay,   "EG1 Delay",   0.0f,   5.0f, 0.0f,  0.001f, 0.5f);
    addFloat (paramEg1Attack,  "EG1 Attack",  0.001f, 5.0f, 0.01f, 0.001f, 0.5f);
    addFloat (paramEg1Release, "EG1 Release", 0.001f, 5.0f, 0.3f,  0.001f, 0.5f);

    // -- EG2 (Full ADSR) --
    addFloat (paramEg2Attack,  "EG2 Attack",  0.001f, 5.0f, 0.01f, 0.001f, 0.5f);
    addFloat (paramEg2Decay,   "EG2 Decay",   0.001f, 2.0f, 0.1f,  0.001f, 0.5f);
    addFloat (paramEg2Sustain, "EG2 Sustain", 0.0f,   1.0f, 0.7f,  0.001f, 1.0f);
    addFloat (paramEg2Release, "EG2 Release", 0.001f, 5.0f, 0.3f,  0.001f, 0.5f);

    // -- ENVELOPE 1 (old style ADSR) --
    addFloat (paramEnv1Attack,  "Env1 Attack",  0.001f, 5.0f, 0.01f, 0.001f, 0.5f);
    addFloat (paramEnv1Decay,   "Env1 Decay",   0.001f, 2.0f, 0.1f,  0.001f, 0.5f);
    addFloat (paramEnv1Sustain, "Env1 Sustain", 0.0f,   1.0f, 0.7f,  0.001f, 1.0f);
    addFloat (paramEnv1Release, "Env1 Release", 0.001f, 5.0f, 0.3f,  0.001f, 0.5f);

    // -- AMP --
    addFloat (paramAmpGain, "Amp Gain", 0.0f, 1.0f, 0.7f, 0.001f, 0.5f);
    addFloat (paramPan,     "Pan",      0.0f, 1.0f, 0.5f);

    // -- VCA ENVELOPE (ADSR for amplitude shaping) --
    addFloat (paramAmpAttack,  "Amp Attack",  0.001f, 5.0f, 0.01f, 0.001f, 0.5f);
    addFloat (paramAmpDecay,   "Amp Decay",   0.001f, 2.0f, 0.1f,  0.001f, 0.5f);
    addFloat (paramAmpSustain, "Amp Sustain", 0.0f,   1.0f, 0.7f,  0.001f, 1.0f);
    addFloat (paramAmpRelease, "Amp Release", 0.001f, 5.0f, 0.3f,  0.001f, 0.5f);

    // -- TAPE DELAY --
    addFloat  (paramTapeDelayEnable,    "Tape Delay Enable",    0.0f, 1.0f, 0.0f, 1.0f);
    addChoice (paramTapeDelayTimeMode,  "Tape Time Mode",       Parameters::tapeDelayTimeModeChoices, 0);
    addFloat  (paramTapeDelayTime,      "Tape Delay Time",      30.0f, 1000.0f, 300.0f, 0.001f, 0.5f);
    addFloat  (paramTapeDelayFeedback,  "Tape Delay Feedback",  0.0f, 0.95f, 0.5f);
    addFloat  (paramTapeDelayMix,       "Tape Delay Mix",       0.0f, 1.0f, 0.5f);
    addFloat  (paramTapeDelayAge,       "Tape Age",             0.0f, 1.0f, 0.5f);
    addFloat  (paramTapeDelaySat,       "Tape Saturation",      0.0f, 1.0f, 0.3f);
    addFloat  (paramTapeDelayWow,       "Tape Wow",             0.0f, 1.0f, 0.0f);
    addFloat  (paramTapeDelayFlutter,   "Tape Flutter",         0.0f, 1.0f, 0.0f);

    return params;
}

const juce::StringArray& Parameters::getRandomizableParamIds()
{
    static const juce::StringArray ids
    {
        paramOsc1PulseWidth,
        paramOsc1Octave,
        paramOsc1Gain,
        paramOsc1Tune,
        paramOsc2PulseWidth,
        paramOsc2Pitch,
        paramOsc2Octave,
        paramOsc2Gain,
        paramOsc2Tune,
        paramSubOctave,
        paramSubGain,
        paramNoiseGain,
        paramNoiseType,
        paramMixerVco1Level,
        paramMixerVco2Level,
        paramMixerSubLevel,
        paramMixerLfoCvAmount,
        paramMixerEgCvAmount,
        paramMixerDrive,
        paramHPFCutoff,
        paramHPFRes,
        paramLPFCutoff,
        paramLPFRes,
        paramLPFDrive,
        paramSatDrive,
        paramLFO1Rate,
        paramLFO1Depth,
        paramLFO2Rate,
        paramLFO2Depth,
        paramLFO3Rate,
        paramLFO3Depth,
        paramLFO4Rate,
        paramLFO4Depth,
        paramEg1Delay,
        paramEg1Attack,
        paramEg1Release,
        paramEg2Attack,
        paramEg2Decay,
        paramEg2Sustain,
        paramEg2Release,
        paramEnv1Attack,
        paramEnv1Decay,
        paramEnv1Sustain,
        paramEnv1Release,
        paramAmpGain,
        paramPan,
        paramGlideTime
    };
    return ids;
}