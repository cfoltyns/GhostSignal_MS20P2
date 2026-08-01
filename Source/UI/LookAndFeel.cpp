/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Industrial hardware-inspired LookAndFeel.
 *              Knobs use a proper addCentredArc white-ring renderer
 *              with a grey track arc showing full travel range.
 *              No white dot indicator — the arc IS the indicator.
 *
 * Color design system:
 *   Background: #1E1E1E
 *   Panel:      #252525
 *   Borders:    #383838
 *   Primary:    #D8D8D8
 *   Secondary:  #888888
 *   Accent:     #DB4437
 *   Disabled:   #555555
 */

#include "LookAndFeel.h"

using namespace juce;

// ─── Colour palette ──────────────────────────────────────────────────────────

const Colour GhostSignalLookAndFeel::bg           { Colour (0xFF1E1E1E) };
const Colour GhostSignalLookAndFeel::panel        { Colour (0xFF252525) };
const Colour GhostSignalLookAndFeel::border       { Colour (0xFF383838) };
const Colour GhostSignalLookAndFeel::textPrimary  { Colour (0xFFD8D8D8) };
const Colour GhostSignalLookAndFeel::textSecondary{ Colour (0xFF888888) };
const Colour GhostSignalLookAndFeel::accent       { Colour (0xFFDB4437) };
const Colour GhostSignalLookAndFeel::accentDark   { Colour (0xFFB03A2E) };
const Colour GhostSignalLookAndFeel::disabled     { Colour (0xFF555555) };

const Colour GhostSignalLookAndFeel::knobBody   { Colour (0xFF2A2A2A) };
const Colour GhostSignalLookAndFeel::knobBorder { Colour (0xFF3C3C3C) };

// ─── Constructor ──────────────────────────────────────────────────────────────

GhostSignalLookAndFeel::GhostSignalLookAndFeel()
{
    // Sliders
    setColour (Slider::thumbColourId,               textPrimary);
    setColour (Slider::rotarySliderFillColourId,    textPrimary);
    setColour (Slider::rotarySliderOutlineColourId, border);
    setColour (Slider::trackColourId,               Colour (0xFF1E1E1E));
    setColour (Slider::backgroundColourId,          panel);

    // Labels
    setColour (Label::textColourId,                 textPrimary);
    setColour (Label::backgroundColourId,           Colours::transparentBlack);

    // Window
    setColour (ResizableWindow::backgroundColourId, bg);

    // ComboBox — industrial dark style
    setColour (ComboBox::backgroundColourId,        Colour (0xFF1E1E1E));
    setColour (ComboBox::outlineColourId,           border);
    setColour (ComboBox::textColourId,              textPrimary);
    setColour (ComboBox::arrowColourId,             textSecondary);
    setColour (ComboBox::buttonColourId,            Colour (0xFF2A2A2A));

    // PopupMenu
    setColour (PopupMenu::backgroundColourId,       panel);
    setColour (PopupMenu::textColourId,             textPrimary);
    setColour (PopupMenu::highlightedBackgroundColourId, accent);
    setColour (PopupMenu::highlightedTextColourId,  Colours::white);
}

// ─── Typography helpers ──────────────────────────────────────────────────────

float GhostSignalLookAndFeel::getSectionTitleFontSize (int panelHeight)
{
    // Uppercase bold section titles
    return jlimit (10.0f, 14.0f, panelHeight * 0.055f);
}

float GhostSignalLookAndFeel::getParamLabelFontSize (int widgetHeight)
{
    // Medium-weight parameter labels
    return jlimit (9.0f, 12.0f, widgetHeight * 0.11f);
}

float GhostSignalLookAndFeel::getValueFontSize (int widgetHeight)
{
    // Small high-contrast value readouts
    return jlimit (8.0f, 11.0f, widgetHeight * 0.09f);
}

float GhostSignalLookAndFeel::getKnobFontSize (int knobDiameter)
{
    // Proportional: ~14% of diameter, clamped 9–16px
    return jlimit (9.0f, 16.0f, knobDiameter * 0.14f);
}

// ─── Rotary slider rendering ──────────────────────────────────────────────────

void GhostSignalLookAndFeel::drawRotarySlider (Graphics& g,
                                               int x, int y, int width, int height,
                                               float sliderPos,
                                               float rotaryStartAngle,
                                               float rotaryEndAngle,
                                               Slider& slider)
{
    // Work in a square, centered within the allocated bounds
    const int diameter = jmin (width, height);
    const float cx = x + width  * 0.5f;
    const float cy = y + height * 0.5f;
    const float r  = diameter * 0.5f;

    // ── Knob body ────────────────────────────────────────────────────────────
    // Outer shadow ring
    {
        const float shadowR = r * 0.94f;
        g.setColour (Colour (0xFF050505));
        g.fillEllipse (cx - shadowR, cy - shadowR, shadowR * 2.0f, shadowR * 2.0f);
    }

    // Body gradient — subtle top-left highlight simulating a slightly convex surface
    const float bodyR = r * 0.86f;
    {
        ColourGradient grad (Colour (0xFF3E3E3E), cx - bodyR * 0.5f, cy - bodyR * 0.6f,
                             Colour (0xFF1C1C1C), cx + bodyR * 0.4f, cy + bodyR * 0.6f,
                             false);
        grad.addColour (0.4f, Colour (0xFF303030));
        g.setGradientFill (grad);
        g.fillEllipse (cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);
    }

    // Hairline border — aged steel rim
    g.setColour (border);
    g.drawEllipse (cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f, 1.0f);

    // ── Arc ring geometry ─────────────────────────────────────────────────────
    // The arc lives in the gap between the knob body and the outer edge of the component.
    // arcRadius is the centre-line of the stroke.
    const float arcRadius    = r * 0.92f;
    const float arcThickness = jlimit (2.5f, 7.0f, r * 0.14f);

    const float toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Track arc — full travel range in dark grey
    {
        Path track;
        track.addCentredArc (cx, cy, arcRadius, arcRadius,
                             0.0f,
                             rotaryStartAngle, rotaryEndAngle,
                             true);

        PathStrokeType stroke (arcThickness, PathStrokeType::curved, PathStrokeType::rounded);
        g.setColour (Colour (0xFF282828));
        g.strokePath (track, stroke);
    }

    // Value arc — white, from start to current position
    if (sliderPos > 0.001f)
    {
        // Subtle glow: a slightly wider, very transparent arc behind the bright one
        {
            Path glow;
            glow.addCentredArc (cx, cy, arcRadius, arcRadius,
                                0.0f,
                                rotaryStartAngle, toAngle,
                                true);
            PathStrokeType glowStroke (arcThickness * 2.2f, PathStrokeType::curved, PathStrokeType::rounded);
            g.setColour (textPrimary.withAlpha (0.08f));
            g.strokePath (glow, glowStroke);
        }

        // Main value arc — crisp white
        {
            Path valueArc;
            valueArc.addCentredArc (cx, cy, arcRadius, arcRadius,
                                    0.0f,
                                    rotaryStartAngle, toAngle,
                                    true);
            PathStrokeType stroke (arcThickness, PathStrokeType::curved, PathStrokeType::rounded);
            g.setColour (textPrimary);
            g.strokePath (valueArc, stroke);
        }
    }

    // Indicate active/selected knob with accent-coloured arc segment
    // (e.g. stepped selectors like waveform/tape mode)
    if (slider.isEnabled())
    {
        // a subtle accent tick at current position for discrete selectors
    }

    // ── Centre face detail ────────────────────────────────────────────────────
    // Tiny engraved-looking circle at the very centre — no pointer line, no dot
    const float centreR = r * 0.12f;
    g.setColour (Colour (0xFF141414));
    g.fillEllipse (cx - centreR, cy - centreR, centreR * 2.0f, centreR * 2.0f);
    g.setColour (border);
    g.drawEllipse (cx - centreR, cy - centreR, centreR * 2.0f, centreR * 2.0f, 0.8f);
}

// ─── Label rendering ──────────────────────────────────────────────────────────

void GhostSignalLookAndFeel::drawLabel (Graphics& g, Label& l)
{
    g.setColour (l.findColour (Label::textColourId));
    g.setFont   (l.getFont());
    g.drawFittedText (l.getText(),
                      l.getLocalBounds(),
                      Justification::centred,
                      1,
                      0.0f);
}

// ─── ComboBox rendering ───────────────────────────────────────────────────────

void GhostSignalLookAndFeel::drawComboBox (Graphics& g, int width, int height,
                                           bool /*isButtonDown*/,
                                           int buttonX, int buttonY,
                                           int buttonW, int buttonH,
                                           ComboBox& box)
{
    const auto bounds = Rectangle<int> (0, 0, width, height).toFloat();
    const float cornerSize = 3.0f;

    // Body
    g.setColour (box.findColour (ComboBox::backgroundColourId));
    g.fillRoundedRectangle (bounds, cornerSize);

    // Border
    g.setColour (box.findColour (ComboBox::outlineColourId));
    g.drawRoundedRectangle (bounds.reduced (0.5f), cornerSize, 1.0f);

    // Dropdown arrow area — subtle separator
    g.setColour (border);
    g.drawVerticalLine (buttonX, 3.0f, static_cast<float> (height - 3));

    // Arrow chevron
    const float arrowCx = buttonX + buttonW * 0.5f;
    const float arrowCy = buttonY + buttonH * 0.5f;
    const float arrowSize = jmin (buttonW, buttonH) * 0.28f;

    Path arrow;
    arrow.startNewSubPath (arrowCx - arrowSize, arrowCy - arrowSize * 0.4f);
    arrow.lineTo          (arrowCx,             arrowCy + arrowSize * 0.6f);
    arrow.lineTo          (arrowCx + arrowSize, arrowCy - arrowSize * 0.4f);

    g.setColour (box.findColour (ComboBox::arrowColourId));
    g.strokePath (arrow, PathStrokeType (1.2f, PathStrokeType::mitered, PathStrokeType::square));
}
