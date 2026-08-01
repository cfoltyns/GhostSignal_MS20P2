/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Rotary waveform selector knob for VCO oscillators.
 *              Snap-to positions with waveform icons drawn in the center.
 */

#pragma once

#include <JuceHeader.h>

class WaveformKnob : public juce::Component
{
public:
    WaveformKnob (const juce::String& labelText);
    ~WaveformKnob() override = default;

    juce::Slider& getSlider() { return slider; }
    void setLabel (const juce::String& text);
    void setSnapToValues (const float* values, int numValues);

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    class WaveformSlider : public juce::Slider
    {
    public:
        WaveformSlider();
        ~WaveformSlider() override = default;

        void paint (juce::Graphics& g) override;
        void drawWaveformIcon (juce::Graphics& g, juce::Rectangle<float> area, int waveIndex) const;
    };

    WaveformSlider slider;
    juce::Label label;
    float labelHeightProportion { 0.22f };
    float snapValues[8];
    int numSnapValues { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformKnob)
};