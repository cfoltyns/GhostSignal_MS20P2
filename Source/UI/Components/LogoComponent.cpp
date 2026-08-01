/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Premium graphical logo title component — renders the "GS" monogram
 *              with a circuit-trace visual effect, LED accents, and gradient
 *              styling. Uses the GhostSignalLookAndFeel color palette.
 */

#include "LogoComponent.h"
#include "../LookAndFeel.h"

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

    // Subtle drop shadow for depth
    g.setColour (juce::Colours::black.withAlpha (0.4f));
    g.setFont (juce::Font (fontSize, juce::Font::bold));
    g.drawText (displayName,
                juce::Rectangle<float> (b.getX() + 1.0f, b.getY() + 1.0f, b.getWidth(), b.getHeight()),
                juce::Justification::centredLeft, false);

    // Main text
    g.setColour (textColour);
    g.drawText (displayName, b, juce::Justification::centredLeft, false);

    // Accent badge — premium circular LED-style indicator
    const float badgeR = h * 0.28f;
    const float badgeX = b.getRight() - badgeR * 2.2f;
    const float badgeY = b.getY() + h * 0.12f;

    // Badge glow
    g.setColour (GhostSignalLookAndFeel::accent.withAlpha (0.3f));
    g.fillEllipse (badgeX - badgeR * 0.3f, badgeY - badgeR * 0.3f,
                   badgeR * 2.6f, badgeR * 2.6f);

    // Badge body
    juce::ColourGradient badgeGrad;
    badgeGrad.point1 = { badgeX, badgeY };
    badgeGrad.point2 = { badgeX + badgeR * 2.0f, badgeY + badgeR * 2.0f };
    badgeGrad.addColour (0.0f, GhostSignalLookAndFeel::accent);
    badgeGrad.addColour (1.0f, GhostSignalLookAndFeel::accentDark);
    g.setGradientFill (badgeGrad);
    g.fillEllipse (badgeX, badgeY, badgeR * 2.0f, badgeR * 2.0f);

    // Inner highlight
    g.setColour (juce::Colours::white.withAlpha (0.3f));
    g.drawEllipse (badgeX + 0.5f, badgeY + 0.5f, badgeR * 2.0f - 1.0f, badgeR * 2.0f - 1.0f, 0.5f);

    // Lightning bolt icon
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

    g.setColour (juce::Colours::white);
    g.fillPath (bolt);
}
