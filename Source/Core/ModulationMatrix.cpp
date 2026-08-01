/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#include "ModulationMatrix.h"
#include <algorithm>

void ModulationMatrix::prepare (double)
{
    reset();
}

void ModulationMatrix::reset()
{
    std::fill (cachedValues.begin(), cachedValues.end(), 0.0f);
}

void ModulationMatrix::setParameters (const std::array<ModSlot, 16>& newSlots)
{
    slots = newSlots;
}

float ModulationMatrix::getModulationValue (ModDestination dest, const std::array<float, 16>& sourceValues)
{
    float totalMod = 0.0f;

    for (const auto& slot : slots)
    {
        if (! slot.enabled || slot.source == ModSource::none)
            continue;

        if (slot.destination == dest)
        {
            float sourceValue = 0.0f;
            switch (slot.source)
            {
                case ModSource::lfo1: sourceValue = sourceValues[(int) ModSource::lfo1]; break;
                case ModSource::lfo2: sourceValue = sourceValues[(int) ModSource::lfo2]; break;
                case ModSource::lfo3: sourceValue = sourceValues[(int) ModSource::lfo3]; break;
                case ModSource::lfo4: sourceValue = sourceValues[(int) ModSource::lfo4]; break;
                case ModSource::env1: sourceValue = sourceValues[(int) ModSource::env1]; break;
                case ModSource::env2: sourceValue = sourceValues[(int) ModSource::env2]; break;
                case ModSource::env3: sourceValue = sourceValues[(int) ModSource::env3]; break;
                case ModSource::env4: sourceValue = sourceValues[(int) ModSource::env4]; break;
                default: break;
            }

            // Apply unipolar/bipolar scaling
            if (slot.bipolar)
                sourceValue = sourceValue * 2.0f - 1.0f; // Convert 0-1 to -1 to 1

            totalMod += sourceValue * slot.amount;
        }
    }

    return totalMod;
}

void ModulationMatrix::process (juce::AudioBuffer<float>&, int)
{
    // This method is for real-time buffer processing if needed
    // Currently the modulation is computed via getModulationValue
}