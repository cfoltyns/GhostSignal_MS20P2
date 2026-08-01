/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#include "VoiceManager.h"
#include <algorithm>

void VoiceManager::prepare (double sampleRate, int maxBlockSize, const VoiceManagerParams& p)
{
    this->sampleRate = sampleRate;
    params = p;

    // Initialize voices
    for (auto& voice : voices)
    {
        if (voice == nullptr)
            voice = std::make_unique<VoiceDSP>();
        voice->prepare (sampleRate, maxBlockSize);
    }

    reset();
}

void VoiceManager::reset()
{
    voiceActive.reset();
    activeVoiceCount = 0;
    lastNote = -1;

    std::fill (noteToVoice, noteToVoice + 128, -1);
    std::fill (voiceNote, voiceNote + maxPolyphony, -1);
    std::fill (voiceChannel, voiceChannel + maxPolyphony, -1);

    portamentoValue = 0.0f;
    portamentoTarget = 0.0f;
    portamentoVoice = -1;

    for (auto& voice : voices)
        if (voice != nullptr)
            voice->reset();
}


void VoiceManager::setParameters (const VoiceManagerParams& p)
{
    params = p;
}

int VoiceManager::findFreeVoice()
{
    // First look for a completely free voice
    for (int i = 0; i < maxPolyphony; ++i)
    {
        if (! voiceActive[i])
            return i;
    }

    // No free voices - need to steal based on mode
    int voiceToSteal = 0;
    if (params.monoPriority == MonoPriority::low)
        voiceToSteal = getLowestVoice();
    else if (params.monoPriority == MonoPriority::high)
        voiceToSteal = getHighestVoice();
    else
        voiceToSteal = getOldestVoice();
    return voiceToSteal;
}

int VoiceManager::getOldestVoice()
{
    for (int i = 0; i < maxPolyphony; ++i)
    {
        if (voiceActive[i])
            return i;
    }
    return 0;
}

int VoiceManager::getHighestVoice()
{
    int highestNote = -1;
    int highestVoice = 0;

    for (int i = 0; i < maxPolyphony; ++i)
    {
        if (voiceActive[i] && voiceNote[i] > highestNote)
        {
            highestNote = voiceNote[i];
            highestVoice = i;
        }
    }
    return highestVoice;
}

int VoiceManager::getLowestVoice()
{
    int lowestNote = 128;
    int lowestVoice = 0;

    for (int i = 0; i < maxPolyphony; ++i)
    {
        if (voiceActive[i] && voiceNote[i] < lowestNote)
        {
            lowestNote = voiceNote[i];
            lowestVoice = i;
        }
    }
    return lowestVoice;
}

int VoiceManager::findVoiceForNote (int noteNumber)
{
    int voiceIndex = noteToVoice[noteNumber];
    if (voiceIndex >= 0 && voiceIndex < maxPolyphony && voiceActive[voiceIndex])
        return voiceIndex;
    return -1;
}

int VoiceManager::allocateVoice (int noteNumber, float velocity, int channel)
{
    int voiceIndex = -1;
    bool isNewVoice = false;

    if (params.mode == VoiceMode::monophonic || params.mode == VoiceMode::legato)
    {
        // Mono modes - sustain notes until all released
        const int lastNoteVoice = (lastNote >= 0 && lastNote < 128) ? noteToVoice[lastNote] : -1;
        if (lastNoteVoice >= 0 && lastNoteVoice < maxPolyphony && voiceActive[lastNoteVoice])
        {

            // Hold existing note
            voiceIndex = noteToVoice[lastNote];
        }
        else
        {
            voiceIndex = findFreeVoice();
            if (voiceIndex >= 0 && !voiceActive[voiceIndex])
                isNewVoice = true;
        }
    }
    else
    {
        // Polyphonic mode - find existing or allocate new
        voiceIndex = findVoiceForNote (noteNumber);
        if (voiceIndex < 0)
        {
            voiceIndex = findFreeVoice();
            if (voiceIndex >= 0 && !voiceActive[voiceIndex])
                isNewVoice = true;
        }
    }

    if (voiceIndex >= 0 && voiceIndex < maxPolyphony)
    {
        // Release any note previously on this voice
        if (voiceNote[voiceIndex] >= 0)
            noteToVoice[voiceNote[voiceIndex]] = -1;

        voiceNote[voiceIndex] = noteNumber;
        noteToVoice[noteNumber] = voiceIndex;
        voiceChannel[voiceIndex] = channel;
        
        // Only increment if this is a newly activated voice, not if stealing
        if (!voiceActive[voiceIndex])
        {
            voiceActive.set (voiceIndex, true);
            activeVoiceCount++;
        }
        else
        {
            voiceActive.set (voiceIndex, true);
        }

        voices[voiceIndex]->noteOn (noteNumber, velocity);

        // Setup portamento
        if (params.portamento > 0.0f && lastNote >= 0)
        {
            float currentFreq = 440.0f * std::pow (2.0f, (lastNote - 69) / 12.0f);
            float targetFreq = 440.0f * std::pow (2.0f, (noteNumber - 69) / 12.0f);
            portamentoTarget = targetFreq;
            portamentoSamples = sampleRate * params.portamento;
            portamentoIncrement = (targetFreq - currentFreq) / (float) portamentoSamples;
            portamentoVoice = voiceIndex;
        }

        lastNote = noteNumber;
    }

    return voiceIndex;
}

void VoiceManager::releaseVoice (int voiceIndex)
{
    if (voiceIndex >= 0 && voiceIndex < maxPolyphony && voiceActive[voiceIndex])
    {
        int noteNumber = voiceNote[voiceIndex];
        voiceActive.set (voiceIndex, false);
        if (noteNumber >= 0 && noteNumber < 128)
            noteToVoice[noteNumber] = -1;
        voiceNote[voiceIndex] = -1;
        if (activeVoiceCount > 0)
            activeVoiceCount--;

        if (voices[voiceIndex] != nullptr)
            voices[voiceIndex]->noteOff();
    }
}

void VoiceManager::releaseVoiceForNote (int noteNumber)
{
    if (noteNumber >= 0 && noteNumber < 128)
    {
        int voiceIndex = noteToVoice[noteNumber];
        if (voiceIndex >= 0 && voiceIndex < maxPolyphony)
        {
            releaseVoice(voiceIndex);
        }
    }
}

void VoiceManager::pitchBendForChannel (int channel, int pitchBendValue)
{
    for (int i = 0; i < maxPolyphony; ++i)
    {
        if (voiceActive[i] && (channel == 0 || voiceChannel[i] == channel))
            voices[i]->pitchBend (pitchBendValue);
    }
}

void VoiceManager::pressureForChannel (int channel, int pressureValue)
{
    for (int i = 0; i < maxPolyphony; ++i)
    {
        if (voiceActive[i] && (channel == 0 || voiceChannel[i] == channel))
            voices[i]->pressure (pressureValue);
    }
}

void VoiceManager::timbreForChannel (int channel, int timbreValue)
{
    for (int i = 0; i < maxPolyphony; ++i)
    {
        if (voiceActive[i] && (channel == 0 || voiceChannel[i] == channel))
            voices[i]->timbre (timbreValue);
    }
}

void VoiceManager::releaseAllVoices()
{
    for (int i = 0; i < maxPolyphony; ++i)
    {
        if (voiceActive[i] && voices[i] != nullptr)
        {
            voices[i]->noteOff();
        }
    }
    voiceActive.reset();
    std::fill (noteToVoice, noteToVoice + 128, -1);
    std::fill (voiceNote, voiceNote + maxPolyphony, -1);
    activeVoiceCount = 0;
}
