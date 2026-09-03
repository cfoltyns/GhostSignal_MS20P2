/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Industrial LookAndFeel — black molded plastic potentiometer
 *              knobs with wide mounting flanges, cylindrical side walls,
 *              grip grooves, and dark-on-dark 3D shading.
 *
 * Design system:
 *   Background:  #0A0A0C  (deep charcoal)
 *   Panel:       #141418  (dark slate)
 *   Panel border: #2A2A3A (subtle)
 *   Text primary: #E0E0E8
 *   Text secondary: #8A8A9A
 *   Accent:       #5C6B5E (muted sage green)
 *   Accent dark:  #47524A
 *   Disabled:     #4A4A5A
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
    static const juce::Colour panelShadow;

    // ─── Industrial molded knob rendering (shared) ─────────────────────────────
    // Physical knob body: drop shadow → mounting flange → cylindrical side wall →
    // grip grooves → top surface → bevels. Optional hover/drag feedback.
    // Shared by the rotary LookAndFeel and the custom WaveformSlider so every
    // knob in the plugin has the same hardware appearance.
    static void drawIndustrialKnobBody (juce::Graphics& g,
                                        juce::Point<float> centre,
                                        float radius,
                                        bool enabled,
                                        bool hovered = false,
                                        bool dragging = false);

    // Small center detail for the industrial knob; the value text or waveform
    // icon is drawn on top.
    static void drawIndustrialKnobCap (juce::Graphics& g,
                                       juce::Point<float> centre,
                                       float capRadius,
                                       bool enabled);

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
