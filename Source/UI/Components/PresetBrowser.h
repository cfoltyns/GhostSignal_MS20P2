/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#pragma once

#include <JuceHeader.h>
#include "../../Core/PresetManager.h"
#include <algorithm>
#include <functional>
#include <vector>

class PresetBrowserPanel final : public juce::Component,
                                 private juce::ListBoxModel
{
public:
    explicit PresetBrowserPanel (PresetManager& managerIn);
    ~PresetBrowserPanel() override;

    void refresh();
    void setSelectedIndex (int combinedIndex);
    int getSelectedPresetIndex() const { return selectedIndex; }
    void setSearchText (const juce::String& text);

    std::function<void (int)> onLoadPreset;
    std::function<void()> onSavePreset;
    std::function<void (int)> onRenamePreset;
    std::function<void (int)> onDeletePreset;
    std::function<void (int)> onToggleFavorite;
    std::function<void()> onClose;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    int getCombinedIndexForRow (int row) const;
    void updateButtons();
    bool matchesFilter (const PresetInfo& info, const juce::String& query) const;
    int getNumRows() override;
    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void selectedRowsChanged (int lastRowSelected) override;
    void listBoxItemDoubleClicked (int rowNumber, const juce::MouseEvent&) override;
    void returnKeyPressed (int lastRowSelected) override;

    PresetManager& manager;
    juce::ListBox listBox { juce::String(), this };
    juce::TextEditor searchBox { juce::String() };
    juce::Label searchHint { juce::String(), "filter" };
    juce::TextButton loadButton { "load" };
    juce::TextButton saveButton { "save" };
    juce::TextButton renameButton { "rename" };
    juce::TextButton deleteButton { "delete" };
    juce::TextButton favoriteButton { "fav" };
    juce::TextButton closeButton { "close" };
    juce::Label titleLabel { juce::String(), "presets" };
    juce::Label subtitleLabel { juce::String(), juce::String() };
    std::vector<int> visibleIndices;
    int selectedIndex { -1 };

    // Text-only button rendering: keeps the action row free of chrome.
    // Held by unique_ptr because the concrete type lives in the .cpp.
    std::unique_ptr<juce::LookAndFeel> flatButtonLookAndFeel;
};
