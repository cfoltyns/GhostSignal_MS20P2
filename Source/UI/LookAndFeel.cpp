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
const Colour GhostSignalLookAndFeel::accent       { Colour (0xFF5C6B5E) };
const Colour GhostSignalLookAndFeel::accentDark   { Colour (0xFF47524A) };
const Colour GhostSignalLookAndFeel::disabled     { Colour (0xFF4A4A5A) };

const Colour GhostSignalLookAndFeel::knobBody   { Colour (0xFF2A2A3A) };
const Colour GhostSignalLookAndFeel::knobRim    { Colour (0xFF5A5A6A) };
const Colour GhostSignalLookAndFeel::knobCenter { Colour (0xFF1A1A2A) };
const Colour GhostSignalLookAndFeel::panelShadow { Colour (0x30000000) };

// ─── Premium knob geometry (shared) ──────────────────────────────────────────
// Proportions relative to the knob's outer radius. Shared by the LookAndFeel
// rotary renderer and the WaveformSlider so every knob in the plugin has
// identical physical proportions.
namespace
{
    constexpr float knobBodyScale = 0.84f;   // metal face radius
    constexpr float knobCapScale  = 0.20f;   // centre cap radius
}

// ─── Premium knob body (shared) ───────────────────────────────────────────────
// Physical layers, outside-in: drop shadow → beveled rim → radial-gradient
// metal face → brushed texture → edge bevels. `hovered`/`dragging` add a
// gentle contrast lift (visual feedback only — no behaviour change).

void GhostSignalLookAndFeel::drawPremiumKnobBody (Graphics& g,
                                                  Point<float> centre,
                                                  float radius,
                                                  bool enabled,
                                                  bool hovered,
                                                  bool dragging)
{
    const float cx = centre.x;
    const float cy = centre.y;
    const float r  = radius;

    // ── Drop shadow — multi-layer soft shadow offset downward for 3D depth ──
    //    Three layers create a realistic falloff: a tight dark core, a mid
    //    shadow, and a soft ambient occlusion bleed into the panel.
    const float  shadowScales[4]  = { 0.96f, 0.985f, 1.005f, 1.03f };
    const uint32 shadowAlphas[4]  = { 0x55000000u, 0x38000000u, 0x22000000u, 0x12000000u };
    const float  shadowOffsets[4] = { 0.5f, 1.5f, 3.0f, 5.0f };
    for (int i = 0; i < 4; ++i)
    {
        const float sr = r * shadowScales[i];
        const float dy = shadowOffsets[i];
        g.setColour (Colour (shadowAlphas[i]));
        g.fillEllipse (cx - sr, cy - sr + dy, sr * 2.0f, sr * 2.0f);
    }

    // ── Outer glow/highlight — subtle top-left light reflection for 3D pop ──
    {
        Path glow;
        glow.addCentredArc (cx, cy, r * 1.02f, r * 1.02f, 0.0f,
                           0.70f * MathConstants<float>::pi,
                           1.70f * MathConstants<float>::pi, true);
        g.setColour (Colour (0x20FFFFFF));
        g.strokePath (glow, PathStrokeType (3.0f, PathStrokeType::curved, PathStrokeType::rounded));
    }

    // ── Beveled outer rim — dark machined ring around the metal face ──
    const float rimR = r * 0.96f;
    g.setColour (Colour (0xFF26262F));
    g.fillEllipse (cx - rimR, cy - rimR, rimR * 2.0f, rimR * 2.0f);

    // Rim shading: light source top-left, shadow bottom-right
    {
        Path rimDark;
        rimDark.addCentredArc (cx, cy, rimR - 0.75f, rimR - 0.75f, 0.0f,
                               -0.15f * MathConstants<float>::pi,
                                0.55f * MathConstants<float>::pi, true);
        g.setColour (Colour (0x7A000000));
        g.strokePath (rimDark, PathStrokeType (1.4f, PathStrokeType::curved, PathStrokeType::rounded));

        Path rimLight;
        rimLight.addCentredArc (cx, cy, rimR - 0.75f, rimR - 0.75f, 0.0f,
                                 0.85f * MathConstants<float>::pi,
                                 1.65f * MathConstants<float>::pi, true);
        g.setColour (Colour (hovered ? 0x66FFFFFF : 0x45FFFFFF));
        g.strokePath (rimLight, PathStrokeType (1.2f, PathStrokeType::curved, PathStrokeType::rounded));
    }

    // ── Knob body — radial-gradient dark metal face ────────────────────────────
    const float bodyR = r * knobBodyScale;
    {
        auto bodyLight = Colour (0xFF40404F);
        auto bodyMid   = Colour (0xFF2B2B39);
        auto bodyDark  = Colour (0xFF191922);

        // Gentle contrast lift on hover / drag (visual feedback only)
        if (hovered)  { bodyLight = bodyLight.brighter (0.06f); bodyMid = bodyMid.brighter (0.06f); }
        if (dragging) { bodyLight = bodyLight.brighter (0.12f); bodyMid = bodyMid.brighter (0.10f); }

        ColourGradient faceGrad (bodyLight,
                                 cx - bodyR * 0.45f, cy - bodyR * 0.55f,
                                 bodyDark,
                                 cx + bodyR * 0.30f, cy + bodyR * 0.55f,
                                 true);
        faceGrad.addColour (0.55f, bodyMid);
        g.setGradientFill (faceGrad);
        g.fillEllipse (cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);

        // Brushed-metal texture — hairline vertical strokes over the face
        g.saveState();
        g.reduceClipRegion (Rectangle<float> (cx - bodyR, cy - bodyR,
                                              bodyR * 2.0f, bodyR * 2.0f).toNearestInt());
        g.setColour (Colour (0x06FFFFFF));
        for (float lineX = cx - bodyR; lineX < cx + bodyR; lineX += 2.0f)
            g.drawVerticalLine (static_cast<int> (lineX), cy - bodyR, cy + bodyR);
        g.restoreState();

        // Body edge bevel — dark outline plus a top-left glint and a
        // bottom-right shade, giving the face its physical thickness.
        g.setColour (Colour (0x8C000000));
        g.drawEllipse (cx - bodyR + 0.5f, cy - bodyR + 0.5f,
                       bodyR * 2.0f - 1.0f, bodyR * 2.0f - 1.0f, 1.1f);

        const float bevelW = jlimit (1.0f, 1.8f, bodyR * 0.05f);

        Path bevelLight;
        bevelLight.addCentredArc (cx, cy, bodyR - 1.0f, bodyR - 1.0f, 0.0f,
                                   0.95f * MathConstants<float>::pi,
                                   1.60f * MathConstants<float>::pi, true);
        g.setColour (Colour (hovered ? 0x59FFFFFF : 0x3DFFFFFF));
        g.strokePath (bevelLight, PathStrokeType (bevelW, PathStrokeType::curved, PathStrokeType::rounded));

        Path bevelDark;
        bevelDark.addCentredArc (cx, cy, bodyR - 1.0f, bodyR - 1.0f, 0.0f,
                                 -0.10f * MathConstants<float>::pi,
                                  0.60f * MathConstants<float>::pi, true);
        g.setColour (Colour (0x5A000000));
        g.strokePath (bevelDark, PathStrokeType (bevelW, PathStrokeType::curved, PathStrokeType::rounded));

        // ── Inner shadow — recessed depth around the cap for 3D effect ──
        {
            const float capRadius = r * knobCapScale;
            const float shadowR = capRadius + (bodyR - capRadius) * 0.35f;
            Path innerShadow;
            innerShadow.addCentredArc (cx, cy, shadowR, shadowR, 0.0f,
                                       -0.20f * MathConstants<float>::pi,
                                        0.60f * MathConstants<float>::pi, true);
            g.setColour (Colour (0x45000000));
            g.strokePath (innerShadow, PathStrokeType ((bodyR - capRadius) * 0.45f,
                                                      PathStrokeType::curved, PathStrokeType::rounded));
        }
    }

    // Disabled knobs get a uniform dimming wash so they read as inactive
    // (same overall impression as the previous flat disabled fill).
    if (! enabled)
    {
        g.setColour (disabled.withAlpha (0.40f));
        g.fillEllipse (cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);
    }
}

// ─── Premium knob centre cap (shared) ─────────────────────────────────────────
// Machined aluminium cap; the value text / waveform icon is drawn on top.

void GhostSignalLookAndFeel::drawPremiumKnobCap (Graphics& g,
                                                 Point<float> centre,
                                                 float capRadius,
                                                 bool enabled)
{
    const float cx   = centre.x;
    const float cy   = centre.y;
    const float capR = capRadius;

    // Machined face — radial gradient with the sheen towards the top-left
    ColourGradient capGrad (Colour (0xFF3E3E50),
                            cx - capR * 0.40f, cy - capR * 0.55f,
                            Colour (0xFF0F0F16),
                            cx + capR * 0.30f, cy + capR * 0.60f,
                            true);
    capGrad.addColour (0.55f, Colour (0xFF23232E));
    g.setGradientFill (capGrad);
    g.fillEllipse (cx - capR, cy - capR, capR * 2.0f, capR * 2.0f);

    // Recessed ring where the cap meets the knob body
    g.setColour (Colour (0xA6000000));
    g.drawEllipse (cx - capR + 0.5f, cy - capR + 0.5f,
                   capR * 2.0f - 1.0f, capR * 2.0f - 1.0f, 1.0f);

    // Tiny top-left glint on the cap edge
    Path capGlint;
    capGlint.addCentredArc (cx, cy, capR - 1.0f, capR - 1.0f, 0.0f,
                            1.05f * MathConstants<float>::pi,
                            1.50f * MathConstants<float>::pi, true);
    g.setColour (Colour (0x38FFFFFF));
    g.strokePath (capGlint, PathStrokeType (1.0f, PathStrokeType::curved, PathStrokeType::rounded));

    // Dim the cap for disabled knobs (value text is skipped separately)
    if (! enabled)
    {
        g.setColour (Colour (0x3A000000));
        g.fillEllipse (cx - capR, cy - capR, capR * 2.0f, capR * 2.0f);
    }
}

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
    return Font (FontOptions (getSectionTitleFontSize (panelHeight), Font::bold));
}

juce::Font GhostSignalLookAndFeel::getParamLabelFont (int widgetHeight)
{
    return Font (FontOptions (getParamLabelFontSize (widgetHeight), Font::bold));
}

juce::Font GhostSignalLookAndFeel::getValueFont (int widgetHeight)
{
    return Font (FontOptions (getValueFontSize (widgetHeight), Font::plain));
}

juce::Font GhostSignalLookAndFeel::getKnobLabelFont (int knobDiameter)
{
    return Font (FontOptions (getKnobFontSize (knobDiameter), Font::bold));
}

// ─── Rotary slider rendering ──────────────────────────────────────────────────
//
// Premium hardware-style knob assembled from shared layers (see
// drawPremiumKnobBody / drawPremiumKnobCap):
//   drop shadow → beveled rim → metal body → position indicator →
//   accent value ring → machined centre cap with readout.
// Hover adds a gentle sheen; dragging brightens the accent ring and pointer.
// Purely cosmetic — no interaction or parameter behaviour is changed.

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

    const bool enabled  = slider.isEnabled();
    const bool hovered  = enabled && slider.isMouseOver();
    const bool dragging = enabled && slider.isMouseButtonDown();

    const float toAngle = rotaryStartAngle
                        + jlimit (0.0f, 1.0f, sliderPos) * (rotaryEndAngle - rotaryStartAngle);

    // ── Value / track arc — aligned with tick marks inside the knob ──────────
    //    Tick marks span from 0.32*diameter to 0.48*diameter (0.64r to 0.96r).
    //    The track arc is centered on the tick midpoint (0.80r) with thickness
    //    spanning the full tick range so the outer ring visually correlates.
    const float arcRadius    = r * 0.80f;
    const float arcThickness = jlimit (3.0f, 8.0f, r * 0.32f);

    // Track arc — full travel range, recessed dark groove aligned with ticks
    {
        // Dark shadow underneath for depth
        {
            Path trackShadow;
            trackShadow.addCentredArc (cx, cy, arcRadius, arcRadius,
                                      0.0f,
                                      rotaryStartAngle, rotaryEndAngle,
                                      true);
            PathStrokeType shadowStroke (arcThickness + 2.0f, PathStrokeType::curved, PathStrokeType::rounded);
            g.setColour (Colour (0x40000000));
            g.strokePath (trackShadow, shadowStroke);
        }

        // Main track groove
        Path track;
        track.addCentredArc (cx, cy, arcRadius, arcRadius,
                             0.0f,
                             rotaryStartAngle, rotaryEndAngle,
                             true);
        PathStrokeType stroke (arcThickness, PathStrokeType::curved, PathStrokeType::rounded);
        g.setColour (Colour (0xFF2E2E3A));
        g.strokePath (track, stroke);

        // Highlight on top edge for 3D groove effect
        {
            Path trackHighlight;
            trackHighlight.addCentredArc (cx, cy, arcRadius, arcRadius,
                                         0.0f,
                                         rotaryStartAngle, rotaryEndAngle,
                                         true);
            PathStrokeType highlightStroke (arcThickness * 0.3f, PathStrokeType::curved, PathStrokeType::rounded);
            g.setColour (Colour (0x25FFFFFF));
            g.strokePath (trackHighlight, highlightStroke);
        }
    }

    // Value arc — muted accent, from the start angle to the current position
    if (enabled && sliderPos > 0.001f)
    {
        // Soft glow behind the value arc (slightly stronger while dragging)
        {
            Path glow;
            glow.addCentredArc (cx, cy, arcRadius, arcRadius,
                                0.0f,
                                rotaryStartAngle, toAngle,
                                true);
            PathStrokeType glowStroke (arcThickness * 2.1f, PathStrokeType::curved, PathStrokeType::rounded);
            g.setColour (accent.withAlpha (dragging ? 0.18f : 0.11f));
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
            g.setColour (dragging ? accent.brighter (0.22f) : accent);
            g.strokePath (valueArc, stroke);
        }

        // Bright dot at the tip of the arc — the exact value is readable at
        // a glance from across the whole synth.
        {
            const float dotR = arcThickness * 0.65f;
            // Outer glow
            g.setColour (accent.withAlpha (0.30f));
            g.fillEllipse (cx + std::cos (toAngle) * arcRadius - dotR * 1.5f,
                           cy + std::sin (toAngle) * arcRadius - dotR * 1.5f,
                           dotR * 3.0f, dotR * 3.0f);
            // Main dot
            g.setColour (dragging ? Colour (0xFFE0E0E8) : accent.brighter (0.35f));
            g.fillEllipse (cx + std::cos (toAngle) * arcRadius - dotR,
                           cy + std::sin (toAngle) * arcRadius - dotR,
                           dotR * 2.0f, dotR * 2.0f);
        }
    }

    // ── Physical knob body (shadow, rim, face, bevels) ─────────────────────────
    drawPremiumKnobBody (g, { cx, cy }, r, enabled, hovered, dragging);

    const float bodyR = r * knobBodyScale;
    const float capR  = r * knobCapScale;

    // ── Position indicator — physical pointer from the cap toward the rim ──────
    // Rotates with the value so every knob's position is readable without
    // reading the numeric centre display.
    if (enabled)
    {
        const float indStart = capR + r * 0.09f;
        const float indEnd   = bodyR * 0.86f;
        const float indW     = jlimit (2.0f, 3.5f, r * 0.085f);

        const float sinA = std::sin (toAngle);
        const float cosA = std::cos (toAngle);
        const Line<float> pointer (cx + cosA * indStart, cy + sinA * indStart,
                                   cx + cosA * indEnd,   cy + sinA * indEnd);

        // Restrained accent halo under the pointer
        g.setColour (accent.withAlpha (0.30f));
        g.drawLine (pointer, indW + jlimit (1.5f, 2.5f, r * 0.06f));

        // Crisp machined pointer with a rounded tip
        g.setColour (dragging ? Colour (0xFFFFFFFF)
                              : textPrimary.withAlpha (hovered ? 0.98f : 0.88f));
        g.drawLine (pointer, indW);

        const float tipR = indW * 0.65f;
        g.fillEllipse (cx + cosA * indEnd - tipR, cy + sinA * indEnd - tipR,
                       tipR * 2.0f, tipR * 2.0f);
    }

    // ── Machined centre cap ────────────────────────────────────────────────────
    drawPremiumKnobCap (g, { cx, cy }, capR, enabled);

    // ── Value text in centre ───────────────────────────────────────────────────
    // Skipped when the knob displays its own centre text (e.g. LFO rate knob
    // showing a tempo division or ms period), signalled via a slider property.
    if (enabled && ! (bool) slider.getProperties().getWithDefault ("hideCenterValue", false))
    {
        const String centreText = slider.getTextFromValue (slider.getValue());

        if (centreText.isNotEmpty())
        {
            float fontSize = jlimit (7.0f, 11.0f, capR * 1.4f);
            g.setColour (textPrimary);
            g.setFont (Font (FontOptions (fontSize, Font::bold)));

            // Shrink the font so multi-digit values (e.g. "100") still fit
            // inside the centre cap instead of being clipped.
            const float maxTextW = capR * 3.0f;
            while (fontSize > 6.0f
                   && GlyphArrangement::getStringWidth (g.getCurrentFont(), centreText) > maxTextW)
            {
                fontSize -= 0.5f;
                g.setFont (Font (FontOptions (fontSize, Font::bold)));
            }

            // The text area is wider than the cap itself so 3-digit values
            // aren't clipped; the text may overhang slightly onto the knob body.
            g.drawFittedText (centreText,
                              Rectangle<int> (roundToInt (cx - maxTextW * 0.5f),
                                              roundToInt (cy - capR),
                                              roundToInt (maxTextW),
                                              roundToInt (capR * 2.0f)),
                              Justification::centred, 1, 0.7f);
        }
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
