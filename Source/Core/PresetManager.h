/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#pragma once

#include <JuceHeader.h>
#include <vector>

enum class PresetCategory
{
    bass,
    lead,
    pad,
    pluck,
    fx,
    arp,
    drum,
    user
};

struct PresetInfo
{
    juce::String name;
    juce::String author;
    PresetCategory category { PresetCategory::user };
    juce::StringArray tags;
    int index { -1 };
    juce::Time date { juce::Time::getCurrentTime() };
    bool favorite { false };
};

class PresetManager
{
public:
    PresetManager() = default;
    ~PresetManager() = default;

    void prepare (juce::AudioProcessorValueTreeState& state);
    
    // Load/save presets
    void loadPreset (int index);
    void savePreset (int index);
    void createPreset (const juce::String& name);
    void deletePreset (int index);
    
    // Factory presets
    void loadFactoryPresets();
    
    // Random/preset browser
    int getRandomPresetIndex();
    int getNumPresets() const { return (int) userPresets.size(); }
    
    // Get preset info
    const PresetInfo& getPresetInfo (int index) const;
    
    // Favorites
    void setFavorite (int index, bool fav);
    bool isFavorite (int index) const;

private:
    juce::AudioProcessorValueTreeState* apvts { nullptr };
    
    juce::File presetFolder;
    std::vector<PresetInfo> userPresets;
    std::vector<PresetInfo> factoryPresets;
    
    juce::XmlElement getParameterState() const;
    void setParameterState (const juce::XmlElement&);
};