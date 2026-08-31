/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Premium graphical logo title component — renders the plugin
 *              name with a drop shadow and the GhostSignalLookAndFeel text
 *              colour palette.
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

    // Subtle drop shadow for depth
    g.setColour (juce::Colours::black.withAlpha (0.4f));
    g.setFont (juce::Font (juce::FontOptions (fontSize, juce::Font::bold)));
    g.drawText (displayName,
                juce::Rectangle<float> (b.getX() + 1.0f, b.getY() + 1.0f, b.getWidth(), b.getHeight()),
                juce::Justification::centredLeft, false);

    // Main text
    g.setColour (textColour);
    g.drawText (displayName, b, juce::Justification::centredLeft, false);
}
