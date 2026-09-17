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

const Colour GhostSignalLookAndFeel::knobBody    { Colour (0xFF2A2A3A) };
const Colour GhostSignalLookAndFeel::panelShadow { Colour (0x30000000) };

// ─── Industrial molded knob geometry (shared) ────────────────────────────────
// Proportions relative to the knob's overall radius. Shared by the LookAndFeel
// rotary renderer and the WaveformSlider so every knob in the plugin has
// identical physical proportions.
namespace
{
    constexpr float knobFlangeScale = 0.88f;   // flange radius / overall radius
    constexpr float knobBodyScale   = 0.65f;   // body radius / overall radius
    constexpr float knobTopScale    = 0.58f;   // top surface radius / overall radius
    constexpr float knobCapScale    = 0.20f;   // center detail radius / overall radius
}

// ─── Industrial molded knob body (shared) ────────────────────────────────────
// Physical layers, outside-in:
//   drop shadow ? mounting flange ? cylindrical side wall ? grip grooves ?
//   top surface ? bevels/highlights.
// `hovered`/`dragging` add a gentle contrast lift (visual feedback only).

void GhostSignalLookAndFeel::drawIndustrialKnobBody (Graphics& g,
                                                      Point<float> centre,
                                                      float radius,
                                                      bool enabled,
                                                      bool hovered,
                                                      bool dragging)
{
    const float cx = centre.x;
    const float cy = centre.y;
    const float r  = radius;

    const float flangeR = r * knobFlangeScale;
    const float bodyR   = r * knobBodyScale;
    const float topR    = r * knobTopScale;

    // Subtle brightness boost when dragging for visual feedback
    const float dragBoost = dragging ? 0.08f : 0.0f;

    const Colour colTopSurf    = Colour (0xFF111617).brighter (dragBoost);
    const Colour colTopHigh    = Colour (0xFF1B2223).brighter (dragBoost);
    const Colour colSideMid    = Colour (0xFF171D1E).brighter (dragBoost);
    const Colour colSideShadow = Colour (0xFF080C0D).brighter (dragBoost);
    const Colour colDeepShadow = Colour (0xFF050707).brighter (dragBoost);
    const Colour colFlangeTop  = Colour (0xFF1A1A1C).brighter (dragBoost);
    const Colour colFlangeBot  = Colour (0xFF08080A).brighter (dragBoost);

    for (int i = 0; i < 4; ++i)
    {
        const float sr = flangeR * (1.0f + i * 0.025f);
        const float dy = 1.0f + i * 1.8f;
        const uint8 alpha = static_cast<uint8>(0x48 - i * 0x0E);
        g.setColour (Colour (static_cast<uint32> (alpha) << 24));
        g.fillEllipse (cx - sr, cy - sr + dy, sr * 2.0f, sr * 2.0f);
    }

    {
        ColourGradient flangeGrad (colFlangeTop, cx, cy - flangeR * 0.3f,
                                   colFlangeBot, cx, cy + flangeR * 0.8f,
                                   false);
        g.setGradientFill (flangeGrad);
        g.fillEllipse (cx - flangeR, cy - flangeR, flangeR * 2.0f, flangeR * 2.0f);

        Path flangeHighlight;
        flangeHighlight.addCentredArc (cx, cy, flangeR - 1.0f, flangeR - 1.0f, 0.0f,
                                       0.75f * MathConstants<float>::pi,
                                       1.75f * MathConstants<float>::pi, true);
        g.setColour (Colour (hovered ? 0x40FFFFFF : 0x28FFFFFF));
        g.strokePath (flangeHighlight, PathStrokeType (1.5f, PathStrokeType::curved, PathStrokeType::rounded));

        Path flangeShadow;
        flangeShadow.addCentredArc (cx, cy, flangeR - 1.0f, flangeR - 1.0f, 0.0f,
                                    -0.25f * MathConstants<float>::pi,
                                     0.25f * MathConstants<float>::pi, true);
        g.setColour (Colour (0x60000000));
        g.strokePath (flangeShadow, PathStrokeType (2.0f, PathStrokeType::curved, PathStrokeType::rounded));
    }

    {
        ColourGradient sideGrad (colSideMid, cx, cy - bodyR,
                                 colSideShadow, cx, cy + bodyR,
                                 false);
        sideGrad.addColour (0.5f, colDeepShadow);
        g.setGradientFill (sideGrad);
        g.fillEllipse (cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);

        Path sideTopHighlight;
        sideTopHighlight.addCentredArc (cx, cy, bodyR - 0.5f, bodyR - 0.5f, 0.0f,
                                        0.70f * MathConstants<float>::pi,
                                        1.80f * MathConstants<float>::pi, true);
        g.setColour (Colour (hovered ? 0x48FFFFFF : 0x30FFFFFF));
        g.strokePath (sideTopHighlight, PathStrokeType (1.0f, PathStrokeType::curved, PathStrokeType::rounded));
    }

    {
        constexpr int numGrooves = 7;
        const float grooveTopY    = cy - bodyR * 0.25f;
        const float grooveBottomY = cy + bodyR * 0.65f;
        const float grooveSpanX   = bodyR * 1.4f;
        const float grooveStartX  = cx - bodyR * 0.7f;

        for (int i = 0; i < numGrooves; ++i)
        {
            const float t = (i + 0.5f) / numGrooves;
            const float gx = grooveStartX + t * grooveSpanX;

            // Dark groove line
            const uint8 grooveAlpha = static_cast<uint8>(0x35 + (i % 3) * 0x08);
            g.setColour (Colour (static_cast<uint32> (grooveAlpha) << 24));
            g.drawVerticalLine (static_cast<int> (gx), grooveTopY, grooveBottomY);

            // Subtle highlight to the left of the groove
            g.setColour (Colour (0x10FFFFFF));
            g.drawVerticalLine (static_cast<int> (gx) - 1, grooveTopY, grooveBottomY);
        }
    }

    {
        ColourGradient topGrad (colTopHigh, cx - topR * 0.4f, cy - topR * 0.5f,
                                colTopSurf, cx + topR * 0.3f, cy + topR * 0.5f,
                                true);
        topGrad.addColour (0.6f, Colour (0xFF151B1C));
        g.setGradientFill (topGrad);
        g.fillEllipse (cx - topR, cy - topR, topR * 2.0f, topR * 2.0f);

        Path topHighlight;
        topHighlight.addCentredArc (cx, cy, topR * 0.85f, topR * 0.85f, 0.0f,
                                    0.95f * MathConstants<float>::pi,
                                    1.55f * MathConstants<float>::pi, true);
        g.setColour (Colour (hovered ? 0x30FFFFFF : 0x1CFFFFFF));
        g.strokePath (topHighlight, PathStrokeType (1.0f, PathStrokeType::curved, PathStrokeType::rounded));
    }

    // ── Soft bevel, inner shadow and mild specular highlight ──────────────────
    // Bevel where the top surface rolls into the side wall, an inner shadow
    // around the lower rim of that surface, and a soft specular blob toward the
    // upper-left light source. Black/white alpha only, so the palette is
    // unchanged; `hovered`/`dragging` lift the highlights very slightly.
    const float lift = dragging ? 0.06f : (hovered ? 0.03f : 0.0f);

    {
        const float bevelR = topR + (bodyR - topR) * 0.45f;

        // Upper bevel catches the light …
        Path bevelLight;
        bevelLight.addCentredArc (cx, cy, bevelR, bevelR, 0.0f,
                                  0.82f * MathConstants<float>::pi,
                                  1.68f * MathConstants<float>::pi, true);
        g.setColour (Colours::white.withAlpha (0.10f + lift));
        g.strokePath (bevelLight, PathStrokeType (1.2f, PathStrokeType::curved, PathStrokeType::rounded));

        // … lower bevel falls away into shadow
        Path bevelDark;
        bevelDark.addCentredArc (cx, cy, bevelR, bevelR, 0.0f,
                                 -0.18f * MathConstants<float>::pi,
                                  0.18f * MathConstants<float>::pi, true);
        g.setColour (Colour (0x3A000000));
        g.strokePath (bevelDark, PathStrokeType (1.8f, PathStrokeType::curved, PathStrokeType::rounded));
    }

    {
        // Inner shadow: the top surface curves away from the light toward its
        // lower-right rim, so that side sinks into the body.
        Path innerShadow;
        innerShadow.addCentredArc (cx, cy, topR * 0.93f, topR * 0.93f, 0.0f,
                                   0.10f * MathConstants<float>::pi,
                                   0.90f * MathConstants<float>::pi, true);
        g.setColour (Colour (0x34000000));
        g.strokePath (innerShadow, PathStrokeType (1.8f, PathStrokeType::curved, PathStrokeType::rounded));

        // Mild specular highlight, offset toward the upper-left light source.
        const float specR = topR * 0.60f;
        const float specX = cx - topR * 0.32f;
        const float specY = cy - topR * 0.28f;

        ColourGradient spec (Colours::white.withAlpha (0.10f + lift * 0.9f), specX, specY,
                             Colours::white.withAlpha (0.0f), specX + specR, specY + specR, true);
        g.setGradientFill (spec);
        g.fillEllipse (specX - specR, specY - specR, specR * 2.0f, specR * 2.0f);
    }

    {
        g.setColour (Colour (0x30000000));
        g.drawEllipse (cx - bodyR + 0.5f, cy - bodyR + 0.5f,
                       bodyR * 2.0f - 1.0f, bodyR * 2.0f - 1.0f, 1.0f);

        g.setColour (Colour (0x20000000));
        g.drawEllipse (cx - topR + 0.5f, cy - topR + 0.5f,
                       topR * 2.0f - 1.0f, topR * 2.0f - 1.0f, 1.0f);
    }

    {
        ColourGradient vignetteGrad (Colour (0x00000000), cx, cy,
                                     Colour (0x18000000), cx + flangeR * 0.7f, cy + flangeR * 0.7f,
                                     true);
        g.setGradientFill (vignetteGrad);
        g.fillEllipse (cx - flangeR, cy - flangeR, flangeR * 2.0f, flangeR * 2.0f);
    }

    // NOTE: radial tick marks are NOT drawn here. They live in the shared
    // drawIndustrialKnobTicks() helper, which each rotary renderer calls with the
    // angles of its own value arc. Drawing a second ring from inside the body
    // would double up on the same annulus (25 marks instead of 13) and fight the
    // arc sweep, so the body stays purely "hardware".

    if (! enabled)
    {
        g.setColour (disabled.withAlpha (0.40f));
        g.fillEllipse (cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);
    }
}

// ─── Industrial knob cap (shared) ────────────────────────────────────────────
// Small center detail for the industrial knob; the value text or waveform
// icon is drawn on top.

void GhostSignalLookAndFeel::drawIndustrialKnobCap (Graphics& g,
                                                    Point<float> centre,
                                                    float capRadius,
                                                    bool enabled)
{
    const float cx   = centre.x;
    const float cy   = centre.y;
    const float capR = capRadius;

    // Small dark circular recess in the center
    ColourGradient capGrad (Colour (0xFF0E1213), cx - capR * 0.3f, cy - capR * 0.4f,
                            Colour (0xFF080A0B), cx + capR * 0.3f, cy + capR * 0.4f,
                            true);
    g.setGradientFill (capGrad);
    g.fillEllipse (cx - capR, cy - capR, capR * 2.0f, capR * 2.0f);

    // Cap edge
    g.setColour (Colour (0x40000000));
    g.drawEllipse (cx - capR + 0.5f, cy - capR + 0.5f,
                   capR * 2.0f - 1.0f, capR * 2.0f - 1.0f, 1.0f);

    // Tiny top-left glint
    Path capGlint;
    capGlint.addCentredArc (cx, cy, capR - 1.0f, capR - 1.0f, 0.0f,
                             1.05f * MathConstants<float>::pi,
                             1.50f * MathConstants<float>::pi, true);
    g.setColour (Colour (0x25FFFFFF));
    g.strokePath (capGlint, PathStrokeType (0.8f, PathStrokeType::curved, PathStrokeType::rounded));

    if (! enabled)
    {
        g.setColour (Colour (0x40000000));
        g.fillEllipse (cx - capR, cy - capR, capR * 2.0f, capR * 2.0f);
    }
}

// ─── Industrial knob tick marks (shared) ─────────────────────────────────────
// Evenly spaced radial marks around the outside of the mounting flange, spanning
// the same sweep as the value arc. They sit clear of the flange rim so no tick
// can touch the value arc, the position pointer or the centre readout, and the
// ring scales with the knob so small knobs get the same marks as large ones.

void GhostSignalLookAndFeel::drawIndustrialKnobTicks (Graphics& g,
                                                      Point<float> centre,
                                                      float radius,
                                                      int numTicks,
                                                      float startAngle,
                                                      float endAngle,
                                                      float innerScale,
                                                      float outerScale)
{
    numTicks = jlimit (11, 13, numTicks);

    if (numTicks < 2 || radius <= 0.0f)
        return;

    const float cx = centre.x;
    const float cy = centre.y;

    // Just outside the flange (knobFlangeScale = 0.88 of the radius) and just
    // inside the component edge, so the thin marks stay fully on-screen.
    const float innerR = radius * innerScale;
    const float outerR = radius * outerScale;

    // Thin and light against the dark knob and panel: readable, never shouty.
    g.setColour (textSecondary.withAlpha (0.55f));

    for (int i = 0; i < numTicks; ++i)
    {
        const float t     = (float) i / (float) (numTicks - 1);
        const float angle = startAngle + t * (endAngle - startAngle);
        const float sn    = std::sin (angle);
        const float cs    = std::cos (angle);

        g.drawLine (cx + cs * innerR, cy + sn * innerR,
                    cx + cs * outerR, cy + sn * outerR,
                    1.0f);
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

// ─── Panel grain tile ─────────────────────────────────────────────────────────
// A 128 x 128 ARGB tile of very low-alpha speckle: half the pixels are a touch
// darker than the panel colour, half a touch lighter. Drawn once and tiled by
// Panel::paint, it gives the flat panel body a faint analogue grain without any
// perceptible tiling seams at these alphas.
// A fixed seed keeps the pattern identical across runs and repaints.

const juce::Image& GhostSignalLookAndFeel::getPanelGrainTile()
{
    static const Image grain = []
    {
        constexpr int tileSize = 128;
        constexpr float darkGrain  = 0.085f;   // peak alpha of the darker speckle
        constexpr float lightGrain = 0.050f;   // peak alpha of the lighter speckle

        Image tile (Image::ARGB, tileSize, tileSize, true);

        Random rng (0x6c6f7767);   // fixed seed — deterministic grain

        for (int y = 0; y < tileSize; ++y)
        {
            for (int x = 0; x < tileSize; ++x)
            {
                const float n = rng.nextFloat();

                const Colour speck = (n < 0.5f)
                    ? Colours::black.withAlpha (n * 2.0f * darkGrain)
                    : Colours::white.withAlpha ((n - 0.5f) * 2.0f * lightGrain);

                tile.setPixelAt (x, y, speck);
            }
        }

        return tile;
    }();

    return grain;
}

// ─── Rotary slider rendering ──────────────────────────────────────────────────
//
// Industrial molded knob assembled from shared layers (see
// drawIndustrialKnobBody / drawIndustrialKnobCap):
//   drop shadow → flange → cylindrical body → grip grooves → top surface →
//   position indicator → value arc → center cap with readout.
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

    // ── Value / track arc — sits on the flange between body and edge ─────────
    const float arcRadius    = r * 0.76f;
    const float arcThickness = jlimit (2.5f, 6.0f, r * 0.12f);

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

    // ── Industrial knob body (flange, side wall, grip grooves, top) ───────────
    drawIndustrialKnobBody (g, { cx, cy }, r, enabled, hovered, dragging);

    // ── Radial tick marks around the outside of the flange ────────────────────
    // 13 marks spanning the same sweep as the value arc, drawn after the body so
    // they sit crisply on top of its drop shadow.
    drawIndustrialKnobTicks (g, { cx, cy }, r, 13, rotaryStartAngle, rotaryEndAngle);

    const float bodyR = r * knobBodyScale;
    const float capR  = r * knobCapScale;

    // ── Position indicator — pointer line on the top surface ───────────────────
    // Rotates with the value so every knob's position is readable without
    // reading the numeric centre display.
    if (enabled)
    {
        const float indStart = capR + r * 0.05f;
        const float indEnd   = bodyR * 0.92f;
        const float indW     = jlimit (1.5f, 2.5f, r * 0.045f);

        const float sinA = std::sin (toAngle);
        const float cosA = std::cos (toAngle);
        const Line<float> pointer (cx + cosA * indStart, cy + sinA * indStart,
                                   cx + cosA * indEnd,   cy + sinA * indEnd);

        // Subtle shadow under pointer
        g.setColour (Colour (0x40000000));
        g.drawLine (pointer, indW + 1.5f);

        // Main pointer line — off-white
        g.setColour (dragging ? Colour (0xFFF0F0F0)
                              : textPrimary.withAlpha (hovered ? 0.95f : 0.85f));
        g.drawLine (pointer, indW);

        const float tipR = indW * 0.5f;
        g.fillEllipse (cx + cosA * indEnd - tipR, cy + sinA * indEnd - tipR,
                       tipR * 2.0f, tipR * 2.0f);
    }

    // ── Industrial centre cap ──────────────────────────────────────────────────
    drawIndustrialKnobCap (g, { cx, cy }, capR, enabled);

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
