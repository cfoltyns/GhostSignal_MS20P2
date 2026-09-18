/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#include "PresetManager.h"

namespace
{
    constexpr int kPresetNameLimit = 64;
    constexpr int kDescriptionLimit = 240;
    constexpr int kTagLimit = 8;
}

void PresetManager::prepare (juce::AudioProcessorValueTreeState& state)
{
    apvts = &state;

    presetFolder = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                       .getChildFile ("GhostSignal/MS20P/Presets");
    presetFolder.createDirectory();

    loadFactoryPresets();
    refreshUserPresets();
    clearCurrentPreset();
}

bool PresetManager::applyState (const juce::ValueTree& state)
{
    if (apvts == nullptr || !state.isValid())
    {
        lastError = "Preset state is unavailable.";
        return false;
    }

    if (state.getType() != apvts->state.getType())
    {
        lastError = "Preset state has an incompatible root type.";
        return false;
    }

    // Keep the existing root object so APVTS parameter adapters remain valid.
    // Replacing its properties and children triggers the public ValueTree
    // notifications that reconnect each adapter to the new PARAM nodes.
    apvts->state.copyPropertiesAndChildrenFrom (state, nullptr);
    apvts->copyState();
    return true;
}

void PresetManager::loadFactoryPresets()
{
    factoryPresets.clear();
    if (apvts == nullptr)
        return;

    std::vector<FactoryPresetSpec> specs;
    specs.reserve (7);
    specs.push_back ({ "Init Patch",
                       "A clean, neutral starting point for building a patch.",
                       PresetCategory::init,
                       juce::StringArray { "init", "clean", "neutral" },
                       {} });
    specs.push_back ({ "MS20 Bass",
                       "Dense low-end saw bass with a tight envelope.",
                       PresetCategory::bass,
                       juce::StringArray { "bass", "low", "saw" },
                       { { "paramOsc1Waveform", 0.0f },
                         { "paramOsc2Waveform", 0.0f },
                         { "paramOsc1Octave", -1.0f },
                         { "paramOsc2Octave", -1.0f },
                         { "paramOsc1Gain", 0.85f },
                         { "paramOsc2Gain", 0.55f },
                         { "paramNoiseGain", 0.08f },
                         { "paramLPFCutoff", 900.0f },
                         { "paramLPFRes", 0.28f },
                         { "paramAmpAttack", 0.005f },
                         { "paramAmpDecay", 0.28f },
                         { "paramAmpSustain", 0.78f },
                         { "paramAmpRelease", 0.14f },
                         { "paramMixerDrive", 0.12f },
                         { "paramTapeDelayEnable", 0.0f } } });
    specs.push_back ({ "Aggressive Lead",
                       "Forward mono lead with drive, glide, and a bright filter.",
                       PresetCategory::lead,
                       juce::StringArray { "lead", "mono", "drive", "glide" },
                       { { "paramVoiceMode", 0.0f },
                         { "paramOsc1Waveform", 0.0f },
                         { "paramOsc2Waveform", 1.0f },
                         { "paramOsc1Octave", 0.0f },
                         { "paramOsc2Octave", 0.0f },
                         { "paramGlideTime", 0.12f },
                         { "paramOsc1Gain", 0.8f },
                         { "paramOsc2Gain", 0.65f },
                         { "paramLPFCutoff", 3600.0f },
                         { "paramLPFRes", 0.58f },
                         { "paramAmpAttack", 0.005f },
                         { "paramAmpDecay", 0.32f },
                         { "paramAmpSustain", 0.72f },
                         { "paramAmpRelease", 0.16f },
                         { "paramMixerDrive", 0.38f },
                         { "paramTapeDelayEnable", 1.0f } } });
    specs.push_back ({ "Space Pad",
                       "Slow, wide pad voicing with a soft filter and long release.",
                       PresetCategory::pad,
                       juce::StringArray { "pad", "poly", "slow", "wide" },
                       { { "paramVoiceMode", 1.0f },
                         { "paramOsc1Waveform", 0.0f },
                         { "paramOsc2Waveform", 2.0f },
                         { "paramOsc1Octave", 0.0f },
                         { "paramOsc2Octave", 1.0f },
                         { "paramOsc1Gain", 0.62f },
                         { "paramOsc2Gain", 0.48f },
                         { "paramLPFCutoff", 1800.0f },
                         { "paramLPFRes", 0.34f },
                         { "paramAmpAttack", 1.15f },
                         { "paramAmpDecay", 0.72f },
                         { "paramAmpSustain", 0.82f },
                         { "paramAmpRelease", 2.45f },
                         { "paramMixerDrive", 0.18f },
                         { "paramTapeDelayEnable", 1.0f } } });
    specs.push_back ({ "Plucky Stab",
                       "Short percussive stabs with a fast filter envelope.",
                       PresetCategory::pluck,
                       juce::StringArray { "pluck", "stab", "percussive" },
                       { { "paramVoiceMode", 1.0f },
                         { "paramOsc1Waveform", 1.0f },
                         { "paramOsc2Waveform", 0.0f },
                         { "paramOsc1Octave", 1.0f },
                         { "paramOsc2Octave", 0.0f },
                         { "paramOsc1Gain", 0.72f },
                         { "paramOsc2Gain", 0.42f },
                         { "paramLPFCutoff", 2600.0f },
                         { "paramLPFRes", 0.42f },
                         { "paramAmpAttack", 0.003f },
                         { "paramAmpDecay", 0.18f },
                         { "paramAmpSustain", 0.28f },
                         { "paramAmpRelease", 0.08f },
                         { "paramMixerDrive", 0.22f },
                         { "paramTapeDelayEnable", 1.0f } } });
    specs.push_back ({ "Noise FX",
                       "Filtered noise texture with delay feedback and tape character.",
                       PresetCategory::fx,
                       juce::StringArray { "noise", "fx", "texture", "delay" },
                       { { "paramOsc1Gain", 0.0f },
                         { "paramOsc2Gain", 0.0f },
                         { "paramNoiseGain", 0.82f },
                         { "paramNoiseType", 2.0f },
                         { "paramHPFCutoff", 340.0f },
                         { "paramLPFCutoff", 7200.0f },
                         { "paramLPFRes", 0.36f },
                         { "paramAmpAttack", 0.04f },
                         { "paramAmpDecay", 0.62f },
                         { "paramAmpSustain", 0.62f },
                         { "paramAmpRelease", 0.48f },
                         { "paramMixerDrive", 0.48f },
                         { "paramTapeDelayEnable", 1.0f },
                         { "paramTapeDelayTime", 380.0f },
                         { "paramTapeDelayFeedback", 0.62f } } });
    specs.push_back ({ "Arp Sequence",
                       "Monophonic sequence-ready patch with a rhythmic filter shape.",
                       PresetCategory::arp,
                       juce::StringArray { "arp", "sequence", "rhythmic", "mono" },
                       { { "paramVoiceMode", 0.0f },
                         { "paramOsc1Waveform", 0.0f },
                         { "paramOsc2Waveform", 1.0f },
                         { "paramOsc1Octave", 0.0f },
                         { "paramOsc2Octave", 1.0f },
                         { "paramGlideTime", 0.08f },
                         { "paramLFO1Rate", 0.32f },
                         { "paramLFO1Depth", 0.24f },
                         { "paramLPFCutoff", 2200.0f },
                         { "paramLPFRes", 0.48f },
                         { "paramAmpAttack", 0.008f },
                         { "paramAmpDecay", 0.22f },
                         { "paramAmpSustain", 0.55f },
                         { "paramAmpRelease", 0.12f },
                         { "paramMixerDrive", 0.28f },
                         { "paramTapeDelayEnable", 1.0f },
                         { "paramTapeDelayTime", 250.0f },
                         { "paramTapeDelayFeedback", 0.34f } } });

    const auto now = juce::Time::getCurrentTime();
    for (const auto& spec : specs)
    {
        PresetRecord record;
        record.info.id = juce::Uuid().toString();
        record.info.name = spec.name;
        record.info.description = spec.description;
        record.info.author = "Ghost Signal";
        record.info.category = spec.category;
        record.info.tags = spec.tags;
        record.info.index = -1;
        record.info.created = now;
        record.info.modified = now;
        record.info.date = now;
        record.info.favorite = false;
        record.info.factory = true;
        record.info.fileName = record.info.name + ".factory";
        record.state = createFactoryState (spec);
        factoryPresets.push_back (std::move (record));
    }

    reindex();
}

void PresetManager::refreshUserPresets()
{
    userPresets.clear();
    if (!presetFolder.isDirectory())
        return;

    juce::Array<juce::File> files;
    presetFolder.findChildFiles (files, juce::File::findFiles, false, "*.preset");

    struct FileModifiedComparator
    {
        int compareElements (const juce::File& a, const juce::File& b) const noexcept
        {
            return a.getLastModificationTime() == b.getLastModificationTime()
                       ? 0
                       : (a.getLastModificationTime() > b.getLastModificationTime() ? -1 : 1);
        }
    };

    FileModifiedComparator comparator;
    files.sort (comparator);

    for (const auto& file : files)
    {
        PresetRecord record;
        if (readPresetFile (file, record))
            userPresets.push_back (std::move (record));
        else
            lastError = "Skipped unreadable preset: " + file.getFileName();
    }

    reindex();

    if (currentPresetIndex >= 0 && currentPresetIndex < getNumPresets())
    {
        const auto id = getPresetInfo (currentPresetIndex).id;
        int found = -1;
        for (int i = 0; i < getNumPresets(); ++i)
        {
            if (getPresetInfo (i).id.compareIgnoreCase (id) == 0)
            {
                found = i;
                break;
            }
        }
        currentPresetIndex = found;
        currentPresetName = found >= 0 ? getPresetInfo (found).name : "Untitled Patch";
    }
    else
    {
        clearCurrentPreset();
    }
}

bool PresetManager::loadPreset (int index)
{
    if (index < 0 || index >= getNumPresets() || apvts == nullptr)
    {
        lastError = "Preset index is out of range.";
        reportStatus (lastError, false);
        return false;
    }

    const auto* record = getPresetRecord (index);
    if (record == nullptr || !record->state.isValid())
    {
        lastError = "Preset state is unavailable.";
        reportStatus (lastError, false);
        return false;
    }

    if (!applyState (record->state))
    {
        reportStatus (lastError, false);
        return false;
    }

    currentPresetIndex = index;
    currentPresetName = record->info.name;
    reportStatus (record->info.name + " loaded.", true);
    notifyChange();
    return true;
}

bool PresetManager::savePreset (const juce::String& name,
                                const juce::String& description,
                                const juce::StringArray& tags,
                                int overwriteIndex)
{
    if (apvts == nullptr)
    {
        lastError = "Preset storage is not ready.";
        reportStatus (lastError, false);
        return false;
    }

    auto cleanName = name.trim();
    for (int i = 0; i < cleanName.length(); ++i)
    {
        if (cleanName[i] < 32)
        {
            lastError = "Preset names cannot contain control characters.";
            reportStatus (lastError, false);
            return false;
        }
    }

    if (cleanName.isEmpty() || cleanName.length() > kPresetNameLimit)
    {
        lastError = "Enter a preset name between 1 and " + juce::String (kPresetNameLimit) + " characters.";
        reportStatus (lastError, false);
        return false;
    }

    auto cleanDescription = description.trim();
    if (cleanDescription.length() > kDescriptionLimit)
        cleanDescription = cleanDescription.substring (0, kDescriptionLimit);

    auto parsedTags = cleanTags (tags);
    auto state = apvts->copyState();
    state.setProperty ("presetFormatVersion", 1, nullptr);

    int userIndex = -1;
    if (overwriteIndex >= 0 && overwriteIndex < getNumPresets())
    {
        if (isFactoryPreset (overwriteIndex))
        {
            lastError = "Factory presets cannot be overwritten.";
            reportStatus (lastError, false);
            return false;
        }
        userIndex = overwriteIndex - getNumFactoryPresets();
        const auto& existing = userPresets[static_cast<size_t> (userIndex)];
        if (existing.info.name.compareIgnoreCase (cleanName) != 0)
        {
            for (size_t i = 0; i < userPresets.size(); ++i)
            {
                if (i != static_cast<size_t> (userIndex)
                    && userPresets[i].info.name.compareIgnoreCase (cleanName) == 0)
                {
                    lastError = "A user preset with this name already exists.";
                    reportStatus (lastError, false);
                    return false;
                }
            }
        }
    }
    else
    {
        for (size_t i = 0; i < userPresets.size(); ++i)
        {
            if (userPresets[i].info.name.compareIgnoreCase (cleanName) == 0)
            {
                lastError = "A user preset with this name already exists.";
                reportStatus (lastError, false);
                return false;
            }
        }
    }

    PresetRecord record;
    if (userIndex >= 0)
    {
        record = userPresets[static_cast<size_t> (userIndex)];
        record.info.name = cleanName;
        record.info.description = cleanDescription;
        record.info.tags = parsedTags;
        record.info.modified = juce::Time::getCurrentTime();
        record.info.date = record.info.modified;
        record.state = state;
        if (!writePresetFile (record))
        {
            reportStatus (lastError, false);
            return false;
        }
        userPresets[static_cast<size_t> (userIndex)] = std::move (record);
    }
    else
    {
        const auto now = juce::Time::getCurrentTime();
        record.info.id = juce::Uuid().toString();
        record.info.name = cleanName;
        record.info.description = cleanDescription;
        record.info.author = "Ghost Signal";
        record.info.category = PresetCategory::user;
        record.info.tags = parsedTags;
        record.info.created = now;
        record.info.modified = now;
        record.info.date = now;
        record.info.factory = false;
        record.info.fileName = record.info.id + ".preset";
        record.state = state;
        record.file = presetFolder.getChildFile (record.info.fileName);
        if (!writePresetFile (record))
        {
            reportStatus (lastError, false);
            return false;
        }
        userPresets.push_back (std::move (record));
    }

    reindex();
    currentPresetIndex = getNumFactoryPresets()
                         + (userIndex >= 0 ? userIndex : static_cast<int> (userPresets.size() - 1));
    currentPresetName = cleanName;
    reportStatus (cleanName + " saved.", true);
    notifyChange();
    return true;
}

bool PresetManager::savePreset (int index)
{
    if (index < 0 || index >= getNumPresets() || isFactoryPreset (index))
        return false;

    const auto& info = getPresetInfo (index);
    return savePreset (info.name, info.description, info.tags, index);
}

void PresetManager::createPreset (const juce::String& name)
{
    savePreset (name);
}

bool PresetManager::renamePreset (int index,
                                  const juce::String& newName,
                                  const juce::String& newDescription,
                                  const juce::StringArray& newTags)
{
    if (index < getNumFactoryPresets() || index >= getNumPresets())
    {
        lastError = "Factory presets cannot be renamed.";
        reportStatus (lastError, false);
        return false;
    }

    auto cleanName = newName.trim();
    for (int i = 0; i < cleanName.length(); ++i)
    {
        if (cleanName[i] < 32)
        {
            lastError = "Preset names cannot contain control characters.";
            reportStatus (lastError, false);
            return false;
        }
    }

    if (cleanName.isEmpty() || cleanName.length() > kPresetNameLimit)
    {
        lastError = "Enter a preset name between 1 and " + juce::String (kPresetNameLimit) + " characters.";
        reportStatus (lastError, false);
        return false;
    }

    auto cleanDescription = newDescription.trim();
    if (cleanDescription.length() > kDescriptionLimit)
        cleanDescription = cleanDescription.substring (0, kDescriptionLimit);

    const int userIndex = index - getNumFactoryPresets();
    for (size_t i = 0; i < userPresets.size(); ++i)
    {
        if (i != static_cast<size_t> (userIndex)
            && userPresets[i].info.name.compareIgnoreCase (cleanName) == 0)
        {
            lastError = "A user preset with this name already exists.";
            reportStatus (lastError, false);
            return false;
        }
    }

    auto& record = userPresets[static_cast<size_t> (userIndex)];
    record.info.name = cleanName;
    record.info.description = cleanDescription;
    record.info.tags = cleanTags (newTags);
    record.info.modified = juce::Time::getCurrentTime();
    record.info.date = record.info.modified;
    if (!writePresetFile (record))
    {
        reportStatus (lastError, false);
        return false;
    }

    reindex();
    if (currentPresetIndex == index)
        currentPresetName = cleanName;
    reportStatus (cleanName + " renamed.", true);
    notifyChange();
    return true;
}

bool PresetManager::deletePreset (int index)
{
    if (index < getNumFactoryPresets() || index >= getNumPresets())
    {
        lastError = "Factory presets cannot be deleted.";
        reportStatus (lastError, false);
        return false;
    }

    const int userIndex = index - getNumFactoryPresets();
    const auto& record = userPresets[static_cast<size_t> (userIndex)];
    const auto deletedName = record.info.name;
    if (record.file.existsAsFile() && !record.file.deleteFile())
    {
        lastError = "Could not delete " + record.file.getFileName() + ".";
        reportStatus (lastError, false);
        return false;
    }

    const bool wasCurrent = index == currentPresetIndex;
    userPresets.erase (userPresets.begin() + userIndex);
    if (wasCurrent)
        clearCurrentPreset();
    else if (currentPresetIndex > index)
        --currentPresetIndex;

    reindex();
    reportStatus (deletedName + " deleted.", true);
    notifyChange();
    return true;
}

int PresetManager::getRandomPresetIndex() const
{
    if (getNumPresets() == 0)
        return -1;
    return juce::Random::getSystemRandom().nextInt (getNumPresets());
}

const PresetInfo& PresetManager::getPresetInfo (int index) const
{
    static PresetInfo empty;
    if (index < 0 || index >= getNumPresets())
        return empty;
    return getPresetRecord (index)->info;
}

const PresetRecord* PresetManager::getPresetRecord (int index) const
{
    if (index < 0 || index >= getNumPresets())
        return nullptr;
    if (index < getNumFactoryPresets())
        return &factoryPresets[static_cast<size_t> (index)];
    return &userPresets[static_cast<size_t> (index - getNumFactoryPresets())];
}

bool PresetManager::isFactoryPreset (int index) const
{
    return index >= 0 && index < getNumFactoryPresets();
}

juce::ValueTree PresetManager::getPresetState (int index) const
{
    const auto* record = getPresetRecord (index);
    return record == nullptr ? juce::ValueTree() : record->state.createCopy();
}

void PresetManager::clearCurrentPreset()
{
    currentPresetIndex = -1;
    currentPresetName = "Untitled Patch";
}

void PresetManager::setFavorite (int index, bool favorite)
{
    if (index < getNumFactoryPresets() || index >= getNumPresets())
    {
        lastError = "Factory favorites are read-only.";
        reportStatus (lastError, false);
        return;
    }

    auto& record = userPresets[static_cast<size_t> (index - getNumFactoryPresets())];
    record.info.favorite = favorite;
    if (!writePresetFile (record))
    {
        reportStatus (lastError, false);
        return;
    }
    reportStatus (record.info.name + (favorite ? " marked as favorite." : " removed from favorites."), true);
    notifyChange();
}

bool PresetManager::isFavorite (int index) const
{
    const auto* record = getPresetRecord (index);
    return record != nullptr && record->info.favorite;
}

int PresetManager::findPresetIndexByName (const juce::String& name) const
{
    for (int i = 0; i < getNumPresets(); ++i)
    {
        if (getPresetInfo (i).name.compareIgnoreCase (name) == 0)
            return i;
    }
    return -1;
}

void PresetManager::reindex()
{
    for (int i = 0; i < getNumFactoryPresets(); ++i)
        factoryPresets[static_cast<size_t> (i)].info.index = i;
    for (int i = 0; i < getNumUserPresets(); ++i)
        userPresets[static_cast<size_t> (i)].info.index = getNumFactoryPresets() + i;
}

void PresetManager::notifyChange()
{
    if (changeListener)
        changeListener();
}

void PresetManager::reportStatus (const juce::String& message, bool success)
{
    lastError = success ? juce::String() : message;
    if (statusListener)
        statusListener (message, success);
}

void PresetManager::setParameterValue (juce::ValueTree& state,
                                       const juce::String& parameterId,
                                       float value) const
{
    if (apvts == nullptr)
        return;

    auto child = state.getChildWithProperty ("id", parameterId);
    if (!child.isValid())
        return;

    auto* parameter = apvts->getParameter (parameterId);
    const float normalized = parameter == nullptr ? value : parameter->convertTo0to1 (value);
    child.setProperty ("value", normalized, nullptr);
}

juce::ValueTree PresetManager::createFactoryState (const FactoryPresetSpec& spec) const
{
    auto state = apvts->copyState();
    state.setProperty ("presetFormatVersion", 1, nullptr);
    for (const auto& override : spec.overrides)
        setParameterValue (state, override.id, override.value);
    return state;
}

bool PresetManager::writePresetFile (const PresetRecord& record)
{
    presetFolder.createDirectory();

    juce::ValueTree root ("PRESET");
    root.setProperty ("formatVersion", 1, nullptr);
    root.setProperty ("id", record.info.id, nullptr);
    root.setProperty ("name", record.info.name, nullptr);
    root.setProperty ("description", record.info.description, nullptr);
    root.setProperty ("author", record.info.author, nullptr);
    root.setProperty ("category", static_cast<int> (record.info.category), nullptr);
    root.setProperty ("tags", record.info.tags.joinIntoString (", "), nullptr);
    root.setProperty ("created", static_cast<juce::int64> (record.info.created.toMilliseconds()), nullptr);
    root.setProperty ("modified", static_cast<juce::int64> (record.info.modified.toMilliseconds()), nullptr);
    root.setProperty ("favorite", record.info.favorite ? 1 : 0, nullptr);
    root.setProperty ("factory", record.info.factory ? 1 : 0, nullptr);

    juce::ValueTree stateNode ("STATE");
    stateNode.appendChild (record.state.createCopy(), nullptr);
    root.appendChild (stateNode, nullptr);

    auto xml = root.createXml();
    if (xml == nullptr)
    {
        lastError = "Could not serialize preset metadata.";
        return false;
    }

    const auto file = record.file.getFullPathName().isNotEmpty()
                          ? record.file
                          : presetFolder.getChildFile (record.info.id + ".preset");
    if (!file.replaceWithText (xml->toString()))
    {
        lastError = "Could not write " + file.getFileName() + ".";
        return false;
    }
    return true;
}

bool PresetManager::readPresetFile (const juce::File& file, PresetRecord& record) const
{
    std::unique_ptr<juce::XmlElement> xml (juce::XmlDocument::parse (file));
    if (xml == nullptr)
        return false;

    auto root = juce::ValueTree::fromXml (*xml);
    if (!root.isValid())
        return false;

    juce::ValueTree state;
    if (root.getType().toString() == "PARAMETERS")
    {
        state = root;
    }
    else
    {
        auto stateNode = root.getChildWithName ("STATE");
        if (stateNode.isValid() && stateNode.getNumChildren() > 0)
            state = stateNode.getChild (0);
    }

    if (!state.isValid())
        return false;

    const auto now = file.getLastModificationTime();
    record.info.id = root.getProperty ("id").toString();
    if (record.info.id.isEmpty())
        record.info.id = juce::Uuid().toString();
    record.info.name = root.getProperty ("name").toString();
    if (record.info.name.isEmpty())
        record.info.name = file.getFileNameWithoutExtension();
    record.info.description = root.getProperty ("description").toString();
    record.info.author = root.getProperty ("author").toString();
    const auto categoryValue = root.getProperty ("category");
    record.info.category = categoryValue.toString().isNotEmpty()
                               ? categoryFromName (categoryValue.toString(), PresetCategory::user)
                               : PresetCategory::user;
    record.info.tags = cleanTags (juce::StringArray::fromTokens (root.getProperty ("tags").toString(), ",", ""));
    record.info.created = juce::Time (root.getProperty ("created").toString().getLargeIntValue());
    record.info.modified = juce::Time (root.getProperty ("modified").toString().getLargeIntValue());
    if (record.info.created == juce::Time())
        record.info.created = now;
    if (record.info.modified == juce::Time())
        record.info.modified = now;
    record.info.date = record.info.modified;
    record.info.favorite = root.getProperty ("favorite").toString().getIntValue() != 0;
    record.info.factory = false;
    record.info.fileName = file.getFileName();
    record.state = state;
    record.file = file;
    return true;
}

juce::StringArray PresetManager::cleanTags (const juce::StringArray& tags)
{
    juce::StringArray result;
    for (const auto& tag : tags)
    {
        const auto parts = juce::StringArray::fromTokens (tag, ",;", "");
        for (const auto& part : parts)
        {
            const auto clean = part.trim().toLowerCase();
            if (clean.isNotEmpty() && !result.contains (clean, true) && result.size() < kTagLimit)
                result.add (clean);
        }
    }
    return result;
}

juce::String PresetManager::categoryName (PresetCategory category)
{
    switch (category)
    {
        case PresetCategory::init:  return "INIT";
        case PresetCategory::bass:  return "BASS";
        case PresetCategory::lead:  return "LEAD";
        case PresetCategory::pad:   return "PAD";
        case PresetCategory::pluck: return "PLUCK";
        case PresetCategory::fx:    return "FX";
        case PresetCategory::arp:   return "ARP";
        case PresetCategory::drum:  return "DRUM";
        case PresetCategory::user:  return "USER";
        default:                    return "USER";
    }
}

PresetCategory PresetManager::categoryFromName (const juce::String& name, PresetCategory fallback)
{
    const auto lower = name.trim().toLowerCase();
    if (lower.containsOnly ("0123456789")
        && lower.getIntValue() >= static_cast<int> (PresetCategory::init)
        && lower.getIntValue() <= static_cast<int> (PresetCategory::user))
        return static_cast<PresetCategory> (lower.getIntValue());
    if (lower == "init") return PresetCategory::init;
    if (lower == "bass") return PresetCategory::bass;
    if (lower == "lead") return PresetCategory::lead;
    if (lower == "pad") return PresetCategory::pad;
    if (lower == "pluck") return PresetCategory::pluck;
    if (lower == "fx") return PresetCategory::fx;
    if (lower == "arp") return PresetCategory::arp;
    if (lower == "drum") return PresetCategory::drum;
    return fallback;
}
