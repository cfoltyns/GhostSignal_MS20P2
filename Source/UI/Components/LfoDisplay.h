/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Visual LFO waveform display with shape adjustment.
 */

#pragma once

#include <JuceHeader.h>

class LfoDisplay : public juce::Component
{
public:
    LfoDisplay();
    ~LfoDisplay() override = default;

    void setWaveform (int wf) { waveform = wf; repaint(); }
    void setShape (float s) { shape = juce::jlimit (0.0f, 1.0f, s); repaint(); }
    void setRate (float r) { rate = juce::jlimit (0.01f, 20.0f, r); repaint(); }
    void setDepth (float d) { depth = juce::jlimit (0.0f, 1.0f, d); repaint(); }

    void paint (juce::Graphics& g) override;
    void resized() override;

    std::function<void(float)> onShapeChanged;

private:
    int waveform { 0 };
    float shape { 0.5f };
    float rate { 1.0f };
    float depth { 0.5f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LfoDisplay)
};
