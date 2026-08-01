/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Patch bay implementation.
 */

#include "PatchBay.h"

void PatchBay::clear()
{
    cables.clear();
    sourceValues.fill (0.0f);
}

void PatchBay::addCable (const PatchCable& cable)
{
    cables.push_back (cable);
}

void PatchBay::removeCable (int index)
{
    if (index >= 0 && index < (int) cables.size())
        cables.erase (cables.begin() + index);
}

void PatchBay::removeCable (const JackID& source, const JackID& destination)
{
    for (auto it = cables.begin(); it != cables.end(); ++it)
    {
        if (it->source == source && it->destination == destination)
        {
            cables.erase (it);
            return;
        }
    }
}

void PatchBay::setCableAmount (int index, float amount)
{
    if (index >= 0 && index < (int) cables.size())
        cables[index].amount = amount;
}

void PatchBay::setCableActive (int index, bool active)
{
    if (index >= 0 && index < (int) cables.size())
        cables[index].active = active;
}

bool PatchBay::isJackConnected (const JackID& jack) const
{
    for (const auto& cable : cables)
    {
        if (cable.source == jack || cable.destination == jack)
            return cable.active;
    }
    return false;
}

float PatchBay::getModulationValue (const JackID& destination) const
{
    float value = 0.0f;
    for (const auto& cable : cables)
    {
        if (cable.destination == destination && cable.active)
        {
            // Sum source values with cable amount scaling
            int sourceIdx = cable.source.moduleIndex * 8 + cable.source.jackIndex;
            if (sourceIdx >= 0 && sourceIdx < 64)
                value += sourceValues[sourceIdx] * cable.amount;
        }
    }
    return value;
}

void PatchBay::setSourceValue (const JackID& source, float value)
{
    int idx = source.moduleIndex * 8 + source.jackIndex;
    if (idx >= 0 && idx < 64)
        sourceValues[idx] = value;
}

std::vector<const PatchCable*> PatchBay::getCablesForJack (const JackID& jack) const
{
    std::vector<const PatchCable*> result;
    for (const auto& cable : cables)
    {
        if (cable.source == jack || cable.destination == jack)
            result.push_back (&cable);
    }
    return result;
}

