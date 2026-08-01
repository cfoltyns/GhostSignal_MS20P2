/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#include "PresetManager.h"

void PresetManager::prepare (juce::AudioProcessorValueTreeState&)
{
    presetFolder = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                    .getChildFile ("GhostSignal/MS20P/Presets");
    presetFolder.createDirectory();
    
    loadFactoryPresets();
}

void PresetManager::loadPreset (int index)
{
    // Simplified - in production this would load actual parameter values
    if (index >= 0 && index < (int) factoryPresets.size() + (int) userPresets.size())
    {
        juce::ignoreUnused (index);
        // Trigger via AudioProcessorValueTreeState in the processor
    }
}

void PresetManager::savePreset (int)
{
    // Simplified - in production this would save parameter values
}

void PresetManager::createPreset (const juce::String& name)
{
    PresetInfo info;
    info.name = name;
    info.index = (int) userPresets.size();
    info.category = PresetCategory::user;
    userPresets.push_back (info);
}

void PresetManager::deletePreset (int index)
{
    if (index >= 0 && index < (int) userPresets.size())
    {
        auto file = presetFolder.getChildFile (userPresets[index].name + ".preset");
        file.deleteFile();
        userPresets.erase (userPresets.begin() + index);
    }
}

void PresetManager::loadFactoryPresets()
{
    factoryPresets.clear();
    
    const juce::StringArray factoryNames = {
        "MS20 Bass",
        "Aggressive Lead",
        "Space Pad",
        "Plucky Stab",
        "Noise FX",
        "Arp Sequence"
    };
    
    for (int i = 0; i < factoryNames.size(); ++i)
    {
        PresetInfo info;
        info.name = factoryNames[i];
        info.index = i;
        info.category = (PresetCategory) i;
        factoryPresets.push_back (info);
    }
}

int PresetManager::getRandomPresetIndex()
{
    const int total = (int) (userPresets.size() + factoryPresets.size());
    if (total == 0)
        return -1;
    
    return rand() % total;
}

const PresetInfo& PresetManager::getPresetInfo (int index) const
{
    static PresetInfo emptyInfo;
    
    if (index >= 0 && index < (int) userPresets.size())
        return userPresets[index];
    if (index >= 0 && index < (int) factoryPresets.size())
        return factoryPresets[index];
    
    return emptyInfo;
}

void PresetManager::setFavorite (int index, bool fav)
{
    if (index >= 0 && index < (int) userPresets.size())
        userPresets[index].favorite = fav;
}

bool PresetManager::isFavorite (int index) const
{
    if (index >= 0 && index < (int) userPresets.size())
        return userPresets[index].favorite;
    return false;
}

juce::XmlElement PresetManager::getParameterState() const
{
    return juce::XmlElement ("PRESET");
}

void PresetManager::setParameterState (const juce::XmlElement&)
{
    // Handled elsewhere
}