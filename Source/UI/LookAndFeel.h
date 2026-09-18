/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Industrial LookAndFeel — knobs finished in the same dark,
 *              grainy surface as the section panels, with a very soft top
 *              highlight, a light contact shadow, and dark-on-dark shading.
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

    // ─── Knob rendering (shared) ────────────────────────────────────────────────
    // Knob surface: contact shadow → grainy rim disc → grainy face with a very
    // soft top highlight. Uses the exact same grain as the section panels.
    // Shared by the rotary LookAndFeel and the WaveformKnob so every knob in
    // the plugin has the same appearance.
    static void drawIndustrialKnobBody (juce::Graphics& g,
                                        juce::Point<float> centre,
                                        float radius,
                                        bool enabled,
                                        bool hovered = false,
                                        bool dragging = false);

    // ─── Knob rendering (shared) ────────────────────────────────────────────────
    // Small center detail for the knob; the value text or waveform icon is
    // drawn on top. Same grain as the knob face, one shade darker.
    static void drawIndustrialKnobCap (juce::Graphics& g,
                                       juce::Point<float> centre,
                                       float capRadius,
                                       bool enabled);

    // Evenly spaced radial tick marks around the outside of a knob's mounting
    // flange, spanning the same sweep as the value arc. The ring scales with the
    // knob, so small knobs get the same marks as large ones, and it sits outside
    // the flange rim so no mark can touch the value arc, position pointer or
    // centre readout. Shared by the rotary renderer and the waveform slider.
    //
    // innerScale/outerScale are the tick band as a fraction of `radius`, letting
    // a caller whose value arc sits further out (the waveform knobs) push the
    // ring toward the rim so the marks still clear the arc.
    static void drawIndustrialKnobTicks (juce::Graphics& g,
                                         juce::Point<float> centre,
                                         float radius,
                                         int numTicks,
                                         float startAngle,
                                         float endAngle,
                                         float innerScale = 0.925f,
                                         float outerScale = 0.995f);

    // ─── Typography helpers ────────────────────────────────────────────────────
    static float getSectionTitleFontSize (int panelHeight);
    static float getParamLabelFontSize (int widgetHeight);
    static float getValueFontSize (int widgetHeight);
    static float getKnobFontSize (int knobDiameter);
    static juce::Font getSectionTitleFont (int panelHeight);
    static juce::Font getParamLabelFont (int widgetHeight);
    static juce::Font getValueFont (int widgetHeight);
        static juce::Font getKnobLabelFont (int knobDiameter);

    // ─── Monospace overrides (preset / modal UI) ────────────────────────────────
    /** Returns a monospace font whose size is clamped to the 9–11 px band so a
        single, readable face is used across the preset browser, save/rename dialog
        and slider popups. Preference order: JetBrains Mono → Cascadia Mono →
        Cascadia Code → Consolas → DejaVu Sans Mono → Liberation Mono → Menlo →
        Monaco → Lucida Console → Courier New. */
    static juce::Font getMonospaceFont (float size, bool bold = false);

    /** Convenience that clamps to the 9–11 px band, then multiplies. */
    static juce::Font getMonoLabelFont (float scale = 1.0f, bool bold = false);

    // ─── Texture helpers ───────────────────────────────────────────────────────
    // Cached, deterministic grain tile used to give panel bodies a faint
    // analogue texture. Built once (thread-safe function-local static) and
    // shared by every panel, so painting stays cheap and the grain never
    // shimmers between repaints.
    static const juce::Image& getPanelGrainTile();

private:
    juce::Font knobFont { juce::Font (juce::FontOptions (12.0f, juce::Font::bold)) };
};
