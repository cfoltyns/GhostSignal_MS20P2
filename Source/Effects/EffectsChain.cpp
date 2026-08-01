/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Effects chain implementation. Spring Reverb has been
 *              removed; the permanent Tape Delay now lives in AudioEngine.
 */

#include "EffectsChain.h"

namespace fx
{

void EffectsChain::prepare (double sr, int blockSize, int channels)
{
    sampleRate = sr;
    maxBlockSize = blockSize;
    numChannels = channels;

    for (auto& fx : effects)
    {
        if (fx.chorus) fx.chorus->prepare (sampleRate);
        if (fx.delay)  fx.delay->prepare (sampleRate);
        if (fx.bitcrusher) fx.bitcrusher->prepare (sampleRate);
    }
}

void EffectsChain::reset()
{
    for (auto& fx : effects)
    {
        if (fx.chorus) fx.chorus->reset();
        if (fx.delay)  fx.delay->reset();
        if (fx.bitcrusher) fx.bitcrusher->reset();
    }
}

int EffectsChain::addEffect (EffectSlot slot)
{
    InternalEffect effect;
    effect.slot = slot;
    effect.slot.id = nextId++;

    switch (slot.category)
    {
        case EffectSlot::Category::vintage:
        {
            switch (slot.vintageType)
            {
                case VintageEffectType::chorus:
                    effect.chorus = std::make_unique<fx::Chorus>();
                    effect.chorus->prepare (sampleRate);
                    break;
                default:
                    break;
            }
            break;
        }
        case EffectSlot::Category::modern:
        {
            // Modern effects will be added as they're implemented
            break;
        }
    }

    effects.push_back (std::move (effect));
    slots.push_back (slot);

    return slot.id;
}

void EffectsChain::removeEffect (int id)
{
    for (auto it = effects.begin(); it != effects.end(); ++it)
    {
        if (it->slot.id == id)
        {
            effects.erase (it);
            break;
        }
    }

    for (auto it = slots.begin(); it != slots.end(); ++it)
    {
        if (it->id == id)
        {
            slots.erase (it);
            break;
        }
    }
}

void EffectsChain::moveEffect (int fromIndex, int toIndex)
{
    if (fromIndex < 0 || fromIndex >= (int) effects.size()) return;
    if (toIndex < 0 || toIndex >= (int) effects.size()) return;

    auto effect = std::move (effects[fromIndex]);
    effects.erase (effects.begin() + fromIndex);
    effects.insert (effects.begin() + toIndex, std::move (effect));

    auto slot = slots[fromIndex];
    slots.erase (slots.begin() + fromIndex);
    slots.insert (slots.begin() + toIndex, slot);
}

void EffectsChain::clear()
{
    effects.clear();
    slots.clear();
}

void EffectsChain::setBypassed (int id, bool bypassed)
{
    for (auto& fx : effects)
    {
        if (fx.slot.id == id)
        {
            fx.slot.bypassed = bypassed;
            return;
        }
    }
}

EffectSlot* EffectsChain::getSlot (int index)
{
    if (index >= 0 && index < (int) slots.size())
        return &slots[index];
    return nullptr;
}

void EffectsChain::process (juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    for (auto& fx : effects)
    {
        if (fx.slot.bypassed)
            continue;

        if (fx.chorus)
            fx.chorus->process (buffer, numSamples);
        else if (fx.delay)
            fx.delay->process (buffer, numSamples);
        else if (fx.bitcrusher)
            fx.bitcrusher->process (buffer, numSamples);
    }
}

} // namespace fx
