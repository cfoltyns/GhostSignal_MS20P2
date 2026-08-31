/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Oscilloscope-style waveform display for the oscillator sections.
 *              Draws the currently selected oscillator waveform (Triangle, Saw,
 *              Square, Pulse, Sine, Noise, Super Saw, Ring Mod) scrolling like
 *              a scope trace while notes play, static when idle. The pulse
 *              width setting live-shapes the Pulse duty cycle.
 *              Uses the GhostSignalLookAndFeel color palette.
 */

#pragma once

#include <JuceHeader.h>

class OscilloscopeDisplay : public juce::Component
{
public:
    OscilloscopeDisplay();
    ~OscilloscopeDisplay() override = default;

    // Waveform shape to draw (index into Parameters::oscWaveformChoices).
    void setWaveform (int wf);

    // Pulse duty cycle 0..1 (only affects the Pulse waveform).
    void setPulseWidth (float pw01);

    // Number of waveform cycles across the display width. This is where the
    // Octave knob is visualised: octave shifts rescale frequency (2× per
    // octave up, ½× per octave down) without changing the waveform's shape.
    void setCycles (float c);

    // While active the trace scrolls like an oscilloscope; idle it freezes.
    void setActive (bool playing);

    // Call from the editor's timer (~30 Hz) to advance the animation.
    void tick();

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    // Waveform value in -1..1 at the given phase (in cycles).
    float sampleWave (float phaseCycles) const;

    int         waveform   { 4 };     // Sine
    float       pulseWidth { 0.5f };
    float       cycles     { 2.0f };
    bool        active     { false };
    float       phase      { 0.0f };
    unsigned int noiseSeed { 1 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscilloscopeDisplay)
};
