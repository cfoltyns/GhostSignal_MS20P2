/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Premium rotary waveform selector knob — snaps between waveform
 *              positions, draws the selected waveform icon in the knob center.
 *              Draws the full knob custom-drawn so the waveform icon is always visible.
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

    // Set range to 0-1 for normalized choice parameter values
    slider.setRange (0.0, 1.0, 0.001);

    // Add listener to trigger callback when waveform changes
    slider.onValueChange = [this]()
    {
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

void WaveformKnob::paint (juce::Graphics& g)
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

    const int labelY = knobAreaH + gap;
    label.setBounds (0, labelY, totalW, labelH);

    label.setFont (GhostSignalLookAndFeel::getKnobLabelFont (knobSize));
}

// ─────────────────────────────────────────────────────────────────────────────
// WaveformSlider — fully custom drawn so icons are always visible
// ─────────────────────────────────────────────────────────────────────────────

WaveformKnob::WaveformSlider::WaveformSlider()
{
    // Use default JUCE LookAndFeel rendering (we draw everything custom in paint)
    setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                         juce::MathConstants<float>::pi * 2.75f,
                         true);
    setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    setMouseDragSensitivity (80);
}

void WaveformKnob::WaveformSlider::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const float cx = bounds.getCentreX();
    const float cy = bounds.getCentreY();
    const float r  = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;

    // ── Knob body ──────────────────────────────────────────────────────────
    const float bodyR = r * 0.88f;
    {
        g.setColour (GhostSignalLookAndFeel::knobBody);
        g.fillEllipse (cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);

        // Brushed metal texture
        g.saveState();
        g.reduceClipRegion (juce::Rectangle<float> (cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f).toNearestInt());
        g.setColour (juce::Colour (0x08FFFFFF));
        for (float lineX = cx - bodyR; lineX < cx + bodyR; lineX += 2.0f)
            g.drawVerticalLine ((int) lineX, cy - bodyR, cy + bodyR);
        g.restoreState();
    }

    // Chrome rim
    g.setColour (GhostSignalLookAndFeel::knobRim);
    g.drawEllipse (cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f, 1.5f);

    // Inner shadow
    {
        const float innerR = bodyR - 2.0f;
        g.setColour (juce::Colour (0x18000000));
        g.drawEllipse (cx - innerR, cy - innerR, innerR * 2.0f, innerR * 2.0f, 1.0f);
    }

    // ── Value arc ───────────────────────────────────────────────────────────
    {
        const float arcRadius    = r * 0.90f;
        const float arcThickness = juce::jlimit (2.5f, 7.0f, r * 0.14f);
        const float sliderPos    = (float) getValue();
        const float startAngle   = juce::MathConstants<float>::pi * 1.25f;
        const float endAngle     = juce::MathConstants<float>::pi * 2.75f;
        const float toAngle      = startAngle + sliderPos * (endAngle - startAngle);

        // Track arc
        {
            juce::Path track;
            track.addCentredArc (cx, cy, arcRadius, arcRadius, 0.0f, startAngle, endAngle, true);
            juce::PathStrokeType stroke (arcThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
            g.setColour (juce::Colour (0xFF3A3A4A));
            g.strokePath (track, stroke);
        }

        // Value arc
        if (sliderPos > 0.001f)
        {
            juce::Path valueArc;
            valueArc.addCentredArc (cx, cy, arcRadius, arcRadius, 0.0f, startAngle, toAngle, true);
            juce::PathStrokeType stroke (arcThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
            g.setColour (GhostSignalLookAndFeel::accent);
            g.strokePath (valueArc, stroke);
        }
    }

    // ── Waveform icon in center ────────────────────────────────────────────
    {
        int waveIndex = 0;
        const float v = (float) getValue();
        const float range = (float) (getMaximum() - getMinimum());
        if (range > 0.0f)
        {
            const int numWaves = 8;
            const int idx = juce::jlimit (0, numWaves - 1,
                                          (int) std::round ((v - (float) getMinimum()) / range * (numWaves - 1)));
            waveIndex = idx;
        }

        const float iconR = r * 0.30f;
        const juce::Rectangle<float> iconArea (cx - iconR, cy - iconR, iconR * 2.0f, iconR * 2.0f);
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