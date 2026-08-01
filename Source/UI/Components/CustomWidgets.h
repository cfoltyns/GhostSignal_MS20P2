/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Custom vector UI widgets: XY pad, envelope editor, step grid editor.
 */

#pragma once

#include <JuceHeader.h>
#include "../../Core/Sequencer.h"

// XY Pad for macros or modulation control
class XYPad : public juce::Component
{
public:
    XYPad()
    {
        setSize (150, 150);
    }

    ~XYPad() override = default;

    std::function<void(float x, float y)> onValueChanged;

    void setXValue (float x) { xPos = juce::jlimit (0.0f, 1.0f, x); repaint(); }
    void setYValue (float y) { yPos = juce::jlimit (0.0f, 1.0f, y); repaint(); }
    float getXValue() const { return xPos; }
    float getYValue() const { return yPos; }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced (4.0f);

        // Background grid
        g.setColour (juce::Colour (0xFF2A2A2A));
        g.fillRoundedRectangle (bounds, 4.0f);

        // Grid lines
        g.setColour (juce::Colour (0xFF3A3A3A));
        for (int i = 1; i < 4; ++i)
        {
            float t = bounds.getWidth() * i / 4.0f;
            g.drawVerticalLine ((int) (bounds.getX() + t), bounds.getY(), bounds.getBottom());
            g.drawHorizontalLine ((int) (bounds.getY() + bounds.getHeight() * i / 4.0f),
                                  bounds.getX(), bounds.getRight());
        }

        // Crosshair
        float cx = bounds.getX() + xPos * bounds.getWidth();
        float cy = bounds.getY() + (1.0f - yPos) * bounds.getHeight();

        g.setColour (juce::Colours::cyan.withAlpha (0.3f));
        g.drawLine (bounds.getX(), cy, bounds.getRight(), cy, 1.0f);
        g.drawLine (cx, bounds.getY(), cx, bounds.getBottom(), 1.0f);

        // Dot
        g.setColour (juce::Colours::cyan);
        g.fillEllipse (cx - 5.0f, cy - 5.0f, 10.0f, 10.0f);
        g.setColour (juce::Colours::white);
        g.fillEllipse (cx - 2.0f, cy - 2.0f, 4.0f, 4.0f);
    }

    void mouseDown (const juce::MouseEvent& e) override { mouseDrag (e); }
    void mouseDrag (const juce::MouseEvent& e) override
    {
        auto bounds = getLocalBounds().toFloat().reduced (4.0f);
        xPos = juce::jlimit (0.0f, 1.0f, (e.position.x - bounds.getX()) / bounds.getWidth());
        yPos = juce::jlimit (0.0f, 1.0f, 1.0f - (e.position.y - bounds.getY()) / bounds.getHeight());
        repaint();
        if (onValueChanged)
            onValueChanged (xPos, yPos);
    }

private:
    float xPos { 0.5f };
    float yPos { 0.5f };
};

// Envelope shape editor (click-and-drag)
class EnvelopeEditor : public juce::Component
{
public:
    EnvelopeEditor()
    {
        setSize (200, 100);
    }

    ~EnvelopeEditor() override = default;

    std::function<void(float a, float h, float d, float s, float r)> onEnvelopeChanged;

    void setAttack (float a)  { attack = a; repaint(); }
    void setHold (float h)    { hold = h; repaint(); }
    void setDecay (float d)   { decay = d; repaint(); }
    void setSustain (float s) { sustain = s; repaint(); }
    void setRelease (float r) { release = r; repaint(); }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced (2.0f);
        float w = bounds.getWidth();
        float h = bounds.getHeight();

        // Background
        g.setColour (juce::Colour (0xFF2A2A2A));
        g.fillRoundedRectangle (bounds, 4.0f);

        // Envelope shape
        const float totalTime = 1.0f;
        const float aTime = attack * totalTime * 0.2f;
        const float hTime = hold * totalTime * 0.1f;
        const float dTime = decay * totalTime * 0.3f;
        const float rTime = release * totalTime * 0.4f;

        const float x0 = bounds.getX();
        const float x1 = x0 + (aTime / totalTime) * w;
        const float x2 = x1 + (hTime / totalTime) * w;
        const float x3 = x2 + (dTime / totalTime) * w;
        const float x4 = bounds.getRight();

        const float y0 = bounds.getBottom();
        const float y1 = bounds.getY() + 2.0f;
        const float ySustain = bounds.getBottom() - sustain * h;

        juce::Path envPath;
        envPath.startNewSubPath (x0, y0);
        envPath.lineTo (x1, y1);           // Attack
        envPath.lineTo (x2, y1);           // Hold
        envPath.lineTo (x3, ySustain);     // Decay
        envPath.lineTo (x4, ySustain);     // Sustain
        envPath.lineTo (x4 + rTime / totalTime * w, y0); // Release

        g.setColour (juce::Colours::limegreen);
        g.strokePath (envPath, juce::PathStrokeType (2.0f));
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        dragSegment (e.position);
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        dragSegment (e.position);
    }

private:
    void dragSegment (juce::Point<float> pos)
    {
        auto bounds = getLocalBounds().toFloat().reduced (2.0f);
        float rx = (pos.x - bounds.getX()) / bounds.getWidth();

        if (rx < 0.2f)
            attack = juce::jlimit (0.01f, 1.0f, 1.0f - (pos.y - bounds.getY()) / bounds.getHeight());
        else if (rx < 0.3f)
            hold = juce::jlimit (0.0f, 1.0f, 1.0f - (pos.y - bounds.getY()) / bounds.getHeight());
        else if (rx < 0.6f)
            decay = juce::jlimit (0.01f, 1.0f, 1.0f - (pos.y - bounds.getY()) / bounds.getHeight());
        else
            sustain = juce::jlimit (0.0f, 1.0f, 1.0f - (pos.y - bounds.getY()) / bounds.getHeight());

        repaint();
        if (onEnvelopeChanged)
            onEnvelopeChanged (attack, hold, decay, sustain, release);
    }

    float attack { 0.1f };
    float hold { 0.0f };
    float decay { 0.2f };
    float sustain { 0.7f };
    float release { 0.3f };
};

// Step grid editor for the 64-step sequencer
class StepGridEditor : public juce::Component
{
public:
    StepGridEditor()
    {
        setSize (640, 80);
    }

    ~StepGridEditor() override = default;

    std::function<void(int step, int note, float velocity)> onStepClicked;

    void setSteps (std::array<StepData, 64>* stepData) { steps = stepData; repaint(); }
    void setNumSteps (int n) { numSteps = juce::jlimit (1, 64, n); repaint(); }
    void setCurrentStep (int s) { currentStep = s; repaint(); }

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        float stepW = bounds.getWidth() / (float) numSteps;
        float h = bounds.getHeight();

        for (int i = 0; i < numSteps; ++i)
        {
            float x = bounds.getX() + i * stepW;

            // Background
            bool isActive = steps && (*steps)[i].active;
            bool isCurrent = (i == currentStep);

            g.setColour (isCurrent ? juce::Colour (0xFF444488) :
                         isActive ? juce::Colour (0xFF3A3A3A) : juce::Colour (0xFF222222));
            g.fillRect (x, bounds.getY(), stepW - 1.0f, h);

            // Note indicator
            if (isActive && steps)
            {
                const auto& s = (*steps)[i];
                float vel = s.velocity;
                float noteY = bounds.getY() + h * (1.0f - vel);
                g.setColour (juce::Colours::limegreen.withAlpha (vel));
                g.fillRect (x, noteY, stepW - 1.0f, h - (noteY - bounds.getY()));

                // Note label
                if (s.note >= 0)
                {
                    g.setColour (juce::Colours::white.withAlpha (0.7f));
                    g.setFont (10.0f);
                    g.drawText (juce::String (s.note), (int) x, (int) bounds.getY(),
                                (int) stepW - 1, (int) h, juce::Justification::centred);
                }
            }
        }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        if (! steps) return;
        auto bounds = getLocalBounds().toFloat();
        float stepW = bounds.getWidth() / (float) numSteps;
        int stepIdx = (int) ((e.position.x - bounds.getX()) / stepW);
        stepIdx = juce::jlimit (0, numSteps - 1, stepIdx);

        auto& s = (*steps)[stepIdx];
        float noteVal = 1.0f - (e.position.y - bounds.getY()) / bounds.getHeight();
        noteVal = juce::jlimit (0.0f, 1.0f, noteVal);
        int note = (int) (noteVal * 127.0f);

        if (s.active && s.note == note)
        {
            s.active = false;
            s.note = -1;
        }
        else
        {
            s.active = true;
            s.note = note;
            s.velocity = 0.8f;
        }

        repaint();
        if (onStepClicked)
            onStepClicked (stepIdx, s.note, s.velocity);
    }

private:
    std::array<StepData, 64>* steps { nullptr };
    int numSteps { 16 };
    int currentStep { 0 };
};

