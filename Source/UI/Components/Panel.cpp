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

    // ── Outer shadow (drop shadow for depth) ────────────────────────────────────
    {
        g.setColour (GhostSignalLookAndFeel::panelShadow);
        g.fillRoundedRectangle (bounds.getX() + 1.0f, bounds.getY() + 2.0f,
                                bounds.getWidth(), bounds.getHeight(), corner);
    }

    // ── Body background ───────────────────────────────────────────────────────
    // Flat, uniform grey — deliberately no gradient, scan lines, stripes, noise
    // or any other texture, so each section reads as a clean solid panel.
    {
        g.setColour (GhostSignalLookAndFeel::panel);
        g.fillRoundedRectangle (bounds, corner);
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
