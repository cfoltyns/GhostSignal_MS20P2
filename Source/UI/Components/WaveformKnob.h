/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Rotary waveform selector knob for VCO oscillators.
 *              Snap-to positions with waveform name drawn in the center.
 *              Can also draw waveform icons if desired.
 */

#pragma once

#include <JuceHeader.h>
#include <functional>
#include <utility>

class WaveformKnob : public juce::Component
{
public:
    WaveformKnob (const juce::String& labelText);
    ~WaveformKnob() override = default;

    juce::Slider& getSlider() { return slider; }
    void setLabel (const juce::String& text);
    void setSnapToValues (const float* values, int numValues);

    // Custom formatter for the center text (receives raw parameter value, returns display string)
    using CentreValueFormatter = std::function<juce::String (double)>;
    void setCentreValueFormatter (CentreValueFormatter fmt);

    // Set discrete text values for specific positions (e.g., waveform names at snap positions)
    void setTextValues (const juce::StringArray& texts, const float* values, int numValues);
    void clearTextValues();

    // Enable/disable automatic center text update based on current value
    void setAutoCenterText (bool autoUpdate) { autoCenterText = autoUpdate; updateCenterTextFromValue(); }

    // Manually set the center text (overrides auto)
    void setCenterText (const juce::String& text);
    void clearCenterText();

    // Show/hide the waveform icon in the center (when true, draws icon instead of text)
    void setShowWaveformIcon (bool show);

    // Callback when waveform value changes
    std::function<void()> onWaveformChanged;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    class WaveformSlider : public juce::Slider
    {
    public:
        WaveformSlider();
        ~WaveformSlider() override = default;

        void paint (juce::Graphics& g) override;
        void drawWaveformIcon (juce::Graphics& g, juce::Rectangle<float> area, int waveIndex) const;

        using CentreValueFormatter = std::function<juce::String (double)>;

        void setCentreValueFormatter (CentreValueFormatter fmt)
        {
            centreValueFormatter = std::move (fmt);
            repaint();
        }

        void setShowWaveformIcon (bool show) { showWaveformIcon = show; }

        juce::String getTextFromValue (double value) override
        {
            if (centreValueFormatter)
                return centreValueFormatter (value);

            return juce::Slider::getTextFromValue (value);
        }

    private:
        CentreValueFormatter centreValueFormatter;
        bool showWaveformIcon { false };
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformSlider)
    };

    WaveformSlider slider;
    juce::Label label;
    float labelHeightProportion { 0.22f };
    float snapValues[8];
    int numSnapValues { 0 };

    // Center text overlay (paints on top of the knob body)
    juce::String centerText;
    juce::Label  centerLabel;
    bool autoCenterText { false };

    // Text values for discrete positions
    juce::StringArray textValues;
    float textValuePositions[8];
    int numTextValues { 0 };
    bool showTextValues { false };

    void updateCenterTextFromValue();
    static int getWaveformIndexFromValue (double value);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformKnob)
};
