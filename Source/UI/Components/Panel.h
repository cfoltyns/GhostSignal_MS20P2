/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Section panel — recessed dark body with brushed-metal texture,
 *              left accent stripe, proportional title bar.
 */

#pragma once

#include <JuceHeader.h>

class Panel : public juce::Component
{
public:
    explicit Panel (const juce::String& title);
    ~Panel() override = default;

    void addAndMakeVisibleChild (juce::Component& c);

    // Returns the height of the title bar area so callers can offset their
    // content layout below the title.
    int getTitleAreaHeight() const;

    void paint   (juce::Graphics& g) override;
    void resized () override;

private:
    juce::String titleText;
    juce::Label  title;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Panel)
};
