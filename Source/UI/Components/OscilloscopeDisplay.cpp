/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Oscilloscope-style waveform display for the oscillator sections.
 *              Draws the currently selected oscillator waveform scrolling like
 *              a scope trace while notes play, static when idle. Styled to
 *              match the LfoDisplay screens.
 */

#include "OscilloscopeDisplay.h"
#include "../LookAndFeel.h"

OscilloscopeDisplay::OscilloscopeDisplay()
{
}

void OscilloscopeDisplay::setWaveform (int wf)
{
    const int clamped = juce::jlimit (0, 7, wf);
    if (clamped == waveform)
        return;

    waveform = clamped;
    repaint();
}

void OscilloscopeDisplay::setPulseWidth (float pw01)
{
    const float clamped = juce::jlimit (0.05f, 0.95f, pw01);
    if (std::abs (clamped - pulseWidth) < 0.001f)
        return;

    pulseWidth = clamped;
    repaint();
}

void OscilloscopeDisplay::setCycles (float c)
{
    // Clamped wide enough to represent every octave setting of the VCOs
    // (-2..+2 => 0.5..8 cycles) and the Sub (-3..+3 => 0.125..8 cycles).
    cycles = juce::jlimit (0.125f, 10.0f, c);
}

void OscilloscopeDisplay::setActive (bool playing)
{
    if (playing == active)
        return;

    active = playing;
    repaint(); // freeze/unfreeze the trace immediately
}

void OscilloscopeDisplay::tick()
{
    if (! active)
        return;

    phase += 0.02f;
    if (phase >= 1.0f)
        phase -= 1.0f;

    // Re-roll the noise pattern a few times per second so Noise looks alive
    if ((juce::Random::getSystemRandom().nextInt (12)) == 0)
        noiseSeed = noiseSeed * 1664525u + 1013904223u;

    repaint();
}

float OscilloscopeDisplay::sampleWave (float phaseCycles) const
{
    const float p = phaseCycles - std::floor (phaseCycles); // wrap 0..1

    switch (waveform)
    {
        case 0: // Triangle
            return p < 0.5f ? (4.0f * p - 1.0f) : (3.0f - 4.0f * p);

        case 1: // Saw
            return 2.0f * p - 1.0f;

        case 2: // Square
            return p < 0.5f ? 1.0f : -1.0f;

        case 3: // Pulse (duty cycle from the PW knob)
            return p < pulseWidth ? 1.0f : -1.0f;

        case 4: // Sine
            return std::sin (juce::MathConstants<float>::twoPi * p);

        case 5: // Noise (stepped sample & hold)
        {
            const int step = (int) (p * 16.0f);
            unsigned int x = noiseSeed * 31u + (unsigned int) step * 747796405u;
            x ^= x >> 16; x *= 0x7feb352du; x ^= x >> 15; x *= 0x846ca68bu; x ^= x >> 16;
            return (float) (x & 0xffffffu) / (float) 0xffffffu * 2.0f - 1.0f;
        }

        case 6: // Super Saw (5 detuned saws)
        {
            float sum = 0.0f;
            for (int det = 0; det < 5; ++det)
            {
                const float ph = p + (float) det * 0.11f;
                sum += 2.0f * (ph - std::floor (ph)) - 1.0f;
            }
            return sum * 0.2f;
        }

        case 7: // Ring Mod (carrier sine x 1.5x sine modulator)
            return std::sin (juce::MathConstants<float>::twoPi * p)
                 * std::sin (3.0f * juce::MathConstants<float>::pi * p);

        default:
            return 0.0f;
    }
}

void OscilloscopeDisplay::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();
    const float w = b.getWidth();
    const float h = b.getHeight();

    // Background — dark screen with subtle gradient (matches LfoDisplay)
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

    // ── Scope trace ───────────────────────────────────────────────────────────
    const float margin    = 3.0f;
    const float midY      = b.getY() + h * 0.5f;
    const float amplitude = (h - 2.0f * margin) * 0.36f;
    const float traceL    = b.getX() + margin;
    const float traceW    = w - 2.0f * margin;

    // Horizontal center line
    g.setColour (juce::Colour (0x30FFFFFF));
    g.drawHorizontalLine ((int) midY, traceL, b.getRight() - margin);

    juce::Path trace;
    const int numPoints = 96;
    for (int i = 0; i <= numPoints; ++i)
    {
        const float t   = (float) i / (float) numPoints;
        const float ph  = t * cycles + phase;
        const float x   = traceL + t * traceW;
        const float y   = midY - amplitude * sampleWave (ph);

        if (i == 0)
            trace.startNewSubPath (x, y);
        else
            trace.lineTo (x, y);
    }

    // Subtle glow behind the trace
    g.setColour (GhostSignalLookAndFeel::accent.withAlpha (0.15f));
    g.strokePath (trace, juce::PathStrokeType (3.0f));

    // Main trace
    g.setColour (GhostSignalLookAndFeel::accent);
    g.strokePath (trace, juce::PathStrokeType (1.5f));
}

void OscilloscopeDisplay::resized()
{
}
