/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Premium industrial LookAndFeel — brushed-metal knobs with
 *              chrome rims, recessed panels with inner shadows, and a
 *              restrained dark color palette with a single warm accent.
 *
 * Design system:
 *   Background:  #0A0A0C  (deep charcoal)
 *   Panel:       #141418  (dark slate)
 *   Panel border: #2A2A3A (subtle)
 *   Text primary: #E0E0E8
 *   Text secondary: #8A8A9A
 *   Accent:       #FF6B35 (warm orange-red)
 *   Accent dark:  #CC552B
 *   Disabled:     #4A4A5A
 *   Knob body:    #2A2A3A (brushed metal)
 *   Knob rim:     #5A5A6A (chrome)
 *   Knob center:  #1A1A2A (machined aluminum)
 */

#pragma once

#include <JuceHeader.h>

class GhostSignalLookAndFeel : public juce::LookAndFeel_V4
{
public:
    GhostSignalLookAndFeel();
    ~GhostSignalLookAndFeel() override = default;

    void drawRotarySlider (juce::Graphics&,
                           int x, int y, int width, int height,
                           float sliderPos,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider&) override;

    void drawLabel (juce::Graphics&, juce::Label&) override;

    void drawComboBox (juce::Graphics&, int width, int height,
                       bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox&) override;

    void drawLinearSlider (juce::Graphics&,
                           int x, int y, int width, int height,
                           float sliderPos,
                           float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle,
                           juce::Slider&) override;

    void drawButtonBackground (juce::Graphics&,
                               juce::Button&,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

    // ─── Colour palette ────────────────────────────────────────────────────────
    static const juce::Colour bg;
    static const juce::Colour panel;
    static const juce::Colour panelBorder;
    static const juce::Colour textPrimary;
    static const juce::Colour textSecondary;
    static const juce::Colour accent;
    static const juce::Colour accentDark;
    static const juce::Colour disabled;
    static const juce::Colour knobBody;
    static const juce::Colour knobRim;
    static const juce::Colour knobCenter;
    static const juce::Colour panelShadow;

    // ─── Typography helpers ────────────────────────────────────────────────────
    static float getSectionTitleFontSize (int panelHeight);
    static float getParamLabelFontSize (int widgetHeight);
    static float getValueFontSize (int widgetHeight);
    static float getKnobFontSize (int knobDiameter);
    static juce::Font getSectionTitleFont (int panelHeight);
    static juce::Font getParamLabelFont (int widgetHeight);
    static juce::Font getValueFont (int widgetHeight);
    static juce::Font getKnobLabelFont (int knobDiameter);

private:
    juce::Font knobFont { juce::Font (juce::FontOptions (12.0f, juce::Font::bold)) };
};
