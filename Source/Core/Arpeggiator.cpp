/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#include "Arpeggiator.h"
#include <algorithm>

void Arpeggiator::prepare (double sampleRate, const ArpeggiatorParams& p)
{
    params = p;
    params.sampleRate = sampleRate;
    reset();
}

void Arpeggiator::reset()
{
    heldNotes.fill (0);
    heldNoteCount = 0;
    currentNoteIndex = 0;
    sequencePosition = 0;
    playing = false;
    goingUp = true;
    stepCounter = 0.0;
    
    // Calculate timing based on tempo (assuming 1/16 note steps)
    const double beatsPerMinute = params.tempo;
    const double beatsPerSecond = beatsPerMinute / 60.0;
    samplesPerStep = params.sampleRate / (beatsPerSecond * 4.0); // 16th notes
}

void Arpeggiator::setParameters (const ArpeggiatorParams& p)
{
    params = p;
    reset();
}

void Arpeggiator::noteOn (int noteNumber)
{
    if (heldNoteCount < 128)
    {
        heldNotes[noteNumber] = 1;
        heldNoteCount++;
    }
    
    // Update sorted notes array
    int idx = 0;
    for (int i = 0; i < 128; ++i)
    {
        if (heldNotes[i] > 0)
            sortedNotes[idx++] = i;
    }
    
    playing = true;
    currentNoteIndex = 0;
}

void Arpeggiator::noteOff (int noteNumber)
{
    heldNotes[noteNumber] = 0;
    heldNoteCount--;
    
    // Update sorted notes
    int idx = 0;
    for (int i = 0; i < 128; ++i)
    {
        if (heldNotes[i] > 0)
            sortedNotes[idx++] = i;
    }
    
    if (heldNoteCount == 0)
        playing = false;
}

void Arpeggiator::allNotesOff()
{
    heldNotes.fill (0);
    heldNoteCount = 0;
    playing = false;
}

int Arpeggiator::getNextNote()
{
    if (! playing || heldNoteCount == 0 || params.mode == ArpMode::off)
        return -1;

    stepCounter += 1.0;
    
    if (stepCounter >= samplesPerStep)
    {
        stepCounter -= samplesPerStep;
        
        if (params.mode == ArpMode::chord)
        {
            // Play all notes at once
            if (sequencePosition == 0)
            {
                sequencePosition = 1;
                return sortedNotes[0]; // Just trigger first note - chord handled elsewhere
            }
            sequencePosition = 0;
            return -1;
        }
        
        int noteCount = heldNoteCount * params.octaveRange;
        
        switch (params.mode)
        {
            case ArpMode::up:
                currentNoteIndex = (currentNoteIndex + 1) % noteCount;
                break;
                
            case ArpMode::down:
                currentNoteIndex = (currentNoteIndex - 1 + noteCount) % noteCount;
                break;
                
            case ArpMode::upDown:
                if (goingUp)
                {
                    currentNoteIndex++;
                    if (currentNoteIndex >= noteCount - 1)
                        goingUp = false;
                }
                else
                {
                    currentNoteIndex--;
                    if (currentNoteIndex <= 0)
                        goingUp = true;
                }
                currentNoteIndex = std::max (0, std::min (noteCount - 1, currentNoteIndex));
                break;
                
            case ArpMode::random:
                currentNoteIndex = rand() % noteCount;
                break;
                
            default:
                break;
        }
        
        return sortedNotes[currentNoteIndex % heldNoteCount];
    }
    
    return -1;
}