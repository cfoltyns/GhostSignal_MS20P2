/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Premium rotary knob with label below.
 *              Uses the GhostSignalLookAndFeel for rendering.
 *              Supports optional text value labels at specific positions,
 *              center text display, LED indicator, and modulation indicator.
 */

#include "LabeledKnob.h"
#include "../LookAndFeel.h"

LabeledKnob::LabeledKnob (const juce::String& labelText)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    // Full 270° rotary travel (far bottom-left to far bottom-right) for every
    // knob, independent of the per-knob setup done in PluginEditor.
    slider.setRotaryParameters (juce::MathConstants<float>::pi * 0.75f,
                                juce::MathConstants<float>::pi * 2.25f,
                                true);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    addAndMakeVisible (slider);

    label.setText (labelText, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, GhostSignalLookAndFeel::textSecondary);
    label.setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    label.setFont (GhostSignalLookAndFeel::getKnobLabelFont (60));
    addAndMakeVisible (label);

    slider.addListener (this);

    // Centre text overlay: added AFTER the slider so it paints on top of the
    // knob body. Non-intercepting so drag still reaches the slider.
    centerLabel.setJustificationType (juce::Justification::centred);
    centerLabel.setEditable (false);
    centerLabel.setInterceptsMouseClicks (false, false);
    centerLabel.setColour (juce::Label::textColourId, GhostSignalLookAndFeel::accent);
    centerLabel.setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    centerLabel.setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
    centerLabel.setVisible (false);
    addAndMakeVisible (centerLabel);
}

void LabeledKnob::setCenterText (const juce::String& text)
{
    centerText = text;
    centerLabel.setText (text, juce::dontSendNotification);
    centerLabel.setVisible (text.isNotEmpty());
    // Tell the LookAndFeel not to draw the raw parameter value (e.g. "0"/"1")
    // in the knob centre while our own centre text is displayed.
    slider.getProperties().set ("hideCenterValue", text.isNotEmpty());
    repaint();
}

void LabeledKnob::clearCenterText()
{
    centerText = {};
    centerLabel.setText ({}, juce::dontSendNotification);
    centerLabel.setVisible (false);
    slider.getProperties().set ("hideCenterValue", false);
    repaint();
}

void LabeledKnob::setCenterValueVisible (bool visible)
{
    // The GhostSignalLookAndFeel draws the raw parameter value in the knob
    // centre unless the "hideCenterValue" slider property is set.
    slider.getProperties().set ("hideCenterValue", ! visible);
    repaint();
}

void LabeledKnob::setCenterValueAsKnobPercent()
{
    // The centre readout (LookAndFeel -> Slider::getTextFromValue) now shows
    // the knob position mapped to 0..100 instead of the raw parameter value.
    slider.showAsKnobPercent = true;
    repaint();
}

void LabeledKnob::setLabel (const juce::String& text)
{
    label.setText (text, juce::dontSendNotification);
}

void LabeledKnob::setTextValues (const juce::StringArray& texts, const float* values, int numValues)
{
    textValues = texts;
    numTextValues = juce::jmin (numValues, 24);
    for (int i = 0; i < numTextValues; ++i)
        textValuePositions[i] = values[i];
    showTextValues = (numTextValues > 0);
}

void LabeledKnob::clearTextValues()
{
    textValues.clear();
    numTextValues = 0;
    showTextValues = false;
}

void LabeledKnob::setSnapToValues (const float* values, int numValues)
{
    numSnapValues = juce::jmin (numValues, 24);
    for (int i = 0; i < numSnapValues; ++i)
        snapValues[i] = values[i];

    if (numSnapValues >= 2)
    {
        slider.setRange (snapValues[0], snapValues[numSnapValues - 1], 1.0);
    }
}

void LabeledKnob::setSnapValuesOnly (const float* values, int numValues)
{
    numSnapValues = juce::jmin (numValues, 24);
    for (int i = 0; i < numSnapValues; ++i)
        snapValues[i] = values[i];
}

void LabeledKnob::clearSnapValues()
{
    numSnapValues = 0;
}

void LabeledKnob::paint (juce::Graphics& g)
{
    if (showTextValues && numTextValues > 0)
    {
        const juce::Rectangle<int> sliderBounds = slider.getBounds();
        const float centreX = (float) sliderBounds.getCentreX();
        const float centreY = (float) sliderBounds.getCentreY();
        const float radiusOuter = (float) sliderBounds.getWidth() * 0.48f;
        const float radiusInner = (float) sliderBounds.getWidth() * 0.32f;

        const float minV = (float) slider.getMinimum();
        const float maxV = (float) slider.getMaximum();

        // Match the rotary arc used in PluginEditor: 0.75 pi to 2.25 pi (270 deg sweep)
        const float rotaryStart = juce::MathConstants<float>::pi * 0.75f;
        const float rotaryEnd   = juce::MathConstants<float>::pi * 2.25f;

        // Size the labels from the available arc spacing so that many divisions
        // around a small knob (e.g. LFO tempo divisions) stay tiny and readable.
        const float labelRadius = radiusOuter + 12.0f;
        const float arcSpan     = rotaryEnd - rotaryStart;
        const float pxPerLabel  = labelRadius * arcSpan / (float) juce::jmax (1, numTextValues);
        const float labelW      = juce::jmax (9.0f, juce::jmin (26.0f, pxPerLabel * 0.72f));
        const float labelH      = juce::jmax (8.0f,  juce::jmin (20.0f, labelW * 0.55f));
        const float labelFont   = juce::jmax (4.5f, juce::jmin (7.0f, labelW * 0.40f));

        for (int i = 0; i < numTextValues; ++i)
        {
            const float val = textValuePositions[i];
            if (maxV <= minV)
                continue;

            const float norm = juce::jlimit (0.0f, 1.0f, (val - minV) / (maxV - minV));
            // Map normalised value to rotary angle range (clockwise from bottom-left)
            const float angle = rotaryStart + norm * (rotaryEnd - rotaryStart);

            // Draw a small tick mark at each label position
            const float tickX1 = centreX + std::cos (angle) * radiusInner;
            const float tickY1 = centreY + std::sin (angle) * radiusInner;
            const float tickX2 = centreX + std::cos (angle) * radiusOuter;
            const float tickY2 = centreY + std::sin (angle) * radiusOuter;
            g.setColour (GhostSignalLookAndFeel::textSecondary.withAlpha (0.6f));
            g.drawLine (tickX1, tickY1, tickX2, tickY2, 1.2f);

            // Position the label slightly outside the arc
            const float textX = centreX + std::cos (angle) * labelRadius;
            const float textY = centreY + std::sin (angle) * labelRadius;

            // Background pill for readability
            g.setColour (juce::Colour (0xBB000000));
            g.fillRoundedRectangle (textX - labelW * 0.5f - 1.0f,
                                    textY - labelH * 0.5f - 1.0f,
                                    labelW + 2.0f, labelH + 2.0f, 3.0f);

            g.setColour (GhostSignalLookAndFeel::textPrimary);
            g.setFont (juce::Font (juce::FontOptions ((float) labelFont, juce::Font::bold)));
            g.drawText (textValues[i],
                        juce::Rectangle<float> (textX - labelW * 0.5f, textY - labelH * 0.5f, labelW, labelH),
                        juce::Justification::centred, false);
        }
    }

    // Draw accent LED indicator in the center of the knob (only if no center text —
    // the centre text itself is drawn by the centerLabel overlay, above the slider)
    if (centerText.isEmpty() && ledActive)
    {
        const juce::Rectangle<int> sliderBounds = slider.getBounds();
        const float cx = (float) sliderBounds.getCentreX();
        const float cy = (float) sliderBounds.getCentreY();
        const float ledRadius = 3.0f;

        g.setColour (GhostSignalLookAndFeel::accent);
        g.fillEllipse (cx - ledRadius, cy - ledRadius, ledRadius * 2.0f, ledRadius * 2.0f);

        // Subtle glow
        g.setColour (GhostSignalLookAndFeel::accent.withAlpha (0.4f));
        g.fillEllipse (cx - ledRadius * 2.0f, cy - ledRadius * 2.0f, ledRadius * 4.0f, ledRadius * 4.0f);
    }

    // Animated LFO modulation ring — a colored ring around the knob showing
    // the LFO movement. The ring track shows that modulation is routed; the
    // bright glowing dot sweeps along it with the LFO's current output value.
    if (showModRing)
    {
        const juce::Rectangle<int> sliderBounds = slider.getBounds();
        const float cx = (float) sliderBounds.getCentreX();
        const float cy = (float) sliderBounds.getCentreY();
        // Ring sits just outside the knob body (body ≈ 0.42 * width)
        const float ringRadius = (float) sliderBounds.getWidth() * 0.47f;

        // Match the rotary arc: 0.75 pi (bottom-left) → 2.25 pi (bottom-right)
        const float rotaryStart = juce::MathConstants<float>::pi * 0.75f;
        const float rotaryEnd   = juce::MathConstants<float>::pi * 2.25f;

        // Map the bipolar LFO output (-1..1) onto the full rotary arc so the
        // dot sweeps bottom-left → top → bottom-right and back as the LFO cycles.
        const float norm = juce::jlimit (0.0f, 1.0f, (lfoModValue + 1.0f) * 0.5f);
        const float angle = rotaryStart + norm * (rotaryEnd - rotaryStart);

        // 1. Base ring track — dim full-travel arc showing modulation is routed
        juce::Path ringTrack;
        ringTrack.addCentredArc (cx, cy, ringRadius, ringRadius, 0.0f,
                                 rotaryStart, rotaryEnd, true);
        // Base ring brightness breathes with the LFO magnitude for extra life
        const float pulse = 0.18f + 0.12f * std::abs (lfoModValue);
        g.setColour (modRingColor.withAlpha (pulse));
        g.strokePath (ringTrack, juce::PathStrokeType (2.0f,
                                                       juce::PathStrokeType::curved,
                                                       juce::PathStrokeType::rounded));

        // 2. Trail arc — comet tail fading behind the moving dot
        {
            const float trailSpan = 0.6f; // radians of tail behind the dot
            const float trailStart = angle - trailSpan;

            const int trailSteps = 8;
            for (int i = 0; i < trailSteps; ++i)
            {
                const float t0 = (float) i / (float) trailSteps;
                const float t1 = (float) (i + 1) / (float) trailSteps;

                juce::Path trailSeg;
                trailSeg.addCentredArc (cx, cy, ringRadius, ringRadius, 0.0f,
                                        trailStart + t0 * trailSpan,
                                        trailStart + t1 * trailSpan, true);

                // Fade from nearly-invisible (oldest) to bright (newest)
                const float alpha = 0.35f * t1 * t1;
                g.setColour (modRingColor.withAlpha (alpha));
                g.strokePath (trailSeg, juce::PathStrokeType (2.2f,
                                                              juce::PathStrokeType::curved,
                                                              juce::PathStrokeType::rounded));
            }
        }

        // 3. Leading arc — short bright segment ahead of the dot (motion direction)
        {
            juce::Path leadSeg;
            leadSeg.addCentredArc (cx, cy, ringRadius, ringRadius, 0.0f,
                                   angle, angle + 0.25f, true);
            g.setColour (modRingColor.withAlpha (0.85f));
            g.strokePath (leadSeg, juce::PathStrokeType (2.6f,
                                                         juce::PathStrokeType::curved,
                                                         juce::PathStrokeType::rounded));
        }

        // 4. Glowing dot at the current LFO position
        const float dotX = cx + std::cos (angle) * ringRadius;
        const float dotY = cy + std::sin (angle) * ringRadius;
        const float dotR = 3.2f;

        // Outer glow halo
        g.setColour (modRingColor.withAlpha (0.35f));
        g.fillEllipse (dotX - dotR * 2.4f, dotY - dotR * 2.4f,
                       dotR * 4.8f, dotR * 4.8f);

        // Mid glow
        g.setColour (modRingColor.withAlpha (0.65f));
        g.fillEllipse (dotX - dotR * 1.5f, dotY - dotR * 1.5f,
                       dotR * 3.0f, dotR * 3.0f);

        // Bright core
        g.setColour (modRingColor.brighter (0.35f));
        g.fillEllipse (dotX - dotR, dotY - dotR, dotR * 2.0f, dotR * 2.0f);
    }
}

void LabeledKnob::resized()
{
    const int totalH = getHeight();
    const int totalW = getWidth();

    // NOTE: the label area is reserved even when the label text is empty
    // (e.g. the noise type knob, whose value text lives in the knob centre).
    // This keeps the knob diameter identical to labelled knobs.
    const int labelH = juce::jmax (16, (int) (totalH * labelHeightProportion));
    const int gap = juce::jmax (2, (int) (totalH * 0.04f));
    const int knobAreaH = totalH - labelH - gap;

    const int knobSize = juce::jmax (minKnobSize, juce::jmin (totalW, knobAreaH));
    const int knobX    = (totalW - knobSize) / 2;
    const int knobY    = (knobAreaH - knobSize) / 2;

    slider.setBounds (knobX, knobY, knobSize, knobSize);

    // Centre the text overlay over the knob body so division / ms readouts
    // (set via setCenterText) sit in the middle of the knob.
    const int centerH = juce::jmax (12, (int) (knobSize * 0.24f));
    centerLabel.setBounds (knobX + 2, knobY + (knobSize - centerH) / 2, knobSize - 4, centerH);
    centerLabel.setFont (juce::Font (juce::FontOptions (
        juce::jlimit (8.0f, 12.0f, (float) knobSize * 0.15f), juce::Font::bold)));

    const int labelY = knobAreaH + gap;
    label.setBounds (0, labelY, totalW, labelH);
    label.setFont (GhostSignalLookAndFeel::getKnobLabelFont (knobSize));
}

void LabeledKnob::sliderValueChanged (juce::Slider* s)
{
    // If snap values were configured, snap the slider to the nearest one.
    // This ensures discrete knobs (e.g. tape delay mode selector) behave
    // like stepped controls rather than continuous sliders.
    if (numSnapValues > 0 && s != nullptr)
    {
        const float   val    = (float) s->getValue();
        float         best   = snapValues[0];
        float         bestDiff = std::abs (val - snapValues[0]);

        for (int i = 1; i < numSnapValues; ++i)
        {
            const float diff = std::abs (val - snapValues[i]);
            if (diff < bestDiff)
            {
                bestDiff = diff;
                best   = snapValues[i];
            }
        }

        // Only trigger a notification if we actually need to snap.
        // sendNotificationSync keeps the SliderAttachment parameter in sync
        // while the value-change check prevents infinite recursion.
        if (std::abs (best - val) > 0.001f)
            s->setValue (best, juce::sendNotification);
    }

    updateCenterTextFromValue();
}

void LabeledKnob::updateCenterTextFromValue()
{
    if (autoCenterText && showTextValues && numTextValues > 0)
    {
        const float val = (float) slider.getValue();
        int closestIdx = 0;
        float minDiff = std::abs (val - textValuePositions[0]);
        for (int i = 1; i < numTextValues; ++i)
        {
            const float diff = std::abs (val - textValuePositions[i]);
            if (diff < minDiff)
            {
                minDiff = diff;
                closestIdx = i;
            }
        }
        setCenterText (textValues[closestIdx]);
    }
}
