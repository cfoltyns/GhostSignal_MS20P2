/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Fully responsive plugin editor.
 *              All layout driven by proportional fractions of getWidth()/getHeight().
 */

#include "PluginEditor.h"
#include <random>

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

    masterVolume.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    masterVolume.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 16);
    addAndMakeVisible (masterVolume);

    addAndMakeVisible (masterVolLabel);
    masterVolLabel.setText ("MASTER", juce::dontSendNotification);
    masterVolLabel.setJustificationType (juce::Justification::centred);
    masterVolLabel.setColour (juce::Label::textColourId, juce::Colour (0xFF888888));
    masterVolLabel.setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    masterVolLabel.setFont (juce::Font (10.0f, juce::Font::bold));

    // ── OSC1 ─────────────────────────────────────────────────────────────────
    addAndMakeVisible (osc1Panel);
    addAndMakeVisible (osc1Waveform);
    addAndMakeVisible (osc1PulseWidth);
    addAndMakeVisible (osc1Octave);
    addAndMakeVisible (osc1Tune);
    {
        const float wavePositions[] = { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f };
        osc1Waveform.setSnapToValues (wavePositions, 7);
    }

    // ── OSC2 ─────────────────────────────────────────────────────────────────
    addAndMakeVisible (osc2Panel);
    addAndMakeVisible (osc2Waveform);
    addAndMakeVisible (osc2PulseWidth);
    addAndMakeVisible (osc2Octave);
    addAndMakeVisible (osc2Tune);
    {
        const float wavePositions[] = { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f };
        osc2Waveform.setSnapToValues (wavePositions, 7);
    }

    // ── SUB / NOISE ───────────────────────────────────────────────────────────
    addAndMakeVisible (subPanel);
    addAndMakeVisible (subOctave);
    {
        const float noisePositions[] = { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f };
        noiseType.setSnapToValues(noisePositions, 5);
        noiseType.setTextValues(juce::StringArray{"Brown", "Pink", "White", "Blue", "Violet"}, noisePositions, 5);
        noiseType.setAutoCenterText (true);
    }
    addAndMakeVisible (noisePanel);
    addAndMakeVisible (noiseType);
    addAndMakeVisible (noiseGain);

    // ── MIXER ─────────────────────────────────────────────────────────────────
    addAndMakeVisible (mixerPanel);
    addAndMakeVisible (mixerVco1Level);
    addAndMakeVisible (mixerVco2Level);
    addAndMakeVisible (mixerSubLevel);
    addAndMakeVisible (mixerDrive);

    // ── FILTER ───────────────────────────────────────────────────────────────
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
    addAndMakeVisible (lfo1Shape);
    addAndMakeVisible (lfo1Dest);
    addAndMakeVisible (lfo1Display);

    // ── LFO 2 ─────────────────────────────────────────────────────────────────
    lfo2Waveform.addItemList (lfoWaveItems, 1);
    lfo2Dest.addItemList (lfoDestItems, 1);
    addAndMakeVisible (lfo2Panel);
    addAndMakeVisible (lfo2Waveform);
    addAndMakeVisible (lfo2Rate);
    addAndMakeVisible (lfo2Depth);
    addAndMakeVisible (lfo2Shape);
    addAndMakeVisible (lfo2Dest);
    addAndMakeVisible (lfo2Display);

    // ── LFO 3 ─────────────────────────────────────────────────────────────────
    lfo3Waveform.addItemList (lfoWaveItems, 1);
    lfo3Dest.addItemList (lfoDestItems, 1);
    addAndMakeVisible (lfo3Panel);
    addAndMakeVisible (lfo3Waveform);
    addAndMakeVisible (lfo3Rate);
    addAndMakeVisible (lfo3Depth);
    addAndMakeVisible (lfo3Shape);
    addAndMakeVisible (lfo3Dest);
    addAndMakeVisible (lfo3Display);

    // ── LFO 4 ─────────────────────────────────────────────────────────────────
    lfo4Waveform.addItemList (lfoWaveItems, 1);
    lfo4Dest.addItemList (lfoDestItems, 1);
    addAndMakeVisible (lfo4Panel);
    addAndMakeVisible (lfo4Waveform);
    addAndMakeVisible (lfo4Rate);
    addAndMakeVisible (lfo4Depth);
    addAndMakeVisible (lfo4Shape);
    addAndMakeVisible (lfo4Dest);
    addAndMakeVisible (lfo4Display);

    // ── TAPE DELAY ────────────────────────────────────────────────────────────
    addAndMakeVisible (tapeDelayPanel);
    addAndMakeVisible (tapeDelayMode);
    {
        const float snapPositions[] = { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f };
        const juce::StringArray modeLabels = { "1/2", "1/4", "1/8", "1/16", "1/32", "Slap", "MS" };
        tapeDelayMode.setSnapToValues (snapPositions, 7);
        tapeDelayMode.setTextValues (modeLabels, snapPositions, 7);
        tapeDelayMode.setAutoCenterText (true);
    }
    addAndMakeVisible (tapeDelayTime);
    addAndMakeVisible (tapeDelayFeedback);
    addAndMakeVisible (tapeDelayMix);
    addAndMakeVisible (tapeDelayAge);
    addAndMakeVisible (tapeDelaySat);
    addAndMakeVisible (tapeDelayWow);
    addAndMakeVisible (tapeDelayFlutter);

    // ── TAPE DELAY ON/OFF BUTTON (circular power button) ──────────────────────
    addAndMakeVisible (tapeDelayOnOff);
    tapeDelayOnOff.setClickingTogglesState (true);
    tapeDelayOnOff.setButtonText ("");
    tapeDelayOnOff.setColour (juce::TextButton::buttonColourId, juce::Colour (0xFF333333));
    tapeDelayOnOff.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xFFDB4437));
    tapeDelayOnOff.setColour (juce::TextButton::textColourOffId, juce::Colours::transparentBlack);
    tapeDelayOnOff.setColour (juce::TextButton::textColourOnId, juce::Colours::black);

    // Mode knob: show time division in the center, update as user turns it
    tapeDelayMode.getSlider().onValueChange = [this]
    {
        updateTimeKnobVisibility();
        const float mode = tapeDelayMode.getSlider().getValue();
        const juce::StringArray labels = { "1/2", "1/4", "1/8", "1/16", "1/32", "Slap", "MS" };
        const int idx = juce::jlimit (0, labels.size() - 1, (int) std::lround (mode));
        tapeDelayMode.setCenterText (labels[idx]);
    };
    tapeDelayMode.setCenterText ("1/2");

    // ── RANDOMIZE BUTTON (two-click confirmation) ─────────────────────────────
    addAndMakeVisible (randomizeButton);
    randomizeLocked = true;
    updateRandomizeButtonAppearance();
    randomizeButton.onClick = [this] { toggleRandomizeLock(); };

    // ── Parameter attachments ─────────────────────────────────────────────────
    auto& apvts = audioProcessor.getAPVTS();

    masterVolAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramMasterVolume, masterVolume);
    osc1WaveAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramOsc1Waveform, osc1Waveform.getSlider());
    osc1PulseWidthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramOsc1PulseWidth, osc1PulseWidth.getSlider());
    osc1OctaveAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramOsc1Octave,   osc1Octave.getSlider());
    osc1TuneAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramOsc1Tune,     osc1Tune.getSlider());
    osc2WaveAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramOsc2Waveform, osc2Waveform.getSlider());
    osc2PulseWidthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramOsc2PulseWidth, osc2PulseWidth.getSlider());
    osc2OctaveAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramOsc2Octave,   osc2Octave.getSlider());
    osc2TuneAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramOsc2Tune,     osc2Tune.getSlider());
    subOctaveAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramSubOctave,    subOctave.getSlider());
    noiseTypeAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>   (apvts, Parameters::paramNoiseType,    noiseType.getSlider());
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
    lfo1ShapeAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramLFO1Shape, lfo1Shape.getSlider());
    lfo1DestAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, Parameters::paramLFO1Dest,  lfo1Dest);

    // LFO2 attachments
    lfo2WaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, Parameters::paramLFO2Waveform, lfo2Waveform);
    lfo2RateAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramLFO2Rate,  lfo2Rate.getSlider());
    lfo2DepthAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramLFO2Depth, lfo2Depth.getSlider());
    lfo2ShapeAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramLFO2Shape, lfo2Shape.getSlider());
    lfo2DestAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, Parameters::paramLFO2Dest,  lfo2Dest);

    // LFO3 attachments
    lfo3WaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, Parameters::paramLFO3Waveform, lfo3Waveform);
    lfo3RateAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramLFO3Rate,  lfo3Rate.getSlider());
    lfo3DepthAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramLFO3Depth, lfo3Depth.getSlider());
    lfo3ShapeAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramLFO3Shape, lfo3Shape.getSlider());
    lfo3DestAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, Parameters::paramLFO3Dest,  lfo3Dest);

    // LFO4 attachments
    lfo4WaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, Parameters::paramLFO4Waveform, lfo4Waveform);
    lfo4RateAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramLFO4Rate,  lfo4Rate.getSlider());
    lfo4DepthAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramLFO4Depth, lfo4Depth.getSlider());
    lfo4ShapeAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramLFO4Shape, lfo4Shape.getSlider());
    lfo4DestAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, Parameters::paramLFO4Dest,  lfo4Dest);

    ampGainAttachment     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramAmpGain,      ampGain.getSlider());
    panAttachment         = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramPan,          pan.getSlider());
    glideTimeAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramGlideTime,    glideTime.getSlider());
    voiceModeAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, Parameters::paramVoiceMode,    voiceModeBox);
    tapeDelayEnableAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, Parameters::paramTapeDelayEnable, tapeDelayOnOff);
    tapeDelayTimeModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, Parameters::paramTapeDelayTimeMode, tapeDelayMode.getSlider());
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

    // ── Set improved knob sensitivity for all rotary sliders ──────────────────
    auto setKnobSensitivity = [](juce::Slider& slider)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                                    juce::MathConstants<float>::pi * 2.75f,
                                    true);
        slider.setMouseDragSensitivity (120);
    };

    setKnobSensitivity (masterVolume);
    setKnobSensitivity (osc1PulseWidth.getSlider());
    setKnobSensitivity (osc1Octave.getSlider());
    setKnobSensitivity (osc1Tune.getSlider());
    setKnobSensitivity (osc2PulseWidth.getSlider());
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
    setKnobSensitivity (lfo1Shape.getSlider());
    setKnobSensitivity (lfo2Rate.getSlider());
    setKnobSensitivity (lfo2Depth.getSlider());
    setKnobSensitivity (lfo2Shape.getSlider());
    setKnobSensitivity (lfo3Rate.getSlider());
    setKnobSensitivity (lfo3Depth.getSlider());
    setKnobSensitivity (lfo3Shape.getSlider());
    setKnobSensitivity (lfo4Rate.getSlider());
    setKnobSensitivity (lfo4Depth.getSlider());
    setKnobSensitivity (lfo4Shape.getSlider());
    setKnobSensitivity (ampGain.getSlider());
    setKnobSensitivity (pan.getSlider());
    setKnobSensitivity (glideTime.getSlider());
    setKnobSensitivity (tapeDelayMode.getSlider());
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

    {
        juce::ColourGradient bg;
        bg.point1 = { 0.0f, 0.0f };
        bg.point2 = { 0.0f, h };
        bg.addColour (0.0f,  juce::Colour (0xFF101010));
        bg.addColour (0.45f, juce::Colour (0xFF0C0C0C));
        bg.addColour (1.0f,  juce::Colour (0xFF080808));
        g.setGradientFill (bg);
        g.fillRect (bounds);
    }

    {
        g.setColour (juce::Colour (0x05FFFFFF));
        for (float lineY = 0.0f; lineY < h; lineY += 3.0f)
            g.drawHorizontalLine (static_cast<int> (lineY), 0.0f, w);
    }

    {
        g.setColour (juce::Colour (0x08FFFFFF));
        g.drawVerticalLine (static_cast<int> (w * 0.36f), 0.0f, h);
        g.drawVerticalLine (static_cast<int> (w * 0.72f), 0.0f, h);
    }

    g.setColour (juce::Colour (0x1AFFFFFF));
    g.drawHorizontalLine (0, 0.0f, w);

    g.setColour (juce::Colour (0x55000000));
    g.drawHorizontalLine (static_cast<int> (h) - 1, 0.0f, w);

    {
        const int margin     = juce::jmax (12, (int) (w * 0.015f));
        const int headerH    = juce::jmax (36, (int) (h * 0.055f));
        const int contentTop = margin + headerH + margin;
        const int contentH   = static_cast<int> (h) - contentTop - margin;
        const int row1H      = (int) (contentH * 0.42f);
        const int dividerY   = contentTop + row1H + margin / 2;

        g.setColour (juce::Colour (0x30FFFFFF));
        g.drawHorizontalLine (dividerY, static_cast<float> (margin), w - static_cast<float> (margin));
        g.setColour (juce::Colour (0x10000000));
        g.drawHorizontalLine (dividerY + 1, static_cast<float> (margin), w - static_cast<float> (margin));

        // Draw lightning bolt badge near logo
        const int logoW = juce::jmax (180, (int) (w * 0.20f));
        const float badgeX = margin + logoW + 15.0f;
        const float badgeY = margin + (headerH - 24.0f) / 2.0f;
        const float badgeW = 12.0f;
        const float badgeH = 24.0f;
        
        juce::Path bolt;
        bolt.startNewSubPath(badgeX + badgeW * 0.8f, badgeY);
        bolt.lineTo(badgeX + badgeW * 0.1f, badgeY + badgeH * 0.55f);
        bolt.lineTo(badgeX + badgeW * 0.5f, badgeY + badgeH * 0.55f);
        bolt.lineTo(badgeX + badgeW * 0.2f, badgeY + badgeH);
        bolt.lineTo(badgeX + badgeW * 0.9f, badgeY + badgeH * 0.45f);
        bolt.lineTo(badgeX + badgeW * 0.5f, badgeY + badgeH * 0.45f);
        bolt.closeSubPath();
        
        g.setColour(juce::Colour(0xFFDB4437));
        g.fillPath(bolt);
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

    const int knobD    = juce::jlimit (44, 88, (int) (w * 0.055f));

    layoutHeaderBar (margin, headerH, knobD);

    const int contentTop = margin + headerH + margin;
    const int contentH   = h - contentTop - margin;

    // Three-row split: 42% / 20% / 38% — with more room for env row
    const int row1H   = (int) (contentH * 0.42f);
    const int rowGap  = margin;
    const int envRowH = (int) (contentH * 0.20f);
    const int row2H   = contentH - row1H - envRowH - 2 * rowGap;
    const int contentW = w - 2 * margin;

    layoutRow1 (margin, contentTop, contentW, row1H, knobD);
    layoutEnvelopeRow (margin, contentTop + row1H + rowGap, contentW, envRowH, knobD);
    layoutRow2 (margin, contentTop + row1H + envRowH + 2 * rowGap, contentW, row2H, knobD);

    updatePulseWidthVisibility();
    syncEnvDisplays();
}

// ─────────────────────────────────────────────────────────────────────────────
// layoutHeaderBar()
// ─────────────────────────────────────────────────────────────────────────────

void PluginEditor::layoutHeaderBar (int margin, int headerH, int largeKnobD)
{
    const int w = getWidth();

    // Master volume knob: right-aligned in header
    const int volSize    = largeKnobD;
    const int volLabelH  = 14;
    const int volTotalH  = volSize + volLabelH;
    const int volX       = w - margin - volSize;
    const int volCentreY = margin + (headerH - volTotalH) / 2;

    masterVolume.setBounds  (volX, volCentreY, volSize, volSize);
    masterVolLabel.setBounds (volX - 6, volCentreY + volSize + 1, volSize + 12, volLabelH);

    // Randomize button: between title and master volume
    const int btnW = juce::jmax (80, (int) (w * 0.07f));
    const int btnH = juce::jmax (22, (int) (headerH * 0.5f));
    const int btnX = volX - margin - btnW;
    const int btnY = margin + (headerH - btnH) / 2;
    randomizeButton.setBounds (btnX, btnY, btnW, btnH);

    // Logo text: left-aligned, space out with a wider gap
    const int logoW = btnX - margin - 3 * margin;
    logoComponent.setBounds (margin, margin, logoW, headerH);
}

// ─────────────────────────────────────────────────────────────────────────────
// layoutRow1() — OSC | SUB/NOISE | MIXER | FILTER
// ─────────────────────────────────────────────────────────────────────────────

void PluginEditor::layoutRow1 (int x, int y, int totalW, int totalH, int knobD)
{
    using R = juce::Rectangle<int>;

    const int mediumKnobD = knobD;
    const int smallKnobD  = (int) (knobD * 0.75f);
    const int gap = juce::jmax (6, (int) (totalW * 0.006f));

    // Column widths — more breathing room between sections
    const int oscW    = (int) (totalW * 0.24f);
    const int subW    = (int) (totalW * 0.10f);
    const int mixerW  = (int) (totalW * 0.20f);
    const int filterW = (int) (totalW * 0.26f);
    const int spacerW = totalW - oscW - subW - mixerW - filterW - 4 * gap;

    // Half-height for OSC panels (stacked vertically)
    const int oscHalf = (totalH - gap) / 2;

    int curX = x;

    // ── OSC1 (top half) — Wave knob + Octave/Tune row ────────────────────────
    {
        const R osc1Bounds (curX, y, oscW, oscHalf);
        osc1Panel.setBounds (osc1Bounds);

        const int titleH  = osc1Panel.getTitleAreaHeight();
        const int padH    = juce::jmax (4, (int) (oscHalf * 0.04f));

        // Waveform knob + Octave + Tune in one row
        const int knobAreaTop = titleH + padH;
        const int knobRowH    = (int) ((oscHalf - knobAreaTop - padH) * 0.55f);
        const R knobArea (curX + padH, y + knobAreaTop,
                          oscW - 2 * padH, knobRowH);
        placeKnobRow ({ &osc1Waveform, &osc1Octave, &osc1Tune }, knobArea, mediumKnobD);

        // PW knob centered below (hidden unless Pulse waveform)
        const int pwRowTop = knobAreaTop + knobRowH + padH;
        const int pwRowH   = oscHalf - pwRowTop - padH;
        const int pwW      = juce::jmin (oscW - 2 * padH, (int) (mediumKnobD * 1.3f));
        const int pwX      = curX + (oscW - pwW) / 2;
        osc1PulseWidth.setBounds (pwX, y + pwRowTop, pwW, pwRowH);
    }

    // ── OSC2 (bottom half) ───────────────────────────────────────────────────
    {
        const int osc2Y = y + oscHalf + gap;
        const R osc2Bounds (curX, osc2Y, oscW, oscHalf);
        osc2Panel.setBounds (osc2Bounds);

        const int titleH  = osc2Panel.getTitleAreaHeight();
        const int padH    = juce::jmax (4, (int) (oscHalf * 0.04f));

        const int knobAreaTop = titleH + padH;
        const int knobRowH    = (int) ((oscHalf - knobAreaTop - padH) * 0.55f);
        const R knobArea (curX + padH, osc2Y + knobAreaTop,
                          oscW - 2 * padH, knobRowH);
        placeKnobRow ({ &osc2Waveform, &osc2Octave, &osc2Tune }, knobArea, mediumKnobD);

        const int pwRowTop = knobAreaTop + knobRowH + padH;
        const int pwRowH   = oscHalf - pwRowTop - padH;
        const int pwW      = juce::jmin (oscW - 2 * padH, (int) (mediumKnobD * 1.3f));
        const int pwX      = curX + (oscW - pwW) / 2;
        osc2PulseWidth.setBounds (pwX, osc2Y + pwRowTop, pwW, pwRowH);
    }

    curX += oscW + gap;

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

        const int noiseComboH = juce::jmax (18, (int) (noiseH * 0.12f));
        const int noiseKnobAreaTop = noiseTitleH + padH + noiseComboH + padH;

        noiseType.setBounds (curX + padH, y + subH + gap + noiseTitleH + padH,
                             subW - 2 * padH, noiseComboH);

        const R noiseArea (curX + padH, y + subH + gap + noiseKnobAreaTop,
                           subW - 2 * padH,
                           noiseH - noiseKnobAreaTop - padH);
        placeKnobRow ({ &noiseGain }, noiseArea, mediumKnobD);
    }

    curX += subW + gap;

    // ── MIXER (horizontal row of 4 knobs) ─────────────────────────────────────
    {
        const R mixerBounds (curX, y, mixerW, totalH);
        mixerPanel.setBounds (mixerBounds);

        const int titleH = mixerPanel.getTitleAreaHeight();
        const int padH   = juce::jmax (4, (int) (totalH * 0.04f));
        const int innerH = totalH - titleH - padH * 2;

        const R mixerArea (curX + padH, y + titleH + padH,
                           mixerW - 2 * padH, innerH);

        // Four knobs horizontal: VCO1 Lvl, VCO2 Lvl, Sub Lvl, Drive
        placeKnobRow ({ &mixerVco1Level, &mixerVco2Level, &mixerSubLevel, &mixerDrive },
                      mixerArea, smallKnobD);
    }

    curX += mixerW + gap;

    // ── FILTER (full height, redesigned) ────────────────────────────────────────
    {
        const R filterBounds (curX, y, filterW, totalH);
        filterPanel.setBounds (filterBounds);

        const int titleH = filterPanel.getTitleAreaHeight();
        const int padH   = juce::jmax (4, (int) (totalH * 0.035f));
        const int innerH = totalH - titleH - padH * 2;

        const int topRowH = (int) (innerH * 0.55f);
        
        // Explicit layout for Filter
        const int colW = (filterW - 2 * padH) / 2;
        const int topY = y + titleH + padH;
        
        const int largeKnobWidgetH = juce::jmin(topRowH, (int)(knobD * 1.35f));
        const int largeKnobY = topY + (topRowH - largeKnobWidgetH) / 2;
        
        lpfCutoff.setBounds(curX + padH, largeKnobY, colW, largeKnobWidgetH);
        hpfCutoff.setBounds(curX + padH + colW, largeKnobY, colW, largeKnobWidgetH);
        
        const int botY = topY + topRowH + gap;
        const int botRowH = innerH - topRowH - gap;
        const int smallKnobWidgetH = juce::jmin(botRowH, (int)(knobD * 1.35f));
        const int smallKnobY = botY + (botRowH - smallKnobWidgetH) / 2;
        
        // Bottom row: LPF Res, HPF Res, LPF Drive (3 evenly spaced)
        const int botRowW  = filterW - 2 * padH;
        const int botColW  = botRowW / 3;
        const int botKnobW = juce::jmin (botColW, colW / 2);
        lpfRes.setBounds   (curX + padH + (botColW - botKnobW) / 2, smallKnobY, botKnobW, smallKnobWidgetH);
        hpfRes.setBounds   (curX + padH + botColW + (botColW - botKnobW) / 2, smallKnobY, botKnobW, smallKnobWidgetH);
        lpfDrive.setBounds (curX + padH + 2 * botColW + (botColW - botKnobW) / 2, smallKnobY, botKnobW, smallKnobWidgetH);
        

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

        // Four ADSR knobs in a horizontal row
        const R knobRow (curX + padH, y + titleH + padH,
                         knobAreaW - padH, innerH);
        placeKnobRow ({ &ampAttack, &ampDecay, &ampSustain, &ampRelease },
                      knobRow, mediumKnobD);

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

        const R knobRow (curX + padH, y + titleH + padH,
                         knobAreaW - padH, innerH);
        placeKnobRow ({ &env1Attack, &env1Decay, &env1Sustain, &env1Release },
                      knobRow, mediumKnobD);

        vcfEnvDisplay.setBounds (curX + knobAreaW, y + titleH + padH,
                                 dispW, innerH);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// layoutRow2() — GLIDE/VOICE | LFO1 | LFO2 | LFO3 | LFO4 | TAPE DELAY | AMP
// ─────────────────────────────────────────────────────────────────────────────

void PluginEditor::layoutRow2 (int x, int y, int totalW, int totalH, int knobD)
{
    using R = juce::Rectangle<int>;

    const int mediumKnobD = knobD;
    const int smallKnobD  = (int) (knobD * 0.75f);
    const int gap = juce::jmax (6, (int) (totalW * 0.006f));

    const int glideW  = (int) (totalW * 0.10f);
    const int lfoW    = (int) (totalW * 0.12f);
    const int tapeW   = (int) (totalW * 0.24f);
    const int ampW    = (int) (totalW * 0.10f);
    const int remainderW = totalW - glideW - 4 * lfoW - tapeW - ampW - 6 * gap;

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
                               LabeledKnob& rateKnob,
                               LabeledKnob& depthKnob,
                               LabeledKnob& shapeKnob,
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

        waveCombo.setBounds (panelX + padH, panelY + titleH + padH,
                             panelW - 2 * padH, comboH);

        const int dispH = juce::jmax (16, (int) (panelH * 0.12f));
        display.setBounds (panelX + padH, panelY + titleH + padH + comboH + padH,
                           panelW - 2 * padH, dispH);

        const int knobAreaTop = titleH + padH + comboH + padH + dispH + padH;
        const R knobArea (panelX + padH, panelY + knobAreaTop,
                          panelW - 2 * padH,
                          panelH - knobAreaTop - padH - comboH);
        placeKnobColumn ({ &rateKnob, &depthKnob, &shapeKnob }, knobArea, mediumKnobD);

        destCombo.setBounds (panelX + padH, panelY + panelH - padH - comboH,
                             panelW - 2 * padH, comboH);
    };

    // ── LFO1 ─────────────────────────────────────────────────────────────────
    layoutLfoPanel (lfo1Panel, lfo1Waveform, lfo1Rate, lfo1Depth, lfo1Shape, lfo1Dest, lfo1Display,
                    curX, y, lfoW, totalH, mediumKnobD, gap);
    curX += lfoW + gap;

    layoutLfoPanel (lfo2Panel, lfo2Waveform, lfo2Rate, lfo2Depth, lfo2Shape, lfo2Dest, lfo2Display,
                    curX, y, lfoW, totalH, mediumKnobD, gap);
    curX += lfoW + gap;

    layoutLfoPanel (lfo3Panel, lfo3Waveform, lfo3Rate, lfo3Depth, lfo3Shape, lfo3Dest, lfo3Display,
                    curX, y, lfoW, totalH, mediumKnobD, gap);
    curX += lfoW + gap;

    layoutLfoPanel (lfo4Panel, lfo4Waveform, lfo4Rate, lfo4Depth, lfo4Shape, lfo4Dest, lfo4Display,
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

        placeKnobRow ({ &tapeDelayMode, &tapeDelayFeedback, &tapeDelayMix }, topRow, smallKnobD);
        placeKnobRow ({ &tapeDelayTime, &tapeDelayAge, &tapeDelaySat, &tapeDelayWow, &tapeDelayFlutter }, botRow, smallKnobD);

        const int btnSize = juce::jmin (tapeBounds.getHeight() / 5, 28);
        tapeDelayOnOff.setBounds (tapeBounds.getRight() - btnSize - 4,
                                  tapeBounds.getY() + 4,
                                  btnSize, btnSize);
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
// placeKnobRow() — evenly distribute knobs across an area
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
    const int knobWidgetH = juce::jmin (area.getHeight(),
                                        (int) (knobD * 1.35f));
    const int knobY = area.getY() + (area.getHeight() - knobWidgetH) / 2;

    int i = 0;
    for (auto* knob : knobs)
    {
        if (knob == nullptr) { ++i; continue; }
        const int cellX = area.getX() + i * cellW;
        knob->setBounds (cellX, knobY, cellW, knobWidgetH);
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
    const int knobWidgetH = juce::jmin (cellH, (int) (knobD * 1.35f));
    const int knobYOffset = (cellH - knobWidgetH) / 2;

    int i = 0;
    for (auto* knob : knobs)
    {
        if (knob == nullptr) { ++i; continue; }
        const int cellY = area.getY() + i * cellH;
        knob->setBounds (area.getX(), cellY + knobYOffset, area.getWidth(), knobWidgetH);
        ++i;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// updatePulseWidthVisibility()
// ─────────────────────────────────────────────────────────────────────────────

void PluginEditor::updatePulseWidthVisibility()
{
    // VCO1 waveform slider value 3 = Pulse
    const float osc1Wave = osc1Waveform.getSlider().getValue();
    const bool osc1ShowPW = (std::abs (osc1Wave - 3.0f) < 0.5f);
    osc1PulseWidth.setVisible (osc1ShowPW);

    const float osc2Wave = osc2Waveform.getSlider().getValue();
    const bool osc2ShowPW = (std::abs (osc2Wave - 3.0f) < 0.5f);
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
    // Show the Time knob only in MS mode (value 6)
    const float mode = tapeDelayMode.getSlider().getValue();
    tapeDelayTime.setVisible (std::abs (mode - 6.0f) < 0.5f);
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
            std::uniform_int_distribution<int> dist (0, numChoices - 1);
            const float value = (float) dist (rng) / (float) (numChoices - 1);
            param->setValueNotifyingHost (value);
        }
    };

    std::uniform_int_distribution<int> waveDist (0, 4);
    if (auto* p = apvts.getParameter (Parameters::paramOsc1Waveform))
        p->setValueNotifyingHost ((float) waveDist (rng) / 6.0f);
    if (auto* p = apvts.getParameter (Parameters::paramOsc2Waveform))
        p->setValueNotifyingHost ((float) waveDist (rng) / 6.0f);

    randomizeFloat (Parameters::paramOsc1Tune, -50.0f, 50.0f);
    randomizeFloat (Parameters::paramOsc2Tune, -50.0f, 50.0f);
    randomizeFloat (Parameters::paramOsc1PulseWidth, 0.1f, 0.9f);
    randomizeFloat (Parameters::paramOsc2PulseWidth, 0.1f, 0.9f);
    randomizeFloat (Parameters::paramOsc1Octave, -1.0f, 1.0f);
    randomizeFloat (Parameters::paramOsc2Octave, -1.0f, 1.0f);
    randomizeFloat (Parameters::paramOsc1Gain, 0.3f, 1.0f);
    randomizeFloat (Parameters::paramOsc2Gain, 0.3f, 1.0f);

    randomizeFloat (Parameters::paramLPFCutoff, 200.0f, 18000.0f);
    randomizeFloat (Parameters::paramLPFRes, 0.0f, 0.8f);
    randomizeFloat (Parameters::paramHPFCutoff, 20.0f, 500.0f);
    randomizeFloat (Parameters::paramHPFRes, 0.0f, 0.5f);
    randomizeFloat (Parameters::paramLPFDrive, 0.0f, 0.7f);

    randomizeFloat (Parameters::paramAmpAttack, 0.001f, 2.0f);
    randomizeFloat (Parameters::paramAmpDecay, 0.001f, 1.0f);
    randomizeFloat (Parameters::paramAmpSustain, 0.0f, 1.0f);
    randomizeFloat (Parameters::paramAmpRelease, 0.001f, 3.0f);
    randomizeFloat (Parameters::paramEnv1Attack, 0.001f, 2.0f);
    randomizeFloat (Parameters::paramEnv1Decay, 0.001f, 1.0f);
    randomizeFloat (Parameters::paramEnv1Sustain, 0.0f, 1.0f);
    randomizeFloat (Parameters::paramEnv1Release, 0.001f, 3.0f);

    randomizeFloat (Parameters::paramLFO1Rate, 0.1f, 10.0f);
    randomizeFloat (Parameters::paramLFO1Depth, 0.0f, 1.0f);
    randomizeFloat (Parameters::paramLFO1Shape, 0.0f, 1.0f);
    randomizeChoice (Parameters::paramLFO1Dest, (int) Parameters::lfoDestinationChoices.size());

    randomizeFloat (Parameters::paramLFO2Rate, 0.1f, 10.0f);
    randomizeFloat (Parameters::paramLFO2Depth, 0.0f, 1.0f);
    randomizeFloat (Parameters::paramLFO2Shape, 0.0f, 1.0f);
    randomizeChoice (Parameters::paramLFO2Dest, (int) Parameters::lfoDestinationChoices.size());

    randomizeFloat (Parameters::paramLFO3Rate, 0.1f, 10.0f);
    randomizeFloat (Parameters::paramLFO3Depth, 0.0f, 1.0f);
    randomizeFloat (Parameters::paramLFO3Shape, 0.0f, 1.0f);
    randomizeChoice (Parameters::paramLFO3Dest, (int) Parameters::lfoDestinationChoices.size());

    randomizeFloat (Parameters::paramLFO4Rate, 0.1f, 10.0f);
    randomizeFloat (Parameters::paramLFO4Depth, 0.0f, 1.0f);
    randomizeFloat (Parameters::paramLFO4Shape, 0.0f, 1.0f);
    randomizeChoice (Parameters::paramLFO4Dest, (int) Parameters::lfoDestinationChoices.size());

    randomizeFloat (Parameters::paramMixerVco1Level, 0.0f, 1.0f);
    randomizeFloat (Parameters::paramMixerVco2Level, 0.0f, 1.0f);
    randomizeFloat (Parameters::paramMixerSubLevel, 0.0f, 0.8f);
    randomizeFloat (Parameters::paramMixerDrive, 0.0f, 0.8f);

    randomizeFloat (Parameters::paramGlideTime, 0.0f, 0.5f);

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
        randomizeButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xFF333333));
        randomizeButton.setColour (juce::TextButton::textColourOffId, juce::Colour (0xFF888888));
    }
    else
    {
        randomizeButton.setButtonText ("RANDOMIZE?");
        randomizeButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xFFDB4437));
        randomizeButton.setColour (juce::TextButton::textColourOffId, juce::Colour (0xFF000000));
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

    auto& apvts = audioProcessor.getAPVTS();
    const bool tapeEnabled = (apvts.getRawParameterValue (Parameters::paramTapeDelayEnable) != nullptr)
                             && (apvts.getRawParameterValue (Parameters::paramTapeDelayEnable)->load() > 0.5f);
    tapeDelayOnOff.setToggleState (tapeEnabled, juce::dontSendNotification);

    syncEnvDisplays();

    auto syncLfoDisplay = [&](LfoDisplay& display,
                               const juce::String& waveId,
                               const juce::String& rateId,
                               const juce::String& depthId,
                               const juce::String& shapeId)
    {
        if (auto* p = apvts.getRawParameterValue (waveId))
            display.setWaveform ((int) p->load());
        if (auto* p = apvts.getRawParameterValue (rateId))
            display.setRate (p->load());
        if (auto* p = apvts.getRawParameterValue (depthId))
            display.setDepth (p->load());
        if (auto* p = apvts.getRawParameterValue (shapeId))
            display.setShape (p->load());
    };

    syncLfoDisplay (lfo1Display, Parameters::paramLFO1Waveform, Parameters::paramLFO1Rate, Parameters::paramLFO1Depth, Parameters::paramLFO1Shape);
    syncLfoDisplay (lfo2Display, Parameters::paramLFO2Waveform, Parameters::paramLFO2Rate, Parameters::paramLFO2Depth, Parameters::paramLFO2Shape);
    syncLfoDisplay (lfo3Display, Parameters::paramLFO3Waveform, Parameters::paramLFO3Rate, Parameters::paramLFO3Depth, Parameters::paramLFO3Shape);
    syncLfoDisplay (lfo4Display, Parameters::paramLFO4Waveform, Parameters::paramLFO4Rate, Parameters::paramLFO4Depth, Parameters::paramLFO4Shape);

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

    const float s1 = apvts.getRawParameterValue (Parameters::paramLFO1Shape) ? apvts.getRawParameterValue (Parameters::paramLFO1Shape)->load() : 0.5f;
    const float s2 = apvts.getRawParameterValue (Parameters::paramLFO2Shape) ? apvts.getRawParameterValue (Parameters::paramLFO2Shape)->load() : 0.5f;
    const float s3 = apvts.getRawParameterValue (Parameters::paramLFO3Shape) ? apvts.getRawParameterValue (Parameters::paramLFO3Shape)->load() : 0.5f;
    const float s4 = apvts.getRawParameterValue (Parameters::paramLFO4Shape) ? apvts.getRawParameterValue (Parameters::paramLFO4Shape)->load() : 0.5f;

    const float dep1 = apvts.getRawParameterValue (Parameters::paramLFO1Depth) ? apvts.getRawParameterValue (Parameters::paramLFO1Depth)->load() : 0.0f;
    const float dep2 = apvts.getRawParameterValue (Parameters::paramLFO2Depth) ? apvts.getRawParameterValue (Parameters::paramLFO2Depth)->load() : 0.0f;
    const float dep3 = apvts.getRawParameterValue (Parameters::paramLFO3Depth) ? apvts.getRawParameterValue (Parameters::paramLFO3Depth)->load() : 0.0f;
    const float dep4 = apvts.getRawParameterValue (Parameters::paramLFO4Depth) ? apvts.getRawParameterValue (Parameters::paramLFO4Depth)->load() : 0.0f;

    auto getPwNorm = [&](const juce::String& id) -> float
    {
        if (auto* p = apvts.getRawParameterValue (id))
            return juce::jlimit (0.0f, 1.0f, (p->load() - 0.01f) / 0.98f);
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
    addMod (d1, lfoPhase1, w1, s1, dep1);
    addMod (d2, lfoPhase2, w2, s2, dep2);
    addMod (d3, lfoPhase3, w3, s3, dep3);
    addMod (d4, lfoPhase4, w4, s4, dep4);

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

    setKnobLed (osc1PulseWidth, Parameters::paramOsc1PulseWidth, 0.5f);
    setKnobLed (osc1Octave,     Parameters::paramOsc1Octave, 0.0f);
    setKnobLed (osc1Tune,       Parameters::paramOsc1Tune, 0.0f);
    setKnobLed (osc2PulseWidth, Parameters::paramOsc2PulseWidth, 0.5f);
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
    setKnobLed (lfo1Shape,      Parameters::paramLFO1Shape, 0.5f);
    setKnobLed (lfo2Rate,       Parameters::paramLFO2Rate, 1.0f);
    setKnobLed (lfo2Depth,      Parameters::paramLFO2Depth, 0.5f);
    setKnobLed (lfo2Shape,      Parameters::paramLFO2Shape, 0.5f);
    setKnobLed (lfo3Rate,       Parameters::paramLFO3Rate, 1.0f);
    setKnobLed (lfo3Depth,      Parameters::paramLFO3Depth, 0.5f);
    setKnobLed (lfo3Shape,      Parameters::paramLFO3Shape, 0.5f);
    setKnobLed (lfo4Rate,       Parameters::paramLFO4Rate, 1.0f);
    setKnobLed (lfo4Depth,      Parameters::paramLFO4Depth, 0.5f);
    setKnobLed (lfo4Shape,      Parameters::paramLFO4Shape, 0.5f);
    setKnobLed (ampGain,        Parameters::paramAmpGain, 0.7f);
    setKnobLed (pan,            Parameters::paramPan, 0.5f);
    setKnobLed (glideTime,      Parameters::paramGlideTime, 0.0f);
    setKnobLed (tapeDelayMode,  Parameters::paramTapeDelayTimeMode, 0.0f);
    setKnobLed (tapeDelayTime,     Parameters::paramTapeDelayTime, 300.0f);
    setKnobLed (tapeDelayFeedback, Parameters::paramTapeDelayFeedback, 0.5f);
    setKnobLed (tapeDelayMix,      Parameters::paramTapeDelayMix, 0.5f);
    setKnobLed (tapeDelayAge,      Parameters::paramTapeDelayAge, 0.5f);
    setKnobLed (tapeDelaySat,      Parameters::paramTapeDelaySat, 0.3f);
    setKnobLed (tapeDelayWow,      Parameters::paramTapeDelayWow, 0.0f);
    setKnobLed (tapeDelayFlutter,  Parameters::paramTapeDelayFlutter, 0.0f);
}