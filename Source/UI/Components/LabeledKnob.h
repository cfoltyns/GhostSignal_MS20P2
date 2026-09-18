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
#include <functional>
#include <utility>

class LabeledKnob : public juce::Component,
                     private juce::Slider::Listener,
                     private juce::Timer
{
public:
    LabeledKnob (const juce::String& labelText);
    ~LabeledKnob() override { stopTimer(); slider.removeListener (this); }

    juce::Slider& getSlider() { return slider; }
    void setLabel (const juce::String& text);
    void setLabelSize (float proportion) { labelHeightProportion = proportion; }

    // Custom formatter for the numeric value drawn in the knob centre.
    // Receives the raw parameter value and returns the display string
    // (e.g. "300ms", "50%", "-3", "-24"). When set it takes precedence
    // over the slider's default value text and keeps readouts consistent
    // (no long decimals or inconsistent widths).
    using CentreValueFormatter = std::function<juce::String (double)>;
    void setCentreValueFormatter (CentreValueFormatter fmt)
    {
        slider.setCentreValueFormatter (std::move (fmt));
    }

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

    // Show/hide the raw numeric value drawn in the knob centre by the
    // LookAndFeel (e.g. "0.50"). When hidden, only the label text and any
    // setCenterText() overlay are displayed.
    void setCenterValueVisible (bool visible);

    // Display the knob position (0..100) as the centre value instead of the
    // raw parameter value (e.g. filter cutoff in Hz). 0 = far left, 100 =
    // far right. Purely cosmetic — the parameter stays untouched, so the
    // audio path and saved states are unaffected. Must be called after the
    // slider attachment has been created.
    void setCenterValueAsKnobPercent();

    // LFO modulation indicator — a single tiny amber LED that travels smoothly
    // around the rim of the knob, driven by the real modulation signal.
    //   • travel arc   ∝ modulation depth (normValue already carries LFO × depth,
    //                    so shallow depth = a small arc, deep depth = a wide sweep)
    //   • travel speed ∝ LFO rate (the light follows the LFO in time)
    // There is no continuous ring and no trail: just one small, dim, soft point
    // of light. normValue: bipolar -1..1 (LFO output × modulation depth).
    void setModulationRing (float normValue, bool show,
                            juce::Colour color = juce::Colour (0xFFE8A020))
    {
        modRingColor = color;

        if (! show)
        {
            clearModulationRing();
            return;
        }

        ledAngleTarget = modLedAngleFor (normValue);

        if (! showModRing || ! isTimerRunning())
        {
            // First appearance: start the light already at its current
            // position so it never flies in from a stale location.
            ledAngleCurrent = ledAngleTarget;
            showModRing = true;
            modLedOverlay.repaint();
            startTimerHz (60);
            return;
        }

        showModRing = true;
        // The 60 Hz easing timer repaints the overlay itself; avoid queuing
        // an extra full-component repaint here at the editor's 30 Hz rate.
    }

    void clearModulationRing()
    {
        if (! showModRing)
            return;

        showModRing = false;
        stopTimer();
        modLedOverlay.repaint();
    }

    // Legacy dot indicator (kept for backward compatibility)
    void setModulationIndicator (float normValue, bool show) { setModulationRing(normValue, show); }
    void clearModulationIndicator() { clearModulationRing(); }

    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void sliderValueChanged (juce::Slider* slider) override;
    void updateCenterTextFromValue();

private:
    // Slider that can display the knob position (0..100) as its value text
    // instead of the raw parameter value. The LookAndFeel centre readout uses
    // getTextFromValue(), so overriding it changes what is shown in the knob.
    class PercentValueSlider : public juce::Slider
    {
    public:
        PercentValueSlider() = default;
        ~PercentValueSlider() override = default;

        using CentreValueFormatter = std::function<juce::String (double)>;

        void setCentreValueFormatter (CentreValueFormatter fmt)
        {
            centreValueFormatter = std::move (fmt);
            repaint();
        }

        juce::String getTextFromValue (double value) override
        {
            if (showAsKnobPercent)
            {
                const int percent = juce::roundToInt (valueToProportionOfLength (value) * 100.0);
                // Full rotation (100) is shown as "-" so the 3-digit value
                // doesn't get clipped in the knob centre cap.
                if (percent >= 100)
                    return "-";
                return juce::String (percent);
            }

            if (centreValueFormatter)
                return centreValueFormatter (value);

            return juce::Slider::getTextFromValue (value);
        }

        double getValueFromText (const juce::String& text) override
        {
            if (showAsKnobPercent)
            {
                if (text.trim() == "-")
                    return proportionOfLengthToValue (1.0);
                return proportionOfLengthToValue (
                    juce::jlimit (0.0, 1.0, text.getDoubleValue() / 100.0));
            }
            return juce::Slider::getValueFromText (text);
        }

        bool showAsKnobPercent { false };

    private:
        // Optional custom centre-readout formatter (raw parameter value →
        // display string). Falls back to juce::Slider::getTextFromValue when
        // unset.
        CentreValueFormatter centreValueFormatter;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PercentValueSlider)
    };

    PercentValueSlider slider;
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

    // Animated LFO modulation indicator state
    bool  showModRing { false };
    juce::Colour modRingColor { juce::Colour (0xFFE8A020) };

    // LED animation: 60 Hz eased travel between the modulation targets the
    // editor pushes in (currently 30 Hz) keeps the light organic and jitter-free.
    float ledAngleTarget  { -juce::MathConstants<float>::halfPi };
    float ledAngleCurrent { -juce::MathConstants<float>::halfPi };

    void timerCallback() override;

    // Map a bipolar modulation value (-1..1 = LFO output × depth) to an angle
    // around the knob. Zero sits at 12 o'clock; the sweep widens with depth.
    float modLedAngleFor (float normValue) const;

    // Draw the travelling LED. Called from the overlay so the light paints on
    // top of the knob flange instead of being hidden behind the slider body.
    void paintModLed (juce::Graphics& g);

    // Transparent layer sitting above the knob purely so the modulation LED can
    // be drawn over the flange. Never intercepts mouse clicks.
    class ModLedOverlay : public juce::Component
    {
    public:
        explicit ModLedOverlay (LabeledKnob& ownerToUse) : owner (ownerToUse)
        {
            setInterceptsMouseClicks (false, false);
        }

        void paint (juce::Graphics& g) override { owner.paintModLed (g); }

    private:
        LabeledKnob& owner;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModLedOverlay)
    };

    ModLedOverlay modLedOverlay { *this };


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LabeledKnob)
};
