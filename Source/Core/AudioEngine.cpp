/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 *
 * Description: Audio engine — synth voice processing, effects chain, and
 *              master Tape Delay (replaces legacy Spring Reverb).
 *
 * Signal flow: Oscillators → Mixer → VCF → VCA → Master Volume → Tape Delay → DAW
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

  // Effects chain (insert effects: chorus, delay, bitcrusher)
  fxChain.prepare(sampleRate, maxBlockSize, numOutputChannels);

  // Tape Delay (master effect after VCA — replaces spring reverb)
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
  switch (noiseTypeChoice) {
      case 0: vp.noise.type = ::dsp::NoiseType::brown; break;
      case 1: vp.noise.type = ::dsp::NoiseType::pink; break;
      case 2: vp.noise.type = ::dsp::NoiseType::white; break;
      case 3: vp.noise.type = ::dsp::NoiseType::blue; break;
      case 4: vp.noise.type = ::dsp::NoiseType::violet; break;
      default: vp.noise.type = ::dsp::NoiseType::white; break;
  }
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
  
  // VCF Envelope (env1 = VCF ADSR)
  vp.env1.attack = getFloat(Parameters::paramEnv1Attack);
  vp.env1.decay = getFloat(Parameters::paramEnv1Decay);
  vp.env1.sustain = getFloat(Parameters::paramEnv1Sustain);
  vp.env1.release = getFloat(Parameters::paramEnv1Release);
  
  // VCA Envelope (env2 = VCA ADSR from amp params)
  vp.env2.attack = getFloat(Parameters::paramAmpAttack);
  vp.env2.decay = getFloat(Parameters::paramAmpDecay);
  vp.env2.sustain = getFloat(Parameters::paramAmpSustain);
  vp.env2.release = getFloat(Parameters::paramAmpRelease);
  
    // LFO waveform choice → enum mapping (5 classic waveforms)
    // The Shape knob provides MS-20-style morphing on Triangle/Square.
    const auto mapLfoWave = [](int ch) -> ::dsp::LfoWaveform {
        static const ::dsp::LfoWaveform m[] = {
            ::dsp::LfoWaveform::sine,       // 0 Sine
            ::dsp::LfoWaveform::triangle,   // 1 Triangle (shape morphs to saw/square)
            ::dsp::LfoWaveform::square,     // 2 Square
            ::dsp::LfoWaveform::sawUp,      // 3 Sawtooth (ramp up)
            ::dsp::LfoWaveform::random,     // 4 Random (S&H style)
        };
        return (ch >= 0 && ch < 5) ? m[ch] : ::dsp::LfoWaveform::sine;
    };

   // LFO1
   vp.lfo1.waveform   = mapLfoWave (getChoice(Parameters::paramLFO1Waveform));
   vp.lfo1.rate       = getFloat(Parameters::paramLFO1Rate);
   vp.lfo1.depth      = getFloat(Parameters::paramLFO1Depth);
   vp.lfo1.shapeMorph = getFloat(Parameters::paramLFO1Shape);
   vp.lfo1Dest        = getChoice(Parameters::paramLFO1Dest);

   // LFO2
   vp.lfo2.waveform   = mapLfoWave (getChoice(Parameters::paramLFO2Waveform));
   vp.lfo2.rate       = getFloat(Parameters::paramLFO2Rate);
   vp.lfo2.depth      = getFloat(Parameters::paramLFO2Depth);
   vp.lfo2.shapeMorph = getFloat(Parameters::paramLFO2Shape);
   vp.lfo2Dest        = getChoice(Parameters::paramLFO2Dest);

   // LFO3
   vp.lfo3.waveform   = mapLfoWave (getChoice(Parameters::paramLFO3Waveform));
   vp.lfo3.rate       = getFloat(Parameters::paramLFO3Rate);
   vp.lfo3.depth      = getFloat(Parameters::paramLFO3Depth);
   vp.lfo3.shapeMorph = getFloat(Parameters::paramLFO3Shape);
   vp.lfo3Dest        = getChoice(Parameters::paramLFO3Dest);

   // LFO4
   vp.lfo4.waveform   = mapLfoWave (getChoice(Parameters::paramLFO4Waveform));
   vp.lfo4.rate       = getFloat(Parameters::paramLFO4Rate);
   vp.lfo4.depth      = getFloat(Parameters::paramLFO4Depth);
   vp.lfo4.shapeMorph = getFloat(Parameters::paramLFO4Shape);
   vp.lfo4Dest        = getChoice(Parameters::paramLFO4Dest);

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

  // Master volume (applied BEFORE Tape Delay)
  if (auto *master = apvts.getRawParameterValue(Parameters::paramMasterVolume))
    buffer.applyGain(*master);

  // Process insert effects chain (chorus, delay, bitcrusher, etc.)
  fxChain.process(buffer);

  // ── Wire Tape Delay parameters ──────────────────────────────────────────
    const bool tapeEnabled = (getFloat(Parameters::paramTapeDelayEnable) > 0.5f);
  tapeDelay.setEnabled(tapeEnabled);

  if (tapeEnabled)
  {
      int timeMode = getChoice(Parameters::paramTapeDelayTimeMode);
      float timeMs = getFloat(Parameters::paramTapeDelayTime);
      float feedback = getFloat(Parameters::paramTapeDelayFeedback);
      float mix = getFloat(Parameters::paramTapeDelayMix);
      float age = getFloat(Parameters::paramTapeDelayAge);
      float sat = getFloat(Parameters::paramTapeDelaySat);
      float wow = getFloat(Parameters::paramTapeDelayWow);
      float flutter = getFloat(Parameters::paramTapeDelayFlutter);

      tapeDelay.setSyncMode(timeMode);
      tapeDelay.setTimeMs(timeMs);
      tapeDelay.setFeedback(feedback);
      tapeDelay.setMix(mix);
      tapeDelay.setTapeAge(age);
      tapeDelay.setSaturation(sat);
      tapeDelay.setWow(wow);
      tapeDelay.setFlutter(flutter);
      tapeDelay.setPlayHead(playHead);

      // Process tape delay (master effect after VCA)
      tapeDelay.process(buffer, buffer.getNumSamples());
  }
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
