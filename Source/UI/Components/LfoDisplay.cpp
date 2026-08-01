/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Visual LFO waveform display with shape adjustment.
 *              Supports five classic waveforms: Sine, Triangle, Square,
 *              Sawtooth (shape-morphable), and Random.
 *              Uses the GhostSignalLookAndFeel color palette.
 */

#include "LfoDisplay.h"
#include "../LookAndFeel.h"

LfoDisplay::LfoDisplay()
{
}

void LfoDisplay::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();
    const float w = b.getWidth();
    const float h = b.getHeight();

    // Background — dark screen with subtle gradient
    juce::ColourGradient bgGrad;
    bgGrad.point1 = { 0.0f, 0.0f };
    bgGrad.point2 = { 0.0f, h };
    bgGrad.addColour (0.0f, GhostSignalLookAndFeel::panel.darker (0.1f));
    bgGrad.addColour (1.0f, GhostSignalLookAndFeel::panel.darker (0.2f));
    g.setGradientFill (bgGrad);
    g.fillRoundedRectangle (b, 4.0f);

    // Border
    g.setColour (GhostSignalLookAndFeel::panelBorder);
    g.drawRoundedRectangle (b.reduced (0.5f), 4.0f, 1.0f);

    // Inner shadow for depth
    g.setColour (GhostSignalLookAndFeel::panelShadow);
    g.drawRoundedRectangle (b.reduced (1.0f), 3.5f, 0.5f);

    // Draw waveform
    const float margin = 4.0f;
    const float waveW = w - 2.0f * margin;
    const float waveH = h - 2.0f * margin;
    const float midY = b.getY() + margin + waveH * 0.5f;

    juce::Path wavePath;
    wavePath.startNewSubPath (b.getX() + margin, midY);

    const int numPoints = 64;
    const float amplitude = waveH * 0.4f * depth;

    for (int i = 0; i <= numPoints; ++i)
    {
        const float x = b.getX() + margin + (float) i / (float) numPoints * waveW;
        float y = midY;

        const float phase = (float) i / (float) numPoints;

        switch (waveform)
        {
            case 0: // Sine
                y = midY - amplitude * std::sin (juce::MathConstants<float>::twoPi * phase);
                break;
            case 1: // Triangle
                y = midY - amplitude * (phase < 0.5f ? (4.0f * phase - 1.0f) : (3.0f - 4.0f * phase));
                break;
            case 2: // Square (shape controls duty cycle / pulse width)
            {
                const float duty = 0.1f + 0.8f * shape; // 10%–90% duty cycle
                y = midY - amplitude * (phase < duty ? 1.0f : -1.0f);
                break;
            }
            case 3: // Sawtooth (saw-up ramp — matches LFO engine's sawUp waveform)
                y = midY - amplitude * (2.0f * phase - 1.0f);
                break;
            case 4: // Random (sample & hold style — stepped noise)
            {
                const int step = i % 8;
                static float lastVal = 0.0f;
                static int lastStep = 0;
                if (step != lastStep)
                {
                    lastVal = (std::rand() % 1000 / 500.0f - 1.0f);
                    lastStep = step;
                }
                y = midY - amplitude * lastVal;
                break;
            }
            default:
                break;
        }

        if (i == 0)
            wavePath.startNewSubPath (x, y);
        else
            wavePath.lineTo (x, y);
    }

    // Draw the waveform path with accent color and subtle glow
    g.setColour (GhostSignalLookAndFeel::accent);
    g.strokePath (wavePath, juce::PathStrokeType (1.5f));

    // Subtle glow behind waveform
    g.setColour (GhostSignalLookAndFeel::accent.withAlpha (0.15f));
    g.strokePath (wavePath, juce::PathStrokeType (3.0f));

    // Draw horizontal center line
    g.setColour (juce::Colour (0x30FFFFFF));
    g.drawHorizontalLine (static_cast<int> (midY), b.getX() + margin, b.getRight() - margin);
}

void LfoDisplay::resized()
{
}
