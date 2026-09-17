/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Premium industrial panel — flat uniform grey body, left accent
 *              stripe, inner shadow, and a plain white section title centred
 *              over a full-width hairline divider.
 */

#include "Panel.h"
#include "../LookAndFeel.h"

Panel::Panel (const juce::String& panelTitle)
{
    titleText = panelTitle;

    title.setText (panelTitle, juce::dontSendNotification);
    title.setJustificationType (juce::Justification::centred);
    title.setColour (juce::Label::textColourId, juce::Colours::white);
    title.setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    title.setFont (GhostSignalLookAndFeel::getSectionTitleFont (28));

    addAndMakeVisible (title);
}

void Panel::addAndMakeVisibleChild (juce::Component& c)
{
    addAndMakeVisible (c);
}

void Panel::paint (juce::Graphics& g)
{
    const auto bounds    = getLocalBounds().toFloat();
    const float h        = bounds.getHeight();
    const float corner   = 6.0f;
    const float titleH   = static_cast<float> (getTitleAreaHeight());
    const float accentW  = 3.0f;  // left stripe width
    const float grainOpacity = 0.55f;  // analogue speckle strength (0 = off)

    // ── Soft ambient drop shadow ──────────────────────────────────────────────
    // The section sits on the editor background under a soft, offset ambient
    // shadow rather than a hard edge. Several progressively larger and fainter
    // rounded rectangles, painted back-to-front, give a smooth falloff. Black
    // alpha only, so the palette is untouched.
    {
        constexpr int shadowLayers = 5;

        for (int i = shadowLayers; i >= 1; --i)
        {
            const float spread = static_cast<float> (i);
            const float alpha  = 0.13f / static_cast<float> (i);

            g.setColour (juce::Colours::black.withAlpha (alpha));
            g.fillRoundedRectangle (bounds.getX() + 1.0f - spread * 0.5f,
                                    bounds.getY() + 2.0f + spread * 0.5f,
                                    bounds.getWidth() + spread,
                                    bounds.getHeight() + spread,
                                    corner + spread * 0.5f);
        }
    }

    // ── Body background ───────────────────────────────────────────────────────
    // Gentle top-to-bottom gradient across the section body: marginally lighter
    // at the top where the panel meets the chassis, sinking darker toward the
    // base. Subtle enough to be felt rather than seen, but it stops large dark
    // areas reading as one dead flat block.
    {
        juce::ColourGradient bodyGrad (GhostSignalLookAndFeel::panel.brighter (0.045f),
                                       bounds.getX(),
                                       bounds.getY(),
                                       GhostSignalLookAndFeel::panel.darker (0.055f),
                                       bounds.getX(),
                                       bounds.getBottom(),
                                       false);
        g.setGradientFill (bodyGrad);
        g.fillRoundedRectangle (bounds, corner);
    }

    // ── Analogue grain ────────────────────────────────────────────────────────
    // A cached, deterministic speckle tile at very low alpha, clipped to the
    // rounded body so it never bleeds past the panel edge. Gives the surface a
    // faint physical tooth without adding visible patterning.
    {
        g.saveState();

        juce::Path bodyClip;
        bodyClip.addRoundedRectangle (bounds, corner);
        g.reduceClipRegion (bodyClip, {});

        g.setTiledImageFill (GhostSignalLookAndFeel::getPanelGrainTile(), 0, 0, grainOpacity);
        g.fillAll();

        g.restoreState();
    }

    // ── Corner ambient occlusion ──────────────────────────────────────────────
    // Darken the top-left and bottom-right interior so each section reads as a
    // well recessed into the chassis rather than a flat card floating on top.
    // Drawn under the accent stripe and title so foreground detail stays crisp.
    {
        g.saveState();

        juce::Path bodyClip;
        bodyClip.addRoundedRectangle (bounds, corner);
        g.reduceClipRegion (bodyClip, {});

        const float aoR = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.42f;

        {
            juce::ColourGradient ao (juce::Colours::black.withAlpha (0.22f),
                                     bounds.getX(), bounds.getY(),
                                     juce::Colours::transparentBlack,
                                     bounds.getX() + aoR, bounds.getY() + aoR,
                                     true);
            g.setGradientFill (ao);
            g.fillRect (bounds);
        }

        {
            juce::ColourGradient ao (juce::Colours::black.withAlpha (0.18f),
                                     bounds.getRight(), bounds.getBottom(),
                                     juce::Colours::transparentBlack,
                                     bounds.getRight() - aoR, bounds.getBottom() - aoR,
                                     true);
            g.setGradientFill (ao);
            g.fillRect (bounds);
        }

        g.restoreState();
    }

    // ── Left accent stripe ────────────────────────────────────────────────────
    {
        juce::Path stripe;
        stripe.addRoundedRectangle (0.0f, 0.0f, accentW, h,
                                    corner, corner,
                                    true, false, true, false);
        g.setColour (GhostSignalLookAndFeel::accent.withAlpha (0.3f));
        g.fillPath (stripe);
    }

    // ── Title divider ─────────────────────────────────────────────────────────
    // Section titles are plain white text centred across the panel body — no
    // filled tab — with a single full-width hairline underneath acting as a
    // subtle divider. Drawn after the accent stripe so the stroke runs unbroken
    // across the whole section. Skipped entirely for panels without a title.
    if (titleText.isNotEmpty())
    {
        const auto  titleFont   = title.getFont();
        const float textHeight  = titleFont.getHeight();
        const float labelCentre = (float) title.getBounds().getCentreY();

        // Bottom edge of the rendered text block.
        const float textBottom = labelCentre + textHeight * 0.5f;

        // Centre the stroke in the gap between the text and the bottom of the
        // title strip, so it reads as a divider beneath the title rather than an
        // underline hugging the glyphs. Clamped to stay inside the title strip.
        const float lineY = juce::jmin (textBottom + juce::jmax (0.0f, (titleH - textBottom) * 0.5f),
                                        titleH - 1.0f);

        // Full width: both ends line up with the panel's inner border.
        const float inset = 1.5f;

        g.setColour (juce::Colours::white.withAlpha (0.35f));
        g.fillRect (bounds.getX() + inset, lineY,
                    bounds.getWidth() - inset * 2.0f, 1.0f);
    }

    // ── Top edge highlight ────────────────────────────────────────────────────
    g.setColour (juce::Colour (0x20FFFFFF));
    g.drawHorizontalLine (0,
                          bounds.getX() + corner,
                          bounds.getRight() - corner);

    // ── Outer border ──────────────────────────────────────────────────────────
    g.setColour (GhostSignalLookAndFeel::panelBorder);
    g.drawRoundedRectangle (bounds.reduced (0.5f), corner, 1.0f);

    // ── Inner border ────────────────────────────────────────────────────────
    g.setColour (juce::Colour (0x10FFFFFF));
    g.drawRoundedRectangle (bounds.reduced (1.5f), corner - 1.0f, 0.8f);
}

void Panel::resized()
{
    const int titleH = getTitleAreaHeight();

    // Full-width bounds so the label's centred justification centres the text
    // within the section, concentric with the full-width title divider.
    title.setBounds (0, 0, getWidth(), titleH);
}

int Panel::getTitleAreaHeight() const
{
    // Panels without a title have no title bar at all.
    if (titleText.isEmpty())
        return 0;

    // Proportional: slightly taller for larger panels, min 24px
    return juce::jlimit (24, 32, (int) (getHeight() * 0.13f));
}
