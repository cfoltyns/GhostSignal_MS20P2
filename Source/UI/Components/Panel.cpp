/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Premium industrial panel — recessed dark body with brushed-metal
 *              scan-line texture, left accent stripe, inner shadow, and
 *              proportional title bar.
 */

#include "Panel.h"
#include "../LookAndFeel.h"

Panel::Panel (const juce::String& panelTitle)
{
    titleText = panelTitle;

    title.setText (panelTitle, juce::dontSendNotification);
    title.setJustificationType (juce::Justification::centredLeft);
    title.setColour (juce::Label::textColourId, GhostSignalLookAndFeel::textPrimary);
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
    const float w        = bounds.getWidth();
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
    {
        juce::ColourGradient bg;
        bg.point1 = { 0.0f, 0.0f };
        bg.point2 = { 0.0f, h };
        bg.addColour (0.00f, GhostSignalLookAndFeel::panel);
        bg.addColour (0.50f, GhostSignalLookAndFeel::panel.darker (0.03f));
        bg.addColour (1.00f, GhostSignalLookAndFeel::panel.darker (0.06f));
        g.setGradientFill (bg);
        g.fillRoundedRectangle (bounds, corner);
    }

    // ── Brushed-metal scan-line texture ────────────────────────────────────────
    {
        g.saveState();
        g.reduceClipRegion (bounds.reduced (0.5f).toNearestInt());
        g.setColour (juce::Colour (0x06FFFFFF));  // ~2.4% white
        for (float lineY = 0.0f; lineY < h; lineY += 3.0f)
            g.drawHorizontalLine (static_cast<int> (bounds.getY() + lineY),
                                  bounds.getX(), bounds.getRight());
        g.restoreState();
    }

    // ── Title bar strip ────────────────────────────────────────────────────────
    // Skipped entirely for panels without a title (titleText empty).
    if (titleText.isNotEmpty())
    {
        juce::Path titlePath;
        titlePath.addRoundedRectangle (0.0f, 0.0f, w, titleH,
                                       corner, corner,
                                       true, true, false, false);
        g.setColour (GhostSignalLookAndFeel::accent);
        g.fillPath (titlePath);

        // Title bar bottom divider
        g.setColour (juce::Colour (0xFF080808));
        g.drawHorizontalLine (static_cast<int> (titleH),
                              accentW, w);
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
    const int titleH  = getTitleAreaHeight();
    const int titlePadLeft = 10;   // left padding: leaves room for accent stripe

    title.setBounds (titlePadLeft,
                     0,
                     getWidth() - titlePadLeft - 4,
                     titleH);
}

int Panel::getTitleAreaHeight() const
{
    // Panels without a title have no title bar at all.
    if (titleText.isEmpty())
        return 0;

    // Proportional: slightly taller for larger panels, min 24px
    return juce::jlimit (24, 32, (int) (getHeight() * 0.13f));
}
