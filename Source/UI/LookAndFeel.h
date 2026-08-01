/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Industrial hardware LookAndFeel - white arc knobs,
 *              matte panels, dark ComboBoxes.
 *
 * Design system:
 *   Background: #1E1E1E
 *   Panel:      #252525
 *   Borders:    #383838
 *   Text:       #D8D8D8 / #888888
 *   Accent:     #DB4437
 *   Disabled:   #555555
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

    static const juce::Colour bg;
    static const juce::Colour panel;
    static const juce::Colour border;
    static const juce::Colour textPrimary;
    static const juce::Colour textSecondary;
    static const juce::Colour accent;
    static const juce::Colour accentDark;
    static const juce::Colour disabled;
    static const juce::Colour knobBody;
    static const juce::Colour knobBorder;

    static float getSectionTitleFontSize (int panelHeight);
    static float getParamLabelFontSize (int widgetHeight);
    static float getValueFontSize (int widgetHeight);
    static float getKnobFontSize (int knobDiameter);

private:
    juce::Font knobFont { juce::Font (12.0f, juce::Font::bold) };
};