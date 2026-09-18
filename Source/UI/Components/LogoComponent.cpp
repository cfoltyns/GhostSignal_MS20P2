/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Condensed industrial wordmark for the GHOST SIGNAL / GS20
 *              header lockup. The text is shaped as vector glyphs so the
 *              horizontal compression remains crisp at every editor size.
 */

#include "LogoComponent.h"

namespace
{
    juce::String findCondensedTypeface()
    {
        // Cached — the CoreText enumeration behind findAllTypefaceNames() is
        // far too expensive to run on every paint.
        static const juce::String cached = []
        {
            const auto available = juce::Font::findAllTypefaceNames();
            const char* candidates[] =
            {
                "Bahnschrift", "Arial Narrow", "Liberation Sans Narrow",
                "DejaVu Sans Condensed", "Roboto Condensed", "sans-serif"
            };

            for (const auto* candidate : candidates)
            {
                if (available.contains (candidate, true))
                    return juce::String (candidate);
            }
            return juce::String();
        }();

        return cached;
    }
}

LogoComponent::LogoComponent()
{
    setRepaintsOnMouseActivity (true);
}

void LogoComponent::resized()
{
}

void LogoComponent::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    if (bounds.isEmpty())
        return;

    const auto display = displayName.trim().toUpperCase();
    const int split = display.lastIndexOf (juce::String (" "));
    const auto primary = (split > 0 ? display.substring (0, split) : display).trim();
    const auto secondary = (split > 0 ? display.substring (split + 1) : juce::String()).trim();
    if (primary.isEmpty())
        return;

    const auto typeface = findCondensedTypeface();
    const float targetSize = juce::jlimit (10.0f, 30.0f, bounds.getHeight() * 0.44f);

    juce::Font primaryFont (juce::FontOptions (typeface, targetSize, juce::Font::bold));
    primaryFont.setHorizontalScale (0.72f);
    primaryFont.setExtraKerningFactor (-0.02f);

    const float secondarySize = targetSize * 0.56f;
    juce::Font secondaryFont (juce::FontOptions (typeface, secondarySize, juce::Font::bold));
    secondaryFont.setHorizontalScale (0.78f);
    secondaryFont.setExtraKerningFactor (-0.01f);

    const float primaryWidth = juce::GlyphArrangement::getStringWidth (primaryFont, primary);
    const float secondaryWidth = secondary.isEmpty() ? 0.0f
                                                     : juce::GlyphArrangement::getStringWidth (secondaryFont, secondary);
    const float gap = targetSize * 0.14f;
    const float naturalWidth = primaryWidth + gap + secondaryWidth;
    const float fitScale = naturalWidth > 0.0f
                               ? juce::jmin (1.0f, bounds.getWidth() / naturalWidth)
                               : 1.0f;

    const float baseline = bounds.getY() + bounds.getHeight() * 0.70f;
    const float primaryX = bounds.getX();
    const float secondaryX = primaryX + primaryWidth * fitScale + gap * fitScale;

    juce::GlyphArrangement primaryGlyphs;
    primaryGlyphs.addLineOfText (primaryFont, primary, primaryX, baseline);
    primaryGlyphs.stretchRangeOfGlyphs (0, -1, fitScale);

    juce::GlyphArrangement secondaryGlyphs;
    if (secondaryWidth > 0.0f)
    {
        secondaryGlyphs.addLineOfText (secondaryFont, secondary, secondaryX, baseline);
        secondaryGlyphs.stretchRangeOfGlyphs (0, -1, fitScale);
    }

    // A soft technical shadow keeps the lockup legible over the panel grain.
    g.setColour (juce::Colours::black.withAlpha (0.38f));
    primaryGlyphs.draw (g, juce::AffineTransform::translation (1.0f, 1.0f));
    secondaryGlyphs.draw (g, juce::AffineTransform::translation (1.0f, 1.0f));

    g.setColour (textColour);
    primaryGlyphs.draw (g);
    g.setColour (juce::Colours::white);
    secondaryGlyphs.draw (g);

    // Small signal trace / LED accent: restrained, but enough to identify the
    // GS20 as a technical instrument rather than a plain text label.
    const float traceWidth = juce::jmin (bounds.getWidth() * 0.13f, 34.0f);
    const float traceY = bounds.getBottom() - juce::jmax (2.0f, bounds.getHeight() * 0.10f);
    g.setColour (juce::Colours::white.withAlpha (0.75f));
    g.fillRect (primaryX, traceY, traceWidth, 1.5f);
    g.fillEllipse (primaryX + traceWidth - 2.0f, traceY - 1.5f, 4.0f, 4.0f);
}
