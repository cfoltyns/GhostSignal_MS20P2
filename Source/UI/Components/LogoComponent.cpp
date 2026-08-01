/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Simple text title component — renders the brand name
 *              as clean left-aligned text in the header bar.
 */

#include "LogoComponent.h"

LogoComponent::LogoComponent()
{
    setRepaintsOnMouseActivity (true);
}

void LogoComponent::resized()
{
}

void LogoComponent::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();
    const float h = b.getHeight();

    const float fontSize = juce::jlimit (12.0f, 28.0f, h * 0.42f);

    g.setColour (juce::Colours::black.withAlpha (0.4f));
    g.setFont (juce::Font (fontSize, juce::Font::bold));
    g.drawText (displayName,
                juce::Rectangle<float> (b.getX() + 1.0f, b.getY() + 1.0f, b.getWidth(), b.getHeight()),
                juce::Justification::centredLeft, false);

    g.setColour (textColour);
    g.drawText (displayName, b, juce::Justification::centredLeft, false);

    const float badgeR = h * 0.28f;
    const float badgeX = b.getRight() - badgeR * 2.2f;
    const float badgeY = b.getY() + h * 0.12f;

    g.setColour (juce::Colour (0xFFDB4437));
    g.fillEllipse (badgeX, badgeY, badgeR * 2.0f, badgeR * 2.0f);

    g.setColour (juce::Colours::black);
    const float boltW = badgeR * 1.2f;
    const float boltH = badgeR * 1.4f;
    const float bx = badgeX + badgeR - boltW * 0.5f;
    const float by = badgeY + badgeR - boltH * 0.5f;

    juce::Path bolt;
    bolt.startNewSubPath (bx + boltW * 0.15f, by);
    bolt.lineTo (bx + boltW * 0.55f, by);
    bolt.lineTo (bx + boltW * 0.45f, by + boltH * 0.38f);
    bolt.lineTo (bx + boltW * 0.85f, by + boltH * 0.38f);
    bolt.lineTo (bx + boltW * 0.25f, by + boltH);
    bolt.lineTo (bx + boltW * 0.35f, by + boltH * 0.62f);
    bolt.lineTo (bx + boltW * 0.05f, by + boltH * 0.62f);
    bolt.closeSubPath();
    g.fillPath (bolt);
}