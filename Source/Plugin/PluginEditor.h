/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Premium industrial plugin editor — fully responsive layout
 *              using proportional math. All setBounds() calls driven by
 *              getWidth()/getHeight() ratios.
 *
 * Signal-flow layout:
 *   Header: Logo | Randomize | Master Volume
 *   Row 1:  VCO1 | VCO2 | Sub/Noise | Mixer | Filter
 *   Row 2:  VCA Env | VCF Env
 *   Row 3:  Glide/Voice | LFO1 | LFO2 | LFO3 | LFO4 | Tape Delay | Amp
 */

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../UI/LookAndFeel.h"
#include "../UI/Components/Panel.h"
#include "../UI/Components/LabeledKnob.h"
#include "../UI/Components/WaveformKnob.h"
#include "../UI/Components/LogoComponent.h"
#include "../UI/Components/EnvDisplay.h"
#include "../UI/Components/LfoDisplay.h"

class PluginEditor  : public juce::AudioProcessorEditor,
                      private juce::Timer
{
public:
    explicit PluginEditor (PluginProcessor& p);
    ~PluginEditor() override;

    void paint   (juce::Graphics& g) override;
    void resized () override;
    void timerCallback() override;

private:
    // ── Layout helpers ────────────────────────────────────────────────────────
    void layoutHeaderBar (int margin, int headerH, int largeKnobD);
    void layoutRow1 (int x, int y, int totalW, int totalH,
                     int knobD, int smallKnobD, int largeKnobD);
    void layoutEnvelopeRow (int x, int y, int totalW, int totalH, int mediumKnobD);
    void layoutRow2 (int x, int y, int totalW, int totalH,
                     int knobD, int smallKnobD);

    // Compute a simulated LFO output value at a given phase for UI animation.
    float computeLfoOutput (int waveform, float phase, float shape) const;

    // Distribute knobs evenly across a rectangle, vertically centred.
    void placeKnobRow (std::initializer_list<juce::Component*> knobs,
                       juce::Rectangle<int> area,
                       int knobD,
                       int panelTitleH = 0);

    void placeKnobColumn (std::initializer_list<juce::Component*> knobs,
                          juce::Rectangle<int> area,
                          int knobD);

    // Place a ComboBox inside a panel, below the title bar.
    void placeComboBox (juce::ComboBox& box,
                        juce::Rectangle<int> panelBounds,
                        int panelTitleH,
                        int comboH);

    // Update pulse width visibility based on waveform selection
    void updatePulseWidthVisibility();

    // Update tape delay time knob visibility based on time mode
    void updateTimeKnobVisibility();

    // Update EnvDisplay from parameter values (two-way sync)
    void syncEnvDisplays();

    // Randomize all eligible parameters
    void randomizePatch();

    // ── Processor reference ───────────────────────────────────────────────────
    PluginProcessor& audioProcessor;

    // ── Look and feel ─────────────────────────────────────────────────────────
    GhostSignalLookAndFeel lnf;

    // ── Scale factor (set by JUCE for HiDPI) ─────────────────────────────────
    float scaleFactor { 1.0f };

    // ── Header ────────────────────────────────────────────────────────────────
    LogoComponent logoComponent;
    juce::Slider masterVolume;
    juce::Label  masterVolLabel;

    // ── OSC 1 ─────────────────────────────────────────────────────────────────
    Panel          osc1Panel     { "VCO1" };
    WaveformKnob   osc1Waveform  { "WAVE" };
    LabeledKnob    osc1PulseWidth{ "PW" };
    LabeledKnob    osc1Octave    { "Octave" };
    LabeledKnob    osc1Tune      { "Tune" };

    // ── OSC 2 ─────────────────────────────────────────────────────────────────
    Panel          osc2Panel     { "VCO2" };
    WaveformKnob   osc2Waveform  { "WAVE" };
    LabeledKnob    osc2PulseWidth{ "PW" };
    LabeledKnob    osc2Octave    { "Octave" };
    LabeledKnob    osc2Tune      { "Tune" };

    // ── SUB ───────────────────────────────────────────────────────────────────
    Panel       subPanel   { "SUB" };
    LabeledKnob subOctave  { "Octave" };

    // ── NOISE ─────────────────────────────────────────────────────────────────
    Panel       noisePanel { "NOISE" };
    juce::ComboBox noiseType { "Noise Color" };
    LabeledKnob noiseGain { "Level" };


    // ── MIXER ─────────────────────────────────────────────────────────────────
    Panel       mixerPanel { "MIXER" };
    LabeledKnob mixerVco1Level { "VCO 1 Lvl" };
    LabeledKnob mixerVco2Level { "VCO 2 Lvl" };
    LabeledKnob mixerSubLevel  { "Sub Lvl" };
    LabeledKnob mixerDrive { "Drive" };

    // ── FILTER ────────────────────────────────────────────────────────────────
    Panel       filterPanel { "FILTER" };
    LabeledKnob hpfCutoff   { "HPF Cut" };
    LabeledKnob hpfRes      { "RES" };
    LabeledKnob lpfCutoff   { "LPF Cut" };
    LabeledKnob lpfRes      { "RES" };
    LabeledKnob lpfDrive    { "LPF Drv" };
    juce::Rectangle<int> filterSeparatorBounds;

    // ── ENVELOPE 2 (VCA ADSR) ─────────────────────────────────────────────────
    Panel       ampPanel   { "VCA ENV" };
    LabeledKnob ampAttack  { "A" };
    LabeledKnob ampDecay   { "D" };
    LabeledKnob ampSustain { "S" };
    LabeledKnob ampRelease { "R" };
    EnvDisplay  ampEnvDisplay;

    // ── ENVELOPE 1 (VCF ADSR) ─────────────────────────────────────────────────
    Panel       env1Panel   { "VCF ENV" };
    LabeledKnob env1Attack  { "A" };
    LabeledKnob env1Decay   { "D" };
    LabeledKnob env1Sustain { "S" };
    LabeledKnob env1Release { "R" };
    EnvDisplay  vcfEnvDisplay;

    // ── AMP (master gain + pan) ───────────────────────────────────────────────
    Panel       ampSectionPanel { "AMP" };
    LabeledKnob ampGain  { "Gain" };
    LabeledKnob pan      { "Pan" };

    // ── GLIDE / VOICE MODE ────────────────────────────────────────────────────
    Panel          glidePanel { "GLIDE" };
    LabeledKnob    glideTime   { "Glide" };
    juce::ComboBox voiceModeBox;

    // ── LFO 1 ─────────────────────────────────────────────────────────────────
    Panel          lfo1Panel   { "LFO1" };
    juce::ComboBox lfo1Waveform;
    LabeledKnob    lfo1Rate    { "Rate" };
    LabeledKnob    lfo1Depth   { "Depth" };
    juce::ComboBox lfo1Dest;
    LfoDisplay     lfo1Display;

    // ── LFO 2 ─────────────────────────────────────────────────────────────────
    Panel          lfo2Panel   { "LFO2" };
    juce::ComboBox lfo2Waveform;
    LabeledKnob    lfo2Rate    { "Rate" };
    LabeledKnob    lfo2Depth   { "Depth" };
    juce::ComboBox lfo2Dest;
    LfoDisplay     lfo2Display;

    // ── LFO 3 ─────────────────────────────────────────────────────────────────
    Panel          lfo3Panel   { "LFO3" };
    juce::ComboBox lfo3Waveform;
    LabeledKnob    lfo3Rate    { "Rate" };
    LabeledKnob    lfo3Depth   { "Depth" };
    juce::ComboBox lfo3Dest;
    LfoDisplay     lfo3Display;

    // ── LFO 4 ─────────────────────────────────────────────────────────────────
    Panel          lfo4Panel   { "LFO4" };
    juce::ComboBox lfo4Waveform;
    LabeledKnob    lfo4Rate    { "Rate" };
    LabeledKnob    lfo4Depth   { "Depth" };
    juce::ComboBox lfo4Dest;
    LfoDisplay     lfo4Display;

    // ── TAPE DELAY ───────────────────────────────────────────────────────────
    Panel          tapeDelayPanel { "TAPE DELAY" };
    LabeledKnob    tapeDelayMode     { "Mode" };
    LabeledKnob    tapeDelayTime     { "Time" };
    LabeledKnob    tapeDelayFeedback { "FB" };
    LabeledKnob    tapeDelayMix      { "Mix" };
    LabeledKnob    tapeDelayAge      { "Age" };
    LabeledKnob    tapeDelaySat      { "Sat" };
    LabeledKnob    tapeDelayWow      { "Wow" };
    LabeledKnob    tapeDelayFlutter  { "Flut" };

    // ── TAPE DELAY ON/OFF BUTTON ─────────────────────────────────────────────
    juce::TextButton tapeDelayOnOff { "PWR" };

    // ── RANDOMIZE BUTTON (lockable) ───────────────────────────────────────────
    juce::TextButton randomizeButton { "RANDOMIZE" };
    bool randomizeLocked { true };
    int randomizeFlashCount { 0 };
    void updateRandomizeButtonAppearance();
    void toggleRandomizeLock();
    void triggerRandomize();

    // ── Parameter attachments ─────────────────────────────────────────────────
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   masterVolAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   osc1WaveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   osc1PulseWidthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   osc1OctaveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   osc1TuneAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   osc2WaveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   osc2PulseWidthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   osc2OctaveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   osc2TuneAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   subOctaveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> noiseTypeAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   noiseGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   mixerVco1LevelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   mixerVco2LevelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   mixerSubLevelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   mixerDriveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   hpfCutoffAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   hpfResAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   lpfCutoffAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   lpfResAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   lpfDriveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   env1AttackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   env1DecayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   env1SustainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   env1ReleaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   ampAttackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   ampDecayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   ampSustainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   ampReleaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   lfo1RateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   lfo1DepthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfo1WaveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfo1DestAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   lfo2RateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   lfo2DepthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfo2WaveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfo2DestAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   lfo3RateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   lfo3DepthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfo3WaveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfo3DestAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   lfo4RateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   lfo4DepthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfo4WaveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfo4DestAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   ampGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   panAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   glideTimeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> voiceModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>   tapeDelayEnableAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   tapeDelayTimeModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   tapeDelayTimeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   tapeDelayFeedbackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   tapeDelayMixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   tapeDelayAgeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   tapeDelaySatAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   tapeDelayWowAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>   tapeDelayFlutterAttachment;

    // ── LFO phase accumulators (for PW modulation animation in timer) ──────────
    float lfoPhase1 { 0.0f };
    float lfoPhase2 { 0.0f };
    float lfoPhase3 { 0.0f };
    float lfoPhase4 { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
