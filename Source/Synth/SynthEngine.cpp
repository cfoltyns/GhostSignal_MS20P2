/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Synth engine implementation.
 */

#include "SynthEngine.h"

namespace dsp
{

void SynthEngine::prepare (double sr, int blockSize, int channels)
{
    sampleRate = sr;
    maxBlockSize = blockSize;
    numChannels = channels;

    VoiceManagerParams vmp;
    vmp.maxVoices = params.maxVoices;
    vmp.mode = params.voiceMode;
    voiceManager.prepare (sampleRate, maxBlockSize, vmp);

    // Prepare drift engines
    for (auto& drift : driftEngines)
    {
        DriftParams dp;
        dp.sampleRate = sampleRate;
        dp.driftAmount = params.driftAmount;
        dp.pitchDrift = 10.0f;   // +/- 10 cents max
        dp.filterDrift = 0.5f;   // +/- 0.5 semitones
        dp.resDrift = 0.1f;
        dp.envTimeDrift = 0.15f;
        dp.satDrift = 0.2f;
        drift.prepare (dp);
    }
}

void SynthEngine::reset()
{
    voiceManager.reset();
    for (auto& drift : driftEngines)
        drift.reset();
}

void SynthEngine::setParameters (const EngineParams& p)
{
    params = p;

    VoiceManagerParams vmp;
    vmp.maxVoices = p.maxVoices;
    vmp.mode = p.voiceMode;
    vmp.portamento = p.glideTime;
    vmp.unisonDetune = p.unisonDetune;
    vmp.unisonVoices = p.unisonVoices;
    vmp.unisonSpread = p.unisonSpread;
    voiceManager.setParameters (vmp);

    // Update drift parameters
    for (auto& drift : driftEngines)
    {
        DriftParams dp;
        dp.sampleRate = sampleRate;
        dp.driftAmount = p.driftAmount;
        dp.pitchDrift = 10.0f;
        dp.filterDrift = 0.5f;
        dp.resDrift = 0.1f;
        dp.envTimeDrift = 0.15f;
        dp.satDrift = 0.2f;
        drift.prepare (dp);
    }
}

int SynthEngine::noteOn (int midiNote, float velocity, int channel)
{
    int voiceIndex = voiceManager.allocateVoice (midiNote, velocity, channel);
    return voiceIndex;
}

void SynthEngine::noteOff (int midiNote)
{
    voiceManager.releaseVoiceForNote (midiNote);
}

void SynthEngine::allNotesOff()
{
    voiceManager.releaseAllVoices();
}

void SynthEngine::pitchBend (int channel, int value)
{
    voiceManager.pitchBendForChannel (channel, value);
}

void SynthEngine::aftertouch (int channel, int value)
{
    voiceManager.pressureForChannel (channel, value);
}

void SynthEngine::timbre (int channel, int value)
{
    voiceManager.timbreForChannel (channel, value);
}

void SynthEngine::applyDriftToVoice (VoiceDSP& voice, int voiceIndex)
{
    if (voiceIndex < 0 || voiceIndex >= (int) driftEngines.size())
        return;

    auto& drift = driftEngines[voiceIndex];
    const auto& dp = drift;

    // Apply static offsets (done on note-on via VoiceDSP)
    // The voice uses these internally
}

void SynthEngine::process (juce::AudioBuffer<float>& buffer,
                            juce::MidiBuffer& midi,
                            const VoiceParams& voiceParams,
                            ModulationMatrix& modMatrix)
{
    const int numSamples = buffer.getNumSamples();
    if (numSamples <= 0)
        return;

    // Update thermal drift for all voices
    for (auto& drift : driftEngines)
        drift.updateThermalDrift (numSamples);

    // Process MIDI
    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();

        if (msg.isNoteOn())
        {
            noteOn (msg.getNoteNumber(), msg.getVelocity() / 127.0f, msg.getChannel());
        }
        else if (msg.isNoteOff())
        {
            noteOff (msg.getNoteNumber());
        }
        else if (msg.isPitchWheel())
        {
            pitchBend (msg.getChannel(), msg.getPitchWheelValue());
        }
        else if (msg.isChannelPressure())
        {
            aftertouch (msg.getChannel(), msg.getChannelPressureValue());
        }
        else if (msg.isController() && msg.getControllerNumber() == 74)
        {
            timbre (msg.getChannel(), msg.getControllerValue());
        }
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
        {
            allNotesOff();
        }
    }

    // Set parameters and drift on all active voices
    const int numActiveVoices = voiceManager.getNumActiveVoices();
    for (int i = 0; i < numActiveVoices; ++i)
    {
        Voice* voiceBase = voiceManager.getVoice (i);
        if (voiceBase == nullptr || !voiceBase->isActive())
            continue;

        auto* voice = dynamic_cast<VoiceDSP*>(voiceBase);
        if (voice == nullptr)
            continue;

        voice->setParameters (voiceParams);

        // Apply drift offsets to voice
        if (i < (int) driftEngines.size())
        {
            auto& drift = driftEngines[i];
            voice->setDrift (
                drift.getPitchOffset(),
                drift.getFilterCutoffOffset(),
                drift.getResonanceOffset(),
                drift.getEnvAttackScale(),
                drift.getEnvDecayScale(),
                drift.getEnvReleaseScale(),
                drift.getSatOffset(),
                drift.getThermalPitchCents()
            );
        }
    }

    // ── Render voices into buffer ──────────────────────────────────
    juce::AudioBuffer<float> voiceBuffer;
    const int voiceChannels = juce::jmax (2, numChannels);
    if (voiceChannels <= 0 || numSamples <= 0)
        return;
    voiceBuffer.setSize (voiceChannels, numSamples, false, false, true);

    // Clear the output buffer before summing voices
    buffer.clear();

    // ── Voice spread / pan ────────────────────────────────────────
    // When spread=0, all voices are centered (both channels 0.707).
    // When spread>0, voices are distributed across the stereo field.
    // Mix gain is normalised so that multiple voices don't clip.
    const float spread = params.voiceSpread;

    // Process all active voices (scan all slots, not just up to activeCount,
    // since voices may be sparsely distributed after voice-stealing / release).
    const int maxVoiceSlots = voiceManager.getMaxVoiceSlots();

    // Per-voice mix gain: keep total roughly constant regardless of voice count
    const int activeCount = juce::jmax (1, numActiveVoices);
    const float mixGain = 0.5f / std::sqrt ((float) activeCount);

    for (int i = 0; i < maxVoiceSlots; ++i)
    {
        Voice* voiceBase = voiceManager.getVoice (i);
        if (voiceBase == nullptr || !voiceBase->isActive())
            continue;

        auto* voice = dynamic_cast<VoiceDSP*>(voiceBase);
        if (voice == nullptr)
            continue;

        voiceBuffer.clear();
        voice->process (voiceBuffer, 0, numSamples);

        // Compute pan: 0=left, 0.5=center, 1=right
        float panPos = 0.5f; // default: center
        if (spread > 0.0f && activeCount > 1)
        {
            // Distribute voices evenly across the stereo field
            panPos = (float) i / (float) (maxVoiceSlots - 1);
            // Scale by spread amount
            panPos = 0.5f + (panPos - 0.5f) * spread;
            panPos = juce::jlimit (0.0f, 1.0f, panPos);
        }

        const float voicePanL = std::cos (panPos * juce::MathConstants<float>::halfPi);
        const float voicePanR = std::sin (panPos * juce::MathConstants<float>::halfPi);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            const float pan = (ch == 0) ? voicePanL : voicePanR;
            const int voiceCh = ch % voiceChannels;
            buffer.addFrom (ch, 0, voiceBuffer, voiceCh, 0,
                            numSamples, pan * mixGain);
        }
    }

}

// ── collectLfoValues ───────────────────────────────────────────────────────────
// Collect latest LFO output values from active voices for UI visualization
// This is called from AudioEngine::process after synthEngine.process()
void SynthEngine::collectLfoValues (float* lfoValues, int numLfos) const
{
    // Initialize to 0 (no modulation)
    for (int l = 0; l < numLfos; ++l)
        lfoValues[l] = 0.0f;

    // Scan active voices and collect the latest LFO output values
    // We take the maximum absolute value across all active voices
    const int numActiveVoices = voiceManager.getNumActiveVoices();
    if (numActiveVoices <= 0)
        return;

    for (int i = 0; i < voiceManager.getMaxVoiceSlots(); ++i)
    {
        const Voice* voiceBase = voiceManager.getVoice (i);
        if (voiceBase == nullptr || !voiceBase->isActive())
            continue;

        auto* voice = dynamic_cast<const VoiceDSP*>(voiceBase);
        if (voice == nullptr)
            continue;

        for (int l = 0; l < numLfos; ++l)
        {
            const float val = voice->getLfoOutput (l);
            // Keep the value with the largest magnitude (most visible modulation)
            if (std::abs (val) > std::abs (lfoValues[l]))
                lfoValues[l] = val;
        }
    }
}

} // namespace dsp

