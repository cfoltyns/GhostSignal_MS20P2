/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Premium industrial LookAndFeel implementation.
 *              Knobs are finished in the same dark, grainy surface as the
 *              section panels — panel colour plus the shared grain tile — with
 *              only a very soft top highlight and a light contact shadow.
 *              Panels have recessed bodies with inner shadows and subtle
 *              beveled edges.
 */

#include "LookAndFeel.h"

using namespace juce;

// ─── Minimal typeface selector for the preset-mono UI ────────────────────────
namespace
{
    static juce::String findMonospaceTypefaceName()
    {
        const auto available = juce::Font::findAllTypefaceNames();
        const char* candidates[] =
        {
            "JetBrains Mono", "Cascadia Mono", "Cascadia Code", "Consolas",
            "DejaVu Sans Mono", "Liberation Mono", "Menlo", "Monaco",
            "Lucida Console", "Courier New"
        };

        for (const auto* candidate : candidates)
        {
            if (available.contains (candidate, true))
                return candidate;
        }
        return juce::String();
    }
}

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

// ─── Knob geometry (shared) ──────────────────────────────────────────────────
// Proportions relative to the knob's overall radius. Shared by the LookAndFeel
// rotary renderer and the WaveformKnob so every knob in the plugin has
// identical physical proportions.
namespace
{
    constexpr float knobFlangeScale = 0.88f;   // rim disc radius / overall radius
    constexpr float knobBodyScale   = 0.65f;   // body radius / overall radius
    constexpr float knobCapScale    = 0.20f;   // center detail radius / overall radius
    constexpr float knobGrainOpacity = 0.55f;  // grain opacity — same as panels
}

// ─── Simplified knob body (shared) ───────────────────────────────────────────
// The knob is made of the same material as the dark section panels: the same
// panel colour and the exact same fine grain tile at the same opacity. On top
// of that, only three things:
//   • a very light contact shadow underneath,
//   • a simple soft rim so the circular shape reads cleanly,
//   • a single very soft highlight across the top (top-left to top-center) —
//     just enough to suggest a slight curve or bevel.
// No brushed metal, no speculars, no sharp reflections, no heavy shadows.
// `hovered`/`dragging` only lift the highlight slightly (visual feedback only).

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

    const float flangeR = r * knobFlangeScale;   // soft rim disc
    const float bodyR   = r * knobBodyScale;     // knob face

    // ── Minimal contact shadow ────────────────────────────────────────────────
    // A few progressively larger, faint black ellipses offset downward, so the
    // knob sits on the panel. Deliberately light — no heavy drop shadow.
    for (int i = 0; i < 4; ++i)
    {
        const float sr = flangeR * (1.0f + i * 0.025f);
        const float dy = 1.0f + i * 1.8f;
        const uint8 alpha = static_cast<uint8> (0x2C - i * 0x0A);
        g.setColour (Colour (static_cast<uint32> (alpha) << 24));
        g.fillEllipse (cx - sr, cy - sr + dy, sr * 2.0f, sr * 2.0f);
    }

    // ── Soft rim (base disc) ──────────────────────────────────────────────────
    // Same panel colour with the same grain as the panels, so the rim reads as
    // part of the chassis surface rather than a separate metal/plastic ring.
    g.setColour (panel);
    g.fillEllipse (cx - flangeR, cy - flangeR, flangeR * 2.0f, flangeR * 2.0f);

    {
        g.saveState();
        Path rimClip;
        rimClip.addEllipse (cx - flangeR, cy - flangeR, flangeR * 2.0f, flangeR * 2.0f);
        g.reduceClipRegion (rimClip);
        g.setTiledImageFill (getPanelGrainTile(), 0, 0, knobGrainOpacity);
        g.fillAll();
        g.restoreState();
    }

    // Simple soft rim: a hairline edge around the base disc plus one faint
    // catch of light along its top-left arc. Nothing harder than that.
    g.setColour (Colours::black.withAlpha (0.28f));
    g.drawEllipse (cx - flangeR + 0.5f, cy - flangeR + 0.5f,
                   flangeR * 2.0f - 1.0f, flangeR * 2.0f - 1.0f, 1.0f);

    {
        const float lift = dragging ? 0.03f : (hovered ? 0.015f : 0.0f);

        Path rimLight;
        rimLight.addCentredArc (cx, cy, flangeR - 0.8f, flangeR - 0.8f, 0.0f,
                                0.80f * MathConstants<float>::pi,
                                1.70f * MathConstants<float>::pi, true);
        g.setColour (Colours::white.withAlpha (0.06f + lift));
        g.strokePath (rimLight, PathStrokeType (1.0f, PathStrokeType::curved, PathStrokeType::rounded));
    }

    // Soft seam where the face meets the rim, so the face stays a clean circle.
    g.setColour (Colours::black.withAlpha (0.20f));
    g.drawEllipse (cx - bodyR - 0.5f, cy - bodyR - 0.5f,
                   bodyR * 2.0f + 1.0f, bodyR * 2.0f + 1.0f, 1.0f);

    // ── Knob face ─────────────────────────────────────────────────────────────
    // Flat panel colour with the identical grain tile at the identical opacity
    // the panels use — nothing else on the surface.
    g.setColour (panel);
    g.fillEllipse (cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);

    {
        g.saveState();
        Path faceClip;
        faceClip.addEllipse (cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);
        g.reduceClipRegion (faceClip);
        g.setTiledImageFill (getPanelGrainTile(), 0, 0, knobGrainOpacity);
        g.fillAll();
        g.restoreState();
    }

    // ── Very soft top highlight ───────────────────────────────────────────────
    // One gentle pool of light biased toward the top-left / top-center, fading
    // to nothing well before the bottom of the face. Just enough to suggest a
    // slight curve; no specular blob, no sharp catchlights.
    {
        const float lift  = dragging ? 0.035f : (hovered ? 0.015f : 0.0f);
        const float hlA   = 0.075f + lift;
        const float hlX   = cx - bodyR * 0.30f;   // biased toward top-left
        const float hlY   = cy - bodyR * 0.45f;
        const float hlRad = bodyR * 1.5f;

        g.saveState();
        Path hlClip;
        hlClip.addEllipse (cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);
        g.reduceClipRegion (hlClip);
        g.setGradientFill (ColourGradient (Colours::white.withAlpha (hlA), hlX, hlY,
                                           Colours::white.withAlpha (0.0f), hlX, hlY + hlRad,
                                           true));
        g.fillEllipse (cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);
        g.restoreState();
    }

    // NOTE: radial tick marks are NOT drawn here. They live in the shared
    // drawIndustrialKnobTicks() helper, which each rotary renderer calls with
    // the angles of its own value arc, so the body stays purely "surface".

    if (! enabled)
    {
        g.setColour (disabled.withAlpha (0.40f));
        g.fillEllipse (cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);
    }
}

// ─── Simplified knob cap (shared) ────────────────────────────────────────────
// Small centre detail under the value text / waveform icon: a barely darker
// recess in the same grain, with only a hairline edge. No glint, no sheen.

void GhostSignalLookAndFeel::drawIndustrialKnobCap (Graphics& g,
                                                    Point<float> centre,
                                                    float capRadius,
                                                    bool enabled)
{
    const float cx   = centre.x;
    const float cy   = centre.y;
    const float capR = capRadius;

    g.setColour (panel.darker (0.10f));
    g.fillEllipse (cx - capR, cy - capR, capR * 2.0f, capR * 2.0f);

    {
        g.saveState();
        Path capClip;
        capClip.addEllipse (cx - capR, cy - capR, capR * 2.0f, capR * 2.0f);
        g.reduceClipRegion (capClip);
        g.setTiledImageFill (getPanelGrainTile(), 0, 0, knobGrainOpacity);
        g.fillAll();
        g.restoreState();
    }

    g.setColour (Colours::black.withAlpha (0.25f));
    g.drawEllipse (cx - capR + 0.5f, cy - capR + 0.5f,
                   capR * 2.0f - 1.0f, capR * 2.0f - 1.0f, 1.0f);

    if (! enabled)
    {
        g.setColour (disabled.withAlpha (0.40f));
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

    // Molded / silkscreened scale marks: each tick gets a soft contact shadow
    // offset away from the top-left light, then a main line whose thickness
    // scales with the knob and whose brightness varies — the end stops and the
    // centre detente read slightly stronger, like printed reference marks.
    const float tickW = jlimit (1.2f, 2.2f, radius * 0.020f);

    for (int i = 0; i < numTicks; ++i)
    {
        const float t     = (float) i / (float) (numTicks - 1);
        const float angle = startAngle + t * (endAngle - startAngle);
        const float sn    = std::sin (angle);
        const float cs    = std::cos (angle);

        const float x1 = cx + cs * innerR;
        const float y1 = cy + sn * innerR;
        const float x2 = cx + cs * outerR;
        const float y2 = cy + sn * outerR;

        // Engraved scale marks: a dark cut into the metal with a faint lit core
        // just above the groove face, so each mark has depth rather than looking
        // like flat ink. End stops and the centre detente read slightly deeper,
        // like reference marks on hardware.
        const bool isKeyMark = (i == 0 || i == numTicks - 1 || i == numTicks / 2);
        const float depth    = isKeyMark ? 0.85f : 0.60f;

        // Contact shadow — offset down-right, away from the light.
        g.setColour (Colour (0x50000000));
        g.drawLine (x1 + 0.75f, y1 + 1.0f, x2 + 0.75f, y2 + 1.0f, tickW);

        // The cut itself.
        g.setColour (Colours::black.withAlpha (depth * 0.60f));
        g.drawLine (x1, y1, x2, y2, tickW);

        // Lit metal core above the groove — the engraved highlight.
        g.setColour (textSecondary.withAlpha (depth));
        g.drawLine (x1, y1 - tickW * 0.30f, x2, y2 - tickW * 0.30f, tickW * 0.50f);
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

// ─── Monospace font (preset / modal UI) ───────────────────────────
// The preset browser and save/rename dialog are deliberately rendered as
// a terminal surface, so their chrome uses a monospace face rather than
// the industrial headings used elsewhere in the plugin. The size is
// clamped to the 9–11 px band so a single, readable face is used
// everywhere — even when a caller passes a larger value.

juce::Font GhostSignalLookAndFeel::getMonospaceFont (float size, bool bold)
{
    const float clamped = jlimit (9.0f, 11.0f, size);
    const int flags = bold ? Font::bold : Font::plain;

    static const juce::String typeface = findMonospaceTypefaceName();

    if (typeface.isNotEmpty())
        return Font (FontOptions (typeface, clamped, flags));

    return Font (FontOptions (clamped, flags));
}

juce::Font GhostSignalLookAndFeel::getMonoLabelFont (float scale, bool bold)
{
    return getMonospaceFont (11.0f * scale, bold);
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
// The knob is assembled from the shared simplified layers (see
// drawIndustrialKnobBody / drawIndustrialKnobCap):
//   contact shadow → grainy rim → grainy face with a soft top highlight →
//   ticks → position pointer → value arc → center cap with readout.
// The value/track arcs, pointer, ticks and readout are untouched — only the
// knob surface itself was simplified to match the panel grain.
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

    // ── Knob body (grainy rim + face, soft top highlight) ──────────────────────
    drawIndustrialKnobBody (g, { cx, cy }, r, enabled, hovered, dragging);

    // ── Radial tick marks around the outside of the flange ────────────────────
    // 13 marks spanning the same sweep as the value arc, drawn after the body so
    // they sit crisply on top of its drop shadow.
    drawIndustrialKnobTicks (g, { cx, cy }, r, 13, rotaryStartAngle, rotaryEndAngle);

    const float bodyR = r * knobBodyScale;
    const float capR  = r * knobCapScale;

    // ── Position indicator — molded ridge on the top surface ───────────────────
    // A substantial physical pointer rather than a drawn line: a recessed
    // channel shadow, a cream painted ridge body, a catchlight along its lit
    // edge, and a small dome tip that catches the light. Rotates with the
    // value so every knob's position is readable without the numeric display.
    if (enabled)
    {
        const float indStart = capR + r * 0.05f;
        const float indEnd   = bodyR * 0.92f;
        const float indW     = jlimit (2.5f, 4.5f, r * 0.062f);

        const float sinA = std::sin (toAngle);
        const float cosA = std::cos (toAngle);
        const Line<float> pointer (cx + cosA * indStart, cy + sinA * indStart,
                                   cx + cosA * indEnd,   cy + sinA * indEnd);

        const float px1 = pointer.getStartX(), py1 = pointer.getStartY();
        const float px2 = pointer.getEndX(),   py2 = pointer.getEndY();

        // Recessed channel — soft shadow offset away from the light source.
        g.setColour (Colour (0x5E000000));
        g.drawLine (px1 + 0.8f, py1 + 1.1f, px2 + 0.8f, py2 + 1.1f, indW + 1.2f);

        // Ridge body — soft cream paint, slightly warm against the dark body.
        g.setColour (dragging ? Colour (0xFFF2EDE0)
                              : Colour (0xFFD9D2C2).withAlpha (hovered ? 0.97f : 0.92f));
        g.drawLine (pointer, indW);

        // Catchlight along the ridge's lit (upper) edge.
        g.setColour (Colours::white.withAlpha (dragging ? 0.55f : 0.38f));
        g.drawLine (px1, py1 - indW * 0.30f, px2, py2 - indW * 0.30f,
                    jmax (0.8f, indW * 0.30f));

        // Dome tip — small rounded cap with its own shadow and catchlight.
        const float tipR = indW * 0.80f;
        const float tipX = cx + cosA * indEnd;
        const float tipY = cy + sinA * indEnd;

        g.setColour (Colour (0x50000000));
        g.fillEllipse (tipX - tipR + 0.8f, tipY - tipR + 1.1f, tipR * 2.0f, tipR * 2.0f);

        g.setColour (dragging ? Colour (0xFFF6F1E4) : Colour (0xFFE2DBC9));
        g.fillEllipse (tipX - tipR, tipY - tipR, tipR * 2.0f, tipR * 2.0f);

        g.setColour (Colours::white.withAlpha (0.45f));
        g.fillEllipse (tipX - tipR * 0.55f, tipY - tipR * 0.70f,
                       tipR * 0.90f, tipR * 0.70f);
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
            g.setColour (textPrimary);
            g.setFont (GhostSignalLookAndFeel::getMonospaceFont (capR * 1.4f, true));

            // Shrink the font so multi-digit values (e.g. "100") still fit
            // inside the centre cap instead of being clipped.
            const float maxTextW = capR * 3.0f;
            while (GlyphArrangement::getStringWidth (g.getCurrentFont(), centreText) > maxTextW)
            {
                const float smaller = jlimit (6.0f, 11.0f, g.getCurrentFont().getHeightInPoints() - 0.5f);
                g.setFont (GhostSignalLookAndFeel::getMonospaceFont (smaller, true));
            }

            // Printed into the flat top face: a soft dark pass offset away from
            // the light gives the digits a recessed, silkscreened feel while
            // staying perfectly legible.
            const auto textArea = Rectangle<int> (roundToInt (cx - maxTextW * 0.5f),
                                                  roundToInt (cy - capR),
                                                  roundToInt (maxTextW),
                                                  roundToInt (capR * 2.0f));

            g.setColour (Colour (0x8C000000));
            g.drawFittedText (centreText, textArea.translated (1, 1),
                              Justification::centred, 1, 0.7f);

            g.setColour (Colour (0xFFF2EDE0));
            g.drawFittedText (centreText, textArea,
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
