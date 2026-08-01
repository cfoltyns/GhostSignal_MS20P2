/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Interactive ADSR envelope display with 4 draggable points.
 *              Correlated to the ADSR knobs for real-time visual feedback.
 *              Points: Attack (peak), Decay (sustain entry), Sustain (level),
 *              Release (end).  All positions are normalized 0..1.
 */

#pragma once

#include <JuceHeader.h>

class EnvDisplay : public juce::Component
{
public:
    EnvDisplay();
    ~EnvDisplay() override = default;

    // --- Setters (called by knob changes) ---
    void setAttack  (float a) { attackVal  = juce::jlimit (0.0f, 1.0f, a); repaint(); }
    void setDecay   (float d) { decayVal   = juce::jlimit (0.0f, 1.0f, d); repaint(); }
    void setSustain (float s) { sustainVal = juce::jlimit (0.0f, 1.0f, s); repaint(); }
    void setRelease (float r) { releaseVal = juce::jlimit (0.0f, 1.0f, r); repaint(); }

    // --- Getters (called by timer for two-way sync) ---
    float getAttack()  const { return attackVal; }
    float getDecay()   const { return decayVal; }
    float getSustain() const { return sustainVal; }
    float getRelease() const { return releaseVal; }

    void paint  (juce::Graphics& g) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp   (const juce::MouseEvent& e) override;

    // --- Callbacks (called when user drags points) ---
    // Values are normalized 0..1, matching the parameter's 0-to-1 range.
    std::function<void(float)> onAttackChanged;
    std::function<void(float)> onDecayChanged;
    std::function<void(float)> onSustainChanged;
    std::function<void(float)> onReleaseChanged;

private:
    // Normalize a raw parameter value (in seconds for A/D/R, 0..1 for S) to 0..1
    // using a time-scale curve so short and long times are both editable.
    static float timeToNorm (float seconds, float maxVal = 5.0f)
    {
        // Exponential-ish mapping: 0 → 0, maxVal → 1
        if (seconds <= 0.0f) return 0.0f;
        const float ratio = juce::jlimit (0.0f, 1.0f, seconds / maxVal);
        return std::sqrt (ratio); // sqrt gives finer control at short times
    }
    static float normToTime (float norm, float maxVal = 5.0f)
    {
        return maxVal * norm * norm;
    }

public:
    // Convenience: set values directly from raw parameter values (seconds)
    void setAttackRaw  (float seconds) { attackVal  = juce::jlimit (0.0f, 1.0f, timeToNorm (seconds)); repaint(); }
    void setDecayRaw   (float seconds) { decayVal   = juce::jlimit (0.0f, 1.0f, timeToNorm (seconds)); repaint(); }
    void setSustainRaw (float norm)    { sustainVal = juce::jlimit (0.0f, 1.0f, norm); repaint(); }
    void setReleaseRaw (float seconds) { releaseVal = juce::jlimit (0.0f, 1.0f, timeToNorm (seconds)); repaint(); }

private:
    float attackVal  { 0.3f };
    float decayVal   { 0.3f };
    float sustainVal { 0.7f };
    float releaseVal { 0.3f };

    enum DragTarget { none, attack, decay, sustain, release };
    DragTarget dragTarget { none };

    bool dragging { false };

    // Visual layout helpers — return pixel rectangles for each draggable point
    juce::Rectangle<float> getAttackPoint()  const;
    juce::Rectangle<float> getDecayPoint()   const;
    juce::Rectangle<float> getSustainPoint() const;
    juce::Rectangle<float> getReleasePoint() const;

    // Path construction for the envelope curve
    juce::Path getEnvelopePath() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnvDisplay)
};
