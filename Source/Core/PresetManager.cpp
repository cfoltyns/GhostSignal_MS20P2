/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#include "PresetManager.h"
#include "PresetManager.h"
#include "Parameters.h"

namespace
{
    constexpr int kPresetNameLimit = 64;
    constexpr int kDescriptionLimit = 240;
    constexpr int kTagLimit = 8;
}

void PresetManager::prepare (juce::AudioProcessorValueTreeState& state)
{
    apvts = &state;

    presetFolder = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                       .getChildFile ("GhostSignal/MS20P/Presets");
    presetFolder.createDirectory();

    loadFactoryPresets();
    refreshUserPresets();
    clearCurrentPreset();
}

bool PresetManager::applyState (const juce::ValueTree& state)
{
    if (apvts == nullptr || !state.isValid())
    {
        lastError = "Preset state is unavailable.";
        return false;
    }

    if (state.getType() != apvts->state.getType())
    {
        lastError = "Preset state has an incompatible root type.";
        return false;
    }

    // Apply each parameter's value individually so JUCE parameter attachments
    // detect the property-changed events. Replacing children entirely via
    // copyPropertiesAndChildrenFrom does NOT trigger individual property
    // changes, leaving the UI out of sync with the actual parameter values.
    for (int i = 0; i < state.getNumChildren(); ++i)
    {
        auto child = state.getChild (i);
        auto id = child.getProperty ("id").toString();
        auto value = child.getProperty ("value");

        if (id.isEmpty())
            continue;

        // Find the matching child in the live state and set its value property
        auto liveChild = apvts->state.getChildWithProperty ("id", id);
        if (liveChild.isValid())
            liveChild.setProperty ("value", value, nullptr);
    }

    return true;
}

void PresetManager::loadFactoryPresets()
{
    factoryPresets.clear();
    if (apvts == nullptr)
        return;

    std::vector<FactoryPresetSpec> specs;
    specs.reserve (7);
    specs.push_back ({ "Init Patch",
                       "A clean, neutral starting point for building a patch.",
                       PresetCategory::init,
                       juce::StringArray { "init", "clean", "neutral" },
                       {} });
    specs.push_back ({ "MS20 Bass",
                       "Dense low-end saw bass with a tight envelope.",
                       PresetCategory::bass,
                       juce::StringArray { "bass", "low", "saw" },
                       { { Parameters::paramOsc1Waveform, 1.0f },
                         { Parameters::paramOsc2Waveform, 1.0f },
                         { Parameters::paramOsc1Octave, -1.0f },
                         { Parameters::paramOsc2Octave, -1.0f },
                         { Parameters::paramOsc1Gain, 0.85f },
                         { Parameters::paramOsc2Gain, 0.55f },
                         { Parameters::paramNoiseGain, 0.08f },
                         { Parameters::paramLPFCutoff, 900.0f },
                         { Parameters::paramLPFRes, 0.28f },
                         { Parameters::paramAmpAttack, 0.005f },
                         { Parameters::paramAmpDecay, 0.28f },
                         { Parameters::paramAmpSustain, 0.78f },
                         { Parameters::paramAmpRelease, 0.14f },
                         { Parameters::paramMixerDrive, 0.12f },
                         { Parameters::paramTapeDelayEnable, 0.0f } } });
    specs.push_back ({ "Aggressive Lead",
                       "Forward mono lead with drive, glide, and a bright filter.",
                       PresetCategory::lead,
                       juce::StringArray { "lead", "mono", "drive", "glide" },
                       { { Parameters::paramVoiceMode, 1.0f },
                         { Parameters::paramOsc1Waveform, 0.0f },
                         { Parameters::paramOsc2Waveform, 1.0f },
                         { Parameters::paramOsc1Octave, 0.0f },
                         { Parameters::paramOsc2Octave, 0.0f },
                         { Parameters::paramGlideTime, 0.12f },
                         { Parameters::paramOsc1Gain, 0.8f },
                         { Parameters::paramOsc2Gain, 0.65f },
                         { Parameters::paramLPFCutoff, 3600.0f },
                         { Parameters::paramLPFRes, 0.58f },
                         { Parameters::paramAmpAttack, 0.005f },
                         { Parameters::paramAmpDecay, 0.32f },
                         { Parameters::paramAmpSustain, 0.72f },
                         { Parameters::paramAmpRelease, 0.16f },
                         { Parameters::paramMixerDrive, 0.38f },
                         { Parameters::paramTapeDelayEnable, 1.0f } } });
    specs.push_back ({ "Space Pad",
                       "Slow, wide pad voicing with a soft filter and long release.",
                       PresetCategory::pad,
                       juce::StringArray { "pad", "poly", "slow", "wide" },
                       { { Parameters::paramVoiceMode, 1.0f },
                         { Parameters::paramOsc1Waveform, 0.0f },
                         { Parameters::paramOsc2Waveform, 2.0f },
                         { Parameters::paramOsc1Octave, 0.0f },
                         { Parameters::paramOsc2Octave, 1.0f },
                         { Parameters::paramOsc1Gain, 0.62f },
                         { Parameters::paramOsc2Gain, 0.48f },
                         { Parameters::paramLPFCutoff, 1800.0f },
                         { Parameters::paramLPFRes, 0.34f },
                         { Parameters::paramAmpAttack, 1.15f },
                         { Parameters::paramAmpDecay, 0.72f },
                         { Parameters::paramAmpSustain, 0.82f },
                         { Parameters::paramAmpRelease, 2.45f },
                         { Parameters::paramMixerDrive, 0.18f },
                         { Parameters::paramTapeDelayEnable, 1.0f } } });
    specs.push_back ({ "Plucky Stab",
                       "Short percussive stabs with a fast filter envelope.",
                       PresetCategory::pluck,
                       juce::StringArray { "pluck", "stab", "percussive" },
                       { { Parameters::paramVoiceMode, 1.0f },
                         { Parameters::paramOsc1Waveform, 1.0f },
                         { Parameters::paramOsc2Waveform, 0.0f },
                         { Parameters::paramOsc1Octave, 1.0f },
                         { Parameters::paramOsc2Octave, 0.0f },
                         { Parameters::paramOsc1Gain, 0.72f },
                         { Parameters::paramOsc2Gain, 0.42f },
                         { Parameters::paramLPFCutoff, 2600.0f },
                         { Parameters::paramLPFRes, 0.42f },
                         { Parameters::paramAmpAttack, 0.003f },
                         { Parameters::paramAmpDecay, 0.18f },
                         { Parameters::paramAmpSustain, 0.28f },
                         { Parameters::paramAmpRelease, 0.08f },
                         { Parameters::paramMixerDrive, 0.22f },
                         { Parameters::paramTapeDelayEnable, 1.0f } } });
    specs.push_back ({ "Noise FX",
                       "Filtered noise texture with delay feedback and tape character.",
                       PresetCategory::fx,
                       juce::StringArray { "noise", "fx", "texture", "delay" },
                       { { Parameters::paramOsc1Gain, 0.0f },
                         { Parameters::paramOsc2Gain, 0.0f },
                         { Parameters::paramNoiseGain, 0.82f },
                         { Parameters::paramNoiseType, 2.0f },
                         { Parameters::paramHPFCutoff, 340.0f },
                         { Parameters::paramLPFCutoff, 7200.0f },
                         { Parameters::paramLPFRes, 0.36f },
                         { Parameters::paramAmpAttack, 0.04f },
                         { Parameters::paramAmpDecay, 0.62f },
                         { Parameters::paramAmpSustain, 0.62f },
                         { Parameters::paramAmpRelease, 0.48f },
                         { Parameters::paramMixerDrive, 0.48f },
                         { Parameters::paramTapeDelayEnable, 1.0f },
                         { Parameters::paramTapeDelayTime, 380.0f },
                         { Parameters::paramTapeDelayFeedback, 0.62f } } });
    specs.push_back ({ "Arp Sequence",
                       "Monophonic sequence-ready patch with a rhythmic filter shape.",
                       PresetCategory::arp,
                       juce::StringArray { "arp", "sequence", "rhythmic", "mono" },
                       { { Parameters::paramVoiceMode, 1.0f },
                         { Parameters::paramOsc1Waveform, 0.0f },
                         { Parameters::paramOsc2Waveform, 1.0f },
                         { Parameters::paramOsc1Octave, 0.0f },
                         { Parameters::paramOsc2Octave, 1.0f },
                         { Parameters::paramGlideTime, 0.08f },
                         { Parameters::paramLFO1Rate, 0.32f },
                         { Parameters::paramLFO1Depth, 0.24f },
                         { Parameters::paramLPFCutoff, 2200.0f },
                         { Parameters::paramLPFRes, 0.48f },
                         { Parameters::paramAmpAttack, 0.008f },
                         { Parameters::paramAmpDecay, 0.22f },
                         { Parameters::paramAmpSustain, 0.55f },
                         { Parameters::paramAmpRelease, 0.12f },
                         { Parameters::paramMixerDrive, 0.28f },
                         { Parameters::paramTapeDelayEnable, 1.0f },
                         { Parameters::paramTapeDelayTime, 250.0f },
                         { Parameters::paramTapeDelayFeedback, 0.34f } } });
    specs.push_back ({ "Sub Furnace",
        "Pure sine -2oct + sub, LPF 120Hz/0res, sustain 0.9. Deep sub.",
        PresetCategory::bass, { "sub","deep","808" },
        { { Parameters::paramOsc1Waveform,4.0f },{ Parameters::paramOsc1Octave,-2.0f },{ Parameters::paramOsc1Gain,0.95f },
          { Parameters::paramMixerVco2Level,0.0f },{ Parameters::paramMixerSubLevel,1.0f },{ Parameters::paramSubOctave,-1.0f },{ Parameters::paramSubGain,1.0f },
          { Parameters::paramLPFCutoff,120.0f },{ Parameters::paramLPFRes,0.0f },
          { Parameters::paramAmpAttack,0.002f },{ Parameters::paramAmpDecay,0.35f },{ Parameters::paramAmpSustain,0.90f },{ Parameters::paramAmpRelease,0.15f },
          { Parameters::paramMixerDrive,0.0f },{ Parameters::paramTapeDelayEnable,0.0f } } });
    specs.push_back ({ "Chrome Bass",
        "Square+saw -1oct, LPF 380Hz/res0.62, sub layer. Moog-style.",
        PresetCategory::bass, { "moog","fat","square" },
        { { Parameters::paramOsc1Waveform,2.0f },{ Parameters::paramOsc1Octave,-1.0f },{ Parameters::paramOsc1Gain,0.95f },
          { Parameters::paramOsc2Waveform,1.0f },{ Parameters::paramOsc2Octave,-1.0f },{ Parameters::paramOsc2Gain,0.40f },
          { Parameters::paramMixerSubLevel,0.3f },{ Parameters::paramSubOctave,-1.0f },{ Parameters::paramSubGain,0.5f },
          { Parameters::paramLPFCutoff,380.0f },{ Parameters::paramLPFRes,0.62f },
          { Parameters::paramAmpAttack,0.003f },{ Parameters::paramAmpDecay,0.28f },{ Parameters::paramAmpSustain,0.75f },{ Parameters::paramAmpRelease,0.08f },
          { Parameters::paramMixerDrive,0.05f },{ Parameters::paramTapeDelayEnable,0.0f } } });
    specs.push_back ({ "Acid Burn",
        "Saw, LPF res0.95, env sweeps cutoff, no sub. 303 squelch.",
        PresetCategory::bass, { "acid","303","resonant" },
        { { Parameters::paramOsc1Waveform,1.0f },{ Parameters::paramOsc1Octave,-1.0f },{ Parameters::paramOsc1Gain,1.0f },
          { Parameters::paramMixerVco2Level,0.0f },{ Parameters::paramMixerSubLevel,0.0f },
          { Parameters::paramLPFCutoff,650.0f },{ Parameters::paramLPFRes,0.95f },
          { Parameters::paramEnv1Attack,0.002f },{ Parameters::paramEnv1Decay,0.65f },{ Parameters::paramEnv1Sustain,0.0f },{ Parameters::paramEnv1Release,0.30f },
          { Parameters::paramAmpAttack,0.003f },{ Parameters::paramAmpDecay,0.35f },{ Parameters::paramAmpSustain,0.65f },{ Parameters::paramAmpRelease,0.06f },
          { Parameters::paramMixerDrive,0.30f },{ Parameters::paramTapeDelayEnable,0.0f } } });
    specs.push_back ({ "Static Bass",
        "Two saws detuned 7c, drive 0.7, cutoff 1800Hz. Reese.",
        PresetCategory::bass, { "reese","dirty","detuned" },
        { { Parameters::paramOsc1Waveform,1.0f },{ Parameters::paramOsc1Octave,-1.0f },{ Parameters::paramOsc1Gain,0.85f },
          { Parameters::paramOsc2Waveform,1.0f },{ Parameters::paramOsc2Octave,-1.0f },{ Parameters::paramOsc2Gain,0.85f },{ Parameters::paramOsc2Tune,7.0f },
          { Parameters::paramMixerSubLevel,0.4f },{ Parameters::paramSubOctave,-1.0f },{ Parameters::paramSubGain,0.4f },
          { Parameters::paramLPFCutoff,1800.0f },{ Parameters::paramLPFRes,0.18f },
          { Parameters::paramAmpAttack,0.005f },{ Parameters::paramAmpDecay,0.30f },{ Parameters::paramAmpSustain,0.72f },{ Parameters::paramAmpRelease,0.12f },
          { Parameters::paramMixerDrive,0.70f },{ Parameters::paramTapeDelayEnable,0.0f } } });
    specs.push_back ({ "Punch Bass",
        "Pulse 25%PW + sub, sustain ZERO, 80ms decay. Pluck.",
        PresetCategory::bass, { "pluck","punch","staccato" },
        { { Parameters::paramOsc1Waveform,3.0f },{ Parameters::paramOsc1PulseWidth,25.0f },{ Parameters::paramOsc1Octave,0.0f },{ Parameters::paramOsc1Gain,0.88f },
          { Parameters::paramMixerVco2Level,0.0f },{ Parameters::paramMixerSubLevel,0.6f },{ Parameters::paramSubOctave,-1.0f },{ Parameters::paramSubGain,0.6f },
          { Parameters::paramLPFCutoff,750.0f },{ Parameters::paramLPFRes,0.20f },
          { Parameters::paramAmpAttack,0.001f },{ Parameters::paramAmpDecay,0.08f },{ Parameters::paramAmpSustain,0.0f },{ Parameters::paramAmpRelease,0.03f },
          { Parameters::paramMixerDrive,0.12f },{ Parameters::paramTapeDelayEnable,0.0f } } });
    // ── LEAD: mono, high cutoff, expressive ─────────────────────────────────
    specs.push_back ({ "Neon Lead",
        "Mono saw+sq, cutoff 15kHz, glide 120ms, tape. Bright.",
        PresetCategory::lead, { "bright","mono","glide" },
        { { Parameters::paramVoiceMode,1.0f },
          { Parameters::paramOsc1Waveform,1.0f },{ Parameters::paramOsc1Gain,0.80f },
          { Parameters::paramOsc2Waveform,2.0f },{ Parameters::paramOsc2Gain,0.55f },
          { Parameters::paramGlideTime,0.12f },
          { Parameters::paramLPFCutoff,15000.0f },{ Parameters::paramLPFRes,0.15f },
          { Parameters::paramAmpAttack,0.005f },{ Parameters::paramAmpDecay,0.25f },{ Parameters::paramAmpSustain,0.78f },{ Parameters::paramAmpRelease,0.22f },
          { Parameters::paramMixerDrive,0.18f },{ Parameters::paramTapeDelayEnable,1.0f },{ Parameters::paramTapeDelayTime,280.0f },{ Parameters::paramTapeDelayMix,0.30f } } });
    specs.push_back ({ "Voltage Lead",
        "Mono tri+saw -5c, cutoff 2500Hz/res0.3, 15ms attack. Smooth.",
        PresetCategory::lead, { "smooth","analog","warm" },
        { { Parameters::paramVoiceMode,1.0f },
          { Parameters::paramOsc1Waveform,0.0f },{ Parameters::paramOsc1Gain,0.78f },
          { Parameters::paramOsc2Waveform,1.0f },{ Parameters::paramOsc2Gain,0.50f },{ Parameters::paramOsc2Tune,-5.0f },
          { Parameters::paramLPFCutoff,2500.0f },{ Parameters::paramLPFRes,0.30f },
          { Parameters::paramAmpAttack,0.015f },{ Parameters::paramAmpDecay,0.45f },{ Parameters::paramAmpSustain,0.82f },{ Parameters::paramAmpRelease,0.35f },
          { Parameters::paramMixerDrive,0.05f },{ Parameters::paramTapeDelayEnable,1.0f } } });
    specs.push_back ({ "Screaming Wire",
        "Mono dual saw, res0.75, drive0.6. Aggressive lead.",
        PresetCategory::lead, { "aggressive","drive","saw" },
        { { Parameters::paramVoiceMode,1.0f },
          { Parameters::paramOsc1Waveform,1.0f },{ Parameters::paramOsc1Gain,0.92f },
          { Parameters::paramOsc2Waveform,1.0f },{ Parameters::paramOsc2Gain,0.88f },
          { Parameters::paramLPFCutoff,5500.0f },{ Parameters::paramLPFRes,0.75f },
          { Parameters::paramAmpAttack,0.003f },{ Parameters::paramAmpDecay,0.30f },{ Parameters::paramAmpSustain,0.72f },{ Parameters::paramAmpRelease,0.12f },
          { Parameters::paramMixerDrive,0.60f },{ Parameters::paramTapeDelayEnable,1.0f } } });
    specs.push_back ({ "Ghost Lead",
        "Mono PWM 45%, LFO modulates PW. Vintage emotional lead.",
        PresetCategory::lead, { "vintage","pwm","emotional" },
        { { Parameters::paramVoiceMode,1.0f },
          { Parameters::paramOsc1Waveform,2.0f },{ Parameters::paramOsc1PulseWidth,45.0f },{ Parameters::paramOsc1Gain,0.75f },
          { Parameters::paramLPFCutoff,4200.0f },{ Parameters::paramLPFRes,0.28f },
          { Parameters::paramLFO1Rate,0.18f },{ Parameters::paramLFO1Depth,0.35f },{ Parameters::paramLFO1Dest,3.0f },
          { Parameters::paramAmpAttack,0.006f },{ Parameters::paramAmpDecay,0.28f },{ Parameters::paramAmpSustain,0.72f },{ Parameters::paramAmpRelease,0.18f },
          { Parameters::paramMixerDrive,0.12f },{ Parameters::paramTapeDelayEnable,1.0f } } });
    specs.push_back ({ "Phase Lead",
        "Mono saw+glide 180ms, LFO wobbles filter. Ethereal.",
        PresetCategory::lead, { "portamento","glide","dreamy" },
        { { Parameters::paramVoiceMode,1.0f },
          { Parameters::paramOsc1Waveform,1.0f },{ Parameters::paramOsc1Gain,0.82f },
          { Parameters::paramGlideTime,0.18f },
          { Parameters::paramLPFCutoff,3200.0f },{ Parameters::paramLPFRes,0.25f },
          { Parameters::paramLFO1Rate,0.20f },{ Parameters::paramLFO1Depth,0.40f },{ Parameters::paramLFO1Dest,9.0f },
          { Parameters::paramAmpAttack,0.008f },{ Parameters::paramAmpDecay,0.35f },{ Parameters::paramAmpSustain,0.78f },{ Parameters::paramAmpRelease,0.25f },
          { Parameters::paramMixerDrive,0.15f },{ Parameters::paramTapeDelayEnable,1.0f } } });
    // PAD
    specs.push_back ({ "Night Drive",
        "Detuned saws -5c, attack 1.2s, release 2.5s. Warm analog pad.",
        PresetCategory::pad, { "warm","analog","slow" },
        { { Parameters::paramOsc1Waveform,1.0f },{ Parameters::paramOsc1Gain,0.65f },
          { Parameters::paramOsc2Waveform,1.0f },{ Parameters::paramOsc2Gain,0.55f },{ Parameters::paramOsc2Tune,-5.0f },
          { Parameters::paramLPFCutoff,2500.0f },{ Parameters::paramLPFRes,0.20f },
          { Parameters::paramAmpAttack,1.2f },{ Parameters::paramAmpDecay,0.80f },{ Parameters::paramAmpSustain,0.80f },{ Parameters::paramAmpRelease,2.5f },
          { Parameters::paramMixerDrive,0.08f },{ Parameters::paramTapeDelayEnable,1.0f } } });
    specs.push_back ({ "Ghost Choir",
        "Triangles -10c apart, cutoff 800Hz, attack 1.5s, release 3s.",
        PresetCategory::pad, { "dark","cinematic" },
        { { Parameters::paramOsc1Waveform,0.0f },{ Parameters::paramOsc1Octave,-1.0f },{ Parameters::paramOsc1Gain,0.60f },
          { Parameters::paramOsc2Waveform,0.0f },{ Parameters::paramOsc2Gain,0.50f },{ Parameters::paramOsc2Tune,-10.0f },
          { Parameters::paramLPFCutoff,800.0f },{ Parameters::paramLPFRes,0.35f },
          { Parameters::paramAmpAttack,1.5f },{ Parameters::paramAmpDecay,1.00f },{ Parameters::paramAmpSustain,0.75f },{ Parameters::paramAmpRelease,3.0f },
          { Parameters::paramMixerDrive,0.05f },{ Parameters::paramTapeDelayEnable,1.0f } } });
    // BRASS
    specs.push_back ({ "Blackout Brass",
        "Saw+sq, fast attack, res0.5 filter sweep. Brass stab.",
        PresetCategory::brass, { "brass","stab","analog" },
        { { Parameters::paramOsc1Waveform,1.0f },{ Parameters::paramOsc1Gain,0.85f },
          { Parameters::paramOsc2Waveform,2.0f },{ Parameters::paramOsc2Gain,0.55f },
          { Parameters::paramLPFCutoff,2500.0f },{ Parameters::paramLPFRes,0.50f },
          { Parameters::paramAmpAttack,0.003f },{ Parameters::paramAmpDecay,0.25f },{ Parameters::paramAmpSustain,0.55f },{ Parameters::paramAmpRelease,0.08f },
          { Parameters::paramMixerDrive,0.15f },{ Parameters::paramTapeDelayEnable,1.0f } } });
    // PLUCK
    specs.push_back ({ "Analog Ritual",
        "Pulse 40%PW+tri, attack 2ms, decay 100ms, sustain 0.",
        PresetCategory::pluck, { "pluck","analog","fast" },
        { { Parameters::paramOsc1Waveform,3.0f },{ Parameters::paramOsc1PulseWidth,40.0f },{ Parameters::paramOsc1Gain,0.80f },
          { Parameters::paramOsc2Waveform,0.0f },{ Parameters::paramOsc2Octave,1.0f },{ Parameters::paramOsc2Gain,0.50f },
          { Parameters::paramLPFCutoff,5000.0f },{ Parameters::paramLPFRes,0.30f },
          { Parameters::paramAmpAttack,0.002f },{ Parameters::paramAmpDecay,0.10f },{ Parameters::paramAmpSustain,0.0f },{ Parameters::paramAmpRelease,0.04f },
          { Parameters::paramMixerDrive,0.12f },{ Parameters::paramTapeDelayEnable,1.0f } } });
    // KEYS
    specs.push_back ({ "Broken Signal",
        "Sq+tri, attack 3ms, decay 400ms, sustain 0. Synth piano.",
        PresetCategory::keys, { "piano","keys","percussive" },
        { { Parameters::paramOsc1Waveform,2.0f },{ Parameters::paramOsc1Gain,0.80f },
          { Parameters::paramOsc2Waveform,0.0f },{ Parameters::paramOsc2Octave,1.0f },{ Parameters::paramOsc2Gain,0.55f },
          { Parameters::paramLPFCutoff,5000.0f },{ Parameters::paramLPFRes,0.15f },
          { Parameters::paramAmpAttack,0.003f },{ Parameters::paramAmpDecay,0.40f },{ Parameters::paramAmpSustain,0.0f },{ Parameters::paramAmpRelease,0.30f },
          { Parameters::paramMixerDrive,0.05f },{ Parameters::paramTapeDelayEnable,1.0f } } });
    // STRINGS
    specs.push_back ({ "Analog Strings",
        "Dual saws -3c, attack 500ms, release 1.2s. Warm strings.",
        PresetCategory::strings, { "strings","warm","analog" },
        { { Parameters::paramOsc1Waveform,1.0f },{ Parameters::paramOsc1Gain,0.70f },
          { Parameters::paramOsc2Waveform,1.0f },{ Parameters::paramOsc2Gain,0.60f },{ Parameters::paramOsc2Tune,-3.0f },
          { Parameters::paramLPFCutoff,4000.0f },{ Parameters::paramLPFRes,0.20f },
          { Parameters::paramAmpAttack,0.50f },{ Parameters::paramAmpDecay,0.60f },{ Parameters::paramAmpSustain,0.75f },{ Parameters::paramAmpRelease,1.2f },
          { Parameters::paramMixerDrive,0.10f },{ Parameters::paramTapeDelayEnable,1.0f } } });
    // BELLS
    specs.push_back ({ "Glass Bell",
        "Sine+ringmod, fast decay 500ms, sustain 0. Bright bell.",
        PresetCategory::bells, { "bell","glass","bright" },
        { { Parameters::paramOsc1Waveform,4.0f },{ Parameters::paramOsc1Octave,2.0f },{ Parameters::paramOsc1Gain,0.75f },
          { Parameters::paramOsc2Waveform,7.0f },{ Parameters::paramOsc2Octave,3.0f },{ Parameters::paramOsc2Gain,0.60f },
          { Parameters::paramLPFCutoff,12000.0f },{ Parameters::paramLPFRes,0.05f },
          { Parameters::paramAmpAttack,0.002f },{ Parameters::paramAmpDecay,0.50f },{ Parameters::paramAmpSustain,0.0f },{ Parameters::paramAmpRelease,0.40f },
          { Parameters::paramMixerDrive,0.0f },{ Parameters::paramTapeDelayEnable,1.0f } } });
    // FX
    specs.push_back ({ "Filter Sweep",
        "White noise through resonant filter, LFO sweeps. FX.",
        PresetCategory::fx, { "sweep","noise","fx" },
        { { Parameters::paramMixerVco1Level,0.0f },{ Parameters::paramMixerVco2Level,0.0f },{ Parameters::paramMixerSubLevel,0.0f },
          { Parameters::paramNoiseGain,0.90f },{ Parameters::paramNoiseType,2.0f },
          { Parameters::paramLPFCutoff,5000.0f },{ Parameters::paramLPFRes,0.60f },
          { Parameters::paramLFO1Rate,0.25f },{ Parameters::paramLFO1Depth,0.70f },{ Parameters::paramLFO1Dest,9.0f },
          { Parameters::paramAmpAttack,0.01f },{ Parameters::paramAmpDecay,0.50f },{ Parameters::paramAmpSustain,0.80f },{ Parameters::paramAmpRelease,0.50f },
          { Parameters::paramMixerDrive,0.30f },{ Parameters::paramTapeDelayEnable,1.0f } } });
    // NOISE
    specs.push_back ({ "Whiteout",
        "White noise, open filter, zero drive.",
        PresetCategory::noise, { "white","noise","bright" },
        { { Parameters::paramMixerVco1Level,0.0f },{ Parameters::paramMixerVco2Level,0.0f },{ Parameters::paramMixerSubLevel,0.0f },
          { Parameters::paramNoiseGain,0.95f },{ Parameters::paramNoiseType,2.0f },
          { Parameters::paramLPFCutoff,20000.0f },{ Parameters::paramLPFRes,0.0f },
          { Parameters::paramAmpAttack,0.001f },{ Parameters::paramAmpDecay,0.10f },{ Parameters::paramAmpSustain,1.0f },{ Parameters::paramAmpRelease,0.05f },
          { Parameters::paramMixerDrive,0.0f },{ Parameters::paramTapeDelayEnable,0.0f } } });
    // ARP
    specs.push_back ({ "Rapid Fire",
        "Mono saw+sq, fast LFO on filter, tape 150ms.",
        PresetCategory::arp, { "fast","arp","rhythmic" },
        { { Parameters::paramVoiceMode,1.0f },
          { Parameters::paramOsc1Waveform,1.0f },{ Parameters::paramOsc1Gain,0.75f },
          { Parameters::paramOsc2Waveform,2.0f },{ Parameters::paramOsc2Octave,1.0f },{ Parameters::paramOsc2Gain,0.50f },
          { Parameters::paramLPFCutoff,6000.0f },{ Parameters::paramLPFRes,0.20f },
          { Parameters::paramLFO1Rate,0.50f },{ Parameters::paramLFO1Depth,0.30f },{ Parameters::paramLFO1Dest,9.0f },
          { Parameters::paramAmpAttack,0.003f },{ Parameters::paramAmpDecay,0.15f },{ Parameters::paramAmpSustain,0.50f },{ Parameters::paramAmpRelease,0.06f },
          { Parameters::paramTapeDelayTime,150.0f },{ Parameters::paramTapeDelayFeedback,0.30f },
          { Parameters::paramMixerDrive,0.15f },{ Parameters::paramTapeDelayEnable,1.0f } } });
    // SEQ
    specs.push_back ({ "Pulse Sequence",
        "Mono pulse 30%PW, LFO chops filter. Rhythmic.",
        PresetCategory::sequence, { "rhythmic","pulse","sequence" },
        { { Parameters::paramVoiceMode,1.0f },
          { Parameters::paramOsc1Waveform,3.0f },{ Parameters::paramOsc1PulseWidth,30.0f },{ Parameters::paramOsc1Gain,0.75f },
          { Parameters::paramMixerVco2Level,0.0f },
          { Parameters::paramLPFCutoff,5000.0f },{ Parameters::paramLPFRes,0.25f },
          { Parameters::paramLFO1Rate,0.40f },{ Parameters::paramLFO1Depth,0.35f },{ Parameters::paramLFO1Dest,9.0f },
          { Parameters::paramAmpAttack,0.003f },{ Parameters::paramAmpDecay,0.12f },{ Parameters::paramAmpSustain,0.0f },{ Parameters::paramAmpRelease,0.05f },
          { Parameters::paramTapeDelayTime,180.0f },{ Parameters::paramTapeDelayFeedback,0.30f },{ Parameters::paramTapeDelayMix,0.40f },
          { Parameters::paramMixerDrive,0.10f },{ Parameters::paramTapeDelayEnable,1.0f } } });
    // PERCUSSION
    specs.push_back ({ "Analog Kick",
        "Saw+noise, sub -2oct, very fast envelope. Kick.",
        PresetCategory::percussion, { "kick","drum","thump" },
        { { Parameters::paramVoiceMode,1.0f },
          { Parameters::paramOsc1Waveform,5.0f },{ Parameters::paramOsc1Octave,-2.0f },{ Parameters::paramOsc1Gain,0.90f },
          { Parameters::paramMixerVco2Level,0.0f },{ Parameters::paramMixerSubLevel,0.0f },
          { Parameters::paramNoiseGain,0.50f },{ Parameters::paramNoiseType,2.0f },
          { Parameters::paramLPFCutoff,80.0f },{ Parameters::paramLPFRes,0.0f },
          { Parameters::paramAmpAttack,0.001f },{ Parameters::paramAmpDecay,0.08f },{ Parameters::paramAmpSustain,0.0f },{ Parameters::paramAmpRelease,0.04f },
          { Parameters::paramMixerDrive,0.50f },{ Parameters::paramTapeDelayEnable,0.0f } } });
    // POLY
    specs.push_back ({ "Poly Vapor",
        "Saw+supersaw, attack 500ms, release 1.5s, tape.",
        PresetCategory::polysynth, { "poly","wide","warm" },
        { { Parameters::paramOsc1Waveform,1.0f },{ Parameters::paramOsc1Gain,0.70f },
          { Parameters::paramOsc2Waveform,6.0f },{ Parameters::paramOsc2Gain,0.60f },
          { Parameters::paramLPFCutoff,4000.0f },{ Parameters::paramLPFRes,0.15f },
          { Parameters::paramAmpAttack,0.50f },{ Parameters::paramAmpDecay,0.60f },{ Parameters::paramAmpSustain,0.80f },{ Parameters::paramAmpRelease,1.5f },
          { Parameters::paramTapeDelayTime,320.0f },{ Parameters::paramTapeDelayFeedback,0.30f },{ Parameters::paramTapeDelayMix,0.35f },
          { Parameters::paramMixerDrive,0.08f },{ Parameters::paramTapeDelayEnable,1.0f } } });
    // MONO
    specs.push_back ({ "Mono Thunder",
        "Unison saws -1oct, glide 150ms, drive 0.65. Massive.",
        PresetCategory::monosynth, { "mono","unison","thick" },
        { { Parameters::paramVoiceMode,2.0f },
          { Parameters::paramOsc1Waveform,1.0f },{ Parameters::paramOsc1Octave,-1.0f },{ Parameters::paramOsc1Gain,0.90f },
          { Parameters::paramOsc2Waveform,1.0f },{ Parameters::paramOsc2Octave,-1.0f },{ Parameters::paramOsc2Gain,0.80f },
          { Parameters::paramGlideTime,0.15f },
          { Parameters::paramLPFCutoff,1200.0f },{ Parameters::paramLPFRes,0.30f },
          { Parameters::paramAmpAttack,0.008f },{ Parameters::paramAmpDecay,0.45f },{ Parameters::paramAmpSustain,0.80f },{ Parameters::paramAmpRelease,0.25f },
          { Parameters::paramMixerDrive,0.65f },{ Parameters::paramTapeDelayEnable,1.0f } } });

    const auto now = juce::Time::getCurrentTime();
    for (const auto& spec : specs)
    {
        PresetRecord record;
        record.info.id = juce::Uuid().toString();
        record.info.name = spec.name;
        record.info.description = spec.description;
        record.info.author = "Ghost Signal";
        record.info.category = spec.category;
        record.info.tags = spec.tags;
        record.info.index = -1;
        record.info.created = now;
        record.info.modified = now;
        record.info.date = now;
        record.info.favorite = false;
        record.info.factory = true;
        record.info.fileName = record.info.name + ".factory";
        record.state = createFactoryState (spec);
        factoryPresets.push_back (std::move (record));
    }

    reindex();
}

void PresetManager::refreshUserPresets()
{
    userPresets.clear();
    if (!presetFolder.isDirectory())
        return;

    juce::Array<juce::File> files;
    presetFolder.findChildFiles (files, juce::File::findFiles, false, "*.preset");

    struct FileModifiedComparator
    {
        int compareElements (const juce::File& a, const juce::File& b) const noexcept
        {
            return a.getLastModificationTime() == b.getLastModificationTime()
                       ? 0
                       : (a.getLastModificationTime() > b.getLastModificationTime() ? -1 : 1);
        }
    };

    FileModifiedComparator comparator;
    files.sort (comparator);

    for (const auto& file : files)
    {
        PresetRecord record;
        if (readPresetFile (file, record))
            userPresets.push_back (std::move (record));
        else
            lastError = "Skipped unreadable preset: " + file.getFileName();
    }

    reindex();

    if (currentPresetIndex >= 0 && currentPresetIndex < getNumPresets())
    {
        const auto id = getPresetInfo (currentPresetIndex).id;
        int found = -1;
        for (int i = 0; i < getNumPresets(); ++i)
        {
            if (getPresetInfo (i).id.compareIgnoreCase (id) == 0)
            {
                found = i;
                break;
            }
        }
        currentPresetIndex = found;
        currentPresetName = found >= 0 ? getPresetInfo (found).name : "Untitled Patch";
    }
    else
    {
        clearCurrentPreset();
    }
}

bool PresetManager::loadPreset (int index)
{
    if (index < 0 || index >= getNumPresets() || apvts == nullptr)
    {
        lastError = "Preset index is out of range.";
        reportStatus (lastError, false);
        return false;
    }

    const auto* record = getPresetRecord (index);
    if (record == nullptr || !record->state.isValid())
    {
        lastError = "Preset state is unavailable.";
        reportStatus (lastError, false);
        return false;
    }

    if (!applyState (record->state))
    {
        reportStatus (lastError, false);
        return false;
    }

    currentPresetIndex = index;
    currentPresetName = record->info.name;
    reportStatus (record->info.name + " loaded.", true);
    notifyChange();
    return true;
}

bool PresetManager::savePreset (const juce::String& name,
                                const juce::String& description,
                                const juce::StringArray& tags,
                                int overwriteIndex)
{
    if (apvts == nullptr)
    {
        lastError = "Preset storage is not ready.";
        reportStatus (lastError, false);
        return false;
    }

    auto cleanName = name.trim();
    for (int i = 0; i < cleanName.length(); ++i)
    {
        if (cleanName[i] < 32)
        {
            lastError = "Preset names cannot contain control characters.";
            reportStatus (lastError, false);
            return false;
        }
    }

    if (cleanName.isEmpty() || cleanName.length() > kPresetNameLimit)
    {
        lastError = "Enter a preset name between 1 and " + juce::String (kPresetNameLimit) + " characters.";
        reportStatus (lastError, false);
        return false;
    }

    auto cleanDescription = description.trim();
    if (cleanDescription.length() > kDescriptionLimit)
        cleanDescription = cleanDescription.substring (0, kDescriptionLimit);

    auto parsedTags = cleanTags (tags);
    auto state = apvts->copyState();
    state.setProperty ("presetFormatVersion", 1, nullptr);

    int userIndex = -1;
    if (overwriteIndex >= 0 && overwriteIndex < getNumPresets())
    {
        if (isFactoryPreset (overwriteIndex))
        {
            lastError = "Factory presets cannot be overwritten.";
            reportStatus (lastError, false);
            return false;
        }
        userIndex = overwriteIndex - getNumFactoryPresets();
        const auto& existing = userPresets[static_cast<size_t> (userIndex)];
        if (existing.info.name.compareIgnoreCase (cleanName) != 0)
        {
            for (size_t i = 0; i < userPresets.size(); ++i)
            {
                if (i != static_cast<size_t> (userIndex)
                    && userPresets[i].info.name.compareIgnoreCase (cleanName) == 0)
                {
                    lastError = "A user preset with this name already exists.";
                    reportStatus (lastError, false);
                    return false;
                }
            }
        }
    }
    else
    {
        for (size_t i = 0; i < userPresets.size(); ++i)
        {
            if (userPresets[i].info.name.compareIgnoreCase (cleanName) == 0)
            {
                lastError = "A user preset with this name already exists.";
                reportStatus (lastError, false);
                return false;
            }
        }
    }

    PresetRecord record;
    if (userIndex >= 0)
    {
        record = userPresets[static_cast<size_t> (userIndex)];
        record.info.name = cleanName;
        record.info.description = cleanDescription;
        record.info.tags = parsedTags;
        record.info.modified = juce::Time::getCurrentTime();
        record.info.date = record.info.modified;
        record.state = state;
        if (!writePresetFile (record))
        {
            reportStatus (lastError, false);
            return false;
        }
        userPresets[static_cast<size_t> (userIndex)] = std::move (record);
    }
    else
    {
        const auto now = juce::Time::getCurrentTime();
        record.info.id = juce::Uuid().toString();
        record.info.name = cleanName;
        record.info.description = cleanDescription;
        record.info.author = "Ghost Signal";
        record.info.category = PresetCategory::user;
        record.info.tags = parsedTags;
        record.info.created = now;
        record.info.modified = now;
        record.info.date = now;
        record.info.factory = false;
        record.info.fileName = record.info.id + ".preset";
        record.state = state;
        record.file = presetFolder.getChildFile (record.info.fileName);
        if (!writePresetFile (record))
        {
            reportStatus (lastError, false);
            return false;
        }
        userPresets.push_back (std::move (record));
    }

    reindex();
    currentPresetIndex = getNumFactoryPresets()
                         + (userIndex >= 0 ? userIndex : static_cast<int> (userPresets.size() - 1));
    currentPresetName = cleanName;
    reportStatus (cleanName + " saved.", true);
    notifyChange();
    return true;
}

bool PresetManager::savePreset (int index)
{
    if (index < 0 || index >= getNumPresets() || isFactoryPreset (index))
        return false;

    const auto& info = getPresetInfo (index);
    return savePreset (info.name, info.description, info.tags, index);
}

void PresetManager::createPreset (const juce::String& name)
{
    savePreset (name);
}

bool PresetManager::renamePreset (int index,
                                  const juce::String& newName,
                                  const juce::String& newDescription,
                                  const juce::StringArray& newTags)
{
    if (index < getNumFactoryPresets() || index >= getNumPresets())
    {
        lastError = "Factory presets cannot be renamed.";
        reportStatus (lastError, false);
        return false;
    }

    auto cleanName = newName.trim();
    for (int i = 0; i < cleanName.length(); ++i)
    {
        if (cleanName[i] < 32)
        {
            lastError = "Preset names cannot contain control characters.";
            reportStatus (lastError, false);
            return false;
        }
    }

    if (cleanName.isEmpty() || cleanName.length() > kPresetNameLimit)
    {
        lastError = "Enter a preset name between 1 and " + juce::String (kPresetNameLimit) + " characters.";
        reportStatus (lastError, false);
        return false;
    }

    auto cleanDescription = newDescription.trim();
    if (cleanDescription.length() > kDescriptionLimit)
        cleanDescription = cleanDescription.substring (0, kDescriptionLimit);

    const int userIndex = index - getNumFactoryPresets();
    for (size_t i = 0; i < userPresets.size(); ++i)
    {
        if (i != static_cast<size_t> (userIndex)
            && userPresets[i].info.name.compareIgnoreCase (cleanName) == 0)
        {
            lastError = "A user preset with this name already exists.";
            reportStatus (lastError, false);
            return false;
        }
    }

    auto& record = userPresets[static_cast<size_t> (userIndex)];
    record.info.name = cleanName;
    record.info.description = cleanDescription;
    record.info.tags = cleanTags (newTags);
    record.info.modified = juce::Time::getCurrentTime();
    record.info.date = record.info.modified;
    if (!writePresetFile (record))
    {
        reportStatus (lastError, false);
        return false;
    }

    reindex();
    if (currentPresetIndex == index)
        currentPresetName = cleanName;
    reportStatus (cleanName + " renamed.", true);
    notifyChange();
    return true;
}

bool PresetManager::deletePreset (int index)
{
    if (index < getNumFactoryPresets() || index >= getNumPresets())
    {
        lastError = "Factory presets cannot be deleted.";
        reportStatus (lastError, false);
        return false;
    }

    const int userIndex = index - getNumFactoryPresets();
    const auto& record = userPresets[static_cast<size_t> (userIndex)];
    const auto deletedName = record.info.name;
    if (record.file.existsAsFile() && !record.file.deleteFile())
    {
        lastError = "Could not delete " + record.file.getFileName() + ".";
        reportStatus (lastError, false);
        return false;
    }

    const bool wasCurrent = index == currentPresetIndex;
    userPresets.erase (userPresets.begin() + userIndex);
    if (wasCurrent)
        clearCurrentPreset();
    else if (currentPresetIndex > index)
        --currentPresetIndex;

    reindex();
    reportStatus (deletedName + " deleted.", true);
    notifyChange();
    return true;
}

int PresetManager::getRandomPresetIndex() const
{
    if (getNumPresets() == 0)
        return -1;
    return juce::Random::getSystemRandom().nextInt (getNumPresets());
}

const PresetInfo& PresetManager::getPresetInfo (int index) const
{
    static PresetInfo empty;
    if (index < 0 || index >= getNumPresets())
        return empty;
    return getPresetRecord (index)->info;
}

const PresetRecord* PresetManager::getPresetRecord (int index) const
{
    if (index < 0 || index >= getNumPresets())
        return nullptr;
    if (index < getNumFactoryPresets())
        return &factoryPresets[static_cast<size_t> (index)];
    return &userPresets[static_cast<size_t> (index - getNumFactoryPresets())];
}

bool PresetManager::isFactoryPreset (int index) const
{
    return index >= 0 && index < getNumFactoryPresets();
}

juce::ValueTree PresetManager::getPresetState (int index) const
{
    const auto* record = getPresetRecord (index);
    return record == nullptr ? juce::ValueTree() : record->state.createCopy();
}

void PresetManager::clearCurrentPreset()
{
    currentPresetIndex = -1;
    currentPresetName = "Untitled Patch";
}

void PresetManager::setFavorite (int index, bool favorite)
{
    if (index < getNumFactoryPresets() || index >= getNumPresets())
    {
        lastError = "Factory favorites are read-only.";
        reportStatus (lastError, false);
        return;
    }

    auto& record = userPresets[static_cast<size_t> (index - getNumFactoryPresets())];
    record.info.favorite = favorite;
    if (!writePresetFile (record))
    {
        reportStatus (lastError, false);
        return;
    }
    reportStatus (record.info.name + (favorite ? " marked as favorite." : " removed from favorites."), true);
    notifyChange();
}

bool PresetManager::isFavorite (int index) const
{
    const auto* record = getPresetRecord (index);
    return record != nullptr && record->info.favorite;
}

int PresetManager::findPresetIndexByName (const juce::String& name) const
{
    for (int i = 0; i < getNumPresets(); ++i)
    {
        if (getPresetInfo (i).name.compareIgnoreCase (name) == 0)
            return i;
    }
    return -1;
}

void PresetManager::reindex()
{
    for (int i = 0; i < getNumFactoryPresets(); ++i)
        factoryPresets[static_cast<size_t> (i)].info.index = i;
    for (int i = 0; i < getNumUserPresets(); ++i)
        userPresets[static_cast<size_t> (i)].info.index = getNumFactoryPresets() + i;
}

void PresetManager::notifyChange()
{
    if (changeListener)
        changeListener();
}

void PresetManager::reportStatus (const juce::String& message, bool success)
{
    lastError = success ? juce::String() : message;
    if (statusListener)
        statusListener (message, success);
}

void PresetManager::setParameterValue (juce::ValueTree& state,
                                       const juce::String& parameterId,
                                       float value) const
{
    if (apvts == nullptr)
        return;

    auto child = state.getChildWithProperty ("id", parameterId);
    if (! child.isValid())
    {
        // An unknown parameter id must never fail silently: the override is
        // dropped, the preset loads as "nothing changed", and shipped sounds
        // appear broken. Break in debug builds and log in release builds.
        juce::Logger::writeToLog ("PresetManager: unknown parameter id '"
                                  + parameterId + "' in preset definition.");
        jassertfalse;
        return;
    }

    auto* parameter = apvts->getParameter (parameterId);
    juce::ignoreUnused (parameter);

    // The APVTS "value" property is stored in plain (denormalised) parameter
    // units: JUCE reads it back via setDenormalisedValue(). Storing a
    // normalised 0..1 value here would be reinterpreted as a plain value and
    // land wildly off target (e.g. a 120 Hz LPF cutoff would become 20 Hz).
    child.setProperty ("value", value, nullptr);
}

juce::ValueTree PresetManager::createFactoryState (const FactoryPresetSpec& spec) const
{
    if (apvts == nullptr)
        return {};

    // Start from the parameter defaults rather than the live state: factory
    // presets must be deterministic and "Init Patch" must be a true reset,
    // even when a preset is captured while the user has tweaked knobs.
    auto state = apvts->copyState();

    for (int i = 0; i < state.getNumChildren(); ++i)
    {
        auto child = state.getChild (i);
        const auto id = child.getProperty ("id").toString();
        if (id.isEmpty())
            continue;

        if (auto* parameter = apvts->getParameter (id))
        {
            // RangedAudioParameter::getDefaultValue() is normalised (0..1),
            // but the state's "value" property holds plain parameter units,
            // so convert the default back before storing it.
            child.setProperty ("value", parameter->convertFrom0to1 (parameter->getDefaultValue()), nullptr);
        }
    }

    state.setProperty ("presetFormatVersion", 1, nullptr);
    for (const auto& override : spec.overrides)
        setParameterValue (state, override.id, override.value);
    return state;
}

bool PresetManager::writePresetFile (const PresetRecord& record)
{
    presetFolder.createDirectory();

    juce::ValueTree root ("PRESET");
    root.setProperty ("formatVersion", 1, nullptr);
    root.setProperty ("id", record.info.id, nullptr);
    root.setProperty ("name", record.info.name, nullptr);
    root.setProperty ("description", record.info.description, nullptr);
    root.setProperty ("author", record.info.author, nullptr);
    root.setProperty ("category", static_cast<int> (record.info.category), nullptr);
    root.setProperty ("tags", record.info.tags.joinIntoString (", "), nullptr);
    root.setProperty ("created", static_cast<juce::int64> (record.info.created.toMilliseconds()), nullptr);
    root.setProperty ("modified", static_cast<juce::int64> (record.info.modified.toMilliseconds()), nullptr);
    root.setProperty ("favorite", record.info.favorite ? 1 : 0, nullptr);
    root.setProperty ("factory", record.info.factory ? 1 : 0, nullptr);

    juce::ValueTree stateNode ("STATE");
    stateNode.appendChild (record.state.createCopy(), nullptr);
    root.appendChild (stateNode, nullptr);

    auto xml = root.createXml();
    if (xml == nullptr)
    {
        lastError = "Could not serialize preset metadata.";
        return false;
    }

    const auto file = record.file.getFullPathName().isNotEmpty()
                          ? record.file
                          : presetFolder.getChildFile (record.info.id + ".preset");
    if (!file.replaceWithText (xml->toString()))
    {
        lastError = "Could not write " + file.getFileName() + ".";
        return false;
    }
    return true;
}

bool PresetManager::readPresetFile (const juce::File& file, PresetRecord& record) const
{
    std::unique_ptr<juce::XmlElement> xml (juce::XmlDocument::parse (file));
    if (xml == nullptr)
        return false;

    auto root = juce::ValueTree::fromXml (*xml);
    if (!root.isValid())
        return false;

    juce::ValueTree state;
    if (root.getType().toString() == "PARAMETERS")
    {
        state = root;
    }
    else
    {
        auto stateNode = root.getChildWithName ("STATE");
        if (stateNode.isValid() && stateNode.getNumChildren() > 0)
            state = stateNode.getChild (0);
    }

    if (!state.isValid())
        return false;

    const auto now = file.getLastModificationTime();
    record.info.id = root.getProperty ("id").toString();
    if (record.info.id.isEmpty())
        record.info.id = juce::Uuid().toString();
    record.info.name = root.getProperty ("name").toString();
    if (record.info.name.isEmpty())
        record.info.name = file.getFileNameWithoutExtension();
    record.info.description = root.getProperty ("description").toString();
    record.info.author = root.getProperty ("author").toString();
    const auto categoryValue = root.getProperty ("category");
    record.info.category = categoryValue.toString().isNotEmpty()
                               ? categoryFromName (categoryValue.toString(), PresetCategory::user)
                               : PresetCategory::user;
    record.info.tags = cleanTags (juce::StringArray::fromTokens (root.getProperty ("tags").toString(), ",", ""));
    record.info.created = juce::Time (root.getProperty ("created").toString().getLargeIntValue());
    record.info.modified = juce::Time (root.getProperty ("modified").toString().getLargeIntValue());
    if (record.info.created == juce::Time())
        record.info.created = now;
    if (record.info.modified == juce::Time())
        record.info.modified = now;
    record.info.date = record.info.modified;
    record.info.favorite = root.getProperty ("favorite").toString().getIntValue() != 0;
    record.info.factory = false;
    record.info.fileName = file.getFileName();
    record.state = state;
    record.file = file;
    return true;
}

juce::StringArray PresetManager::cleanTags (const juce::StringArray& tags)
{
    juce::StringArray result;
    for (const auto& tag : tags)
    {
        const auto parts = juce::StringArray::fromTokens (tag, ",;", "");
        for (const auto& part : parts)
        {
            const auto clean = part.trim().toLowerCase();
            if (clean.isNotEmpty() && !result.contains (clean, true) && result.size() < kTagLimit)
                result.add (clean);
        }
    }
    return result;
}

juce::String PresetManager::categoryName (PresetCategory category)
{
    switch (category)
    {
        case PresetCategory::init:  return "INIT";
        case PresetCategory::bass:  return "BASS";
        case PresetCategory::lead:  return "LEAD";
        case PresetCategory::pad:   return "PAD";
        case PresetCategory::pluck: return "PLUCK";
        case PresetCategory::brass: return "BRASS";
        case PresetCategory::keys: return "KEYS";
        case PresetCategory::strings: return "STRINGS";
        case PresetCategory::bells: return "BELLS";
        case PresetCategory::fx:    return "FX";
        case PresetCategory::noise: return "NOISE";
        case PresetCategory::sequence: return "SEQ";
        case PresetCategory::arp:   return "ARP";
        case PresetCategory::percussion: return "PERC";
        case PresetCategory::polysynth: return "POLY";
        case PresetCategory::monosynth: return "MONO";
        case PresetCategory::user:  return "USER";
        default:                    return "USER";
    }
}

PresetCategory PresetManager::categoryFromName (const juce::String& name, PresetCategory fallback)
{
    const auto lower = name.trim().toLowerCase();
    if (lower.containsOnly ("0123456789")
        && lower.getIntValue() >= static_cast<int> (PresetCategory::init)
        && lower.getIntValue() <= static_cast<int> (PresetCategory::user))
        return static_cast<PresetCategory> (lower.getIntValue());
    if (lower == "init") return PresetCategory::init;
    if (lower == "bass") return PresetCategory::bass;
    if (lower == "lead") return PresetCategory::lead;
    if (lower == "pad") return PresetCategory::pad;
    if (lower == "pluck") return PresetCategory::pluck;
    if (lower == "brass") return PresetCategory::brass;
    if (lower == "keys") return PresetCategory::keys;
    if (lower == "strings") return PresetCategory::strings;
    if (lower == "bells") return PresetCategory::bells;
    if (lower == "fx") return PresetCategory::fx;
    if (lower == "noise") return PresetCategory::noise;
    if (lower == "seq") return PresetCategory::sequence;
    if (lower == "sequence") return PresetCategory::sequence;
    if (lower == "arp") return PresetCategory::arp;
    if (lower == "perc") return PresetCategory::percussion;
    if (lower == "percussion") return PresetCategory::percussion;
    if (lower == "poly") return PresetCategory::polysynth;
    if (lower == "polysynth") return PresetCategory::polysynth;
    if (lower == "mono") return PresetCategory::monosynth;
    if (lower == "monosynth") return PresetCategory::monosynth;
    return fallback;
}
