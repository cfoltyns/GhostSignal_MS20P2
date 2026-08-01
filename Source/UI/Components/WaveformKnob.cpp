/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Premium rotary waveform selector knob — snaps between waveform
 *              positions, draws the selected waveform icon in the knob center.
 *              Uses the GhostSignalLookAndFeel for rendering.
 */

#include "WaveformKnob.h"
#include "../LookAndFeel.h"
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
        slider.setRange (snapValues[0], snapValues[numSnapValues - 1], 1.0);
}

void WaveformKnob::paint (juce::Graphics& g)
{
    // WaveformKnob delegates rendering to the WaveformSlider and LabeledKnob
    // via the LookAndFeel. No custom paint needed here.
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

    const int labelY = knobAreaH + gap;
    label.setBounds (0, labelY, totalW, labelH);

    label.setFont (GhostSignalLookAndFeel::getKnobLabelFont (knobSize));
}

WaveformKnob::WaveformSlider::WaveformSlider()
{
    setLookAndFeel (nullptr); // Use default JUCE LookAndFeel to render the arc track
    setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                         juce::MathConstants<float>::pi * 2.75f,
                         true);
    setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    setMouseDragSensitivity (80);
}

void WaveformKnob::WaveformSlider::paint (juce::Graphics& g)
{
    juce::Slider::paint (g);

    // Draw the waveform icon in the center of the knob
    auto bounds = getLocalBounds().toFloat();

    int waveIndex = 0;
    const float v = getValue();
    const float range = (float) getMaximum() - (float) getMinimum();
    if (range > 0.0f)
    {
        const int numWaves = 7; // matches VCO waveform list
        const int idx = juce::jlimit (0, numWaves - 1,
                                      (int) std::round ((v - (float) getMinimum()) / range * (numWaves - 1)));
        waveIndex = idx;
    }

    const float iconR = (float) getWidth() * 0.18f;
    const juce::Rectangle<float> iconArea (bounds.getCentreX() - iconR,
                                           bounds.getCentreY() - iconR,
                                           iconR * 2.0f, iconR * 2.0f);
    drawWaveformIcon (g, iconArea, waveIndex);
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
    const float amp = (y1 - y0) * 0.38f;

    juce::Path p;
    const int segments = 24;

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
            for (int i = 0; i <= segments; ++i)
            {
                const float x = x0 + (x1 - x0) * i / (float) segments;
                const float phase = (x - x0) / (x1 - x0);
                const float tri = 1.0f - 4.0f * std::abs (phase - 0.5f);
                addSample (x, midY - tri * amp);
            }
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
            const float duty = 0.3f; // narrower pulse
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
        case 5: // Noise — deterministic zigzag
        {
            p.startNewSubPath (x0, midY);
            float noiseVal = 0.0f;
            for (int i = 1; i <= segments; ++i)
            {
                const float x = x0 + (x1 - x0) * i / (float) segments;
                // Deterministic pseudo-random using sine hash
                noiseVal = std::sin (i * 12.9898f) * 43758.5453f;
                noiseVal = noiseVal - std::floor (noiseVal); // 0..1
                const float y = midY + (noiseVal - 0.5f) * 2.0f * amp;
                p.lineTo (x, y);
            }
            break;
        }
        case 6: // Super Saw — multiple overlapping saws
        {
            for (int i = 0; i <= segments; ++i)
            {
                const float x = x0 + (x1 - x0) * i / (float) segments;
                const float phase = (x - x0) / (x1 - x0);
                float sum = 0.0f;
                for (int det = 0; det < 4; ++det)
                {
                    const float p = phase + det * 0.13f;
                    const float saw = 2.0f * (p - std::floor (p)) - 1.0f;
                    sum += saw * 0.25f;
                }
                addSample (x, midY - sum * amp);
            }
            break;
        }
        default:
            break;
    }

    g.strokePath (p, juce::PathStrokeType (1.6f, juce::PathStrokeType::mitered, juce::PathStrokeType::square));
}
