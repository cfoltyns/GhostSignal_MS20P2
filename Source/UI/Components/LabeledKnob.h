/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Sleek minimalist rotary knob with label below.
 *              Supports optional text value labels at specific positions.
 */

#pragma once

#include <JuceHeader.h>

class LabeledKnob : public juce::Component, private juce::Slider::Listener
{
public:
    LabeledKnob (const juce::String& labelText);
    ~LabeledKnob() override { slider.removeListener (this); }

    juce::Slider& getSlider() { return slider; }
    void setLabel (const juce::String& text);
    void setLabelSize (float proportion) { labelHeightProportion = proportion; }

    void setTextValues (const juce::StringArray& texts, const float* values, int numValues);
    void clearTextValues();

    void setSnapToValues (const float* values, int numValues);
    // Fill the snap list without touching the slider range/interval.
    // Used for the LFO sync toggle so snapping works across the full
    // 0..1 param range while only the sync half is quantised.
    void setSnapValuesOnly (const float* values, int numValues);
    void clearSnapValues();

    void setLedActive (bool active) { ledActive = active; repaint(); }
    bool isLedActive() const { return ledActive; }

    // Override the minimum slider (knob) diameter. Default is 48px.
    // Useful for cramped spaces where a smaller knob is acceptable.
    void setMinKnobSize (int minSize) { minKnobSize = minSize; }
    int getMinKnobSize() const { return minKnobSize; }

    void setCenterText (const juce::String& text);
    void clearCenterText();
    void setAutoCenterText (bool autoUpdate) { autoCenterText = autoUpdate; updateCenterTextFromValue(); }

    // Modulation indicator — draws a small dot on the knob perimeter showing
    // where an external modulation source (e.g. LFO) is currently pushing the value.
    // normValue: 0..1 mapped onto the knob's rotary arc.
    void setModulationIndicator (float normValue, bool show) { modulationValue = normValue; showModIndicator = show; repaint(); }
    void clearModulationIndicator() { showModIndicator = false; repaint(); }

    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void sliderValueChanged (juce::Slider* slider) override;
    void updateCenterTextFromValue();

private:
    juce::Slider slider;
    juce::Label label;
    float labelHeightProportion { 0.22f };
    int minKnobSize { 48 };

    juce::StringArray textValues;
    float textValuePositions[24];   // supports up to 24 steps (e.g. LFO tempo divisions)
    int numTextValues { 0 };
    bool showTextValues { false };

    float snapValues[24];           // supports up to 24 discrete knob positions
    int numSnapValues { 0 };

    bool ledActive { false };
    juce::String centerText;
    juce::Label  centerLabel;   // overlay — paints ABOVE the slider so the
                                // knob body can't cover the centre text
    bool autoCenterText { false };

    // Modulation indicator state
    float modulationValue { 0.0f };
    bool  showModIndicator { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LabeledKnob)
};
