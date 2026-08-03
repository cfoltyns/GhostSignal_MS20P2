/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Interactive ADSR envelope display with 4 draggable points.
 *              Uses the GhostSignalLookAndFeel color palette.
 */

#include "EnvDisplay.h"
#include "../LookAndFeel.h"

EnvDisplay::EnvDisplay()
{
    setRepaintsOnMouseActivity (true);
}

// ─── Layout helpers ────────────────────────────────────────────────────────────

static constexpr float marginL = 3.0f;   // reduced for edge-to-edge curve
static constexpr float marginR = 3.0f;
static constexpr float marginT = 3.0f;
static constexpr float marginB = 3.0f;

// Time-axis fractions — must sum to 1.0 so the curve spans edge-to-edge
static constexpr float attackFrac   = 0.22f;
static constexpr float decayFrac    = 0.22f;
static constexpr float sustainFrac  = 0.10f;
static constexpr float releaseFrac  = 0.46f;

juce::Rectangle<float> EnvDisplay::getAttackPoint() const
{
    const auto b = getLocalBounds().toFloat().reduced (marginL, marginT);
    const float x = b.getX() + attackVal * attackFrac * b.getWidth();
    const float y = b.getY();                       // Top of display = peak
    const float pad = 5.0f;
    return { x - pad, y - pad, pad * 2, pad * 2 };
}

juce::Rectangle<float> EnvDisplay::getDecayPoint() const
{
    const auto b = getLocalBounds().toFloat().reduced (marginL, marginT);
    const float startX = b.getX() + attackVal * attackFrac * b.getWidth();
    float t = attackVal * attackFrac + decayVal * decayFrac;
    float x = b.getX() + t * b.getWidth();
    const float y = b.getY() + (1.0f - sustainVal) * b.getHeight();
    const float pad = 5.0f;
    return { x - pad, y - pad, pad * 2, pad * 2 };
}

juce::Rectangle<float> EnvDisplay::getSustainPoint() const
{
    const auto b = getLocalBounds().toFloat().reduced (marginL, marginT);
    const float startX = b.getX() + attackVal * attackFrac * b.getWidth();
    float t = attackVal * attackFrac + decayVal * decayFrac + sustainFrac;
    float x = b.getX() + t * b.getWidth();
    const float y = b.getY() + (1.0f - sustainVal) * b.getHeight();
    const float pad = 5.0f;
    return { x - pad, y - pad, pad * 2, pad * 2 };
}

juce::Rectangle<float> EnvDisplay::getReleasePoint() const
{
    const auto b = getLocalBounds().toFloat().reduced (marginL, marginT);
    const float startX = b.getX() + attackVal * attackFrac * b.getWidth();
    float t = attackVal * attackFrac + decayVal * decayFrac + sustainFrac + releaseVal * releaseFrac;
    float x = b.getX() + juce::jmin (t, 1.0f) * b.getWidth();
    const float y = b.getBottom();
    const float pad = 5.0f;
    return { x - pad, y - pad, pad * 2, pad * 2 };
}

juce::Path EnvDisplay::getEnvelopePath() const
{
    const auto b = getLocalBounds().toFloat().reduced (marginL, marginT);
    const float startX = b.getX();
    const float endX   = b.getRight();

    const float peakY  = b.getY();
    const float baseY  = b.getBottom();
    const float sustY  = b.getY() + (1.0f - sustainVal) * b.getHeight();

    const float attackX  = startX + attackVal  * attackFrac  * b.getWidth();
    const float decayX   = startX + (attackVal * attackFrac + decayVal * decayFrac) * b.getWidth();
    const float sustainX = startX + (attackVal * attackFrac + decayVal * decayFrac + sustainFrac) * b.getWidth();
    const float releaseX = startX + juce::jmin (
        attackVal * attackFrac + decayVal * decayFrac + sustainFrac + releaseVal * releaseFrac,
        1.0f) * b.getWidth();

    juce::Path path;
    path.startNewSubPath (startX, baseY);        // Start at bottom-left
    path.lineTo (attackX, peakY);                 // Attack to peak
    path.lineTo (decayX,  sustY);                 // Decay to sustain level
    path.lineTo (sustainX, sustY);                // Sustain flat segment
    path.lineTo (releaseX, baseY);                // Release to bottom

    return path;
}

// ─── Paint ────────────────────────────────────────────────────────────────────

void EnvDisplay::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();
    const float w = b.getWidth();
    const float h = b.getHeight();

    // Background — dark screen with subtle gradient
    juce::ColourGradient bgGrad;
    bgGrad.point1 = { 0.0f, 0.0f };
    bgGrad.point2 = { 0.0f, h };
    bgGrad.addColour (0.0f, GhostSignalLookAndFeel::panel.darker (0.1f));
    bgGrad.addColour (1.0f, GhostSignalLookAndFeel::panel.darker (0.2f));
    g.setGradientFill (bgGrad);
    g.fillRoundedRectangle (b, 4.0f);

    // Border
    g.setColour (GhostSignalLookAndFeel::panelBorder);
    g.drawRoundedRectangle (b.reduced (0.5f), 4.0f, 1.0f);

    // Inner shadow for depth
    g.setColour (GhostSignalLookAndFeel::panelShadow);
    g.drawRoundedRectangle (b.reduced (1.0f), 3.5f, 0.5f);

    // Grid lines
    g.setColour (juce::Colour (0x12FFFFFF));
    for (int i = 1; i < 4; ++i)
    {
        const float x = b.getX() + w * i / 4.0f;
        g.drawVerticalLine ((int) x, b.getY(), b.getBottom());
    }
    for (int i = 1; i < 3; ++i)
    {
        const float y = b.getY() + h * i / 3.0f;
        g.drawHorizontalLine ((int) y, b.getX(), b.getRight());
    }

    // Envelope fill — subtle gradient using accent color
    {
        juce::Path fillPath = getEnvelopePath();
        fillPath.lineTo (fillPath.getBounds().getBottomRight());
        fillPath.closeSubPath();

        juce::ColourGradient fillGrad (
            GhostSignalLookAndFeel::accent.withAlpha (0.35f), juce::jmax (b.getX(), fillPath.getBounds().getCentreX()), b.getY(),
            GhostSignalLookAndFeel::accent.withAlpha (0.10f), juce::jmax (b.getX(), fillPath.getBounds().getCentreX()), b.getBottom(),
            false);
        g.setGradientFill (fillGrad);
        g.fillPath (fillPath);
    }

    // Envelope curve
    const juce::Path envPath = getEnvelopePath();
    g.setColour (GhostSignalLookAndFeel::accent);
    g.strokePath (envPath, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Subtle glow behind envelope curve
    g.setColour (GhostSignalLookAndFeel::accent.withAlpha (0.15f));
    g.strokePath (envPath, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Stage separator lines (dotted)
    g.setColour (juce::Colour (0x20FFFFFF));
    const auto inner = b.toFloat().reduced (marginL, marginT);
    const float sep1X = inner.getX() + attackFrac * inner.getWidth();
    const float sep2X = inner.getX() + (attackFrac + decayFrac) * inner.getWidth();
    const float sep3X = inner.getX() + (attackFrac + decayFrac + sustainFrac) * inner.getWidth();
    for (float xx : { sep1X, sep2X, sep3X })
    {
        for (float yy = inner.getY(); yy < inner.getBottom(); yy += 4.0f)
            g.drawVerticalLine ((int) xx, yy, yy + 2.0f);
    }

    // Draggable points
    const auto drawPoint = [&] (const juce::Rectangle<float>& pt,
                                bool isDragging,
                                const juce::String& label)
    {
        g.setColour (isDragging ? GhostSignalLookAndFeel::accent : GhostSignalLookAndFeel::textPrimary);
        g.fillEllipse (pt);
        g.setColour (GhostSignalLookAndFeel::panel.darker (0.3f));
        g.drawEllipse (pt, 1.0f);

        // Label below or above the point
        g.setColour (GhostSignalLookAndFeel::textSecondary);
        g.setFont (juce::Font (juce::FontOptions (8.0f, juce::Font::plain)));
        juce::Rectangle<float> labelBounds = pt.translated (0.0f, isDragging ? -12.0f : 8.0f);
        labelBounds.setHeight (10.0f);
        labelBounds.setWidth (14.0f);
        g.drawText (label, labelBounds, juce::Justification::centred, false);
    };

    drawPoint (getAttackPoint(),  dragTarget == attack,  "A");
    drawPoint (getDecayPoint(),   dragTarget == decay,   "D");
    drawPoint (getSustainPoint(), dragTarget == sustain, "S");
    drawPoint (getReleasePoint(), dragTarget == release, "R");
}

// ─── Resized ──────────────────────────────────────────────────────────────────

void EnvDisplay::resized()
{
}

// ─── Mouse handling ────────────────────────────────────────────────────────────

void EnvDisplay::mouseDown (const juce::MouseEvent& e)
{
    const auto pos = e.position;
    if (getAttackPoint().contains (pos))       dragTarget = attack;
    else if (getDecayPoint().contains (pos))   dragTarget = decay;
    else if (getSustainPoint().contains (pos)) dragTarget = sustain;
    else if (getReleasePoint().contains (pos)) dragTarget = release;
    else dragTarget = none;
}

void EnvDisplay::mouseDrag (const juce::MouseEvent& e)
{
    if (dragTarget == none)
        return;

    const auto b = getLocalBounds().toFloat().reduced (marginL, marginT);

    switch (dragTarget)
    {
        case attack:
        {
            // Horizontal drag → attack time
            const float relX = (e.position.x - b.getX()) / b.getWidth();
            attackVal = juce::jlimit (0.01f, 0.99f, relX / attackFrac);
            if (onAttackChanged) onAttackChanged (attackVal);
            break;
        }
        case decay:
        {
            // Horizontal drag → decay time (relative to attack end)
            const float attackEndX = b.getX() + attackVal * attackFrac * b.getWidth();
            const float relX = (e.position.x - attackEndX) / (decayFrac * b.getWidth());
            decayVal = juce::jlimit (0.01f, 0.99f, relX);
            if (onDecayChanged) onDecayChanged (decayVal);
            break;
        }
        case sustain:
        {
            // Vertical drag → sustain level
            const float relY = (e.position.y - b.getY()) / b.getHeight();
            sustainVal = juce::jlimit (0.0f, 1.0f, 1.0f - relY);
            if (onSustainChanged) onSustainChanged (sustainVal);
            break;
        }
        case release:
        {
            // Horizontal drag → release time
            const float startX = b.getX() + (attackVal * attackFrac
                                             + decayVal * decayFrac
                                             + sustainFrac) * b.getWidth();
            const float releaseMaxX = b.getRight();
            const float relX = (e.position.x - startX) / juce::jmax (1.0f, releaseMaxX - startX);
            releaseVal = juce::jlimit (0.01f, 0.99f, relX / releaseFrac);
            if (onReleaseChanged) onReleaseChanged (releaseVal);
            break;
        }
        default:
            break;
    }

    repaint();
}

void EnvDisplay::mouseUp (const juce::MouseEvent&)
{
    dragTarget = none;
}

// ─── Raw-value helpers (for external sync) ────────────────────────────────────

// These are defined in the header as inline convenience methods.
// They convert between raw parameter values (seconds) and normalized 0..1.
