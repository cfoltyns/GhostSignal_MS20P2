/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Premium rotary waveform selector knob — snaps between waveform
 *              positions, draws the selected waveform name in the knob centre.
 *              Optionally draws waveform icons instead of text when requested.
 */

#include "WaveformKnob.h"
#include "../LookAndFeel.h"
#include "../../Core/Parameters.h"
#include <random>

WaveformKnob::WaveformKnob (const juce::String& labelText)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    addAndMakeVisible (slider);

    label.setText (labelText, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId,       GhostSignalLookAndFeel::textSecondary);
    label.setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    label.setFont (GhostSignalLookAndFeel::getKnobLabelFont (60));
    addAndMakeVisible (label);

    // Set range to 0-1 for normalized choice parameter values
    slider.setRange (0.0, 1.0, 0.001);

    // Center text overlay: added AFTER the slider so it paints on top of the knob body.
    centerLabel.setJustificationType (juce::Justification::centred);
    centerLabel.setEditable (false);
    centerLabel.setInterceptsMouseClicks (false, false);
    centerLabel.setColour (juce::Label::textColourId, GhostSignalLookAndFeel::textPrimary);
    centerLabel.setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    centerLabel.setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
    centerLabel.setVisible (false);
    addAndMakeVisible (centerLabel);

    // Add listener to trigger callback when waveform changes
    slider.onValueChange = [this]()
    {
        if (autoCenterText)
            updateCenterTextFromValue();

        if (onWaveformChanged)
            onWaveformChanged();
    };
}

void WaveformKnob::setLabel (const juce::String& text)
{
    label.setText (text, juce::dontSendNotification);
}

void WaveformKnob::setSnapToValues (const float* values, int numValues)
{
    numSnapValues = juce::jmin (numValues, 8);
    for (int i = 0; i < numSnapValues; ++i)
        snapValues[i] = values[i];

    if (numSnapValues >= 2)
        slider.setRange (snapValues[0], snapValues[numSnapValues - 1], 0.001);
}

void WaveformKnob::setCentreValueFormatter (CentreValueFormatter fmt)
{
    slider.setCentreValueFormatter (std::move (fmt));
    repaint();
}

void WaveformKnob::setTextValues (const juce::StringArray& texts, const float* values, int numValues)
{
    textValues = texts;
    numTextValues = juce::jmin (numValues, 8);
    for (int i = 0; i < numTextValues; ++i)
        textValuePositions[i] = values[i];
    showTextValues = (numTextValues > 0);
}

void WaveformKnob::clearTextValues()
{
    textValues.clear();
    numTextValues = 0;
    showTextValues = false;
    centerLabel.setVisible (false);
}

void WaveformKnob::setCenterText (const juce::String& text)
{
    centerText = text;
    centerLabel.setText (text, juce::dontSendNotification);
    centerLabel.setVisible (text.isNotEmpty());
    slider.getProperties().set ("hideCenterValue", text.isNotEmpty());
    repaint();
}

void WaveformKnob::clearCenterText()
{
    centerText = {};
    centerLabel.setText ({}, juce::dontSendNotification);
    centerLabel.setVisible (false);
    slider.getProperties().set ("hideCenterValue", false);
    repaint();
}

void WaveformKnob::updateCenterTextFromValue()
{
    if (showTextValues && numTextValues > 0)
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

int WaveformKnob::getWaveformIndexFromValue (double value)
{
    const int numChoices = (int) Parameters::oscWaveformChoices.size();
    int idx = juce::jlimit (0, numChoices - 1, (int) juce::roundToInt (value * numChoices));
    return idx;
}

void WaveformKnob::paint (juce::Graphics&)
{
    // WaveformKnob delegates rendering to the WaveformSlider
}

void WaveformKnob::resized()
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

    // Centre the text overlay over the knob body
    const int centerH = juce::jmax (12, (int) (knobSize * 0.24f));
    centerLabel.setBounds (knobX + 2, knobY + (knobSize - centerH) / 2, knobSize - 4, centerH);
    centerLabel.setFont (juce::Font (juce::FontOptions (
        juce::jlimit (9.0f, 16.0f, (float) knobSize * 0.18f), juce::Font::bold)));

    const int labelY = knobAreaH + gap;
    label.setBounds (0, labelY, totalW, labelH);
    label.setFont (GhostSignalLookAndFeel::getKnobLabelFont (knobSize));
}

// ─────────────────────────────────────────────────────────────────────────────
// WaveformSlider — fully custom drawn so icons are always visible
// ─────────────────────────────────────────────────────────────────────────────

WaveformKnob::WaveformSlider::WaveformSlider()
{
    setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    setRotaryParameters (juce::MathConstants<float>::pi * 0.75f,
                         juce::MathConstants<float>::pi * 2.25f,
                         true);
    setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    setMouseDragSensitivity (80);
}

void WaveformKnob::WaveformSlider::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const float cx = bounds.getCentreX();
    const float cy = bounds.getCentreY();
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;

    const bool enabled  = isEnabled();
    const bool hovered  = enabled && isMouseOver();
    const bool dragging = enabled && isMouseButtonDown();

    // Shared industrial knob body — identical layers to every other knob in the
    // plugin (drop shadow, flange, cylindrical side wall, grip grooves, top).
    GhostSignalLookAndFeel::drawIndustrialKnobBody (g, { cx, cy }, radius, enabled, hovered, dragging);

    // Rotary value arc just outside the body (matches the LookAndFeel geometry)
    const float rotaryStart = juce::MathConstants<float>::pi * 0.75f;
    const float rotaryEnd   = juce::MathConstants<float>::pi * 2.25f;
    const float sliderAngle = rotaryStart + (float) getValue() * (rotaryEnd - rotaryStart);

    if (enabled && (float) getValue() > 0.001f)
    {
        const float arcR = radius * 0.93f;
        juce::Path arc;
        arc.addCentredArc (cx, cy, arcR, arcR, 0.0f, rotaryStart, sliderAngle, true);
        g.setColour (dragging ? GhostSignalLookAndFeel::accent.brighter (0.22f)
                              : GhostSignalLookAndFeel::accent);
        g.strokePath (arc, juce::PathStrokeType (juce::jlimit (2.0f, 4.0f, radius * 0.09f),
                                                 juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
    }

    // Shared industrial centre cap (waveform icon drawn on top)
    GhostSignalLookAndFeel::drawIndustrialKnobCap (g, { cx, cy }, radius * 0.20f, enabled);

    // Draw waveform icon in the centre if enabled
    if (showWaveformIcon)
    {
        juce::Rectangle<float> iconArea (cx - radius * 0.35f, cy - radius * 0.35f,
                                         radius * 0.70f, radius * 0.70f);
        const int waveIndex = getWaveformIndexFromValue (getValue());
        drawWaveformIcon (g, iconArea, waveIndex);
    }
}

void WaveformKnob::WaveformSlider::drawWaveformIcon (juce::Graphics& g,
                                                     juce::Rectangle<float> area,
                                                     int waveIndex) const
{
    g.setColour (GhostSignalLookAndFeel::textPrimary);

    const float x0 = area.getX();
    const float x1 = area.getRight();
    const float y0 = area.getY();
    const float y1 = area.getBottom();
    const float midY = (y0 + y1) * 0.5f;
    const float amp = (y1 - y0) * 0.40f;

    juce::Path p;
    const int segments = 32;

    auto addSample = [&](float x, float y)
    {
        if (p.isEmpty())
            p.startNewSubPath (x, y);
        else
            p.lineTo (x, y);
    };

    switch (waveIndex)
    {
        case 0: // Triangle
        {
            p.startNewSubPath (x0, midY + amp);
            p.lineTo (x0 + (x1 - x0) * 0.5f, midY - amp);
            p.lineTo (x1, midY + amp);
            break;
        }
        case 1: // Saw
        {
            for (int i = 0; i <= segments; ++i)
            {
                const float x = x0 + (x1 - x0) * i / (float) segments;
                const float phase = (x - x0) / (x1 - x0);
                const float saw = 2.0f * phase - 1.0f;
                addSample (x, midY - saw * amp);
            }
            break;
        }
        case 2: // Square
        {
            const float halfW = (x1 - x0) * 0.5f;
            p.startNewSubPath (x0, midY + amp);
            p.lineTo (x0 + halfW, midY + amp);
            p.lineTo (x0 + halfW, midY - amp);
            p.lineTo (x1, midY - amp);
            break;
        }
        case 3: // Pulse
        {
            const float duty = 0.3f;
            const float pulseW = (x1 - x0) * duty;
            p.startNewSubPath (x0, midY + amp);
            p.lineTo (x0 + pulseW, midY + amp);
            p.lineTo (x0 + pulseW, midY - amp);
            p.lineTo (x1, midY - amp);
            break;
        }
        case 4: // Sine
        {
            for (int i = 0; i <= segments; ++i)
            {
                const float x = x0 + (x1 - x0) * i / (float) segments;
                const float phase = (x - x0) / (x1 - x0) * juce::MathConstants<float>::twoPi;
                addSample (x, midY - std::sin (phase) * amp);
            }
            break;
        }
        case 5: // Noise
        {
            p.startNewSubPath (x0, midY);
            float noiseVal = 0.0f;
            for (int i = 1; i <= segments; ++i)
            {
                const float x = x0 + (x1 - x0) * i / (float) segments;
                noiseVal = std::sin (i * 12.9898f) * 43758.5453f;
                noiseVal = noiseVal - std::floor (noiseVal);
                const float y = midY + (noiseVal - 0.5f) * 2.0f * amp;
                p.lineTo (x, y);
            }
            break;
        }
        case 6: // Super Saw
        {
            for (int i = 0; i <= segments; ++i)
            {
                const float x = x0 + (x1 - x0) * i / (float) segments;
                const float phase = (x - x0) / (x1 - x0);
                float sum = 0.0f;
                for (int det = 0; det < 5; ++det)
                {
                    const float ph = phase + det * 0.11f;
                    const float saw = 2.0f * (ph - std::floor (ph)) - 1.0f;
                    sum += saw * 0.2f;
                }
                addSample (x, midY - sum * amp);
            }
            break;
        }
        case 7: // Ring Mod (carrier sine x 1.5x sine modulator)
        {
            for (int i = 0; i <= segments; ++i)
            {
                const float x = x0 + (x1 - x0) * i / (float) segments;
                const float phase = (x - x0) / (x1 - x0) * juce::MathConstants<float>::twoPi;
                const float ring = std::sin (phase) * std::sin (1.5f * phase);
                addSample (x, midY - ring * amp);
            }
            break;
        }
        default:
            break;
    }

    g.strokePath (p, juce::PathStrokeType (2.2f, juce::PathStrokeType::mitered, juce::PathStrokeType::square));
}

