/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#pragma once

#include <JuceHeader.h>
#include <functional>
#include <vector>

enum class PresetCategory
{
    init,
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
    juce::String id;
    juce::String name;
    juce::String description;
    juce::String author;
    PresetCategory category { PresetCategory::user };
    juce::StringArray tags;
    int index { -1 };
    juce::Time created { juce::Time::getCurrentTime() };
    juce::Time modified { juce::Time::getCurrentTime() };
    juce::Time date { juce::Time::getCurrentTime() };
    bool favorite { false };
    bool factory { false };
    juce::String fileName;
};

struct PresetRecord
{
    PresetInfo info;
    juce::ValueTree state;
    juce::File file;
};

class PresetManager
{
public:
    using ChangeCallback = std::function<void()>;
    using StatusCallback = std::function<void (const juce::String&, bool)>;

    PresetManager() = default;
    ~PresetManager() = default;

    void prepare (juce::AudioProcessorValueTreeState& state);

    // Apply a complete APVTS state while preserving parameter adapters.
    bool applyState (const juce::ValueTree& state);

    // Combined indices are factory presets first, followed by user presets.
    bool loadPreset (int index);
    bool savePreset (const juce::String& name,
                     const juce::String& description = juce::String(),
                     const juce::StringArray& tags = juce::StringArray(),
                     int overwriteIndex = -1);
    bool savePreset (int index);
    void createPreset (const juce::String& name);
    bool renamePreset (int index,
                       const juce::String& newName,
                       const juce::String& newDescription = juce::String(),
                       const juce::StringArray& newTags = juce::StringArray());
    bool deletePreset (int index);

    // Factory presets and disk refresh.
    void loadFactoryPresets();
    void refreshUserPresets();

    // Browser information.
    int getRandomPresetIndex() const;
    int getNumPresets() const { return getNumFactoryPresets() + getNumUserPresets(); }
    int getNumFactoryPresets() const { return static_cast<int> (factoryPresets.size()); }
    int getNumUserPresets() const { return static_cast<int> (userPresets.size()); }

    const PresetInfo& getPresetInfo (int index) const;
    const PresetRecord* getPresetRecord (int index) const;
    bool isFactoryPreset (int index) const;
    juce::ValueTree getPresetState (int index) const;

    // Current selection and persistence location.
    int getCurrentPresetIndex() const { return currentPresetIndex; }
    juce::String getCurrentPresetName() const { return currentPresetName; }
    void clearCurrentPreset();
    const juce::File& getPresetFolder() const { return presetFolder; }

    // Favorites.
    void setFavorite (int index, bool favorite);
    bool isFavorite (int index) const;

    // Utilities.
    int findPresetIndexByName (const juce::String& name) const;
    juce::String getLastError() const { return lastError; }
    void setChangeListener (ChangeCallback callback) { changeListener = std::move (callback); }
    void setStatusListener (StatusCallback callback) { statusListener = std::move (callback); }

    static juce::String categoryName (PresetCategory category);

private:
    struct ParameterOverride
    {
        juce::String id;
        float value { 0.0f };
    };

    struct FactoryPresetSpec
    {
        juce::String name;
        juce::String description;
        PresetCategory category { PresetCategory::init };
        juce::StringArray tags;
        std::vector<ParameterOverride> overrides;
    };

    juce::AudioProcessorValueTreeState* apvts { nullptr };
    juce::File presetFolder;
    std::vector<PresetRecord> factoryPresets;
    std::vector<PresetRecord> userPresets;

    int currentPresetIndex { -1 };
    juce::String currentPresetName { "Untitled Patch" };
    juce::String lastError;
    ChangeCallback changeListener;
    StatusCallback statusListener;

    void reindex();
    void notifyChange();
    void reportStatus (const juce::String& message, bool success);
    void setParameterValue (juce::ValueTree& state, const juce::String& parameterId, float value) const;
    juce::ValueTree createFactoryState (const FactoryPresetSpec& spec) const;
    bool writePresetFile (const PresetRecord& record);
    bool readPresetFile (const juce::File& file, PresetRecord& record) const;
    static juce::StringArray cleanTags (const juce::StringArray& tags);
    static PresetCategory categoryFromName (const juce::String& name, PresetCategory fallback);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetManager)
};
