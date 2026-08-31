/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Graphical logo title component — renders the "GS" monogram
 *              with a circuit-trace visual effect, LED accents, and gradient
 *              styling. Replaces the plain text title label.
 */

#pragma once

#include <JuceHeader.h>

class LogoComponent : public juce::Component
{
public:
    LogoComponent();
    ~LogoComponent() override = default;

    void setText (const juce::String& newText) { displayName = newText; repaint(); }
    const juce::String& getText() const { return displayName; }

    void setTextColour (juce::Colour c) { textColour = c; repaint(); }

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    juce::String displayName { "GHOST SIGNAL  MS20P" };
    juce::Colour textColour { juce::Colour (0xFFE0E0E0) };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LogoComponent)
};
