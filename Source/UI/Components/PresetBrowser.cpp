/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#include "PresetBrowser.h"

#include "MinimalStyle.h"

// The preset surfaces share one palette, metric set and type scale, defined
// once in MinimalStyle.h so the browser and the save/rename dialog cannot
// drift apart. Every shared name is qualified with its namespace so the
// source of each value stays obvious at the call site.

PresetBrowserPanel::PresetBrowserPanel (PresetManager& managerIn)
    : manager (managerIn)
{
    // The panel paints an opaque flat surface across its whole bounds, so JUCE
    // can skip repainting everything behind it.
    setOpaque (true);
    setWantsKeyboardFocus (true);

    // Heading: small, tracked, lowercase. It reads as a window label rather
    // than a title, which keeps the whole surface flat.
    titleLabel.setFont (MinimalStyle::getTrackedMonoFont (10.0f));
    titleLabel.setColour (juce::Label::textColourId, MinimalStyle::kMutedColour);
    addAndMakeVisible (titleLabel);

    subtitleLabel.setFont (MinimalStyle::getMonoFont (9.5f));
    subtitleLabel.setColour (juce::Label::textColourId, MinimalStyle::kFaintColour);
    addAndMakeVisible (subtitleLabel);

    // Search: no box, no border, no fill. A hairline rule is drawn underneath it
    // in paint(), so the editor reads as a bare input line on the surface.
    searchBox.setFont (MinimalStyle::getMonoFont (11.0f));
    searchBox.setColour (juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    searchBox.setColour (juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    searchBox.setColour (juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    searchBox.setColour (juce::TextEditor::shadowColourId, juce::Colours::transparentBlack);
    searchBox.setColour (juce::TextEditor::textColourId, MinimalStyle::kTextColour);
    searchBox.setColour (juce::TextEditor::highlightColourId, MinimalStyle::kRowSelectedFill);
    searchBox.setJustification (juce::Justification::centredLeft);
    searchBox.setIndents (0, 0);
    searchBox.setBorder (juce::BorderSize<int> (0));
    searchBox.onTextChange = [this]
    {
        searchHint.setVisible (searchBox.getText().isEmpty());
        refresh();
    };
    addAndMakeVisible (searchBox);

    searchHint.setFont (MinimalStyle::getMonoFont (10.5f));
    searchHint.setColour (juce::Label::textColourId, MinimalStyle::kFaintColour);
    searchHint.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (searchHint);

    // Single-line rows, no list border: the rows themselves are the only
    // structure in the middle of the panel.
    listBox.setRowHeight (MinimalStyle::kRowHeight);
    listBox.setOutlineThickness (0);
    listBox.setColour (juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
    listBox.setColour (juce::ListBox::outlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (listBox);

    // Actions are rendered as bare text by MinimalStyle::FlatTextButtonLookAndFeel, so no
    // fill or border colours are set here - only the look and feel is applied.
    // The borrower outlives this panel, so reset it in the destructor.
    flatButtonLookAndFeel = std::make_unique<MinimalStyle::FlatTextButtonLookAndFeel>();

    const auto configureButton = [this] (juce::TextButton& button)
    {
        button.setLookAndFeel (flatButtonLookAndFeel.get());
        button.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        button.setColour (juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
    };
    configureButton (loadButton);
    configureButton (saveButton);
    configureButton (renameButton);
    configureButton (deleteButton);
    configureButton (favoriteButton);
    configureButton (closeButton);

    loadButton.onClick = [this]
    {
        if (selectedIndex >= 0 && onLoadPreset)
            onLoadPreset (selectedIndex);
    };
    saveButton.onClick = [this]
    {
        if (onSavePreset)
            onSavePreset();
    };
    renameButton.onClick = [this]
    {
        if (selectedIndex >= 0 && onRenamePreset)
            onRenamePreset (selectedIndex);
    };
    deleteButton.onClick = [this]
    {
        if (selectedIndex >= 0 && onDeletePreset)
            onDeletePreset (selectedIndex);
    };
    favoriteButton.onClick = [this]
    {
        if (selectedIndex >= 0 && onToggleFavorite)
            onToggleFavorite (selectedIndex);
    };
    closeButton.onClick = [this]
    {
        if (onClose)
            onClose();
    };

    addAndMakeVisible (loadButton);
    addAndMakeVisible (saveButton);
    addAndMakeVisible (renameButton);
    addAndMakeVisible (deleteButton);
    addAndMakeVisible (favoriteButton);
    addAndMakeVisible (closeButton);
    refresh();
}

PresetBrowserPanel::~PresetBrowserPanel()
{
    // The buttons borrow flatButtonLookAndFeel, which is declared last in the
    // header and is therefore destroyed first. Detach it explicitly so no
    // button is left holding a pointer to a dead look and feel.
    for (auto* button : { &loadButton, &saveButton, &renameButton,
                          &deleteButton, &favoriteButton, &closeButton })
        button->setLookAndFeel (nullptr);
}

void PresetBrowserPanel::refresh()
{
    const juce::String selectedId = selectedIndex >= 0 && selectedIndex < manager.getNumPresets()
                                        ? manager.getPresetInfo (selectedIndex).id
                                        : juce::String();
    const auto query = searchBox.getText().trim().toLowerCase();
    visibleIndices.clear();

    for (int i = 0; i < manager.getNumPresets(); ++i)
    {
        if (matchesFilter (manager.getPresetInfo (i), query))
            visibleIndices.push_back (i);
    }

    selectedIndex = -1;
    if (selectedId.isNotEmpty())
    {
        for (size_t i = 0; i < visibleIndices.size(); ++i)
        {
            if (manager.getPresetInfo (visibleIndices[i]).id.compareIgnoreCase (selectedId) == 0)
            {
                selectedIndex = visibleIndices[i];
                break;
            }
        }
    }
    if (selectedIndex < 0 && !visibleIndices.empty())
        selectedIndex = visibleIndices.front();

    listBox.updateContent();
    if (selectedIndex >= 0)
        listBox.selectRow (static_cast<int> (std::distance (visibleIndices.begin(),
                                                            std::find (visibleIndices.begin(),
                                                                       visibleIndices.end(),
                                                                       selectedIndex))),
                           false,
                           false);

    // Compact counters only: "shown/total". No labels, no units - the numbers
    // are self-evident next to the list.
    subtitleLabel.setText (juce::String (visibleIndices.size()) + "/"
                               + juce::String (manager.getNumPresets()),
                           juce::dontSendNotification);
    updateButtons();
}

void PresetBrowserPanel::setSelectedIndex (int combinedIndex)
{
    const auto it = std::find (visibleIndices.begin(), visibleIndices.end(), combinedIndex);
    if (it == visibleIndices.end())
        return;

    const int row = static_cast<int> (std::distance (visibleIndices.begin(), it));
    selectedIndex = combinedIndex;
    listBox.selectRow (row, false, false);
    updateButtons();
}

void PresetBrowserPanel::setSearchText (const juce::String& text)
{
    searchBox.setText (text, juce::dontSendNotification);
    refresh();
}

void PresetBrowserPanel::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds();
    if (bounds.isEmpty())
        return;

    // Flat, square, near-black surface: no rounding, no gradient, no panel
    // fill. A single hairline rectangle is the only edge treatment.
    g.fillAll (MinimalStyle::kSurfaceColour);
    g.setColour (MinimalStyle::kHairlineColour);
    g.drawRect (bounds, 1);

    // Rule under the search line, so the editor reads as a bare input field
    // sitting directly on the surface rather than as a bezelled box.
    const auto searchBounds = searchBox.getBounds();
    if (! searchBounds.isEmpty())
        g.fillRect (searchBounds.getX(), searchBounds.getBottom(),
                    searchBounds.getWidth(), 1);

    // Rule above the action row. Whitespace and this one line are what
    // separate the list from the actions.
    if (loadButton.getWidth() > 0)
    {
        const int ruleY = loadButton.getBounds().getY() - MinimalStyle::kActionGap / 2;
        g.fillRect (bounds.getX() + MinimalStyle::kPad, ruleY,
                    juce::jmax (0, bounds.getWidth() - 2 * MinimalStyle::kPad), 1);
    }
}

void PresetBrowserPanel::resized()
{
    auto area = getLocalBounds().reduced (MinimalStyle::kPad);
    if (area.isEmpty())
        return;

    titleLabel.setBounds (area.removeFromTop (16));
    area.removeFromTop (2);
    subtitleLabel.setBounds (area.removeFromTop (14));
    area.removeFromTop (MinimalStyle::kPad);

    // The search editor shares its bounds with the hint text; the editor has no
    // indents of its own, so the two line up exactly.
    searchBox.setBounds (area.removeFromTop (20));
    searchHint.setBounds (searchBox.getBounds());
    searchHint.setVisible (searchBox.getText().isEmpty());
    area.removeFromTop (MinimalStyle::kPad);

    // Action row: bare lowercase labels, right-aligned, separated by
    // whitespace alone. Fixed widths keep the row from reflowing when a label
    // changes between "fav" and "unfav".
    auto actionArea = area.removeFromBottom (20);
    area.removeFromBottom (MinimalStyle::kPad);

    struct ActionSpec { juce::TextButton* button; int width; };
    const ActionSpec actions[] =
    {
        { &loadButton,     44 },
        { &saveButton,     44 },
        { &renameButton,   58 },
        { &deleteButton,   58 },
        { &favoriteButton, 46 },
        { &closeButton,    44 }
    };
    const int numActions = static_cast<int> (sizeof (actions) / sizeof (actions[0]));

    int totalWidth = MinimalStyle::kActionGap * (numActions - 1);
    for (const auto& action : actions)
        totalWidth += action.width;

    int x = juce::jmax (actionArea.getX(), actionArea.getRight() - totalWidth);
    for (const auto& action : actions)
    {
        action.button->setBounds (x, actionArea.getY(), action.width, actionArea.getHeight());
        x += action.width + MinimalStyle::kActionGap;
    }

    listBox.setBounds (area);
}

int PresetBrowserPanel::getCombinedIndexForRow (int row) const
{
    if (row < 0 || row >= static_cast<int> (visibleIndices.size()))
        return -1;
    return visibleIndices[static_cast<size_t> (row)];
}

void PresetBrowserPanel::updateButtons()
{
    const bool hasSelection = selectedIndex >= 0 && selectedIndex < manager.getNumPresets();
    const bool userSelected = hasSelection && !manager.isFactoryPreset (selectedIndex);
    loadButton.setEnabled (hasSelection);
    saveButton.setEnabled (true);
    renameButton.setEnabled (userSelected);
    deleteButton.setEnabled (userSelected);
    favoriteButton.setEnabled (userSelected);
    favoriteButton.setButtonText (hasSelection && manager.isFavorite (selectedIndex) ? "UNFAV" : "FAV");
}

bool PresetBrowserPanel::matchesFilter (const PresetInfo& info, const juce::String& query) const
{
    if (query.isEmpty())
        return true;

    const auto matchesText = [query] (const juce::String& text)
    {
        return text.toLowerCase().contains (query);
    };
    if (matchesText (info.name) || matchesText (info.description)
        || matchesText (PresetManager::categoryName (info.category)))
        return true;
    for (const auto& tag : info.tags)
    {
        if (matchesText (tag))
            return true;
    }
    return false;
}

int PresetBrowserPanel::getNumRows()
{
    return static_cast<int> (visibleIndices.size());
}

void PresetBrowserPanel::paintListBoxItem (int rowNumber,
                                           juce::Graphics& g,
                                           int width,
                                           int height,
                                           bool rowIsSelected)
{
    const auto index = getCombinedIndexForRow (rowNumber);
    if (index < 0)
        return;

    const auto& info = manager.getPresetInfo (index);
    const juce::Rectangle<int> row (0, 0, width, height);

    if (rowIsSelected)
    {
        // Selection is a whisper of fill and nothing else - no marker bar, no
        // full-bleed highlight, no rounded pill. The row text brightens too.
        g.setColour (MinimalStyle::kRowSelectedFill);
        g.fillRect (row);
    }

    // Meta column: lowercase, muted, right-aligned. Factory/user is the one
    // distinction the action row reacts to, so it is the only metadata kept.
    const juce::String meta (info.factory ? "factory" : "user");
    const int metaWidth = 52;
    const int metaX = juce::jmax (MinimalStyle::kPad, width - MinimalStyle::kPad - metaWidth);

    g.setColour (rowIsSelected ? MinimalStyle::kTextColour : MinimalStyle::kRowTextColour);
    g.setFont (MinimalStyle::getMonoFont (11.0f));
    g.drawFittedText (info.name,
                      MinimalStyle::kPad, 0,
                      juce::jmax (10, metaX - MinimalStyle::kPad - 20), height,
                      juce::Justification::centredLeft,
                      1);

    g.setColour (rowIsSelected ? MinimalStyle::kMutedColour : MinimalStyle::kFaintColour);
    g.setFont (MinimalStyle::getMonoFont (9.0f));
    g.drawFittedText (meta,
                      metaX, 0, metaWidth, height,
                      juce::Justification::centredRight,
                      1);

    // Favourites are flagged with a monospace asterisk rather than a coloured
    // dot, so the list stays monochrome.
    if (manager.isFavorite (index))
    {
        g.setColour (rowIsSelected ? MinimalStyle::kTextColour : MinimalStyle::kMutedColour);
        g.setFont (MinimalStyle::getMonoFont (10.0f));
        g.drawFittedText ("*",
                          metaX - 16, 0, 10, height,
                          juce::Justification::centredRight,
                          1);
    }
}

void PresetBrowserPanel::selectedRowsChanged (int)
{
    const int row = listBox.getSelectedRow();
    selectedIndex = getCombinedIndexForRow (row);
    updateButtons();
}

void PresetBrowserPanel::listBoxItemDoubleClicked (int rowNumber, const juce::MouseEvent&)
{
    const int index = getCombinedIndexForRow (rowNumber);
    if (index >= 0 && onLoadPreset)
        onLoadPreset (index);
}

void PresetBrowserPanel::returnKeyPressed (int rowNumber)
{
    const int index = getCombinedIndexForRow (rowNumber);
    if (index >= 0 && onLoadPreset)
        onLoadPreset (index);
}
