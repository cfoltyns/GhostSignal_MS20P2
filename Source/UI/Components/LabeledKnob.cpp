/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Responsive rotary knob with proportional label.
 *              Supports optional text value labels at specific positions.
 */

#include "LabeledKnob.h"

LabeledKnob::LabeledKnob (const juce::String& labelText)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    addAndMakeVisible (slider);

    label.setText (labelText, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId,       juce::Colour (0xFFCCCCCC));
    label.setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    label.setFont (juce::Font (12.0f, juce::Font::bold));
    addAndMakeVisible (label);

    slider.addListener (this);
}

void LabeledKnob::setLabel (const juce::String& text)
{
    label.setText (text, juce::dontSendNotification);
}

void LabeledKnob::setTextValues (const juce::StringArray& texts, const float* values, int numValues)
{
    textValues = texts;
    numTextValues = juce::jmin (numValues, 8);
    for (int i = 0; i < numTextValues; ++i)
        textValuePositions[i] = values[i];
    showTextValues = (numTextValues > 0);
}

void LabeledKnob::clearTextValues()
{
    textValues.clear();
    numTextValues = 0;
    showTextValues = false;
}

void LabeledKnob::setSnapToValues (const float* values, int numValues)
{
    numSnapValues = juce::jmin (numValues, 8);
    for (int i = 0; i < numSnapValues; ++i)
        snapValues[i] = values[i];

    if (numSnapValues >= 2)
    {
        slider.setRange (snapValues[0], snapValues[numSnapValues - 1], 1.0);
    }
}

void LabeledKnob::paint (juce::Graphics& g)
{
    if (showTextValues && numTextValues > 0)
    {
        const juce::Rectangle<int> sliderBounds = slider.getBounds();
        const float centreX = (float) sliderBounds.getCentreX();
        const float centreY = (float) sliderBounds.getCentreY();
        const float radiusOuter = (float) sliderBounds.getWidth() * 0.48f;
        const float radiusInner = (float) sliderBounds.getWidth() * 0.32f;

        const float minV = (float) slider.getMinimum();
        const float maxV = (float) slider.getMaximum();

        // Match the rotary arc used in PluginEditor: 1.25π to 2.75π (270° sweep)
        const float rotaryStart = juce::MathConstants<float>::pi * 1.25f;
        const float rotaryEnd   = juce::MathConstants<float>::pi * 2.75f;

        for (int i = 0; i < numTextValues; ++i)
        {
            const float val = textValuePositions[i];
            if (maxV <= minV)
                continue;

            const float norm = juce::jlimit (0.0f, 1.0f, (val - minV) / (maxV - minV));
            // Map normalised value to rotary angle range (clockwise from bottom-left)
            const float angle = rotaryStart + norm * (rotaryEnd - rotaryStart);

            // Draw a small tick mark at each label position
            const float tickX1 = centreX + std::cos (angle) * radiusInner;
            const float tickY1 = centreY + std::sin (angle) * radiusInner;
            const float tickX2 = centreX + std::cos (angle) * radiusOuter;
            const float tickY2 = centreY + std::sin (angle) * radiusOuter;
            g.setColour (juce::Colour (0x66FFFFFF));
            g.drawLine (tickX1, tickY1, tickX2, tickY2, 1.2f);

            // Position the label slightly outside the arc
            const float labelRadius = radiusOuter + 14.0f;
            const float textX = centreX + std::cos (angle) * labelRadius;
            const float textY = centreY + std::sin (angle) * labelRadius;

            const float textW = 32.0f;
            const float textH = 16.0f;

            // Background pill for readability
            g.setColour (juce::Colour (0xBB000000));
            g.fillRoundedRectangle (textX - textW * 0.5f - 1.0f,
                                    textY - textH * 0.5f - 1.0f,
                                    textW + 2.0f, textH + 2.0f, 3.0f);

            g.setColour (juce::Colour (0xFFDDDDDD));
            g.setFont (juce::Font ((float) juce::jmax (8, sliderBounds.getWidth() / 9), juce::Font::bold));
            g.drawText (textValues[i],
                        juce::Rectangle<float> (textX - textW * 0.5f, textY - textH * 0.5f, textW, textH),
                        juce::Justification::centred, false);
        }
    }

    // Draw center text (if set) - used for mode/value display in the knob center
    if (centerText.isNotEmpty())
    {
        const juce::Rectangle<int> sliderBounds = slider.getBounds();
        const float cx = (float) sliderBounds.getCentreX();
        const float cy = (float) sliderBounds.getCentreY();
        const float fontSize = juce::jlimit (8.0f, 14.0f, sliderBounds.getWidth() * 0.12f);

        // Background pill
        const float textW = fontSize * centerText.length() * 0.7f;
        const float textH = fontSize * 1.3f;
        g.setColour (juce::Colour (0xBB000000));
        g.fillRoundedRectangle (cx - textW * 0.5f - 2.0f, cy - textH * 0.5f - 1.0f,
                                textW + 4.0f, textH + 2.0f, 3.0f);
        g.setColour (juce::Colour (0xFFDB4437));
        g.setFont (juce::Font (fontSize, juce::Font::bold));
        g.drawText (centerText,
                    juce::Rectangle<float> (cx - textW * 0.5f, cy - textH * 0.5f, textW, textH),
                    juce::Justification::centred, false);
    }
    // Draw amber LED indicator in the center of the knob (only if no center text)
    else if (ledActive)
    {
        const juce::Rectangle<int> sliderBounds = slider.getBounds();
        const float cx = (float) sliderBounds.getCentreX();
        const float cy = (float) sliderBounds.getCentreY();
        const float ledRadius = 3.0f;

        g.setColour (juce::Colour (0xFFDB4437));
        g.fillEllipse (cx - ledRadius, cy - ledRadius, ledRadius * 2.0f, ledRadius * 2.0f);

        // Subtle glow
        g.setColour (juce::Colour (0x40DB4437));
        g.fillEllipse (cx - ledRadius * 2.0f, cy - ledRadius * 2.0f, ledRadius * 4.0f, ledRadius * 4.0f);
    }

    // Draw modulation indicator dot (if active) — shows where an LFO is
    // currently pushing the knob value, on the rotary arc perimeter.
    if (showModIndicator)
    {
        const juce::Rectangle<int> sliderBounds = slider.getBounds();
        const float cx = (float) sliderBounds.getCentreX();
        const float cy = (float) sliderBounds.getCentreY();
        const float radius = (float) sliderBounds.getWidth() * 0.43f;

        const float rotaryStart = juce::MathConstants<float>::pi * 1.25f;
        const float rotaryEnd   = juce::MathConstants<float>::pi * 2.75f;
        const float angle = rotaryStart + juce::jlimit (0.0f, 1.0f, modulationValue)
                              * (rotaryEnd - rotaryStart);

        const float dotX = cx + std::cos (angle) * radius;
        const float dotY = cy + std::sin (angle) * radius;

        g.setColour (juce::Colour (0x80DB4437));
        g.fillEllipse (dotX - 2.5f, dotY - 2.5f, 5.0f, 5.0f);
    }
}

void LabeledKnob::resized()
{
    const int totalH = getHeight();
    const int totalW = getWidth();

    const int labelH = juce::jmax (16, (int) (totalH * labelHeightProportion));
    const int gap = juce::jmax (2, (int) (totalH * 0.04f));
    const int knobAreaH = totalH - labelH - gap;

    const int knobSize = juce::jmax (48, juce::jmin (totalW, knobAreaH));
    const int knobX    = (totalW - knobSize) / 2;
    const int knobY    = (knobAreaH - knobSize) / 2;

    slider.setBounds (knobX, knobY, knobSize, knobSize);

    const int labelY = knobAreaH + gap;
    label.setBounds (0, labelY, totalW, labelH);

    const float fontSize = juce::jlimit (10.0f, 15.0f, (float) totalH * 0.17f);
    label.setFont (juce::Font (fontSize, juce::Font::bold));
}

void LabeledKnob::sliderValueChanged (juce::Slider*)
{
    updateCenterTextFromValue();
}

void LabeledKnob::updateCenterTextFromValue()
{
    if (autoCenterText && showTextValues && numTextValues > 0)
    {
        const float val = (float) slider.getValue();
        int closestIdx = 0;
        float minDiff = std::abs (val - textValuePositions[0]);
        for (int i = 1; i < numTextValues; ++i)
        {
            const float diff = std::abs (val - textValuePositions[i]);
            if (diff < minDiff)
            {
                minDiff = diff;
                closestIdx = i;
            }
        }
        setCenterText (textValues[closestIdx]);
    }
}
