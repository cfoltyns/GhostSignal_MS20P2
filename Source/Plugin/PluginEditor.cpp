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
 *   Header: Logo | Randomize
 *   Row 1:  VCO1 | VCO2 | Sub/Noise | Mixer | Filter
 *   Row 2:  VCA Env | VCF Env
 *   Row 3:  Glide/Voice | LFO1 | LFO2 | LFO3 | LFO4 | Tape Delay | Amp
 *   Bottom-right: Master Volume
 */

#include "PluginEditor.h"
#include <random>
#include <cmath>

// ── LFO tempo-sync helpers ────────────────────────────────────────────────────
namespace
{
    // The 12 tempo-synced LFO divisions (1/4, 1/8, 1/16, 1/32 with straight,
    // triplet and dotted variants) live in Parameters — shared with the DSP so
    // the UI and audio can never disagree about which division is selected.

    // Snap values (full 0..1 positions) used by the rate knob while in sync.
    const float* lfoSyncSnapValues()
    {
        static float vals[Parameters::lfoSyncDivisionCount];
        static bool initialised = false;
        if (!initialised)
        {
            for (int i = 0; i < Parameters::lfoSyncDivisionCount; ++i)
                vals[i] = Parameters::lfoSyncParamValue (i);
            initialised = true;
        }
        return vals;
    }

    // Format a free-running period (ms) for the Rate knob centre label.
    inline juce::String lfoRateMsLabel (float periodMs)
    {
        if (periodMs >= 1000.0f)
            return juce::String (periodMs / 1000.0, 1) + "s";
        return juce::String (juce::roundToInt (periodMs)) + "ms";
    }

    // Octave knob snap positions (whole octave steps).
    constexpr float octaveSnapOsc1[] = { -2.0f, -1.0f, 0.0f, 1.0f, 2.0f };
    constexpr float octaveSnapOsc2[] = { -3.0f, -2.0f, -1.0f, 0.0f, 1.0f, 2.0f, 3.0f };
    constexpr float octaveSnapSub[]  = { -3.0f, -2.0f, -1.0f, 0.0f, 1.0f, 2.0f, 3.0f };
}

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

PluginEditor::PluginEditor (PluginProcessor& p)
    : juce::AudioProcessorEditor (&p),
      audioProcessor (p)
{
    setLookAndFeel (&lnf);
    setResizable (true, true);
    setResizeLimits (900, 560, 2200, 1400);
    setSize (1280, 780);

    // ── Header ────────────────────────────────────────────────────────────────
    addAndMakeVisible (logoComponent);
    logoComponent.setText ("ghost signal MS20P");

    // ── OSC1 ─────────────────────────────────────────────────────────────────
    addAndMakeVisible (osc1Panel);
    addAndMakeVisible (osc1Waveform);
    addAndMakeVisible (osc1PulseWidth);
    addAndMakeVisible (osc1Octave);
    addAndMakeVisible (osc1Tune);
    osc1Waveform.addItemList (Parameters::oscWaveformChoices, 1);
    osc1Waveform.onChange = [this]
    {
        updatePulseWidthVisibility();
        resized(); // re-position the PW knob (only laid out when visible)
    };

    // ── OSC2 ─────────────────────────────────────────────────────────────────
    addAndMakeVisible (osc2Panel);
    addAndMakeVisible (osc2Waveform);
    addAndMakeVisible (osc2PulseWidth);
    addAndMakeVisible (osc2Octave);
    addAndMakeVisible (osc2Tune);
    osc2Waveform.addItemList (Parameters::oscWaveformChoices, 1);
    osc2Waveform.onChange = [this]
    {
        updatePulseWidthVisibility();
        resized(); // re-position the PW knob (only laid out when visible)
    };

    // ── SUB / NOISE ───────────────────────────────────────────────────────────
    addAndMakeVisible (subPanel);
    addAndMakeVisible (subOctave);
    addAndMakeVisible (noisePanel);

    // Noise type knob: a discrete selector stepping through the noise colours.
    // The selected colour name is shown in the centre of the knob — no tick
    // marks or labels are drawn around the knob itself.
    noiseTypeKnob.getSlider().setRange (0.0, (double) Parameters::noiseTypeChoices.size() - 1.0, 1.0);
    updateNoiseTypeCenterText();
    noiseTypeKnob.getSlider().onValueChange = [this] { updateNoiseTypeCenterText(); };
    addAndMakeVisible (noiseTypeKnob);

    // ── MIXER ─────────────────────────────────────────────────────────────────
    addAndMakeVisible (mixerPanel);
    addAndMakeVisible (mixerVco1Level);
    addAndMakeVisible (mixerVco2Level);
    addAndMakeVisible (mixerSubLevel);
    addAndMakeVisible (noiseGain);
    addAndMakeVisible (mixerDrive);

    // ── FILTER ────────────────────────────────────────────────────────────────
    addAndMakeVisible (filterPanel);
    addAndMakeVisible (hpfCutoff);
    hpfCutoff.setAutoCenterText (true);
    addAndMakeVisible (hpfRes);
    hpfRes.setAutoCenterText (true);
    addAndMakeVisible (lpfCutoff);
    lpfCutoff.setAutoCenterText (true);
    addAndMakeVisible (lpfRes);
    lpfRes.setAutoCenterText (true);
    addAndMakeVisible (lpfDrive);
    lpfDrive.setAutoCenterText (true);

    // ── VCA ENV (with EnvDisplay) ─────────────────────────────────────────────
    addAndMakeVisible (ampPanel);
    addAndMakeVisible (ampAttack);
    addAndMakeVisible (ampDecay);
    addAndMakeVisible (ampSustain);
    addAndMakeVisible (ampRelease);
    addAndMakeVisible (ampEnvDisplay);

    // ── VCF ENV (with EnvDisplay) ─────────────────────────────────────────────
    addAndMakeVisible (env1Panel);
    addAndMakeVisible (env1Attack);
    addAndMakeVisible (env1Decay);
    addAndMakeVisible (env1Sustain);
    addAndMakeVisible (env1Release);
    addAndMakeVisible (vcfEnvDisplay);

    // ── AMP (master gain + pan) ───────────────────────────────────────────────
    addAndMakeVisible (ampSectionPanel);
    addAndMakeVisible (ampGain);
    addAndMakeVisible (pan);

    // ── GLIDE / VOICE MODE ────────────────────────────────────────────────────
    addAndMakeVisible (glidePanel);
    addAndMakeVisible (glideTime);
    voiceModeBox.addItemList ({"Poly", "Mono", "Unison"}, 1);
    addAndMakeVisible (voiceModeBox);

    // ── LFO Waveform & Destination choice lists ──────────────────────────────
    const juce::StringArray lfoWaveItems = Parameters::lfoWaveformChoices;
    const juce::StringArray lfoDestItems = Parameters::lfoDestinationChoices;

    // ── LFO 1 ─────────────────────────────────────────────────────────────────
    lfo1Waveform.addItemList (lfoWaveItems, 1);
    lfo1Dest.addItemList (lfoDestItems, 1);
    addAndMakeVisible (lfo1Panel);
    addAndMakeVisible (lfo1Waveform);
    addAndMakeVisible (lfo1Rate);
    addAndMakeVisible (lfo1Depth);
    addAndMakeVisible (lfo1Dest);
    addAndMakeVisible (lfo1Display);

    // ── LFO 2 ─────────────────────────────────────────────────────────────────
    lfo2Waveform.addItemList (lfoWaveItems, 1);
    lfo2Dest.addItemList (lfoDestItems, 1);
    addAndMakeVisible (lfo2Panel);
    addAndMakeVisible (lfo2Waveform);
    addAndMakeVisible (lfo2Rate);
    addAndMakeVisible (lfo2Depth);
    addAndMakeVisible (lfo2Dest);
    addAndMakeVisible (lfo2Display);

    // ── LFO 3 ─────────────────────────────────────────────────────────────────
    lfo3Waveform.addItemList (lfoWaveItems, 1);
    lfo3Dest.addItemList (lfoDestItems, 1);
    addAndMakeVisible (lfo3Panel);
    addAndMakeVisible (lfo3Waveform);
    addAndMakeVisible (lfo3Rate);
    addAndMakeVisible (lfo3Depth);
    addAndMakeVisible (lfo3Dest);
    addAndMakeVisible (lfo3Display);

    // ── LFO 4 ─────────────────────────────────────────────────────────────────
    lfo4Waveform.addItemList (lfoWaveItems, 1);
    lfo4Dest.addItemList (lfoDestItems, 1);
    addAndMakeVisible (lfo4Panel);
    addAndMakeVisible (lfo4Waveform);
    addAndMakeVisible (lfo4Rate);
    addAndMakeVisible (lfo4Depth);
    addAndMakeVisible (lfo4Dest);
    addAndMakeVisible (lfo4Display);

    // ── LFO sync toggles (lock the Rate knob to DAW tempo divisions) ─────────
    auto configureSyncButton = [this](juce::TextButton& btn, int lfoIndex)
    {
        addAndMakeVisible (btn);
        btn.setButtonText ("SYNC");
        btn.setClickingTogglesState (true);
        btn.setLookAndFeel (&lnf);
        btn.setColour (juce::TextButton::buttonColourId, GhostSignalLookAndFeel::knobBody);
        btn.setColour (juce::TextButton::buttonOnColourId, GhostSignalLookAndFeel::accent);
        btn.setColour (juce::TextButton::textColourOffId, GhostSignalLookAndFeel::textSecondary);
        btn.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
        btn.setAlpha (0.85f);
        btn.onClick = [this, lfoIndex] { applyLfoSyncMode (lfoIndex); };
    };
    configureSyncButton (lfo1Sync, 0);
    configureSyncButton (lfo2Sync, 1);
    configureSyncButton (lfo3Sync, 2);
    configureSyncButton (lfo4Sync, 3);

    // ── TAPE DELAY ────────────────────────────────────────────────────────────
    addAndMakeVisible (tapeDelayPanel);
    addAndMakeVisible (tapeDelayMode);
    tapeDelayMode.addItemList (Parameters::tapeDelayTimeModeChoices, 1);
    addAndMakeVisible (tapeDelayTime);
    addAndMakeVisible (tapeDelayFeedback);
    addAndMakeVisible (tapeDelayMix);
    addAndMakeVisible (tapeDelayAge);
    addAndMakeVisible (tapeDelaySat);
    addAndMakeVisible (tapeDelayWow);
    addAndMakeVisible (tapeDelayFlutter);

    // ── TAPE DELAY ON/OFF BUTTON (labeled toggle with LED indicator) ───────────
    addAndMakeVisible (tapeDelayOnOff);
    tapeDelayOnOff.setClickingTogglesState (true);
    tapeDelayOnOff.setButtonText ("TAPE");
    tapeDelayOnOff.setColour (juce::TextButton::buttonColourId, GhostSignalLookAndFeel::knobBody);
    tapeDelayOnOff.setColour (juce::TextButton::buttonOnColourId, GhostSignalLookAndFeel::accent);
    tapeDelayOnOff.setColour (juce::TextButton::textColourOffId, GhostSignalLookAndFeel::textSecondary);
    tapeDelayOnOff.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
    // Use the GhostSignal look and feel for consistent button rendering
    tapeDelayOnOff.setLookAndFeel (&lnf);
    tapeDelayOnOff.setAlpha (0.85f);
    // Mode combo box: show/hide time knob based on selected mode
    tapeDelayMode.onChange = [this] { updateTimeKnobVisibility(); };

    // ── RANDOMIZE BUTTON (two-click confirmation) ─────────────────────────────
    addAndMakeVisible (randomizeButton);
    randomizeLocked = true;
    updateRandomizeButtonAppearance();
    randomizeButton.onClick = [this] { toggleRandomizeLock(); };

    // ── Parameter attachments ─────────────────────────────────────────────────
    auto& apvts = audioProcessor.getAPVTS();

    osc1WaveAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, Parameters::paramOsc1Waveform, osc1Waveform);
    osc1PulseWidthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramOsc1PulseWidth, osc1PulseWidth.getSlider());
    osc1OctaveAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramOsc1Octave,   osc1Octave.getSlider());
    osc1TuneAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramOsc1Tune,     osc1Tune.getSlider());
    osc2WaveAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, Parameters::paramOsc2Waveform, osc2Waveform);
    osc2PulseWidthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramOsc2PulseWidth, osc2PulseWidth.getSlider());
    osc2OctaveAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramOsc2Octave,   osc2Octave.getSlider());
    osc2TuneAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramOsc2Tune,     osc2Tune.getSlider());
    subOctaveAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramSubOctave,    subOctave.getSlider());
    noiseTypeAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramNoiseType,    noiseTypeKnob.getSlider());
    noiseGainAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramNoiseGain,    noiseGain.getSlider());
    mixerVco1LevelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramMixerVco1Level, mixerVco1Level.getSlider());
    mixerVco2LevelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramMixerVco2Level, mixerVco2Level.getSlider());
    mixerSubLevelAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramMixerSubLevel,  mixerSubLevel.getSlider());
    mixerDriveAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramMixerDrive,   mixerDrive.getSlider());
    hpfCutoffAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramHPFCutoff,    hpfCutoff.getSlider());
    hpfResAttachment      = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramHPFRes,       hpfRes.getSlider());
    lpfCutoffAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramLPFCutoff,    lpfCutoff.getSlider());
    lpfResAttachment      = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramLPFRes,       lpfRes.getSlider());
    lpfDriveAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramLPFDrive,     lpfDrive.getSlider());
    env1AttackAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramEnv1Attack,   env1Attack.getSlider());
    env1DecayAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramEnv1Decay,    env1Decay.getSlider());
    env1SustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramEnv1Sustain,  env1Sustain.getSlider());
    env1ReleaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramEnv1Release,  env1Release.getSlider());
    ampAttackAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramAmpAttack,    ampAttack.getSlider());
    ampDecayAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramAmpDecay,     ampDecay.getSlider());
    ampSustainAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramAmpSustain,   ampSustain.getSlider());
    ampReleaseAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramAmpRelease,   ampRelease.getSlider());

    // LFO1 attachments
    lfo1WaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, Parameters::paramLFO1Waveform, lfo1Waveform);
    lfo1RateAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramLFO1Rate,  lfo1Rate.getSlider());
    lfo1DepthAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramLFO1Depth, lfo1Depth.getSlider());
    lfo1DestAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, Parameters::paramLFO1Dest,  lfo1Dest);

    // LFO2 attachments
    lfo2WaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, Parameters::paramLFO2Waveform, lfo2Waveform);
    lfo2RateAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramLFO2Rate,  lfo2Rate.getSlider());
    lfo2DepthAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramLFO2Depth, lfo2Depth.getSlider());
    lfo2DestAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, Parameters::paramLFO2Dest,  lfo2Dest);

    // LFO3 attachments
    lfo3WaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, Parameters::paramLFO3Waveform, lfo3Waveform);
    lfo3RateAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramLFO3Rate,  lfo3Rate.getSlider());
    lfo3DepthAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramLFO3Depth, lfo3Depth.getSlider());
    lfo3DestAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, Parameters::paramLFO3Dest,  lfo3Dest);

    // LFO4 attachments
    lfo4WaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, Parameters::paramLFO4Waveform, lfo4Waveform);
    lfo4RateAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramLFO4Rate,  lfo4Rate.getSlider());
    lfo4DepthAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramLFO4Depth, lfo4Depth.getSlider());
    lfo4DestAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, Parameters::paramLFO4Dest,  lfo4Dest);

    ampGainAttachment     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramAmpGain,      ampGain.getSlider());
    panAttachment         = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramPan,          pan.getSlider());
    glideTimeAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramGlideTime,    glideTime.getSlider());
    voiceModeAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, Parameters::paramVoiceMode,    voiceModeBox);
    tapeDelayEnableAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, Parameters::paramTapeDelayEnable, tapeDelayOnOff);
    tapeDelayTimeModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, Parameters::paramTapeDelayTimeMode, tapeDelayMode);
    tapeDelayTimeAttachment     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramTapeDelayTime, tapeDelayTime.getSlider());
    tapeDelayFeedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramTapeDelayFeedback, tapeDelayFeedback.getSlider());
    tapeDelayMixAttachment      = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramTapeDelayMix, tapeDelayMix.getSlider());
    tapeDelayAgeAttachment      = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramTapeDelayAge, tapeDelayAge.getSlider());
    tapeDelaySatAttachment      = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramTapeDelaySat, tapeDelaySat.getSlider());
    tapeDelayWowAttachment      = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramTapeDelayWow, tapeDelayWow.getSlider());
    tapeDelayFlutterAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramTapeDelayFlutter, tapeDelayFlutter.getSlider());

    // ── EnvDisplay two-way sync (knob → display and display → knob) ──────────
    auto makeTimeCallback = [this](const juce::String& paramId, float maxVal) {
        return [this, paramId, maxVal](float norm) {
            float seconds = maxVal * norm * norm;
            if (auto* param = audioProcessor.getAPVTS().getParameter(paramId))
                param->setValueNotifyingHost(param->convertTo0to1(seconds));
        };
    };
    auto makeLevelCallback = [this](const juce::String& paramId) {
        return [this, paramId](float norm) {
            if (auto* param = audioProcessor.getAPVTS().getParameter(paramId))
                param->setValueNotifyingHost(norm);
        };
    };

    ampEnvDisplay.onAttackChanged  = makeTimeCallback(Parameters::paramAmpAttack, 5.0f);
    ampEnvDisplay.onDecayChanged   = makeTimeCallback(Parameters::paramAmpDecay, 2.0f);
    ampEnvDisplay.onSustainChanged = makeLevelCallback(Parameters::paramAmpSustain);
    ampEnvDisplay.onReleaseChanged = makeTimeCallback(Parameters::paramAmpRelease, 5.0f);

    vcfEnvDisplay.onAttackChanged  = makeTimeCallback(Parameters::paramEnv1Attack, 5.0f);
    vcfEnvDisplay.onDecayChanged   = makeTimeCallback(Parameters::paramEnv1Decay, 2.0f);
    vcfEnvDisplay.onSustainChanged = makeLevelCallback(Parameters::paramEnv1Sustain);
    vcfEnvDisplay.onReleaseChanged = makeTimeCallback(Parameters::paramEnv1Release, 5.0f);

    updatePulseWidthVisibility();
    // Adopt the initial PW mode without animating (state comes from the
    // restored patch, not a user interaction this session).
    osc1PwModeShown = osc1PulseWidth.isVisible();
    osc2PwModeShown = osc2PulseWidth.isVisible();
    updateTimeKnobVisibility();

    // ── Octave knobs snap to whole octave steps ──────────────────────────────
    osc1Octave.setSnapToValues (octaveSnapOsc1, 5);
    osc2Octave.setSnapToValues (octaveSnapOsc2, 7);
    subOctave.setSnapToValues (octaveSnapSub, 7);

    // ── Initialise LFO sync toggle states from their persisted sync params ───
    for (int i = 0; i < 4; ++i)
    {
        LabeledKnob&    rate = (i == 0 ? lfo1Rate : i == 1 ? lfo2Rate : i == 2 ? lfo3Rate : lfo4Rate);
        juce::TextButton& btn = (i == 0 ? lfo1Sync : i == 1 ? lfo2Sync : i == 2 ? lfo3Sync : lfo4Sync);
        const juce::String syncId = (i == 0 ? Parameters::paramLFO1Sync
                                     : i == 1 ? Parameters::paramLFO2Sync
                                     : i == 2 ? Parameters::paramLFO3Sync
                                              : Parameters::paramLFO4Sync);

        const bool wasSync = (apvts.getRawParameterValue (syncId)
                              && apvts.getRawParameterValue (syncId)->load() > 0.5f);
        btn.setToggleState (wasSync, juce::dontSendNotification);

        // Keep the division / ms label current while the knob is dragged
        rate.getSlider().onValueChange = [this, i] { refreshLfoRateLabel (i); };

        applyLfoSyncMode (i);
    }

    // ── Set improved knob sensitivity for all rotary sliders ──────────────────
    auto setKnobSensitivity = [](juce::Slider& slider)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                                    juce::MathConstants<float>::pi * 2.75f,
                                    true);
        slider.setMouseDragSensitivity (120);
    };

    setKnobSensitivity (osc1PulseWidth.getSlider());
    osc1PulseWidth.getSlider().setTextValueSuffix ("%");
    osc1PulseWidth.setMinKnobSize (40);
    setKnobSensitivity (osc1Octave.getSlider());
    setKnobSensitivity (osc1Tune.getSlider());
    setKnobSensitivity (osc2PulseWidth.getSlider());
    osc2PulseWidth.getSlider().setTextValueSuffix ("%");
    osc2PulseWidth.setMinKnobSize (40);
    setKnobSensitivity (osc2Octave.getSlider());
    setKnobSensitivity (osc2Tune.getSlider());
    setKnobSensitivity (subOctave.getSlider());
    setKnobSensitivity (noiseGain.getSlider());
    setKnobSensitivity (mixerVco1Level.getSlider());
    setKnobSensitivity (mixerVco2Level.getSlider());
    setKnobSensitivity (mixerSubLevel.getSlider());
    setKnobSensitivity (mixerDrive.getSlider());
    setKnobSensitivity (hpfCutoff.getSlider());
    setKnobSensitivity (hpfRes.getSlider());
    setKnobSensitivity (lpfCutoff.getSlider());
    setKnobSensitivity (lpfRes.getSlider());
    setKnobSensitivity (lpfDrive.getSlider());

    setKnobSensitivity (env1Attack.getSlider());
    setKnobSensitivity (env1Decay.getSlider());
    setKnobSensitivity (env1Sustain.getSlider());
    setKnobSensitivity (env1Release.getSlider());
    setKnobSensitivity (ampAttack.getSlider());
    setKnobSensitivity (ampDecay.getSlider());
    setKnobSensitivity (ampSustain.getSlider());
    setKnobSensitivity (ampRelease.getSlider());
    setKnobSensitivity (lfo1Rate.getSlider());
    setKnobSensitivity (lfo1Depth.getSlider());
    setKnobSensitivity (lfo2Rate.getSlider());
    setKnobSensitivity (lfo2Depth.getSlider());
    setKnobSensitivity (lfo3Rate.getSlider());
    setKnobSensitivity (lfo3Depth.getSlider());
    setKnobSensitivity (lfo4Rate.getSlider());
    setKnobSensitivity (lfo4Depth.getSlider());
    setKnobSensitivity (ampGain.getSlider());
    setKnobSensitivity (pan.getSlider());
    setKnobSensitivity (glideTime.getSlider());
    setKnobSensitivity (tapeDelayTime.getSlider());
    setKnobSensitivity (tapeDelayFeedback.getSlider());
    setKnobSensitivity (tapeDelayMix.getSlider());
    setKnobSensitivity (tapeDelayAge.getSlider());
    setKnobSensitivity (tapeDelaySat.getSlider());
    setKnobSensitivity (tapeDelayWow.getSlider());
    setKnobSensitivity (tapeDelayFlutter.getSlider());

    startTimerHz (30);
}

PluginEditor::~PluginEditor()
{
    setLookAndFeel (nullptr);
}

// ─────────────────────────────────────────────────────────────────────────────
// paint()
// ─────────────────────────────────────────────────────────────────────────────

void PluginEditor::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const float w = bounds.getWidth();
    const float h = bounds.getHeight();

    // Deep charcoal background with subtle vertical gradient
    {
        juce::ColourGradient bg;
        bg.point1 = { 0.0f, 0.0f };
        bg.point2 = { 0.0f, h };
        bg.addColour (0.0f,  GhostSignalLookAndFeel::bg);
        bg.addColour (0.45f, GhostSignalLookAndFeel::bg.darker (0.03f));
        bg.addColour (1.0f,  GhostSignalLookAndFeel::bg.darker (0.06f));
        g.setGradientFill (bg);
        g.fillRect (bounds);
    }

    // Subtle brushed-metal scan-line texture across entire background
    {
        g.setColour (juce::Colour (0x04FFFFFF));
        for (float lineY = 0.0f; lineY < h; lineY += 3.0f)
            g.drawHorizontalLine (static_cast<int> (lineY), 0.0f, w);
    }

    // Vertical dividers between major sections (subtle)
    // Aligned to the VCO1 | VCO2 | Sub | Mixer | Filter | Spacer boundaries
    {
        g.setColour (juce::Colour (0x08FFFFFF));
        g.drawVerticalLine (static_cast<int> (w * 0.43f), 0.0f, h);
        g.drawVerticalLine (static_cast<int> (w * 0.86f), 0.0f, h);
    }

    // Top edge highlight
    g.setColour (juce::Colour (0x1AFFFFFF));
    g.drawHorizontalLine (0, 0.0f, w);

    // Bottom edge shadow
    g.setColour (juce::Colour (0x55000000));
    g.drawHorizontalLine (static_cast<int> (h) - 1, 0.0f, w);

    {
        const int margin     = juce::jmax (12, (int) (w * 0.015f));
        const int headerH    = juce::jmax (36, (int) (h * 0.055f));
        const int contentTop = margin + headerH + margin;
        const int contentH   = static_cast<int> (h) - contentTop - margin;
        const int row1H      = (int) (contentH * 0.42f);
        const int dividerY   = contentTop + row1H + margin / 2;

        // Divider between row 1 and row 2
        g.setColour (juce::Colour (0x30FFFFFF));
        g.drawHorizontalLine (dividerY, static_cast<float> (margin), w - static_cast<float> (margin));
        g.setColour (juce::Colour (0x10000000));
        g.drawHorizontalLine (dividerY + 1, static_cast<float> (margin), w - static_cast<float> (margin));
    }

    // Draw thin vertical separator line between LPF and HPF in the filter panel
    if (! filterSeparatorBounds.isEmpty())
    {
        g.setColour (GhostSignalLookAndFeel::panelBorder);
        g.fillRect (filterSeparatorBounds.toFloat().reduced (0, 1));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// resized()
// ─────────────────────────────────────────────────────────────────────────────

void PluginEditor::resized()
{
    const int w = getWidth();
    const int h = getHeight();

    const int margin   = juce::jmax (10, (int) (w * 0.012f));
    const int gap      = juce::jmax (6,  (int) (w * 0.006f));
    const int headerH  = juce::jmax (36, (int) (h * 0.055f));

    // Large knob for important controls (filter cutoff)
    const int largeKnobD = juce::jlimit (56, 100, (int) (w * 0.065f));
    // Medium knob for standard controls
    const int knobD      = juce::jlimit (44, 88, (int) (w * 0.055f));
    // Small knob for secondary controls
    const int smallKnobD = (int) (knobD * 0.75f);

    layoutHeaderBar (margin, headerH, largeKnobD);

    const int contentTop = margin + headerH + margin;
    const int contentH   = h - contentTop - margin;

    // Three-row split: 42% / 20% / 38%
    const int row1H   = (int) (contentH * 0.42f);
    const int rowGap  = margin;
    const int envRowH = (int) (contentH * 0.20f);
    const int row2H   = contentH - row1H - envRowH - 2 * rowGap - margin;
    const int contentW = w - 2 * margin;

    // Update PW visibility before layout so waveform/PW positioning is correct
    updatePulseWidthVisibility();

    layoutRow1 (margin, contentTop, contentW, row1H, knobD, smallKnobD, largeKnobD);
    layoutEnvelopeRow (margin, contentTop + row1H + rowGap, contentW, envRowH, knobD);
    layoutRow2 (margin, contentTop + row1H + envRowH + 2 * rowGap, contentW, row2H, knobD, smallKnobD);

    syncEnvDisplays();
}

// ─────────────────────────────────────────────────────────────────────────────
// layoutHeaderBar()
// ─────────────────────────────────────────────────────────────────────────────

void PluginEditor::layoutHeaderBar (int margin, int headerH, int largeKnobD)
{
    const int w = getWidth();

    // Randomize button: right-aligned in header
    const int btnW = juce::jmax (80, (int) (w * 0.07f));
    const int btnH = juce::jmax (22, (int) (headerH * 0.5f));
    const int btnX = w - margin - btnW;
    const int btnY = margin + (headerH - btnH) / 2;
    randomizeButton.setBounds (btnX, btnY, btnW, btnH);

    // Logo text: left-aligned, snug up against the randomize button
    const int logoW = btnX - 2 * margin;
    logoComponent.setBounds (margin, margin, logoW, headerH);
}

// ─────────────────────────────────────────────────────────────────────────────
// layoutRow1() — VCO1 | VCO2 | Sub/Noise | Mixer | Filter
// ─────────────────────────────────────────────────────────────────────────────

void PluginEditor::layoutRow1 (int x, int y, int totalW, int totalH,
                               int knobD, int smallKnobD, int largeKnobD)
{
    using R = juce::Rectangle<int>;

    const int mediumKnobD = knobD;
    const int gap = juce::jmax (6, (int) (totalW * 0.006f));

    // Column widths — VCO1 and VCO2 are now separate columns with a gap
    // between them for better spacing and visual breathing room.
    //
    // NOTE: These ratios are normalised against the USABLE width (totalW
    // minus the inter-panel gaps), so the five panels always stretch
    // edge-to-edge with no unused spacer on the right. Previously the raw
    // ratios summed to only 0.85 and the leftover "spacerW" was never used,
    // leaving a large empty strip on the right side of the row. The Filter
    // panel (widest column) absorbs any integer-rounding remainder.
    constexpr float rVco1   = 0.15f;
    constexpr float rVco2   = 0.15f;
    constexpr float rSub    = 0.11f;
    constexpr float rMixer  = 0.17f;
    constexpr float rFilter = 0.27f;
    const float ratioSum = rVco1 + rVco2 + rSub + rMixer + rFilter;

    const int usableW = juce::jmax (100, totalW - 5 * gap);
    const int vco1W   = (int) (usableW * rVco1 / ratioSum);
    const int vco2W   = (int) (usableW * rVco2 / ratioSum);
    const int subW    = (int) (usableW * rSub  / ratioSum);
    const int mixerW  = (int) (usableW * rMixer / ratioSum);
    const int usedW   = vco1W + vco2W + subW + mixerW;
    const int filterW = juce::jmax (120, usableW - usedW);

    int curX = x;

    // ── VCO1 (own column) — Wave ComboBox at top, PW below centred when
    // Pulse is selected, Octave/Tune anchored to the bottom ────────
    {
        const R osc1Bounds (curX, y, vco1W, totalH);
        osc1Panel.setBounds (osc1Bounds);

        const int titleH  = osc1Panel.getTitleAreaHeight();
        const int padH    = juce::jmax (6, (int) (totalH * 0.035f));
        // Dedicated vertical breathing room — larger than the horizontal
        // padding so the combo / PW knob / Octave-Tune stack doesn't feel
        // cramped inside the oscillator panel.
        const int vGap    = juce::jmax (10, (int) (totalH * 0.06f));
        // Whole-stack upward shift (design feedback): everything in the
        // oscillator section sits this many px higher than the old layout.
        constexpr int stackUpShift = 40;

        // PW knob is smaller so it fits in the gap below the pulldown.
        // Match the knob's min size to the available diameter — the default
        // 48px minimum would overflow the small widget and clip the top.
        const int pwKnobD   = juce::jlimit (30, 42, (int) (mediumKnobD * 0.70f));
        const int pwWidgetH = pwKnobD + 18;
        osc1PulseWidth.setMinKnobSize (pwKnobD);


        // Design nudges: open up the stack — the combo rises an extra 30px
        // past the strict minimum and the PW knob sits 10px higher, so the
        // knobs don't feel cramped against each other.
        constexpr int comboExtraUp = 30;
        constexpr int pwExtraUp    = 10;

        const bool pwVisible  = osc1PulseWidth.isVisible();
        const bool pwWasShown = osc1PwModeShown;

        const int comboH = juce::jmax (16, (int) ((totalH - titleH - 2 * vGap) * 0.16f));

        // Bottom: Octave and Tune knobs — anchored near the bottom of the
        // panel, lifted by stackUpShift so the whole stack sits higher.
        const int octHBase = juce::jmax (50, (int) (mediumKnobD * 1.2f));
        // Vertical budget guard: on short windows the full stack (title,
        // menu, PW knob, Octave/Tune row, bottom pad) doesn't fit, which
        // used to let the waveform menu drop below the Octave/Tune knobs.
        // Shrink the knob row first, then the bottom lift, so the menu is
        // guaranteed to stay above the knobs at any editor size.
        int octH        = octHBase;
        int bottomShift = stackUpShift;
        auto minStackH  = [&] (int oh, int sh)
        {
            return titleH + vGap / 2 + comboH
                 + (pwVisible ? 2 * vGap + pwWidgetH + comboExtraUp : vGap)
                 + oh + vGap + sh;
        };
        while (minStackH (octH, bottomShift) > totalH && (octH > 40 || bottomShift > 0))
        {
            if (octH > 40) --octH;
            else           --bottomShift;
        }
        const int bottomPadY = y + totalH - vGap - bottomShift;
        const int row2Top    = bottomPadY - octH;

        // Row 1: Waveform ComboBox. By default it sits vertically centred in
        // the panel body. When the PW knob is shown it slides up only as far
        // as needed to make room for the knob between it and the Octave/Tune
        // row (clamped so it never rises into the panel title area).
        // comboTopMax keeps the menu above the Octave/Tune row on short
        // windows where the lifted knob row would otherwise rise past it.
        const int comboTopMax = juce::jmax (titleH + vGap / 2, row2Top - comboH - vGap);
        // Centred position lifted by stackUpShift, clamped so the combo can
        // never rise into the panel title area nor sink below the knob row.
        const int comboTopCentered = juce::jlimit (titleH + vGap / 2, comboTopMax,
                                                   titleH + (totalH - titleH - comboH) / 2 - stackUpShift);

        int row1Top = comboTopCentered;
        if (pwVisible)
        {
            const int comboTopNeeded = row2Top - 2 * vGap - pwWidgetH - comboH - comboExtraUp;
            row1Top = juce::jlimit (titleH + vGap / 2, comboTopCentered, comboTopNeeded);
        }
        // Hard guarantee: the menu never drops below the Octave/Tune row.
        row1Top = juce::jmin (row1Top, comboTopMax);

        const juce::Rectangle<int> comboTarget (curX + padH, y + row1Top,
                                                vco1W - 2 * padH, comboH);

        // Animate the combo between its centred and made-room positions only
        // when the PW visibility mode changes — plain resizes snap directly.
        auto& animator = juce::Desktop::getInstance().getAnimator();
        if (pwVisible != pwWasShown)
            animator.animateComponent (&osc1Waveform, comboTarget, 1.0f, 200, false, 0.0, 0.0);
        else
            osc1Waveform.setBounds (comboTarget);

        const R row2Area (curX + padH, row2Top, vco1W - 2 * padH, bottomPadY - row2Top);
        placeKnobRow ({ &osc1Octave, &osc1Tune }, row2Area, mediumKnobD);

        // Pulse-width knob: centred in the gap between the pulldown and the
        // Octave/Tune knobs — visible only when Pulse is selected. Nudged
        // down a couple of px so the top edge of the knob is never tight.
        // Fades in/out when the waveform mode changes.
        if (pwVisible)
        {
            const int pwBandTop = y + row1Top + comboH + vGap;
            // Sit the knob (comboExtraUp - pwExtraUp) px below the band top —
            // i.e. pwExtraUp px higher than the old tight layout — but never
            // low enough to crowd the Octave/Tune row below.
            const int pwTopMin = pwBandTop;
            const int pwTopMax = juce::jmax (pwTopMin, row2Top - pwWidgetH - vGap);
            const int pwY      = juce::jlimit (pwTopMin, pwTopMax,
                                               pwBandTop + (comboExtraUp - pwExtraUp));
            const int pwX       = curX + padH + (vco1W - 2 * padH - pwWidgetH) / 2;
            osc1PulseWidth.setBounds (pwX, pwY, pwWidgetH, pwWidgetH);

            if (! pwWasShown)
            {
                osc1PulseWidth.setAlpha (0.0f);
                animator.fadeIn (&osc1PulseWidth, 200);
            }
        }
        else if (pwWasShown)
        {
            animator.fadeOut (&osc1PulseWidth, 200); // hides itself on completion
        }

        osc1PwModeShown = pwVisible;
    }

    curX += vco1W + gap;

    // ── VCO2 (own column to the right of VCO1) ────────────────────────────────────
    {
        const R osc2Bounds (curX, y, vco2W, totalH);
        osc2Panel.setBounds (osc2Bounds);

        const int titleH  = osc2Panel.getTitleAreaHeight();
        const int padH    = juce::jmax (6, (int) (totalH * 0.035f));
        // PW knob is smaller so it fits in the gap below the pulldown.
        // Match the knob's min size to the available diameter — the default
        // 48px minimum would overflow the small widget and clip the top.
        const int pwKnobD   = juce::jlimit (30, 42, (int) (mediumKnobD * 0.70f));
        const int pwWidgetH = pwKnobD + 18;
        osc2PulseWidth.setMinKnobSize (pwKnobD);


        // Dedicated vertical breathing room — larger than the horizontal
        // padding so the combo / PW knob / Octave-Tune stack doesn't feel
        // cramped inside the oscillator panel.
        const int vGap    = juce::jmax (10, (int) (totalH * 0.06f));
        // Whole-stack upward shift (design feedback): everything in the
        // oscillator section sits this many px higher than the old layout.
        constexpr int stackUpShift = 40;

        // Design nudges: open up the stack — the combo rises an extra 30px
        // past the strict minimum and the PW knob sits 10px higher, so the
        // knobs don't feel cramped against each other.
        constexpr int comboExtraUp = 30;
        constexpr int pwExtraUp    = 10;

        const bool pwVisible  = osc2PulseWidth.isVisible();
        const bool pwWasShown = osc2PwModeShown;

        const int comboH = juce::jmax (16, (int) ((totalH - titleH - 2 * vGap) * 0.16f));

        // Bottom: Octave and Tune knobs — anchored near the bottom of the
        // panel, lifted by stackUpShift so the whole stack sits higher.
        const int octHBase = juce::jmax (50, (int) (mediumKnobD * 1.2f));
        // Vertical budget guard: on short windows the full stack (title,
        // menu, PW knob, Octave/Tune row, bottom pad) doesn't fit, which
        // used to let the waveform menu drop below the Octave/Tune knobs.
        // Shrink the knob row first, then the bottom lift, so the menu is
        // guaranteed to stay above the knobs at any editor size.
        int octH        = octHBase;
        int bottomShift = stackUpShift;
        auto minStackH  = [&] (int oh, int sh)
        {
            return titleH + vGap / 2 + comboH
                 + (pwVisible ? 2 * vGap + pwWidgetH + comboExtraUp : vGap)
                 + oh + vGap + sh;
        };
        while (minStackH (octH, bottomShift) > totalH && (octH > 40 || bottomShift > 0))
        {
            if (octH > 40) --octH;
            else           --bottomShift;
        }
        const int bottomPadY = y + totalH - vGap - bottomShift;
        const int row2Top    = bottomPadY - octH;

        // Row 1: Waveform ComboBox. By default it sits vertically centred in
        // the panel body. When the PW knob is shown it slides up only as far
        // as needed to make room for the knob between it and the Octave/Tune
        // row (clamped so it never rises into the panel title area).
        // comboTopMax keeps the menu above the Octave/Tune row on short
        // windows where the lifted knob row would otherwise rise past it.
        const int comboTopMax = juce::jmax (titleH + vGap / 2, row2Top - comboH - vGap);
        // Centred position lifted by stackUpShift, clamped so the combo can
        // never rise into the panel title area nor sink below the knob row.
        const int comboTopCentered = juce::jlimit (titleH + vGap / 2, comboTopMax,
                                                   titleH + (totalH - titleH - comboH) / 2 - stackUpShift);

        int row1Top = comboTopCentered;
        if (pwVisible)
        {
            const int comboTopNeeded = row2Top - 2 * vGap - pwWidgetH - comboH - comboExtraUp;
            row1Top = juce::jlimit (titleH + vGap / 2, comboTopCentered, comboTopNeeded);
        }
        // Hard guarantee: the menu never drops below the Octave/Tune row.
        row1Top = juce::jmin (row1Top, comboTopMax);

        const juce::Rectangle<int> comboTarget (curX + padH, y + row1Top,
                                                vco2W - 2 * padH, comboH);

        // Animate the combo between its centred and made-room positions only
        // when the PW visibility mode changes — plain resizes snap directly.
        auto& animator = juce::Desktop::getInstance().getAnimator();
        if (pwVisible != pwWasShown)
            animator.animateComponent (&osc2Waveform, comboTarget, 1.0f, 200, false, 0.0, 0.0);
        else
            osc2Waveform.setBounds (comboTarget);

        const R row2Area (curX + padH, row2Top, vco2W - 2 * padH, bottomPadY - row2Top);
        placeKnobRow ({ &osc2Octave, &osc2Tune }, row2Area, mediumKnobD);

        // Pulse-width knob: centred in the gap between the pulldown and the
        // Octave/Tune knobs — visible only when Pulse is selected. Nudged
        // down a couple of px so the top edge of the knob is never tight.
        // Fades in/out when the waveform mode changes.
        if (pwVisible)
        {
            const int pwBandTop = y + row1Top + comboH + vGap;
            // Sit the knob (comboExtraUp - pwExtraUp) px below the band top —
            // i.e. pwExtraUp px higher than the old tight layout — but never
            // low enough to crowd the Octave/Tune row below.
            const int pwTopMin = pwBandTop;
            const int pwTopMax = juce::jmax (pwTopMin, row2Top - pwWidgetH - vGap);
            const int pwY      = juce::jlimit (pwTopMin, pwTopMax,
                                               pwBandTop + (comboExtraUp - pwExtraUp));
            const int pwX       = curX + padH + (vco2W - 2 * padH - pwWidgetH) / 2;
            osc2PulseWidth.setBounds (pwX, pwY, pwWidgetH, pwWidgetH);

            if (! pwWasShown)
            {
                osc2PulseWidth.setAlpha (0.0f);
                animator.fadeIn (&osc2PulseWidth, 200);
            }
        }
        else if (pwWasShown)
        {
            animator.fadeOut (&osc2PulseWidth, 200); // hides itself on completion
        }

        osc2PwModeShown = pwVisible;
    }

    curX += vco2W + gap;

    // ── SUB + NOISE (stacked vertically) ─────────────────────────────────────
    {
        const int subH   = (int) (totalH * 0.55f);
        const int noiseH = totalH - subH - gap;

        const R subBounds   (curX, y, subW, subH);
        const R noiseBounds (curX, y + subH + gap, subW, noiseH);

        subPanel  .setBounds (subBounds);
        noisePanel.setBounds (noiseBounds);

        const int subTitleH   = subPanel.getTitleAreaHeight();
        const int noiseTitleH = noisePanel.getTitleAreaHeight();
        const int padH = juce::jmax (4, (int) (subH * 0.04f));

        const R subArea (curX + padH, y + subTitleH,
                         subW - 2 * padH,
                         subH - subTitleH - padH);
        placeKnobRow ({ &subOctave }, subArea, mediumKnobD);

        // Noise panel: the Type knob is the only control — centre it.
        const R noiseArea (curX + padH, y + subH + gap + noiseTitleH + padH,
                           subW - 2 * padH,
                           noiseH - noiseTitleH - padH * 2);
        placeKnobRow ({ &noiseTypeKnob }, noiseArea, mediumKnobD);
    }

    curX += subW + gap;

    // ── MIXER (2-2-1 layout: VCO1/VCO2 top, Sub/Drive middle, LPF Drive bottom) ─
    {
        const R mixerBounds (curX, y, mixerW, totalH);
        mixerPanel.setBounds (mixerBounds);

        const int titleH = mixerPanel.getTitleAreaHeight();
        const int padH   = juce::jmax (4, (int) (totalH * 0.04f));
        const int innerH = totalH - titleH - padH * 2;

        // Three rows: 2 knobs, 2 knobs, 2 knobs
        const int rowH = (innerH - 2 * gap) / 3;
        const int bigKnobD = (int) (smallKnobD * 1.15f);

        // Top row: VCO1 Lvl, VCO2 Lvl
        const R topRow (curX + padH, y + titleH + padH, mixerW - 2 * padH, rowH);
        placeKnobRow ({ &mixerVco1Level, &mixerVco2Level }, topRow, bigKnobD);

        // Middle row: Sub Lvl, Noise Lvl
        const R midRow (curX + padH, y + titleH + padH + rowH + gap, mixerW - 2 * padH, rowH);
        placeKnobRow ({ &mixerSubLevel, &noiseGain }, midRow, smallKnobD);

        // Bottom row: Drive, LPF Drive
        const R botRow (curX + padH, y + titleH + padH + 2 * (rowH + gap), mixerW - 2 * padH, rowH);
        placeKnobRow ({ &mixerDrive, &lpfDrive }, botRow, smallKnobD);
    }

    curX += mixerW + gap;

    // ── FILTER (LPF left | thin separator | HPF right) ────────────────────────
    {
        const R filterBounds (curX, y, filterW, totalH);
        filterPanel.setBounds (filterBounds);

        const int titleH = filterPanel.getTitleAreaHeight();
        const int padH   = juce::jmax (4, (int) (totalH * 0.035f));
        const int innerH = totalH - titleH - padH * 2;

        const int topRowH = (int) (innerH * 0.55f);
        const int botRowH = innerH - topRowH - gap;

        const int topY = y + titleH + padH;
        const int botY = topY + topRowH + gap;

        const int innerW   = filterW - 2 * padH;
        const int sepW     = juce::jmax (1, (int) (filterW * 0.006f));
        const int colW     = (innerW - sepW) / 2;

        // Column centre X positions (LPF Cut & LPF Res share lpfCenterX,
        // HPF Cut & HPF Res share hpfCenterX)
        const int lpfCenterX = curX + padH + colW / 2;
        const int hpfCenterX = curX + padH + colW + sepW + colW / 2;

        // Store separator line bounds for drawing in paint()
        filterSeparatorBounds = juce::Rectangle<int> (curX + padH + colW, topY, sepW, innerH);

        // ── Top row: large cutoff knobs, centered in each column ──
        const int largeKnobSize = juce::jmin (colW, topRowH, (int) (largeKnobD * 1.5f));
        const int largeKnobY    = topY + (topRowH - largeKnobSize) / 2;
        lpfCutoff.setBounds (lpfCenterX - largeKnobSize / 2, largeKnobY, largeKnobSize, largeKnobSize);
        hpfCutoff.setBounds (hpfCenterX - largeKnobSize / 2, largeKnobY, largeKnobSize, largeKnobSize);

        // ── Bottom row: LPF Res and HPF Res, aligned with their cutoff knobs ──
        const int smallKnobSize  = juce::jmin (colW / 2, botRowH, (int) (knobD * 1.35f));
        const int smallKnobY     = botY + (botRowH - smallKnobSize) / 2;

        // LPF Res — same X-centre as LPF Cut
        lpfRes.setBounds (lpfCenterX - smallKnobSize / 2, smallKnobY, smallKnobSize, smallKnobSize);

        // HPF Res — same X-centre as HPF Cut
        hpfRes.setBounds (hpfCenterX - smallKnobSize / 2, smallKnobY, smallKnobSize, smallKnobSize);
    }

    curX += filterW + gap;
}

// ─────────────────────────────────────────────────────────────────────────────
// layoutEnvelopeRow() — VCA ENV (left) | VCF ENV (right)
// ADSR knobs arranged horizontally left-to-right, EnvDisplay to the right.
// ─────────────────────────────────────────────────────────────────────────────

void PluginEditor::layoutEnvelopeRow (int x, int y, int totalW, int totalH, int mediumKnobD)
{
    using R = juce::Rectangle<int>;

    const int gap = juce::jmax (6, (int) (totalW * 0.006f));

    const int halfW = (totalW - gap) / 2;
    const int vcaW  = halfW;
    const int vcfW  = halfW;

    int curX = x;

    // ── VCA ENV (left half): ADSR knobs horizontal on left, EnvDisplay right ─
    {
        const R vcaBounds (curX, y, vcaW, totalH);
        ampPanel.setBounds (vcaBounds);

        const int titleH = ampPanel.getTitleAreaHeight();
        const int padH   = juce::jmax (4, (int) (totalH * 0.06f));
        const int innerH = totalH - titleH - padH * 2;

        // Knobs take ~48% of width (4 across), display takes the rest
        const int knobAreaW = (int) (vcaW * 0.48f);
        const int dispW     = vcaW - knobAreaW - padH;

        // Four ADSR knobs in a horizontal row — 25% smaller to open up the row
        const R knobRow (curX + padH, y + titleH + padH,
                         knobAreaW - padH, innerH);
        placeKnobRow ({ &ampAttack, &ampDecay, &ampSustain, &ampRelease },
                      knobRow, (int) (mediumKnobD * 0.75f));

        ampEnvDisplay.setBounds (curX + knobAreaW, y + titleH + padH,
                                 dispW, innerH);
    }

    curX += vcaW + gap;

    // ── VCF ENV (right half) ─────────────────────────────────────────────────
    {
        const R vcfBounds (curX, y, vcfW, totalH);
        env1Panel.setBounds (vcfBounds);

        const int titleH = env1Panel.getTitleAreaHeight();
        const int padH   = juce::jmax (4, (int) (totalH * 0.06f));
        const int innerH = totalH - titleH - padH * 2;

        const int knobAreaW = (int) (vcfW * 0.48f);
        const int dispW     = vcfW - knobAreaW - padH;

        // Four ADSR knobs in a horizontal row — 25% smaller to open up the row
        const R knobRow (curX + padH, y + titleH + padH,
                         knobAreaW - padH, innerH);
        placeKnobRow ({ &env1Attack, &env1Decay, &env1Sustain, &env1Release },
                      knobRow, (int) (mediumKnobD * 0.75f));

        vcfEnvDisplay.setBounds (curX + knobAreaW, y + titleH + padH,
                                 dispW, innerH);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// layoutRow2() — GLIDE/VOICE | LFO1 | LFO2 | LFO3 | LFO4 | TAPE DELAY | AMP
// ─────────────────────────────────────────────────────────────────────────────

void PluginEditor::layoutRow2 (int x, int y, int totalW, int totalH,
                               int knobD, int smallKnobD)
{
    using R = juce::Rectangle<int>;

    const int mediumKnobD = knobD;
    const int gap = juce::jmax (6, (int) (totalW * 0.006f));

    // NOTE: Ratios are normalised against the USABLE width (totalW minus the
    // six inter-panel gaps), so the seven panels always fill the row
    // edge-to-edge. Previously the raw ratios summed to only 0.92 and the
    // leftover "remainderW" was computed but never used, leaving an empty
    // strip on the right. The Amp panel absorbs any integer rounding.
    constexpr float rGlide = 0.10f;
    constexpr float rLfo   = 0.12f;
    constexpr float rTape  = 0.24f;
    constexpr float rAmp   = 0.10f;
    const float ratioSum = rGlide + 4 * rLfo + rTape + rAmp;

    const int usableW = juce::jmax (100, totalW - 6 * gap);
    const int glideW  = (int) (usableW * rGlide / ratioSum);
    const int lfoW    = (int) (usableW * rLfo   / ratioSum);
    const int tapeW   = (int) (usableW * rTape  / ratioSum);
    const int usedW   = glideW + 4 * lfoW + tapeW;
    const int ampW    = juce::jmax (80, usableW - usedW);

    int curX = x;

    // ── GLIDE / VOICE MODE ────────────────────────────────────────────────────
    {
        const R glideBounds (curX, y, glideW, totalH);
        glidePanel.setBounds (glideBounds);

        const int titleH = glidePanel.getTitleAreaHeight();
        const int padH   = juce::jmax (4, (int) (totalH * 0.04f));

        const int knobAreaTop = titleH + padH;
        const R knobArea (curX + padH, y + knobAreaTop,
                          glideW - 2 * padH,
                          (totalH - knobAreaTop - padH) / 2);
        placeKnobRow ({ &glideTime }, knobArea, knobD);

        const int comboH = juce::jmax (20, (int) (totalH * 0.10f));
        const int comboY = y + titleH + padH + knobArea.getHeight() + padH;
        voiceModeBox.setBounds (curX + padH, comboY,
                                glideW - 2 * padH, comboH);
    }

    curX += glideW + gap;

    // ── Helper lambda for LFO panel layout ────────────────────────────────────
    auto layoutLfoPanel = [&](Panel& panel,
                               juce::ComboBox& waveCombo,
                               juce::TextButton& syncButton,
                               LabeledKnob& rateKnob,
                               LabeledKnob& depthKnob,
                               juce::ComboBox& destCombo,
                               LfoDisplay& display,
                               int panelX, int panelY, int panelW, int panelH,
                               int knobD, int gap)
    {
        const R lfoBounds (panelX, panelY, panelW, panelH);
        panel.setBounds (lfoBounds);

        const int titleH = panel.getTitleAreaHeight();
        const int padH   = juce::jmax (4, (int) (panelH * 0.04f));
        const int comboH = juce::jmax (16, (int) (panelH * 0.10f));

        // Top row: waveform combo + SYNC toggle, side by side
        const int syncBtnW = juce::jmax (36, (int) (panelW * 0.24f));
        const int waveW    = panelW - 2 * padH - syncBtnW;
        waveCombo.setBounds (panelX + padH, panelY + titleH + padH,
                             waveW, comboH);
        syncButton.setBounds (panelX + padH + waveW + padH, panelY + titleH + padH,
                              syncBtnW, comboH);

        // Waveform display below the top row
        const int dispH = juce::jmax (16, (int) (panelH * 0.12f));
        display.setBounds (panelX + padH, panelY + titleH + padH + comboH + padH,
                           panelW - 2 * padH, dispH);

        // Rate + Depth knobs — smaller so the sync toggle fits in the panel
        const int knobAreaTop = titleH + padH + comboH + padH + dispH + padH;
        const int lfoKnobD    = juce::jmax (30, (int) (knobD * 0.80f));
        const R knobArea (panelX + padH, panelY + knobAreaTop,
                          panelW - 2 * padH,
                          panelH - knobAreaTop - padH - comboH);
        placeKnobRow ({ &rateKnob, &depthKnob }, knobArea, lfoKnobD);

        // Dest combo at the bottom
        destCombo.setBounds (panelX + padH, panelY + panelH - padH - comboH,
                             panelW - 2 * padH, comboH);
    };

    // ── LFO1 ─────────────────────────────────────────────────────────────────
    layoutLfoPanel (lfo1Panel, lfo1Waveform, lfo1Sync, lfo1Rate, lfo1Depth, lfo1Dest, lfo1Display,
                    curX, y, lfoW, totalH, mediumKnobD, gap);
    curX += lfoW + gap;

    layoutLfoPanel (lfo2Panel, lfo2Waveform, lfo2Sync, lfo2Rate, lfo2Depth, lfo2Dest, lfo2Display,
                    curX, y, lfoW, totalH, mediumKnobD, gap);
    curX += lfoW + gap;

    layoutLfoPanel (lfo3Panel, lfo3Waveform, lfo3Sync, lfo3Rate, lfo3Depth, lfo3Dest, lfo3Display,
                    curX, y, lfoW, totalH, mediumKnobD, gap);
    curX += lfoW + gap;

    layoutLfoPanel (lfo4Panel, lfo4Waveform, lfo4Sync, lfo4Rate, lfo4Depth, lfo4Dest, lfo4Display,
                    curX, y, lfoW, totalH, mediumKnobD, gap);
    curX += lfoW + gap;

    // ── TAPE DELAY ────────────────────────────────────────────────────────────
    {
        const R tapeBounds (curX, y, tapeW, totalH);
        tapeDelayPanel.setBounds (tapeBounds);

        const int titleH = tapeDelayPanel.getTitleAreaHeight();
        const int padH   = juce::jmax (4, (int) (totalH * 0.04f));

        const int knobAreaTop = titleH + padH;
        const int knobAreaH = totalH - knobAreaTop - padH;
        const int knobRowH = (knobAreaH - gap) / 2;

        const R topRow (curX + padH, y + knobAreaTop,
                        tapeW - 2 * padH, knobRowH);
        const R botRow (curX + padH, y + knobAreaTop + knobRowH + gap,
                        tapeW - 2 * padH, knobRowH);

        // Mode combo box takes the left ~1/3 of the top row
        const int modeComboW = (tapeW - 2 * padH) / 3;
        const int modeComboH = juce::jmax (18, (int) (knobRowH * 0.45f));
        const int modeComboX = curX + padH;
        const int modeComboY = y + knobAreaTop + (knobRowH - modeComboH) / 2;
        tapeDelayMode.setBounds (modeComboX, modeComboY, modeComboW, modeComboH);

        // Feedback and Mix knobs take the right ~2/3
        const int knobStartX = modeComboX + modeComboW + padH;
        const int remainingW = (tapeW - 2 * padH) - modeComboW - padH;
        const R fbMixArea (knobStartX, y + knobAreaTop, remainingW, knobRowH);
        placeKnobRow ({ &tapeDelayFeedback, &tapeDelayMix }, fbMixArea, smallKnobD);
        placeKnobRow ({ &tapeDelayTime, &tapeDelayAge, &tapeDelaySat, &tapeDelayWow, &tapeDelayFlutter }, botRow, smallKnobD);

        // Tape delay on/off button — labeled toggle positioned in the
        // title bar area, right-aligned
        const int btnW = juce::jmin (tapeW, juce::jmax (56, (int) (tapeW * 0.18f)));
        const int btnH = juce::jlimit (20, 30, (int) (titleH * 0.8f));
        tapeDelayOnOff.setBounds (tapeBounds.getRight() - btnW - padH,
                                  y + (titleH - btnH) / 2,
                                  btnW, btnH);
    }

    curX += tapeW + gap;

    // ── AMP (master gain + pan) ───────────────────────────────────────────────
    {
        const R ampBounds (curX, y, ampW, totalH);
        ampSectionPanel.setBounds (ampBounds);

        const int titleH = ampSectionPanel.getTitleAreaHeight();
        const int padH   = juce::jmax (4, (int) (totalH * 0.04f));
        const R area (curX + padH, y + titleH + padH,
                      ampW - 2 * padH,
                      totalH - titleH - 2 * padH);
        placeKnobRow ({ &ampGain, &pan }, area, smallKnobD);
    }

    curX += ampW + gap;
}

// ─────────────────────────────────────────────────────────────────────────────
// computeLfoOutput() — simulate an LFO output for UI animation
// ─────────────────────────────────────────────────────────────────────────────

float PluginEditor::computeLfoOutput (int waveform, float phase, float shape) const
{
    switch (waveform)
    {
        case 0: return std::sin (juce::MathConstants<float>::twoPi * phase);
        case 1: return (phase < 0.5f) ? (4.0f * phase - 1.0f) : (3.0f - 4.0f * phase);
        case 2:
        {
            const float duty = 0.1f + 0.8f * shape;
            return (phase < duty) ? 1.0f : -1.0f;
        }
        case 3: return 2.0f * phase - 1.0f;
        case 4: return std::sin (phase * 123.456f) * 0.8f;
        default: return 0.0f;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// placeKnobRow() — evenly distribute knobs across an area, centered in cells
// ─────────────────────────────────────────────────────────────────────────────

void PluginEditor::placeKnobRow (std::initializer_list<juce::Component*> knobs,
                                 juce::Rectangle<int> area,
                                 int knobD,
                                 int /*panelTitleH*/)
{
    const int count = static_cast<int> (knobs.size());
    if (count == 0 || area.getWidth() <= 0 || area.getHeight() <= 0)
        return;

    const int cellW = area.getWidth() / count;
    // Minimum widget height is 66px to prevent clipping: LabeledKnob has a
    // minimum internal knob size of 48px plus a 16px label and 2px gap.
    const int knobWidgetH = juce::jmax (66,
                                        juce::jmin (area.getHeight(),
                                                    (int) (knobD * 1.5f)));

    int i = 0;
    for (auto* knob : knobs)
    {
        if (knob == nullptr) { ++i; continue; }
        const int cellX = area.getX() + i * cellW;
        // Center the knob widget within its cell as a square
        const int knobX = cellX + (cellW - knobWidgetH) / 2;
        const int knobY = area.getY() + (area.getHeight() - knobWidgetH) / 2;
        knob->setBounds (knobX, knobY, knobWidgetH, knobWidgetH);
        ++i;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// placeKnobColumn() — evenly distribute knobs vertically across an area
// ─────────────────────────────────────────────────────────────────────────────

void PluginEditor::placeKnobColumn (std::initializer_list<juce::Component*> knobs,
                                    juce::Rectangle<int> area,
                                    int knobD)
{
    const int count = static_cast<int> (knobs.size());
    if (count == 0 || area.getWidth() <= 0 || area.getHeight() <= 0)
        return;

    const int cellH = area.getHeight() / count;
    // Minimum widget height is 66px to prevent clipping: LabeledKnob has a
    // minimum internal knob size of 48px plus a 16px label and 2px gap.
    const int knobWidgetH = juce::jmax (66,
                                        juce::jmin (cellH,
                                                    (int) (knobD * 1.5f)));
    const int knobYOffset = (cellH - knobWidgetH) / 2;
    // Center horizontally within the cell as a square
    const int knobXOffset = (area.getWidth() - knobWidgetH) / 2;

    int i = 0;
    for (auto* knob : knobs)
    {
        if (knob == nullptr) { ++i; continue; }
        const int cellY = area.getY() + i * cellH;
        knob->setBounds (area.getX() + knobXOffset, cellY + knobYOffset,
                         knobWidgetH, knobWidgetH);
        ++i;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// updatePulseWidthVisibility()
// ─────────────────────────────────────────────────────────────────────────────

void PluginEditor::updatePulseWidthVisibility()
{
    // Only the "Pulse" waveform has a pulse-width control.
    // Pulse is the 4th oscillator waveform (ComboBox ID 4, item index 3).
    const int osc1WaveId = osc1Waveform.getSelectedId();
    const bool osc1ShowPW = (osc1WaveId == 4);
    osc1PulseWidth.setVisible (osc1ShowPW);

    const int osc2WaveId = osc2Waveform.getSelectedId();
    const bool osc2ShowPW = (osc2WaveId == 4);
    osc2PulseWidth.setVisible (osc2ShowPW);
}

// ─────────────────────────────────────────────────────────────────────────────
// syncEnvDisplays() — update EnvDisplay from parameter values
// ─────────────────────────────────────────────────────────────────────────────

void PluginEditor::syncEnvDisplays()
{
    auto& apvts = audioProcessor.getAPVTS();

    auto secondsToNorm = [](float seconds, float maxVal) -> float {
        if (seconds <= 0.0f) return 0.0f;
        return std::sqrt (juce::jlimit (0.0f, 1.0f, seconds / maxVal));
    };

    if (auto* a = apvts.getRawParameterValue (Parameters::paramAmpAttack))
        ampEnvDisplay.setAttack (secondsToNorm (a->load(), 5.0f));
    if (auto* d = apvts.getRawParameterValue (Parameters::paramAmpDecay))
        ampEnvDisplay.setDecay (secondsToNorm (d->load(), 2.0f));
    if (auto* s = apvts.getRawParameterValue (Parameters::paramAmpSustain))
        ampEnvDisplay.setSustain (s->load());
    if (auto* r = apvts.getRawParameterValue (Parameters::paramAmpRelease))
        ampEnvDisplay.setRelease (secondsToNorm (r->load(), 5.0f));

    if (auto* a = apvts.getRawParameterValue (Parameters::paramEnv1Attack))
        vcfEnvDisplay.setAttack (secondsToNorm (a->load(), 5.0f));
    if (auto* d = apvts.getRawParameterValue (Parameters::paramEnv1Decay))
        vcfEnvDisplay.setDecay (secondsToNorm (d->load(), 2.0f));
    if (auto* s = apvts.getRawParameterValue (Parameters::paramEnv1Sustain))
        vcfEnvDisplay.setSustain (s->load());
    if (auto* r = apvts.getRawParameterValue (Parameters::paramEnv1Release))
        vcfEnvDisplay.setRelease (secondsToNorm (r->load(), 5.0f));
}

// ─────────────────────────────────────────────────────────────────────────────
// updateTimeKnobVisibility()
// ─────────────────────────────────────────────────────────────────────────────

void PluginEditor::updateTimeKnobVisibility()
{
    // Show the Time knob only in MS mode (choice ID 7, since items start at 1)
    tapeDelayTime.setVisible (tapeDelayMode.getSelectedId() == 7);
}

// ─────────────────────────────────────────────────────────────────────────────
// updateNoiseTypeCenterText() — show the selected noise colour in the centre
// of the noise type knob (no labels are drawn around the knob).
// ─────────────────────────────────────────────────────────────────────────────

void PluginEditor::updateNoiseTypeCenterText()
{
    const int numTypes = Parameters::noiseTypeChoices.size();
    const int idx = juce::jlimit (0, numTypes - 1,
                                  (int) (noiseTypeKnob.getSlider().getValue() + 0.5f));
    if (idx == lastNoiseTypeIndex)
        return;

    lastNoiseTypeIndex = idx;
    noiseTypeKnob.setCenterText (Parameters::noiseTypeChoices[idx]);
}

// ─────────────────────────────────────────────────────────────────────────────
// applyLfoSyncMode() — enforce the SYNC toggle for one LFO
// ─────────────────────────────────────────────────────────────────────────────

void PluginEditor::applyLfoSyncMode (int lfoIndex)
{
    LabeledKnob&    rate = (lfoIndex == 0 ? lfo1Rate : lfoIndex == 1 ? lfo2Rate : lfoIndex == 2 ? lfo3Rate : lfo4Rate);
    juce::TextButton& btn = (lfoIndex == 0 ? lfo1Sync : lfoIndex == 1 ? lfo2Sync : lfoIndex == 2 ? lfo3Sync : lfo4Sync);
    juce::Slider&  slider = rate.getSlider();
    const juce::String syncId = (lfoIndex == 0 ? Parameters::paramLFO1Sync
                                 : lfoIndex == 1 ? Parameters::paramLFO2Sync
                                 : lfoIndex == 2 ? Parameters::paramLFO3Sync
                                                 : Parameters::paramLFO4Sync);
    auto& apvts = audioProcessor.getAPVTS();

    const bool syncOn = btn.getToggleState();

    // Persist the sync mode as a real boolean parameter.
    if (auto* p = apvts.getParameter (syncId))
        p->setValueNotifyingHost (syncOn ? 1.0f : 0.0f);

    if (syncOn)
    {
        // Enter tempo sync: preserve the knob's current position by mapping it
        // to the nearest of the 12 divisions across the full 0..1 range.
        const float cur = slider.getValue();
        const int idx = Parameters::lfoSyncIndexForValue (cur);
        const float snapped = Parameters::lfoSyncParamValue (idx);

        slider.setRange (0.0, 1.0, 0.001);
        rate.setSnapValuesOnly (lfoSyncSnapValues(), Parameters::lfoSyncDivisionCount);
        rate.clearTextValues(); // no room around the knob — only show the selection in the centre
        slider.setValue (snapped, juce::sendNotificationSync);
        rate.setCenterText (Parameters::lfoSyncLabel (idx));
    }
    else
    {
        // Exit tempo sync: the whole 0..1 range now maps to free-running ms.
        slider.setRange (0.0, 1.0, 0.001);
        rate.clearSnapValues();
        rate.clearTextValues(); // hide the division labels around the knob
        refreshLfoRateLabel (lfoIndex);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// refreshLfoRateLabel() — show the current tempo division (sync ON) or the
// free-running ms period (sync OFF) in the rate knob
// ─────────────────────────────────────────────────────────────────────────────

void PluginEditor::refreshLfoRateLabel (int lfoIndex)
{
    LabeledKnob&    rate = (lfoIndex == 0 ? lfo1Rate : lfoIndex == 1 ? lfo2Rate : lfoIndex == 2 ? lfo3Rate : lfo4Rate);
    juce::TextButton& btn = (lfoIndex == 0 ? lfo1Sync : lfoIndex == 1 ? lfo2Sync : lfoIndex == 2 ? lfo3Sync : lfo4Sync);

    if (btn.getToggleState())
    {
        // Sync ON — show the nearest tempo division in the knob centre.
        rate.setCenterText (Parameters::lfoSyncLabel (Parameters::lfoSyncIndexForValue ((float) rate.getSlider().getValue())));
    }
    else
    {
        // Sync OFF — MS timed: show the current free-running period.
        rate.setCenterText (lfoRateMsLabel (Parameters::lfoFreePeriodMs ((float) rate.getSlider().getValue())));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// randomizePatch()
// ─────────────────────────────────────────────────────────────────────────────

void PluginEditor::randomizePatch()
{
    auto& apvts = audioProcessor.getAPVTS();
    std::mt19937 rng (std::random_device{}());

    auto randomizeFloat = [&](const juce::String& paramId, float min, float max)
    {
        if (auto* param = apvts.getParameter (paramId))
        {
            std::uniform_real_distribution<float> dist (min, max);
            param->setValueNotifyingHost (param->convertTo0to1 (dist (rng)));
        }
    };

    auto randomizeChoice = [&](const juce::String& paramId, int numChoices)
    {
        if (auto* param = apvts.getParameter (paramId))
        {
            if (numChoices <= 1)
                return;
            std::uniform_int_distribution<int> dist (0, numChoices - 1);
            const float value = (float) dist (rng) / (float) (numChoices - 1);
            param->setValueNotifyingHost (value);
        }
    };

    // Waveforms (use full range 0..6 for 7 choices)
    std::uniform_int_distribution<int> waveDist (0, 6);
    if (auto* p = apvts.getParameter (Parameters::paramOsc1Waveform))
        p->setValueNotifyingHost ((float) waveDist (rng) / 6.0f);
    if (auto* p = apvts.getParameter (Parameters::paramOsc2Waveform))
        p->setValueNotifyingHost ((float) waveDist (rng) / 6.0f);

    // Noise type (choice parameter — use randomizeChoice, not randomizeFloat)
    randomizeChoice (Parameters::paramNoiseType, Parameters::noiseTypeChoices.size());

    randomizeFloat (Parameters::paramOsc1Tune, -50.0f, 50.0f);
    randomizeFloat (Parameters::paramOsc2Tune, -50.0f, 50.0f);
    randomizeFloat (Parameters::paramOsc1PulseWidth, 10.0f, 90.0f);
    randomizeFloat (Parameters::paramOsc2PulseWidth, 10.0f, 90.0f);
    randomizeFloat (Parameters::paramOsc1Octave, -1.0f, 1.0f);
    randomizeFloat (Parameters::paramOsc2Octave, -1.0f, 1.0f);
    randomizeFloat (Parameters::paramOsc1Gain, 0.3f, 1.0f);
    randomizeFloat (Parameters::paramOsc2Gain, 0.3f, 1.0f);

    // Filter — randomize LPF first, then constrain HPF to stay below LPF
    const float lpfCutoff = std::uniform_real_distribution<float> (200.0f, 18000.0f) (rng);
    randomizeFloat (Parameters::paramLPFCutoff, lpfCutoff, lpfCutoff);
    const float hpfCutoff = std::uniform_real_distribution<float> (20.0f, std::min (500.0f, lpfCutoff * 0.4f)) (rng);
    randomizeFloat (Parameters::paramHPFCutoff, hpfCutoff, hpfCutoff);
    randomizeFloat (Parameters::paramLPFRes, 0.0f, 0.8f);
    randomizeFloat (Parameters::paramHPFRes, 0.0f, 0.5f);
    randomizeFloat (Parameters::paramLPFDrive, 0.0f, 0.7f);

    // Amp envelope — enforce minimum sustain so sound doesn't die
    randomizeFloat (Parameters::paramAmpAttack, 0.001f, 2.0f);
    randomizeFloat (Parameters::paramAmpDecay, 0.001f, 1.0f);
    randomizeFloat (Parameters::paramAmpSustain, 0.1f, 1.0f);
    randomizeFloat (Parameters::paramAmpRelease, 0.001f, 3.0f);
    randomizeFloat (Parameters::paramEnv1Attack, 0.001f, 2.0f);
    randomizeFloat (Parameters::paramEnv1Decay, 0.001f, 1.0f);
    randomizeFloat (Parameters::paramEnv1Sustain, 0.0f, 1.0f);
    randomizeFloat (Parameters::paramEnv1Release, 0.001f, 3.0f);

    randomizeFloat (Parameters::paramLFO1Rate, 0.0f, 1.0f);
    randomizeFloat (Parameters::paramLFO1Depth, 0.0f, 1.0f);
    randomizeChoice (Parameters::paramLFO1Dest, (int) Parameters::lfoDestinationChoices.size());

    randomizeFloat (Parameters::paramLFO2Rate, 0.0f, 1.0f);
    randomizeFloat (Parameters::paramLFO2Depth, 0.0f, 1.0f);
    randomizeChoice (Parameters::paramLFO2Dest, (int) Parameters::lfoDestinationChoices.size());

    randomizeFloat (Parameters::paramLFO3Rate, 0.0f, 1.0f);
    randomizeFloat (Parameters::paramLFO3Depth, 0.0f, 1.0f);
    randomizeChoice (Parameters::paramLFO3Dest, (int) Parameters::lfoDestinationChoices.size());

    randomizeFloat (Parameters::paramLFO4Rate, 0.0f, 1.0f);
    randomizeFloat (Parameters::paramLFO4Depth, 0.0f, 1.0f);
    randomizeChoice (Parameters::paramLFO4Dest, (int) Parameters::lfoDestinationChoices.size());

    // Mixer levels — ensure at least one source is audible
    const float vco1Level = std::uniform_real_distribution<float> (0.0f, 1.0f) (rng);
    const float vco2Level = std::uniform_real_distribution<float> (0.0f, 1.0f) (rng);
    const float subLevel  = std::uniform_real_distribution<float> (0.0f, 0.8f) (rng);
    randomizeFloat (Parameters::paramMixerVco1Level, vco1Level, vco1Level);
    randomizeFloat (Parameters::paramMixerVco2Level, vco2Level, vco2Level);
    randomizeFloat (Parameters::paramMixerSubLevel, subLevel, subLevel);

    // Guarantee at least one oscillator/sub source is audible
    if (vco1Level < 0.2f && vco2Level < 0.2f && subLevel < 0.1f)
    {
        randomizeFloat (Parameters::paramMixerVco1Level, 0.4f, 1.0f);
    }

    randomizeFloat (Parameters::paramMixerDrive, 0.0f, 0.8f);
    updatePulseWidthVisibility();
    randomizeLocked = true;
    updateRandomizeButtonAppearance();
}

// ─────────────────────────────────────────────────────────────────────────────
// updateRandomizeButtonAppearance()
// ─────────────────────────────────────────────────────────────────────────────

void PluginEditor::updateRandomizeButtonAppearance()
{
    if (randomizeLocked)
    {
        randomizeButton.setButtonText ("RANDOMIZE");
        randomizeButton.setColour (juce::TextButton::buttonColourId, GhostSignalLookAndFeel::knobBody);
        randomizeButton.setColour (juce::TextButton::textColourOffId, GhostSignalLookAndFeel::textSecondary);
    }
    else
    {
        randomizeButton.setButtonText ("RANDOMIZE?");
        randomizeButton.setColour (juce::TextButton::buttonColourId, GhostSignalLookAndFeel::accent);
        randomizeButton.setColour (juce::TextButton::textColourOffId, juce::Colours::black);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// toggleRandomizeLock()
// ─────────────────────────────────────────────────────────────────────────────

void PluginEditor::toggleRandomizeLock()
{
    if (randomizeLocked)
    {
        randomizeLocked = false;
        randomizeFlashCount = 90;
        updateRandomizeButtonAppearance();
    }
    else
    {
        randomizeLocked = true;
        randomizeFlashCount = 0;
        triggerRandomize();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// triggerRandomize()
// ─────────────────────────────────────────────────────────────────────────────

void PluginEditor::triggerRandomize()
{
    randomizePatch();
    randomizeFlashCount = 15;
    updateRandomizeButtonAppearance();
}

// ─────────────────────────────────────────────────────────────────────────────
// timerCallback()
// ─────────────────────────────────────────────────────────────────────────────

void PluginEditor::timerCallback()
{
    if (randomizeFlashCount > 0)
    {
        --randomizeFlashCount;
        if (randomizeFlashCount == 0 && ! randomizeLocked)
        {
            randomizeLocked = true;
            updateRandomizeButtonAppearance();
        }
    }

    updatePulseWidthVisibility();
    updateTimeKnobVisibility();
    updateNoiseTypeCenterText();

    auto& apvts = audioProcessor.getAPVTS();
    const bool tapeEnabled = (apvts.getRawParameterValue (Parameters::paramTapeDelayEnable) != nullptr)
                             && (apvts.getRawParameterValue (Parameters::paramTapeDelayEnable)->load() > 0.5f);
    tapeDelayOnOff.setToggleState (tapeEnabled, juce::dontSendNotification);

    syncEnvDisplays();

    auto syncLfoDisplay = [&](LfoDisplay& display,
                               const juce::String& waveId,
                               const juce::String& rateId,
                               const juce::String& depthId)
    {
        if (auto* p = apvts.getRawParameterValue (waveId))
            display.setWaveform ((int) p->load());
        if (auto* p = apvts.getRawParameterValue (rateId))
            display.setRate (p->load());
        if (auto* p = apvts.getRawParameterValue (depthId))
            display.setDepth (p->load());
    };

    syncLfoDisplay (lfo1Display, Parameters::paramLFO1Waveform, Parameters::paramLFO1Rate, Parameters::paramLFO1Depth);
    syncLfoDisplay (lfo2Display, Parameters::paramLFO2Waveform, Parameters::paramLFO2Rate, Parameters::paramLFO2Depth);
    syncLfoDisplay (lfo3Display, Parameters::paramLFO3Waveform, Parameters::paramLFO3Rate, Parameters::paramLFO3Depth);
    syncLfoDisplay (lfo4Display, Parameters::paramLFO4Waveform, Parameters::paramLFO4Rate, Parameters::paramLFO4Depth);

    auto advancePhase = [](float& phase, float rate)
    {
        phase += rate / 30.0f;
        if (phase >= 1.0f) phase -= 1.0f;
    };

    const float r1 = apvts.getRawParameterValue (Parameters::paramLFO1Rate) ? apvts.getRawParameterValue (Parameters::paramLFO1Rate)->load() : 1.0f;
    const float r2 = apvts.getRawParameterValue (Parameters::paramLFO2Rate) ? apvts.getRawParameterValue (Parameters::paramLFO2Rate)->load() : 1.0f;
    const float r3 = apvts.getRawParameterValue (Parameters::paramLFO3Rate) ? apvts.getRawParameterValue (Parameters::paramLFO3Rate)->load() : 1.0f;
    const float r4 = apvts.getRawParameterValue (Parameters::paramLFO4Rate) ? apvts.getRawParameterValue (Parameters::paramLFO4Rate)->load() : 1.0f;

    advancePhase (lfoPhase1, r1);
    advancePhase (lfoPhase2, r2);
    advancePhase (lfoPhase3, r3);
    advancePhase (lfoPhase4, r4);

    const int d1 = apvts.getRawParameterValue (Parameters::paramLFO1Dest) ? (int) apvts.getRawParameterValue (Parameters::paramLFO1Dest)->load() : 0;
    const int d2 = apvts.getRawParameterValue (Parameters::paramLFO2Dest) ? (int) apvts.getRawParameterValue (Parameters::paramLFO2Dest)->load() : 0;
    const int d3 = apvts.getRawParameterValue (Parameters::paramLFO3Dest) ? (int) apvts.getRawParameterValue (Parameters::paramLFO3Dest)->load() : 0;
    const int d4 = apvts.getRawParameterValue (Parameters::paramLFO4Dest) ? (int) apvts.getRawParameterValue (Parameters::paramLFO4Dest)->load() : 0;

    const int w1 = apvts.getRawParameterValue (Parameters::paramLFO1Waveform) ? (int) apvts.getRawParameterValue (Parameters::paramLFO1Waveform)->load() : 0;
    const int w2 = apvts.getRawParameterValue (Parameters::paramLFO2Waveform) ? (int) apvts.getRawParameterValue (Parameters::paramLFO2Waveform)->load() : 0;
    const int w3 = apvts.getRawParameterValue (Parameters::paramLFO3Waveform) ? (int) apvts.getRawParameterValue (Parameters::paramLFO3Waveform)->load() : 0;
    const int w4 = apvts.getRawParameterValue (Parameters::paramLFO4Waveform) ? (int) apvts.getRawParameterValue (Parameters::paramLFO4Waveform)->load() : 0;


    const float dep1 = apvts.getRawParameterValue (Parameters::paramLFO1Depth) ? apvts.getRawParameterValue (Parameters::paramLFO1Depth)->load() : 0.0f;
    const float dep2 = apvts.getRawParameterValue (Parameters::paramLFO2Depth) ? apvts.getRawParameterValue (Parameters::paramLFO2Depth)->load() : 0.0f;
    const float dep3 = apvts.getRawParameterValue (Parameters::paramLFO3Depth) ? apvts.getRawParameterValue (Parameters::paramLFO3Depth)->load() : 0.0f;
    const float dep4 = apvts.getRawParameterValue (Parameters::paramLFO4Depth) ? apvts.getRawParameterValue (Parameters::paramLFO4Depth)->load() : 0.0f;

    auto getPwNorm = [&](const juce::String& id) -> float
    {
        if (auto* p = apvts.getRawParameterValue (id))
            return juce::jlimit (0.0f, 1.0f, p->load() / 100.0f); // percent -> 0..1
        return 0.5f;
    };

    const float pw1Base = getPwNorm (Parameters::paramOsc1PulseWidth);
    const float pw2Base = getPwNorm (Parameters::paramOsc2PulseWidth);

    float pw1Mod = 0.0f, pw2Mod = 0.0f;
    auto addMod = [&](int dest, float phase, int wave, float shape, float depth)
    {
        const float lfoOut = computeLfoOutput (wave, phase, shape) * depth * 0.3f;
        if (dest == 3) pw1Mod += lfoOut;
        else if (dest == 4) pw2Mod += lfoOut;
    };
    addMod (d1, lfoPhase1, w1, 0.5f, dep1);
    addMod (d2, lfoPhase2, w2, 0.5f, dep2);
    addMod (d3, lfoPhase3, w3, 0.5f, dep3);
    addMod (d4, lfoPhase4, w4, 0.5f, dep4);

    const bool pw1Modulated = (std::abs (pw1Mod) > 0.001f);
    const bool pw2Modulated = (std::abs (pw2Mod) > 0.001f);
    osc1PulseWidth.setModulationIndicator (juce::jlimit (0.0f, 1.0f, pw1Base + pw1Mod), pw1Modulated);
    osc2PulseWidth.setModulationIndicator (juce::jlimit (0.0f, 1.0f, pw2Base + pw2Mod), pw2Modulated);
    if (! pw1Modulated) osc1PulseWidth.clearModulationIndicator();
    if (! pw2Modulated) osc2PulseWidth.clearModulationIndicator();

    auto setKnobLed = [&](LabeledKnob& knob, const juce::String& paramId, float defaultVal)
    {
        auto* p = apvts.getRawParameterValue (paramId);
        if (p != nullptr)
            knob.setLedActive (std::abs (p->load() - defaultVal) > 0.01f);
    };

    setKnobLed (osc1PulseWidth, Parameters::paramOsc1PulseWidth, 50.0f);
    setKnobLed (osc1Octave,     Parameters::paramOsc1Octave, 0.0f);
    setKnobLed (osc1Tune,       Parameters::paramOsc1Tune, 0.0f);
    setKnobLed (osc2PulseWidth, Parameters::paramOsc2PulseWidth, 50.0f);
    setKnobLed (osc2Octave,     Parameters::paramOsc2Octave, 0.0f);
    setKnobLed (osc2Tune,       Parameters::paramOsc2Tune, 0.0f);
    setKnobLed (subOctave,      Parameters::paramSubOctave, 0.0f);
    setKnobLed (noiseGain,      Parameters::paramNoiseGain, 0.5f);
    setKnobLed (mixerVco1Level, Parameters::paramMixerVco1Level, 0.8f);
    setKnobLed (mixerVco2Level, Parameters::paramMixerVco2Level, 0.8f);
    setKnobLed (mixerSubLevel,  Parameters::paramMixerSubLevel, 0.5f);
    setKnobLed (mixerDrive,     Parameters::paramMixerDrive, 0.0f);
    setKnobLed (hpfCutoff,      Parameters::paramHPFCutoff, 20.0f);
    setKnobLed (hpfRes,         Parameters::paramHPFRes, 0.0f);
    setKnobLed (lpfCutoff,      Parameters::paramLPFCutoff, 20000.0f);
    setKnobLed (lpfRes,         Parameters::paramLPFRes, 0.0f);
    setKnobLed (lpfDrive,       Parameters::paramLPFDrive, 0.0f);

    setKnobLed (env1Attack,     Parameters::paramEnv1Attack, 0.01f);
    setKnobLed (env1Decay,      Parameters::paramEnv1Decay, 0.1f);
    setKnobLed (env1Sustain,    Parameters::paramEnv1Sustain, 0.7f);
    setKnobLed (env1Release,    Parameters::paramEnv1Release, 0.3f);
    setKnobLed (ampAttack,      Parameters::paramAmpAttack, 0.01f);
    setKnobLed (ampDecay,       Parameters::paramAmpDecay, 0.1f);
    setKnobLed (ampSustain,     Parameters::paramAmpSustain, 0.7f);
    setKnobLed (ampRelease,     Parameters::paramAmpRelease, 0.3f);
    setKnobLed (lfo1Rate,       Parameters::paramLFO1Rate, 1.0f);
    setKnobLed (lfo1Depth,      Parameters::paramLFO1Depth, 0.5f);
    setKnobLed (lfo2Rate,       Parameters::paramLFO2Rate, 1.0f);
    setKnobLed (lfo2Depth,      Parameters::paramLFO2Depth, 0.5f);
    setKnobLed (lfo3Rate,       Parameters::paramLFO3Rate, 1.0f);
    setKnobLed (lfo3Depth,      Parameters::paramLFO3Depth, 0.5f);
    setKnobLed (lfo4Rate,       Parameters::paramLFO4Rate, 1.0f);
    setKnobLed (lfo4Depth,      Parameters::paramLFO4Depth, 0.5f);
    setKnobLed (ampGain,        Parameters::paramAmpGain, 0.7f);
    setKnobLed (pan,            Parameters::paramPan, 0.5f);
    setKnobLed (glideTime,      Parameters::paramGlideTime, 0.0f);
    setKnobLed (tapeDelayTime,     Parameters::paramTapeDelayTime, 300.0f);
    setKnobLed (tapeDelayFeedback, Parameters::paramTapeDelayFeedback, 0.5f);
    setKnobLed (tapeDelayMix,      Parameters::paramTapeDelayMix, 0.5f);
    setKnobLed (tapeDelayAge,      Parameters::paramTapeDelayAge, 0.5f);
    setKnobLed (tapeDelaySat,      Parameters::paramTapeDelaySat, 0.3f);
    setKnobLed (tapeDelayWow,      Parameters::paramTapeDelayWow, 0.0f);
    setKnobLed (tapeDelayFlutter,  Parameters::paramTapeDelayFlutter, 0.0f);
}
