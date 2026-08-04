/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#pragma once

#include <JuceHeader.h>

// Forward declarations for structured parameter groups (to be expanded later)
struct VoiceParameter;
struct GlobalParameter;

class Parameters
{
public:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // ── Global ──────────────────────────────────────────────────────────────
    inline static const juce::String paramMasterVolume { "master_volume" };
    inline static const juce::String paramVoiceMode     { "voice_mode" };
    inline static const juce::String paramGlideTime     { "glide_time" };
    inline static const juce::String paramMasterTune    { "master_tune" };

    // ── VCO1 ─────────────────────────────────────────────────────────────────
    inline static const juce::String paramOsc1Waveform   { "osc1_waveform" };
    inline static const juce::String paramOsc1PulseWidth { "osc1_pulse_width" };
    inline static const juce::String paramOsc1Scale      { "osc1_scale" };
    inline static const juce::String paramOsc1Octave     { "osc1_octave" };
    inline static const juce::String paramOsc1Gain       { "osc1_gain" };
    inline static const juce::String paramOsc1Tune       { "osc1_tune" };

    // ── VCO2 ─────────────────────────────────────────────────────────────────
    inline static const juce::String paramOsc2Waveform { "osc2_waveform" };
    inline static const juce::String paramOsc2PulseWidth { "osc2_pulse_width" };
    inline static const juce::String paramOsc2Pitch    { "osc2_pitch" };
    inline static const juce::String paramOsc2Scale    { "osc2_scale" };
    inline static const juce::String paramOsc2Octave   { "osc2_octave" };
    inline static const juce::String paramOsc2Gain     { "osc2_gain" };
    inline static const juce::String paramOsc2Tune       { "osc2_tune" };

    // ── SUB ───────────────────────────────────────────────────────────────────
    inline static const juce::String paramSubOctave { "sub_octave" };
    inline static const juce::String paramSubGain   { "sub_gain" };

    // ── NOISE ─────────────────────────────────────────────────────────────────
    inline static const juce::String paramNoiseGain { "noise_gain" };
    inline static const juce::String paramNoiseType { "noise_type" };

    // ── MIXER ────────────────────────────────────────────────────────────────
    inline static const juce::String paramMixerVco1Level  { "mixer_vco1_level" };
    inline static const juce::String paramMixerVco2Level  { "mixer_vco2_level" };
    inline static const juce::String paramMixerSubLevel   { "mixer_sub_level" };
    inline static const juce::String paramMixerLfoCvAmount{ "mixer_lfo_cv_amount" };
    inline static const juce::String paramMixerEgCvAmount { "mixer_eg_cv_amount" };
    inline static const juce::String paramMixerDrive      { "mixer_drive" };

    // ── FILTER ───────────────────────────────────────────────────────────────
    inline static const juce::String paramHPFCutoff { "hpf_cutoff" };
    inline static const juce::String paramHPFRes    { "hpf_res" };
    inline static const juce::String paramLPFCutoff { "lpf_cutoff" };
    inline static const juce::String paramLPFRes    { "lpf_res" };
    inline static const juce::String paramLPFDrive  { "lpf_drive" };

    // ── SATURATION ────────────────────────────────────────────────────────────
    inline static const juce::String paramSatDrive { "sat_drive" };

    // ── LFO / Modulation Generator ────────────────────────────────────────────
    inline static const juce::String paramLfoShape { "lfo_shape" };
    inline static const juce::String paramLfoRate  { "lfo_rate" };

    // ── LFO1 ────────────────────────────────────────────────────────────────────
    inline static const juce::String paramLFO1Waveform  { "lfo1_waveform" };
    inline static const juce::String paramLFO1Rate      { "lfo1_rate" };
    inline static const juce::String paramLFO1Depth     { "lfo1_depth" };
    inline static const juce::String paramLFO1Dest      { "lfo1_dest" };

    // ── LFO2 ────────────────────────────────────────────────────────────────────
    inline static const juce::String paramLFO2Waveform  { "lfo2_waveform" };
    inline static const juce::String paramLFO2Rate      { "lfo2_rate" };
    inline static const juce::String paramLFO2Depth     { "lfo2_depth" };
    inline static const juce::String paramLFO2Dest      { "lfo2_dest" };

    // ── LFO3 ────────────────────────────────────────────────────────────────────
    inline static const juce::String paramLFO3Waveform  { "lfo3_waveform" };
    inline static const juce::String paramLFO3Rate      { "lfo3_rate" };
    inline static const juce::String paramLFO3Depth     { "lfo3_depth" };
    inline static const juce::String paramLFO3Dest      { "lfo3_dest" };

    // ── LFO4 ────────────────────────────────────────────────────────────────────
    inline static const juce::String paramLFO4Waveform  { "lfo4_waveform" };
    inline static const juce::String paramLFO4Rate      { "lfo4_rate" };
    inline static const juce::String paramLFO4Depth     { "lfo4_depth" };
    inline static const juce::String paramLFO4Dest      { "lfo4_dest" };

    // ── EG1 (Delay / Attack / Release — pitch mod source) ─────────────────────
    inline static const juce::String paramEg1Delay   { "eg1_delay" };
    inline static const juce::String paramEg1Attack  { "eg1_attack" };
    inline static const juce::String paramEg1Release { "eg1_release" };

    // ── EG2 (Full ADSR — drives Filter + VCA) ─────────────────────────────────
    inline static const juce::String paramEg2Attack  { "eg2_attack" };
    inline static const juce::String paramEg2Decay   { "eg2_decay" };
    inline static const juce::String paramEg2Sustain { "eg2_sustain" };
    inline static const juce::String paramEg2Release { "eg2_release" };

    // ── ENVELOPE 1 (old style ADSR - used by PluginEditor/AudioEngine) ────────
    inline static const juce::String paramEnv1Attack  { "env1_attack" };
    inline static const juce::String paramEnv1Decay   { "env1_decay" };
    inline static const juce::String paramEnv1Sustain { "env1_sustain" };
    inline static const juce::String paramEnv1Release { "env1_release" };

    // ── AMP (master gain + pan) ───────────────────────────────────────────────
    inline static const juce::String paramAmpGain { "amp_gain" };
    inline static const juce::String paramPan     { "pan" };

    // ── VCA ENVELOPE (ADSR for amplitude shaping) ─────────────────────────────
    inline static const juce::String paramAmpAttack  { "amp_attack" };
    inline static const juce::String paramAmpDecay   { "amp_decay" };
    inline static const juce::String paramAmpSustain { "amp_sustain" };
    inline static const juce::String paramAmpRelease { "amp_release" };

    // ── TAPE DELAY ────────────────────────────────────────────────────────────
    inline static const juce::String paramTapeDelayEnable    { "tape_delay_enable" };
    inline static const juce::String paramTapeDelayTimeMode  { "tape_delay_time_mode" };
    inline static const juce::String paramTapeDelayTime      { "tape_delay_time" };
    inline static const juce::String paramTapeDelayFeedback  { "tape_delay_feedback" };
    inline static const juce::String paramTapeDelayMix       { "tape_delay_mix" };
    inline static const juce::String paramTapeDelayAge       { "tape_delay_age" };
    inline static const juce::String paramTapeDelaySat       { "tape_delay_saturation" };
    inline static const juce::String paramTapeDelayWow       { "tape_delay_wow" };
    inline static const juce::String paramTapeDelayFlutter   { "tape_delay_flutter" };

    // helpers
    inline static constexpr float masterVolumeMin = 0.0f;
    inline static constexpr float masterVolumeMax = 1.0f;
    inline static constexpr float masterVolumeDefault = 0.8f;

    // OSC waveform options
    static const juce::StringArray oscWaveformChoices;

    // LFO waveform options
    static const juce::StringArray lfoWaveformChoices;
    // LFO destination options (routing targets)
    static const juce::StringArray lfoDestinationChoices;

    // Noise color options
    static const juce::StringArray noiseTypeChoices;

    // Tape delay time mode options (tempo sync divisions)
    static const juce::StringArray tapeDelayTimeModeChoices;

    // List of parameter IDs that are "knobs" eligible for the Randomize feature.
    // (Excludes discrete choices like waveform/scale/voice mode, and master volume.)
    static const juce::StringArray& getRandomizableParamIds();
};
