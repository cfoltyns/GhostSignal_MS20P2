/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#include "Voice.h"

void VoiceDSP::prepare (double sampleRate, int /*maxBlockSize*/)
{
    voiceSampleRate = sampleRate;

    osc1.prepare (sampleRate);
    osc2.prepare (sampleRate);
    sub.prepare (sampleRate);
    noise.prepare (sampleRate);
    mixer.prepare (sampleRate);
    hpf.prepare (sampleRate);
    lpf.prepare (sampleRate);
    env1.prepare (sampleRate);
    env2.prepare (sampleRate);
    env3.prepare (sampleRate);
    env4.prepare (sampleRate);
    lfo1.prepare (sampleRate);
    lfo2.prepare (sampleRate);
    lfo3.prepare (sampleRate);
    lfo4.prepare (sampleRate);

    osc1Buffer.setSize (2, 1024);
    osc2Buffer.setSize (2, 1024);
    subBuffer.setSize (2, 1024);
    noiseBuffer.setSize (2, 1024);
    mixBuffer.setSize (2, 1024);

    reset();
}

void VoiceDSP::reset()
{
    osc1.reset();
    osc2.reset();
    sub.reset();
    noise.reset();
    env1.reset();
    env2.reset();
    env3.reset();
    env4.reset();
    lfo1.reset();
    lfo2.reset();
    lfo3.reset();
    lfo4.reset();
}

void VoiceDSP::noteOn (int midiNoteNumber, float velocity)
{
    Voice::active = true;
    Voice::noteNumber = midiNoteNumber;
    Voice::velocity = velocity;

    const float freq = 440.0f * std::pow (2.0f, (midiNoteNumber - 69) / 12.0f);
    currentNote = (float) midiNoteNumber;
    params.osc1.frequency = freq;
    params.osc2.frequency = freq;
    params.sub.frequency = freq;

    // Setup glide/portamento
    if (glideEnabled && glideTime > 0.0f && glideCurrentNote != (float) midiNoteNumber)
    {
        glideTargetNote = (float) midiNoteNumber;
        glideStartNote  = glideCurrentNote;
        glideActive     = true;
        // Constant-time glide: the full interval always takes glideTime
        // seconds regardless of interval size. Sample-count is stored as a
        // double so large values never lose precision.
        glideSamplesTotal = juce::jmax (1.0, (double) glideTime * voiceSampleRate);
        glideSamplesDone  = 0.0;
    }
    else
    {
        glideCurrentNote = (float) midiNoteNumber;
        glideTargetNote = (float) midiNoteNumber;
        glideActive = false;
    }

    osc1.setParameters (params.osc1);
    osc2.setParameters (params.osc2);
    sub.setParameters (params.sub);
    noise.setParameters (params.noise);
    hpf.setParameters (params.hpf);
    lpf.setParameters (params.lpf);
    mixer.setParameters (params.mixer);
    saturation.setParameters (params.saturation);
    env1.setParameters (params.env1);
    env2.setParameters (params.env2);
    env3.setParameters (params.env3);
    env4.setParameters (params.env4);
    lfo1.setParameters (params.lfo1);
    lfo2.setParameters (params.lfo2);
    lfo3.setParameters (params.lfo3);
    lfo4.setParameters (params.lfo4);

    env1.noteOn (velocity);
    env2.noteOn (velocity);
    env3.noteOn (velocity);
    env4.noteOn (velocity);
}

void VoiceDSP::noteOff()
{
    Voice::active = false;
    env1.noteOff();
    env2.noteOff();
    env3.noteOff();
    env4.noteOff();
}

void VoiceDSP::pitchBend (int pitchBendValue)
{
    currentPitchBend = pitchBendValue / 8192.0f;
}

void VoiceDSP::pressure (int pressureValue)
{
    currentPressure = pressureValue / 127.0f;
}

void VoiceDSP::timbre (int timbreValue)
{
    currentTimbre = timbreValue / 127.0f;
}

void VoiceDSP::setParameters (const dsp::VoiceParams& p)
{
    params = p;
    const float freq = 440.0f * std::pow (2.0f, (currentNote - 69.0f) / 12.0f);
    params.osc1.frequency = freq;
    params.osc2.frequency = freq;
    params.sub.frequency = freq;
    
    // Update glide state from parameters
    glideEnabled = p.glideEnabled;
    glideTime = p.glideTime;
}

void VoiceDSP::setDrift (float pitchCents, float filterCutoffSemitones, float resOff,
                          float envAttack, float envDecay, float envRelease,
                          float satOff, float thermalPitch)
{
    driftPitchCents = pitchCents;
    driftFilterCutoff = filterCutoffSemitones;
    driftResonance = resOff;
    driftEnvAttackScale = envAttack;
    driftEnvDecayScale = envDecay;
    driftEnvReleaseScale = envRelease;
    driftSatOffset = satOff;
    driftThermalPitch = thermalPitch;
}

void VoiceDSP::process (juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    if (! isActive() && ! env1.isActive() && ! env2.isActive())
        return;

    const int blockSize = 1024;
    int remaining = numSamples;
    int offset = startSample;

    while (remaining > 0)
    {
        const int thisBlock = juce::jmin (remaining, blockSize);

        // Process all 4 LFOs
        lfo1.process (thisBlock);
        lfo2.process (thisBlock);
        lfo3.process (thisBlock);
        lfo4.process (thisBlock);

        // Process envelopes
        const float envVal1 = env1.process (thisBlock);  // VCF envelope (0..1)
        const float envVal2 = env2.process (thisBlock);  // VCA envelope (0..1)

        // Compute LFO outputs (each in -1..1 range, multiplied by depth)
        const float lfoOut1 = lfo1.getOutput() * params.lfo1.depth;
        const float lfoOut2 = lfo2.getOutput() * params.lfo2.depth;
        const float lfoOut3 = lfo3.getOutput() * params.lfo3.depth;
        const float lfoOut4 = lfo4.getOutput() * params.lfo4.depth;

        // Apply destination routing — accumulate modulation per target
        float osc1FreqMod  = 0.0f;  // pitch semitones
        float osc2FreqMod  = 0.0f;
        float osc1PWMMod   = 0.0f;  // additive PWM offset
        float osc2PWMMod   = 0.0f;
        float osc1FineMod  = 0.0f;  // additive fine-tune (semitones)
        float osc2FineMod  = 0.0f;
        float osc1GainMod  = 0.0f;  // additive gain offset
        float osc2GainMod  = 0.0f;
        float cutoffMod    = 0.0f;  // multiplicative cutoff factor
        float resMod       = 0.0f;  // additive resonance
        float hpfCutoffMod = 0.0f;  // multiplicative HPF cutoff factor
        float hpfResMod    = 0.0f;  // additive HPF resonance
        float ampGainMod   = 0.0f;  // additive amp gain offset
        float panMod       = 0.0f;  // additive pan offset

        const float lfoVals[4]  = { lfoOut1, lfoOut2, lfoOut3, lfoOut4 };
        const int   lfoDests[4] = { params.lfo1Dest, params.lfo2Dest, params.lfo3Dest, params.lfo4Dest };

        for (int i = 0; i < 4; ++i)
        {
            const float v = lfoVals[i];  // -1..1
            switch (lfoDests[i])
            {
                case  1: osc1FreqMod  += v * 12.0f; break;  // VCO1 Pitch ±12 st
                case  2: osc2FreqMod  += v * 12.0f; break;  // VCO2 Pitch ±12 st
                case  3: osc1PWMMod   += v * 0.30f; break;  // VCO1 PWM ±30%
                case  4: osc2PWMMod   += v * 0.30f; break;  // VCO2 PWM ±30%
                case  5: osc1FineMod  += v * 2.0f;  break;  // VCO1 Tune ±2 st
                case  6: osc2FineMod  += v * 2.0f;  break;  // VCO2 Tune ±2 st
                case  7: osc1GainMod  += v * 0.5f;  break;  // VCO1 Level ±50%
                case  8: osc2GainMod  += v * 0.5f;  break;  // VCO2 Level ±50%
                case  9: cutoffMod    += v * 1.5f;  break;  // Filter Cutoff ×0.5..2.5
                case 10: resMod       += v * 0.3f;   break;  // Filter Res ±30%
                case 11: hpfCutoffMod += v * 1.0f;  break;  // HPF Cutoff ×0..2
                case 12: hpfResMod    += v * 0.3f;   break;  // HPF Res ±30%
                case 13: ampGainMod   += v * 0.5f;  break;  // Amp Gain ±50%
                case 14: panMod       += v * 0.3f;   break;  // Pan ±30%
                default: break;
            }
        }
        
        // Apply glide/portamento — linear interpolation from the starting
        // note to the target over exactly glideTime seconds. Bounded by
        // construction (t is clamped to 0..1), so no instability or NaN is
        // possible no matter where the Glide knob is set.
        if (glideActive)
        {
            glideSamplesDone += thisBlock;

            if (glideSamplesDone >= glideSamplesTotal)
            {
                glideCurrentNote = glideTargetNote;
                glideActive = false;
            }
            else
            {
                const float t = (float) (glideSamplesDone / glideSamplesTotal);
                glideCurrentNote = glideStartNote
                                 + (glideTargetNote - glideStartNote) * t;
            }
        }

        // Pitch bend. The note is clamped to a wide-but-finite MIDI range so
        // a runaway modulation value can never produce an inf/NaN frequency
        // (which would silence the voice until it was reset).
        const float pbSemitones = currentPitchBend * 48.0f;
        const float effectiveNote = juce::jlimit (-128.0f, 256.0f,
            (glideActive ? glideCurrentNote : currentNote) + pbSemitones);
        
        const float baseFreq = 440.0f * std::pow (2.0f, (effectiveNote - 69.0f) / 12.0f);

        // Apply LFO pitch modulation (frequency multiplier from semitones)
        const float osc1PitchMod = std::pow (2.0f, osc1FreqMod / 12.0f);
        const float osc2PitchMod = std::pow (2.0f, osc2FreqMod / 12.0f);

        // Apply fine tune + LFO fine-tune to oscillator 1
        const float osc1FineTune = params.osc1.fineTune + osc1FineMod;
        const float osc1TuneFactor = std::pow (2.0f, osc1FineTune / 12.0f);
        params.osc1.frequency = baseFreq * osc1PitchMod * osc1TuneFactor;

        // Apply fine tune + LFO fine-tune to oscillator 2
        const float osc2FineTune = params.osc2.fineTune + osc2FineMod;
        const float osc2TuneFactor = std::pow (2.0f, osc2FineTune / 12.0f);
        params.osc2.frequency = baseFreq * osc2PitchMod * osc2TuneFactor;

        // Apply LFO PWM modulation
        params.osc1.pwm = juce::jlimit (0.01f, 0.99f, params.osc1.pwm + osc1PWMMod);
        params.osc2.pwm = juce::jlimit (0.01f, 0.99f, params.osc2.pwm + osc2PWMMod);

        // Apply LFO oscillator gain modulation
        params.osc1.gain = juce::jlimit (0.0f, 1.0f, params.osc1.gain + osc1GainMod);
        params.osc2.gain = juce::jlimit (0.0f, 1.0f, params.osc2.gain + osc2GainMod);

        // Apply VCF envelope + LFO to LPF cutoff
        const float envModAmount = envVal1 * 5.0f;
        const float modulatedCutoff = params.lpf.cutoff * (1.0f + envModAmount + cutoffMod);
        params.lpf.cutoff = juce::jlimit (20.0f, 20000.0f, modulatedCutoff);

        // Apply LFO resonance modulation
        params.lpf.resonance = juce::jlimit (0.0f, 1.0f, params.lpf.resonance + resMod);

        // Apply LFO modulation to HPF cutoff and resonance. The HPF cutoff
        // range is 20..1000 Hz (see Parameters::addFloat for paramHPFCutoff),
        // so the multiplicative factor is clamped to keep it sweepable but
        // always inside the parameter's own bounds.
        params.hpf.cutoff    = juce::jlimit (20.0f, 1000.0f,
                                             params.hpf.cutoff * (1.0f + hpfCutoffMod));
        params.hpf.resonance = juce::jlimit (0.0f, 1.0f, params.hpf.resonance + hpfResMod);

        osc1.setParameters (params.osc1);
        osc2.setParameters (params.osc2);
        sub.setParameters (params.sub);
        noise.setParameters (params.noise);
        mixer.setParameters (params.mixer);
        hpf.setParameters (params.hpf);
        lpf.setParameters (params.lpf);
        saturation.setParameters (params.saturation);

        // Render sources
        osc1Buffer.clear();
        osc2Buffer.clear();
        subBuffer.clear();
        noiseBuffer.clear();

        osc1.process (osc1Buffer, thisBlock);
        osc2.process (osc2Buffer, thisBlock);
        sub.process (subBuffer, thisBlock);
        noise.process (noiseBuffer, thisBlock);

        mixBuffer.clear();
        mixer.process (mixBuffer, &osc1Buffer, &osc2Buffer, &subBuffer, &noiseBuffer, thisBlock);

        // HPF -> LPF -> Saturation
        hpf.process (mixBuffer, thisBlock);
        lpf.process (mixBuffer, thisBlock);
        saturation.process (mixBuffer, thisBlock);

        // Amp with VCA envelope, pan and gain — with LFO modulation applied
        const float ampVal = envVal2 * velocity;
        const float effectiveGain = juce::jlimit (0.0f, 1.0f, params.ampGain + ampGainMod) * ampVal;
        const float effectivePan  = juce::jlimit (0.0f, 1.0f, params.pan + panMod);
        const float panLeft  = std::cos (effectivePan * juce::MathConstants<float>::halfPi);
        const float panRight = std::sin (effectivePan * juce::MathConstants<float>::halfPi);

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            for (int i = 0; i < thisBlock; ++i)
            {
                float s = mixBuffer.getSample (channel, i) * effectiveGain;
                if (channel == 0) s *= panLeft;
                else if (channel == 1) s *= panRight;
                buffer.setSample (channel, offset + i, s);
            }
        }

        remaining -= thisBlock;
        offset += thisBlock;
    }
}