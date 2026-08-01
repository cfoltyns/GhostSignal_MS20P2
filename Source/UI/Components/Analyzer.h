/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#pragma once

#include <JuceHeader.h>

class Analyzer : public juce::Component
{
public:
    Analyzer();
    ~Analyzer() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void pushBuffer (const juce::AudioBuffer<float>& buffer, int numSamples);
    
    void setColourScheme (juce::Colour background, juce::Colour line)
    {
        bgColour = background;
        lineColour = line;
    }

private:
    void processFFT();

    juce::AudioBuffer<float> fifo;
    int fifoIndex { 0 };
    int fftOrder { 12 }; // 4096 points
    int fftSize { 4096 };
    float fftData[4096] { 0 };
    float magnitude[2048] { 0 };
    
    // Waveform display buffer
    juce::AudioBuffer<float> waveformBuffer;
    int waveformIndex { 0 };
    int waveformSize { 1024 };

    juce::Colour bgColour { juce::Colours::black };
    juce::Colour lineColour { juce::Colours::lime };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Analyzer)
};