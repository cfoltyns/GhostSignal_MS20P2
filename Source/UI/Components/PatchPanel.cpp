/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#include "PatchPanel.h"

using namespace juce;

//==============================================================================
PatchJackComponent::PatchJackComponent (const PatchJack& j)
    : jack (j)
{
    setSize (20, 20);
}

void PatchJackComponent::paint (Graphics& g)
{
    auto bounds = getBounds().reduced (2).toFloat();

    // Outer ring
    g.setColour (jack.colour.brighter());
    g.fillEllipse (bounds);

    // Inner circle
    g.setColour (jack.colour);
    g.fillEllipse (bounds.reduced (3));

    // Highlight if dragging
    if (isDragging)
    {
        g.setColour (Colours::yellow.withAlpha (0.5f));
        g.drawEllipse (bounds, 2.0f);
    }
}

void PatchJackComponent::mouseDown (const MouseEvent&)
{
    isDragging = true;
    repaint();
}

void PatchJackComponent::mouseUp (const MouseEvent&)
{
    isDragging = false;
    repaint();
}

void PatchJackComponent::mouseDrag (const MouseEvent&)
{
    repaint();
}

//==============================================================================
PatchPanel::PatchPanel()
{
    // MS-20 style patch points - create standard jacks
    addJack ({"osc1Pulse", "OSC1 Pulse", JackType::output, Colours::orange});
    addJack ({"osc1Saw", "OSC1 Saw", JackType::output, Colours::yellow});
    addJack ({"osc1Tri", "OSC1 Triangle", JackType::output, Colours::green});
    addJack ({"osc2Pulse", "OSC2 Pulse", JackType::output, Colours::orange});
    addJack ({"osc2Saw", "OSC2 Saw", JackType::output, Colours::yellow});
    addJack ({"osc2Tri", "OSC2 Triangle", JackType::output, Colours::green});
    addJack ({"noiseOut", "Noise", JackType::output, Colours::white});
    addJack ({"extIn", "Ext In", JackType::input, Colours::blue});
    addJack ({"VCF_in", "VCF In", JackType::input, Colours::blue});
    addJack ({"VCF_out", "VCF Out", JackType::output, Colours::green});
    addJack ({"HPF_out", "HPF Out", JackType::output, Colours::green});
    addJack ({"env1_out", "ENV1 Out", JackType::output, Colours::purple});
    addJack ({"env2_out", "ENV2 Out", JackType::output, Colours::purple});
    addJack ({"lfo1_out", "LFO1 Out", JackType::output, Colours::cyan});
    addJack ({"lfo2_out", "LFO2 Out", JackType::output, Colours::cyan});

    // Inputs
    addJack ({"VCF_freq", "VCF Freq", JackType::input, Colours::blue});
    addJack ({"VCF_res", "VCF Res", JackType::input, Colours::blue});
}

PatchPanel::~PatchPanel()
{
}

void PatchPanel::paint (Graphics& g)
{
    // Background
    g.setColour (Colours::black.brighter (0.1f));
    g.fillAll();

    // Draw cables
    for (const auto& cable : cables)
    {
        // Find positions of source and dest jacks
        Point<float> start, end;
        for (const auto& jack : jacks)
        {
            if (jack->getId() == cable.sourceId)
                start = jack->getPosition().toFloat();
            if (jack->getId() == cable.destId)
                end = jack->getPosition().toFloat();
        }

        if (start.isFinite() && end.isFinite())
        {
            g.setColour (cable.colour.withAlpha (0.7f));
            g.drawLine (start.x, start.y, end.x, end.y, cable.thickness);
        }
    }

    // Title
    g.setColour (Colours::white);
    g.drawText ("PATCH PANEL", getLocalBounds(), Justification::centredTop);
}

void PatchPanel::resized()
{
    // Position jacks in a grid pattern
    int y = 40;
    int x = 10;
    int col = 0;

    for (auto& jack : jacks)
    {
        jack->setTopLeftPosition (x, y + col * 25);
        col++;
        if (col > 15)
        {
            col = 0;
            x += 200;
        }
    }
}

void PatchPanel::addJack (const PatchJack& jack)
{
    auto jackComp = std::make_unique<PatchJackComponent> (jack);
    addAndMakeVisible (jackComp.get());
    jacks.push_back (std::move (jackComp));
}

void PatchPanel::addCable (const String& sourceId, const String& destId)
{
    // Remove existing cable from this source
    cables.erase (std::remove_if (cables.begin(), cables.end(),
        [&sourceId] (const PatchCable& c) { return c.sourceId == sourceId; }),
        cables.end());

    if (destId.isNotEmpty())
    {
        PatchCable cable;
        cable.sourceId = sourceId;
        cable.destId = destId;
        cables.push_back (cable);
    }
    
    repaint();
}

void PatchPanel::clearCables()
{
    cables.clear();
    repaint();
}

std::vector<String> PatchPanel::getConnectionsForSource (const String& sourceId) const
{
    std::vector<String> result;
    for (const auto& cable : cables)
    {
        if (cable.sourceId == sourceId)
            result.push_back (cable.destId);
    }
    return result;
}