/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Shared styling for the preset surfaces (the preset browser
 *              panel and the save/rename dialog). The look is deliberately
 *              minimal: a flat near-black surface, one-pixel hairlines, square
 *              corners, monospace type and lowercase labels. There is no accent
 *              colour at all - state is communicated through brightness - so the
 *              preset UI never competes with the instrument behind it.
 *
 *              Every definition is `inline` or a namespace-scope `const`, so
 *              including this header from several translation units is safe.
 */

#pragma once

#include <JuceHeader.h>

namespace MinimalStyle
{
    // ── Palette ──────────────────────────────────────────────────────────────
    //   surface   #0A0A0C   flat, untextured near-black
    //   hairline  white 4%  one-pixel rules only
    //   text      #E3E3E6   primary / selected row
    //   row text  #A8A8AE   list rows
    //   muted     #78787F   secondary
    //   faint     #4E4E55   tertiary and disabled
    const juce::Colour kSurfaceColour   (0xff0a0a0c);
    const juce::Colour kHairlineColour  (0x0affffff);
    const juce::Colour kTextColour      (0xffe3e3e6);
    const juce::Colour kRowTextColour   (0xffa8a8ae);
    const juce::Colour kMutedColour     (0xff78787f);
    const juce::Colour kFaintColour     (0xff4e4e55);
    const juce::Colour kRowSelectedFill (0x0fffffff);
    const juce::Colour kHoverFill       (0x14ffffff);

    // ─── Metrics ─────────────────────────────────────────────────────────────
    // Tight outer padding and a short, single-line row. Whitespace does the
    // grouping work that borders and fills would otherwise have to.
    constexpr int kPad       = 14;
    constexpr int kRowHeight = 22;
    constexpr int kActionGap = 12;

    // ─ Typography ──────────────────────────────────────────────────────────
    // The preset UI reads as a terminal surface, so all of its chrome uses a
    // monospace face rather than the industrial headings used elsewhere in the
    // plugin. Falls back through the faces that ship with Windows, macOS and
    // Linux.
    inline juce::String findMonospaceTypeface()
    {
        const auto available = juce::Font::findAllTypefaceNames();
        const char* candidates[] =
        {
            "JetBrains Mono", "Cascadia Mono", "Cascadia Code", "Consolas",
            "DejaVu Sans Mono", "Liberation Mono", "Menlo", "Monaco",
            "Lucida Console", "Courier New"
        };

        for (const auto* candidate : candidates)
        {
            if (available.contains (candidate, true))
                return candidate;
        }
        return juce::String();
    }

    inline juce::Font getMonoFont (float size, bool bold = false)
    {
        const auto typeface = findMonospaceTypeface();
        const int flags = bold ? juce::Font::bold : juce::Font::plain;

        if (typeface.isNotEmpty())
            return juce::Font (juce::FontOptions (typeface, size, flags));

        return juce::Font (juce::FontOptions (size, flags));
    }

    // Slight positive tracking keeps short lowercase label text legible at the
    // small sizes this UI uses.
    inline juce::Font getTrackedMonoFont (float size)
    {
        auto font = getMonoFont (size);
        font.setExtraKerningFactor (0.08f);
        return font;
    }

    // ─── Text-only button rendering ───────────────────────────────────────────
    // No fills, no borders, no rounded corners: the label alone is the button.
    // Hover and press are expressed only as a faint wash behind the text and a
    // lift in text brightness, so an action row stays monochrome.
    class FlatTextButtonLookAndFeel final : public juce::LookAndFeel_V4
    {
    public:
        void drawButtonBackground (juce::Graphics& g,
                                   juce::Button& button,
                                   const juce::Colour&,
                                   bool shouldDrawButtonAsHighlighted,
                                   bool shouldDrawButtonAsDown) override
        {
            if (! button.isEnabled() || (! shouldDrawButtonAsHighlighted && ! shouldDrawButtonAsDown))
                return;

            g.setColour (shouldDrawButtonAsDown ? kHoverFill : kRowSelectedFill);
            g.fillRect (button.getLocalBounds().toFloat());
        }

        void drawButtonText (juce::Graphics& g,
                             juce::TextButton& button,
                             bool shouldDrawButtonAsHighlighted,
                             bool shouldDrawButtonAsDown) override
        {
            // Dim when idle, bright on hover or press, faint when unavailable.
            const auto colour = ! button.isEnabled() ? kFaintColour
                                                    : (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown
                                                           ? kTextColour
                                                           : kMutedColour);

            g.setColour (colour);
            g.setFont (getTrackedMonoFont (10.0f));
            g.drawFittedText (button.getButtonText().toLowerCase(),
                              button.getLocalBounds(),
                              juce::Justification::centred,
                              1);
        }
    };

    // ─── Bare text editor ─────────────────────────────────────────────────────
    // No box, no border, no fill: a hairline rule is drawn under the bounds by
    // the owning component, so the field reads as an input line on the surface.
    inline void styleTextEditor (juce::TextEditor& editor, float fontSize)
    {
        editor.setFont (getMonoFont (fontSize));
        editor.setColour (juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
        editor.setColour (juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
        editor.setColour (juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
        editor.setColour (juce::TextEditor::shadowColourId, juce::Colours::transparentBlack);
        editor.setColour (juce::TextEditor::textColourId, kTextColour);
        editor.setColour (juce::TextEditor::highlightColourId, kRowSelectedFill);
        editor.setJustification (juce::Justification::centredLeft);
        editor.setIndents (0, 0);
        editor.setBorder (juce::BorderSize<int> (0));
    }

    // Paints the flat surface plus a hairline rule under each supplied
    // component. Components with empty bounds are skipped, so this is safe to
    // call before the first resized().
    inline void paintSurface (juce::Graphics& g,
                              juce::Rectangle<int> bounds,
                              const std::vector<juce::Component*>& ruled)
    {
        g.fillAll (kSurfaceColour);
        g.setColour (kHairlineColour);
        g.drawRect (bounds, 1);

        for (auto* component : ruled)
        {
            const auto b = component->getBounds();
            if (b.isEmpty())
                continue;

            g.fillRect (b.getX(), b.getBottom(), b.getWidth(), 1);
        }
    }
}