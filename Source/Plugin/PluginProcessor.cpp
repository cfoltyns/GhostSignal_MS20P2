/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: PluginProcessor implementation.
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"

PluginProcessor::PluginProcessor()
    : apvts (*this, nullptr, juce::Identifier ("Parameters"), Parameters::createParameterLayout())
{
    // Ensure engine is in a safe state even before prepareToPlay is called
    engine.prepare (44100.0, 512, 2);
}

PluginProcessor::~PluginProcessor()
{
}

// This function is called by the plugin host to create the plugin instance
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}

void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock, getTotalNumOutputChannels());
}

void PluginProcessor::releaseResources()
{
    engine.reset();
}

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    
    // Safety: guard against zero-length buffers or uninitialized state
    if (buffer.getNumSamples() <= 0)
        return;
    
    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    engine.process (buffer, midi, apvts, getPlayHead());
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

// Number of oscillator waveform choices — used to tag saved states and to
// migrate legacy (7-choice) states when "Ring Mod" (index 7) was added.
// NOTE: if more waveforms are added, bump this and extend the migration.
static constexpr int kOscWaveformChoiceCount = 8;
static constexpr int kLegacyOscWaveformChoiceCount = 7;

void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    // Tag the state with the waveform choice count so older states can be
    // detected and remapped on load (see setStateInformation).
    state.setProperty ("waveformChoiceCount", kOscWaveformChoiceCount, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml == nullptr)
        return;

    auto tree = juce::ValueTree::fromXml (*xml);

    // Legacy states were saved when the waveform parameters had 7 choices.
    // Choice parameters are stored normalised (index / (count - 1)), so
    // appending "Ring Mod" as index 7 would shift every saved waveform.
    // Remap legacy normalised values back to their original choice index.
    if (! tree.hasProperty ("waveformChoiceCount"))
    {
        const int legacyMax = kLegacyOscWaveformChoiceCount - 1;         // 6
        const int newMax    = kOscWaveformChoiceCount - 1;               // 7

        auto remapWaveform = [&tree, legacyMax, newMax] (const juce::String& paramId)
        {
            for (int i = 0; i < tree.getNumChildren(); ++i)
            {
                auto child = tree.getChild (i);
                if (child.getType() == juce::Identifier ("PARAM")
                    && child.getProperty ("id").toString() == paramId)
                {
                    const float norm = (float) (double) child.getProperty ("value");
                    const int oldIdx = juce::jlimit (0, legacyMax,
                                                     (int) std::round (norm * (float) legacyMax));
                    child.setProperty ("value", (double) oldIdx / (double) newMax, nullptr);
                }
            }
        };

        remapWaveform (Parameters::paramOsc1Waveform);
        remapWaveform (Parameters::paramOsc2Waveform);
    }

    apvts.replaceState (tree);
}