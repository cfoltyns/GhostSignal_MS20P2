/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#include "AudioEngine.h"
#include "Parameters.h"
#include "Synth/Voice.h"

void AudioEngine::prepare(double newSampleRate, int newMaxBlockSize,
                          int newNumOutputChannels) {
  reset();
  sampleRate = newSampleRate;
  maxBlockSize = newMaxBlockSize;
  numOutputChannels = newNumOutputChannels;

  // Prepare synth engine
  ::dsp::EngineParams ep;
  ep.maxVoices = 32;
  ep.voiceMode = VoiceMode::polyphonic;
  synthEngine.prepare(sampleRate, maxBlockSize, numOutputChannels);
  synthEngine.setParameters(ep);

  // Modulation Matrix
  modMatrix.prepare(sampleRate);

  // Sequencer
  sequencer.prepare(sampleRate);

  // Effects chain
  fxChain.prepare(sampleRate, maxBlockSize, numOutputChannels);

  // Tape Delay
  tapeDelay.prepare(sampleRate, maxBlockSize, numOutputChannels);
}

void AudioEngine::reset() {
  synthEngine.reset();
  modMatrix.reset();
  sequencer.reset();
  fxChain.reset();
  tapeDelay.reset();
}

void AudioEngine::process(juce::AudioBuffer<float> &buffer,
                           juce::MidiBuffer &midi,
                           juce::AudioProcessorValueTreeState &apvts,
                           juce::AudioPlayHead *playHead) {
  auto getFloat = [&](const juce::String &id) -> float {
    if (auto *p = apvts.getRawParameterValue(id))
      return p->load();
    return 0.0f;
  };

  auto getChoice = [&](const juce::String &id) -> int {
    if (auto *p = apvts.getRawParameterValue(id))
      return (int)p->load();
    return 0;
  };

  // Build voice params from APVTS
  ::dsp::VoiceParams vp;

    // Osc1
    vp.osc1.waveform = (::dsp::Waveform)juce::jlimit(
        0, 6, getChoice(Parameters::paramOsc1Waveform));
    vp.osc1.pwm = getFloat(Parameters::paramOsc1PulseWidth);
    vp.osc1.octave = (int)getFloat(Parameters::paramOsc1Octave);
    vp.osc1.gain = getFloat(Parameters::paramOsc1Gain);
    vp.osc1.fineTune = getFloat(Parameters::paramOsc1Tune) / 100.0f;

    // Osc2
    vp.osc2.waveform = (::dsp::Waveform)juce::jlimit(
        0, 6, getChoice(Parameters::paramOsc2Waveform));
    vp.osc2.pwm = getFloat(Parameters::paramOsc2PulseWidth);
    vp.osc2.octave = (int)getFloat(Parameters::paramOsc2Octave);
    vp.osc2.gain = getFloat(Parameters::paramOsc2Gain);
    vp.osc2.fineTune = getFloat(Parameters::paramOsc2Tune) / 100.0f;

  // Sub
  vp.sub.octave = (int)getFloat(Parameters::paramSubOctave);
  vp.sub.gain = getFloat(Parameters::paramSubGain);

  // Noise
  int noiseTypeChoice = getChoice(Parameters::paramNoiseType);
  vp.noise.type = (noiseTypeChoice == 0) ? ::dsp::NoiseType::pink : ::dsp::NoiseType::white;
  vp.noise.gain = getFloat(Parameters::paramNoiseGain);

  // Mixer
  vp.mixer.osc1Level = getFloat(Parameters::paramMixerVco1Level);
  vp.mixer.osc2Level = getFloat(Parameters::paramMixerVco2Level);
  vp.mixer.subLevel = getFloat(Parameters::paramMixerSubLevel);
  vp.mixer.noiseLevel = getFloat(Parameters::paramNoiseGain);
  vp.mixer.drive = getFloat(Parameters::paramMixerDrive);

  // HPF
  vp.hpf.cutoff = getFloat(Parameters::paramHPFCutoff);
  vp.hpf.resonance = getFloat(Parameters::paramHPFRes);

  // LPF
  vp.lpf.cutoff = getFloat(Parameters::paramLPFCutoff);
  vp.lpf.resonance = getFloat(Parameters::paramLPFRes);
  vp.lpf.drive = getFloat(Parameters::paramLPFDrive);

  // Amp
  vp.ampGain = getFloat(Parameters::paramAmpGain);
  vp.pan = getFloat(Parameters::paramPan);

  // VCF Envelope
  vp.env1.attack = getFloat(Parameters::paramEnv1Attack);
  vp.env1.decay = getFloat(Parameters::paramEnv1Decay);
  vp.env1.sustain = getFloat(Parameters::paramEnv1Sustain);
  vp.env1.release = getFloat(Parameters::paramEnv1Release);

  // VCA Envelope
  vp.env2.attack = getFloat(Parameters::paramAmpAttack);
  vp.env2.decay = getFloat(Parameters::paramAmpDecay);
  vp.env2.sustain = getFloat(Parameters::paramAmpSustain);
  vp.env2.release = getFloat(Parameters::paramAmpRelease);

  // LFO1
  vp.lfo1.rate = getFloat(Parameters::paramLFO1Rate);
  vp.lfo1.depth = getFloat(Parameters::paramLFO1Depth);

  // Glide
  vp.glideTime = getFloat(Parameters::paramGlideTime);
  vp.glideEnabled = (getFloat(Parameters::paramGlideTime) > 0.0f);

  // ---- Update engine parameters ----
  ::dsp::EngineParams ep;
  ep.maxVoices = 32;

  int voiceModeChoice = getChoice(Parameters::paramVoiceMode);
  switch (voiceModeChoice)
  {
      case 0: ep.voiceMode = VoiceMode::polyphonic; break;
      case 1: ep.voiceMode = VoiceMode::monophonic; break;
      case 2: ep.voiceMode = VoiceMode::unison; break;
      default: ep.voiceMode = VoiceMode::polyphonic; break;
  }

  ep.glideTime = getFloat(Parameters::paramGlideTime);
  ep.masterTune = getFloat(Parameters::paramMasterTune);
  synthEngine.setParameters(ep);

  // Process sequencer
  sequencer.process(buffer.getNumSamples());
  if (sequencer.hasNewNote()) {
    synthEngine.noteOn(sequencer.getCurrentNote(),
                       sequencer.getCurrentVelocity());
  }

  // Process synth engine (handles MIDI, voices, drift)
  synthEngine.process(buffer, midi, vp, modMatrix);

  // Master volume
  if (auto *master = apvts.getRawParameterValue(Parameters::paramMasterVolume))
    buffer.applyGain(*master);

  // Process effects chain
  fxChain.process(buffer);

  // ---- Wire tape delay parameters ----
  const bool delayEnabled = (getChoice(Parameters::paramTapeDelayEnable) > 0);
  tapeDelay.setEnabled(delayEnabled);
  tapeDelay.setPlayHead(playHead);
  tapeDelay.setSyncMode(getChoice(Parameters::paramTapeDelayTimeMode));
  tapeDelay.setTimeMs(getFloat(Parameters::paramTapeDelayTime));
  tapeDelay.setFeedback(getFloat(Parameters::paramTapeDelayFeedback));
  tapeDelay.setMix(getFloat(Parameters::paramTapeDelayMix));
  tapeDelay.setTapeAge(getFloat(Parameters::paramTapeDelayAge));
  tapeDelay.setSaturation(getFloat(Parameters::paramTapeDelaySat));
  tapeDelay.setWow(getFloat(Parameters::paramTapeDelayWow));
  tapeDelay.setFlutter(getFloat(Parameters::paramTapeDelayFlutter));

  // Process tape delay (after master output, before DAW output)
  tapeDelay.process(buffer, buffer.getNumSamples());
}

void AudioEngine::handleMidi(const juce::MidiBuffer &midi) {
  // MIDI is now handled by SynthEngine::process
}

void AudioEngine::renderVoices(juce::AudioBuffer<float> &buffer,
                               int numSamples) {
  // Voice rendering is now handled by SynthEngine::process
}

void AudioEngine::applyMasterGain(juce::AudioBuffer<float> &buffer) {
  const float masterGain = 0.8f;
  buffer.applyGain(masterGain);
}
