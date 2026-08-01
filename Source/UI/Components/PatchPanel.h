/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#pragma once

#include <JuceHeader.h>
#include <vector>

enum class JackType
{
    input,
    output,
    audio,
    cv,
    gate,
    mixed
};

struct PatchJack
{
    juce::String id;
    juce::String name;
    JackType type { JackType::cv };
    juce::Colour colour { juce::Colours::white };
    juce::Point<float> position;
    bool isInput { true };
};

struct PatchCable
{
    juce::String sourceId;
    juce::String destId;
    juce::Colour colour { juce::Colours::yellow };
    float thickness { 2.0f };
};

class PatchJackComponent : public juce::Component
{
public:
    PatchJackComponent (const PatchJack& jack);
    ~PatchJackComponent() override = default;

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;

    const juce::String& getId() const { return jack.id; }

private:
    PatchJack jack;
    bool isDragging { false };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PatchJackComponent)
};

class PatchPanel : public juce::Component
{
public:
    PatchPanel();
    ~PatchPanel() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void addJack (const PatchJack& jack);
    void addCable (const juce::String& sourceId, const juce::String& destId);
    void clearCables();

    std::vector<juce::String> getConnectionsForSource (const juce::String& sourceId) const;

private:
    std::vector<std::unique_ptr<PatchJackComponent>> jacks;
    std::vector<PatchCable> cables;

    PatchJackComponent* jackUnderMouse { nullptr };
    PatchJackComponent* draggingCableFrom { nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PatchPanel)
};