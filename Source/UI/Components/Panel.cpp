/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Industrial hardware panel — recessed dark body, brushed-metal
 *              scan-line texture, left accent stripe, larger section title.
 */

#include "Panel.h"

Panel::Panel (const juce::String& panelTitle)
{
    titleText = panelTitle;

    title.setText (panelTitle, juce::dontSendNotification);
    title.setJustificationType (juce::Justification::centredLeft);
    title.setColour (juce::Label::textColourId, juce::Colour (0xFFD8D8D8));
    title.setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    title.setFont (juce::Font (12.0f, juce::Font::bold));

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
    const float corner   = 5.0f;
    const float titleH   = static_cast<float> (getTitleAreaHeight());
    const float accentW  = 2.0f;  // left stripe width

    // ── Body background ───────────────────────────────────────────────────────
    {
        juce::ColourGradient bg;
        bg.point1 = { 0.0f, 0.0f };
        bg.point2 = { 0.0f, h };
        bg.addColour (0.00f, juce::Colour (0xFF1E1E1E));
        bg.addColour (0.50f, juce::Colour (0xFF181818));
        bg.addColour (1.00f, juce::Colour (0xFF141414));
        g.setGradientFill (bg);
        g.fillRoundedRectangle (bounds, corner);
    }

    // ── Brushed-metal scan-line texture ───────────────────────────────────────
    // Very subtle horizontal lines every 3px — simulates brushed aluminium grain
    {
        g.saveState();
        g.reduceClipRegion (bounds.reduced (0.5f).toNearestInt());
        g.setColour (juce::Colour (0x06FFFFFF));  // ~2.4% white
        for (float lineY = 0.0f; lineY < h; lineY += 3.0f)
            g.drawHorizontalLine (static_cast<int> (bounds.getY() + lineY),
                                  bounds.getX(), bounds.getRight());
        g.restoreState();
    }

    // ── Title bar strip ───────────────────────────────────────────────────────
    {
        juce::Path titlePath;
        titlePath.addRoundedRectangle (0.0f, 0.0f, w, titleH,
                                       corner, corner,
                                       true, true, false, false);
        juce::ColourGradient titleBg;
        titleBg.point1 = { 0.0f, 0.0f };
        titleBg.point2 = { 0.0f, titleH };
        titleBg.addColour (0.0f, juce::Colour (0xFF2A2A2A));
        titleBg.addColour (1.0f, juce::Colour (0xFF1E1E1E));
        g.setGradientFill (titleBg);
        g.fillPath (titlePath);
    }

    // Title bar bottom divider
    g.setColour (juce::Colour (0xFF111111));
    g.drawHorizontalLine (static_cast<int> (titleH),
                          accentW, w);

    // ── Left accent stripe ────────────────────────────────────────────────────
    // White at 25% alpha — visual anchor, like a chassis rail
    {
        juce::Path stripe;
        stripe.addRoundedRectangle (0.0f, 0.0f, accentW, h,
                                    corner, corner,
                                    true, false, true, false);
        g.setColour (juce::Colour (0x40FFFFFF));
        g.fillPath (stripe);
    }

    // ── Top edge highlight ────────────────────────────────────────────────────
    g.setColour (juce::Colour (0x30FFFFFF));
    g.drawHorizontalLine (0,
                          bounds.getX() + corner,
                          bounds.getRight() - corner);

    // ── Outer border ─────────────────────────────────────────────────────────
    // Single pixel, dark — creates slight inset / shadow impression
    g.setColour (juce::Colour (0xFF080808));
    g.drawRoundedRectangle (bounds.reduced (0.5f), corner, 1.0f);

    // ── Inner border ─────────────────────────────────────────────────────────
    // Very subtle lighter line just inside the outer border
    g.setColour (juce::Colour (0x18FFFFFF));
    g.drawRoundedRectangle (bounds.reduced (1.5f), corner - 1.0f, 0.8f);
}

void Panel::resized()
{
    const int titleH  = getTitleAreaHeight();
    const int titlePadLeft = 8;   // left padding: leaves room for accent stripe

    title.setBounds (titlePadLeft,
                     0,
                     getWidth() - titlePadLeft - 4,
                     titleH);
}

int Panel::getTitleAreaHeight() const
{
    // Proportional: slightly taller for larger panels, min 22px
    return juce::jlimit (22, 28, (int) (getHeight() * 0.13f));
}