/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Effects chain supporting dynamic insertion, removal,
 *              and reordering of effects. Spring Reverb has been
 *              replaced by the permanent Tape Delay in AudioEngine.
 */

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <memory>
#include "EffectTypes.h"
#include "Chorus.h"
#include "Delay.h"
#include "BitCrusher.h"

namespace fx
{

// Additional effect types beyond the basic ones
enum class VintageEffectType
{
    tapeDelay,
    chorus,
    ensemble,
    phaser,
    flanger
};

enum class ModernEffectType
{
    shimmerReverb,
    granularDelay,
    multiBandSat,
    compressor,
    limiter,
    eq
};

// A generic effect slot holding any effect
struct EffectSlot
{
    int id { 0 };
    juce::String name;
    bool bypassed { false };
    float mix { 1.0f };

    enum class Category { vintage, modern } category { Category::vintage };

    union
    {
        VintageEffectType vintageType { VintageEffectType::chorus };
        ModernEffectType modernType;
    };

    // Effect-specific parameters stored as generic floats
    std::array<float, 8> params { 0.0f };
};

class EffectsChain
{
public:
    EffectsChain() = default;
    ~EffectsChain() = default;

    void prepare (double sampleRate, int maxBlockSize, int numChannels);
    void reset();

    // Chain management
    int addEffect (EffectSlot slot);
    void removeEffect (int id);
    void moveEffect (int fromIndex, int toIndex);
    void clear();
    void setBypassed (int id, bool bypassed);

    // Parameter access
    EffectSlot* getSlot (int index);
    int getNumSlots() const { return (int) slots.size(); }

    // Process the full chain
    void process (juce::AudioBuffer<float>& buffer);

    // Get slots for UI
    const std::vector<EffectSlot>& getSlots() const { return slots; }

    // Access internal effects for parameter updates
    struct InternalEffect
    {
        EffectSlot slot;
        std::unique_ptr<fx::Chorus> chorus;
        std::unique_ptr<fx::Delay> delay;
        std::unique_ptr<fx::BitCrusher> bitcrusher;
    };
    std::vector<InternalEffect>& getInternalEffects() { return effects; }

private:
    std::vector<InternalEffect> effects;
    std::vector<EffectSlot> slots;

    double sampleRate { 44100.0 };
    int maxBlockSize { 512 };
    int numChannels { 2 };
    int nextId { 1 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EffectsChain)
};

} // namespace fx
