/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Logical audio, CV, and MIDI routing for the patch panel.
 * Maps visual cable connections to DSP signal paths.
 */

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <array>
#include <functional>

// Jack types on the patch panel
enum class JackType
{
    audioOutput,
    audioInput,
    cvOutput,
    cvInput,
    midiOutput,
    midiInput,
    mpeOutput,
    mpeInput
};

// Cable color
enum class CableColor
{
    yellow,   // Audio
    cyan,     // Modulation
    magenta,  // MIDI
    white     // MPE
};

struct JackID
{
    int moduleIndex { 0 };    // Which module (0=osc1, 1=osc2, 2=filter, etc.)
    int jackIndex { 0 };      // Which jack on that module
    JackType type { JackType::audioInput };
    juce::String name;       // Display name

    bool operator==(const JackID& o) const
    {
        return moduleIndex == o.moduleIndex && jackIndex == o.jackIndex && type == o.type;
    }
};

struct PatchCable
{
    JackID source;
    JackID destination;
    CableColor color { CableColor::cyan };
    float amount { 1.0f };    // Modulation amount
    bool active { true };
};

class PatchBay
{
public:
    PatchBay() = default;
    ~PatchBay() = default;

    void clear();
    void addCable (const PatchCable& cable);
    void removeCable (int index);
    void removeCable (const JackID& source, const JackID& destination);
    void setCableAmount (int index, float amount);
    void setCableActive (int index, bool active);

    const std::vector<PatchCable>& getCables() const { return cables; }
    int getNumCables() const { return (int) cables.size(); }

    // Check if a jack is connected
    bool isJackConnected (const JackID& jack) const;

    // Get modulation value for a destination (sum of all connected sources)
    float getModulationValue (const JackID& destination) const;

    // Set source values (call before getModulationValue)
    void setSourceValue (const JackID& source, float value);

    // Get all cables connected to a jack
    std::vector<const PatchCable*> getCablesForJack (const JackID& jack) const;

private:
    std::vector<PatchCable> cables;
    std::array<float, 64> sourceValues; // Cached source values
};

