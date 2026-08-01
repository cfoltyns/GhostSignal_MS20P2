/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Premium industrial LookAndFeel implementation.
 *              Knobs feature brushed-metal texture, chrome rim, machined
 *              aluminum center cap, and a dual-arc value indicator.
 *              Panels have recessed bodies with inner shadows and subtle
 *              beveled edges.
 */

#include "LookAndFeel.h"

using namespace juce;

// ─── Colour palette ──────────────────────────────────────────────────────────

const Colour GhostSignalLookAndFeel::bg           { Colour (0xFF0A0A0C) };
const Colour GhostSignalLookAndFeel::panel        { Colour (0xFF141418) };
const Colour GhostSignalLookAndFeel::panelBorder  { Colour (0xFF2A2A3A) };
const Colour GhostSignalLookAndFeel::textPrimary  { Colour (0xFFE0E0E8) };
const Colour GhostSignalLookAndFeel::textSecondary{ Colour (0xFF8A8A9A) };
const Colour GhostSignalLookAndFeel::accent       { Colour (0xFFFF6B35) };
const Colour GhostSignalLookAndFeel::accentDark   { Colour (0xFFCC552B) };
const Colour GhostSignalLookAndFeel::disabled     { Colour (0xFF4A4A5A) };

const Colour GhostSignalLookAndFeel::knobBody   { Colour (0xFF2A2A3A) };
const Colour GhostSignalLookAndFeel::knobRim    { Colour (0xFF5A5A6A) };
const Colour GhostSignalLookAndFeel::knobCenter { Colour (0xFF1A1A2A) };
const Colour GhostSignalLookAndFeel::panelShadow { Colour (0x30000000) };

// ─── Constructor ──────────────────────────────────────────────────────────────

GhostSignalLookAndFeel::GhostSignalLookAndFeel()
{
    // Sliders
    setColour (Slider::thumbColourId,               textPrimary);
    setColour (Slider::rotarySliderFillColourId,    accent);
    setColour (Slider::rotarySliderOutlineColourId, Colour (0xFF3A3A4A));
    setColour (Slider::trackColourId,               Colour (0xFF0A0A0C));
    setColour (Slider::backgroundColourId,          panel);

    // Labels
    setColour (Label::textColourId,                 textPrimary);
    setColour (Label::backgroundColourId,           Colours::transparentBlack);

    // Window
    setColour (ResizableWindow::backgroundColourId, bg);

    // ComboBox — premium dark style
    setColour (ComboBox::backgroundColourId,        Colour (0xFF0A0A0C));
    setColour (ComboBox::outlineColourId,           panelBorder);
    setColour (ComboBox::textColourId,              textPrimary);
    setColour (ComboBox::arrowColourId,             textSecondary);
    setColour (ComboBox::buttonColourId,            Colour (0xFF2A2A3A));

    // PopupMenu
    setColour (PopupMenu::backgroundColourId,       panel);
    setColour (PopupMenu::textColourId,             textPrimary);
    setColour (PopupMenu::highlightedBackgroundColourId, accent);
    setColour (PopupMenu::highlightedTextColourId,  Colours::white);

    // Buttons
    setColour (TextButton::buttonColourId,          Colour (0xFF2A2A3A));
    setColour (TextButton::buttonOnColourId,        accent);
    setColour (TextButton::textColourOffId,         textPrimary);
    setColour (TextButton::textColourOnId,          Colours::white);
}

// ─── Typography helpers ──────────────────────────────────────────────────────

float GhostSignalLookAndFeel::getSectionTitleFontSize (int panelHeight)
{
    return jlimit (11.0f, 15.0f, panelHeight * 0.06f);
}

float GhostSignalLookAndFeel::getParamLabelFontSize (int widgetHeight)
{
    return jlimit (9.0f, 12.0f, widgetHeight * 0.11f);
}

float GhostSignalLookAndFeel::getValueFontSize (int widgetHeight)
{
    return jlimit (8.0f, 11.0f, widgetHeight * 0.09f);
}

float GhostSignalLookAndFeel::getKnobFontSize (int knobDiameter)
{
    return jlimit (9.0f, 16.0f, knobDiameter * 0.14f);
}

juce::Font GhostSignalLookAndFeel::getSectionTitleFont (int panelHeight)
{
    return Font (getSectionTitleFontSize (panelHeight), Font::bold);
}

juce::Font GhostSignalLookAndFeel::getParamLabelFont (int widgetHeight)
{
    return Font (getParamLabelFontSize (widgetHeight), Font::bold);
}

juce::Font GhostSignalLookAndFeel::getValueFont (int widgetHeight)
{
    return Font (getValueFontSize (widgetHeight), Font::plain);
}

juce::Font GhostSignalLookAndFeel::getKnobLabelFont (int knobDiameter)
{
    return Font (getKnobFontSize (knobDiameter), Font::bold);
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

    const bool enabled = slider.isEnabled();

    // ── Outer shadow (subtle drop shadow for depth) ────────────────────────────
    {
        const float shadowR = r * 0.96f;
        g.setColour (panelShadow);
        g.fillEllipse (cx - shadowR, cy - shadowR + 1.0f, shadowR * 2.0f, shadowR * 2.0f);
    }

    // ── Knob body — brushed metal texture ──────────────────────────────────────
    const float bodyR = r * 0.88f;
    {
        // Base body colour
        g.setColour (enabled ? knobBody : disabled);
        g.fillEllipse (cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);

        // Brushed metal texture — subtle vertical lines
        g.saveState();
        g.reduceClipRegion (Rectangle<float> (cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f).toNearestInt());
        g.setColour (Colour (0x08FFFFFF));
        for (float lineX = cx - bodyR; lineX < cx + bodyR; lineX += 2.0f)
            g.drawVerticalLine (static_cast<int> (lineX), cy - bodyR, cy + bodyR);
        g.restoreState();
    }

    // ── Chrome rim ─────────────────────────────────────────────────────────────
    g.setColour (enabled ? knobRim : disabled.darker (0.3f));
    g.drawEllipse (cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f, 1.5f);

    // ── Inner shadow for bevel effect ───────────────────────────────────────────
    {
        const float innerR = bodyR - 2.0f;
        g.setColour (Colour (0x18000000));
        g.drawEllipse (cx - innerR, cy - innerR, innerR * 2.0f, innerR * 2.0f, 1.0f);
    }

    // ── Arc ring geometry ───────────────────────────────────────────────────────
    const float arcRadius    = r * 0.90f;
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
        g.setColour (Colour (0xFF3A3A4A));
        g.strokePath (track, stroke);
    }

    // Value arc — accent colour, from start to current position
    if (sliderPos > 0.001f)
    {
        // Subtle glow behind the value arc
        {
            Path glow;
            glow.addCentredArc (cx, cy, arcRadius, arcRadius,
                                0.0f,
                                rotaryStartAngle, toAngle,
                                true);
            PathStrokeType glowStroke (arcThickness * 2.2f, PathStrokeType::curved, PathStrokeType::rounded);
            g.setColour (accent.withAlpha (0.12f));
            g.strokePath (glow, glowStroke);
        }

        // Main value arc
        {
            Path valueArc;
            valueArc.addCentredArc (cx, cy, arcRadius, arcRadius,
                                    0.0f,
                                    rotaryStartAngle, toAngle,
                                    true);
            PathStrokeType stroke (arcThickness, PathStrokeType::curved, PathStrokeType::rounded);
            g.setColour (enabled ? accent : disabled);
            g.strokePath (valueArc, stroke);
        }
    }

    // ── Machined aluminum center cap ────────────────────────────────────────────
    const float centerR = r * 0.14f;
    {
        // Center cap gradient
        ColourGradient capGrad (Colour (0xFF2A2A3A), cx - centerR * 0.5f, cy - centerR * 0.6f,
                                Colour (0xFF12121A), cx + centerR * 0.4f, cy + centerR * 0.6f,
                                false);
        capGrad.addColour (0.5f, knobCenter);
        g.setGradientFill (capGrad);
        g.fillEllipse (cx - centerR, cy - centerR, centerR * 2.0f, centerR * 2.0f);

        // Center cap rim
        g.setColour (knobRim.withAlpha (0.6f));
        g.drawEllipse (cx - centerR, cy - centerR, centerR * 2.0f, centerR * 2.0f, 0.8f);
    }

    // ── Value text in center ────────────────────────────────────────────────────
    if (enabled)
    {
        const float fontSize = jlimit (7.0f, 11.0f, centerR * 1.4f);
        g.setColour (textPrimary);
        g.setFont (Font (fontSize, Font::bold));
        g.drawText (slider.getTextFromValue (slider.getValue()),
                    Rectangle<float> (cx - centerR, cy - centerR, centerR * 2.0f, centerR * 2.0f),
                    Justification::centred, false);
    }
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
    const float cornerSize = 4.0f;

    // Body with subtle gradient
    ColourGradient bodyGrad (Colour (0xFF1A1A22), 0.0f, 0.0f,
                             Colour (0xFF141418), 0.0f, (float) height,
                             false);
    g.setGradientFill (bodyGrad);
    g.fillRoundedRectangle (bounds, cornerSize);

    // Border
    g.setColour (box.findColour (ComboBox::outlineColourId));
    g.drawRoundedRectangle (bounds.reduced (0.5f), cornerSize, 1.0f);

    // Dropdown arrow area — subtle separator
    g.setColour (panelBorder);
    g.drawVerticalLine (buttonX, 4.0f, static_cast<float> (height - 4));

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

// ─── Linear slider rendering ──────────────────────────────────────────────────

void GhostSignalLookAndFeel::drawLinearSlider (Graphics& g,
                                               int x, int y, int width, int height,
                                               float sliderPos,
                                               float minSliderPos, float maxSliderPos,
                                               Slider::SliderStyle style,
                                               Slider& slider)
{
    if (style == Slider::LinearBar || style == Slider::LinearBarVertical)
    {
        // Fallback to default for bar styles
        LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos,
                                          minSliderPos, maxSliderPos, style, slider);
        return;
    }

    const auto bounds = Rectangle<int> (x, y, width, height).toFloat();
    const float cornerSize = 3.0f;

    // Track background
    g.setColour (Colour (0xFF2A2A3A));
    g.fillRoundedRectangle (bounds, cornerSize);

    // Track fill
    const float fillHeight = jmax (2.0f, height * sliderPos);
    const auto fillBounds = Rectangle<float> (x, y + height - fillHeight, width, fillHeight);
    g.setColour (slider.findColour (Slider::rotarySliderFillColourId));
    g.fillRoundedRectangle (fillBounds, cornerSize);

    // Track border
    g.setColour (panelBorder);
    g.drawRoundedRectangle (bounds, cornerSize, 1.0f);

    // Thumb
    const float thumbSize = jmin (width, height) * 0.6f;
    const float thumbY = y + height - fillHeight;
    g.setColour (textPrimary);
    g.fillEllipse (x + width * 0.5f - thumbSize * 0.5f, thumbY - thumbSize * 0.5f,
                   thumbSize, thumbSize);
}

// ─── Button background rendering ──────────────────────────────────────────────

void GhostSignalLookAndFeel::drawButtonBackground (Graphics& g,
                                                   Button& button,
                                                   const Colour& backgroundColour,
                                                   bool shouldDrawButtonAsHighlighted,
                                                   bool shouldDrawButtonAsDown)
{
    const auto bounds = button.getLocalBounds().toFloat();
    const float cornerSize = 4.0f;

    // Button body
    Colour buttonColour = backgroundColour;
    if (shouldDrawButtonAsDown)
        buttonColour = buttonColour.darker (0.15f);
    if (shouldDrawButtonAsHighlighted)
        buttonColour = buttonColour.brighter (0.1f);

    // Gradient for depth
    ColourGradient grad (buttonColour.brighter (0.05f), 0.0f, 0.0f,
                         buttonColour.darker (0.05f), 0.0f, bounds.getHeight(),
                         false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (bounds, cornerSize);

    // Border
    g.setColour (panelBorder);
    g.drawRoundedRectangle (bounds.reduced (0.5f), cornerSize, 1.0f);

    // Inner shadow for inset effect
    g.setColour (panelShadow);
    g.drawRoundedRectangle (bounds.reduced (1.0f), cornerSize - 0.5f, 0.5f);
}
