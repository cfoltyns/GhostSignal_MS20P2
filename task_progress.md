# GhostSignal MS20P - Implementation Plan

## 1. Fix CMakeLists.txt source list corruption
- [x] Fix broken line 52 (literal \r\n characters corrupting source list)

## 2. Expand LFO destination choices in Parameters
- [x] Update lfoDestinationChoices to cover: pitch, PWM, tuning, oscillator levels, filter cutoff, filter res, amp gain, pan

## 3. Wire all 4 LFOs in AudioEngine.cpp
- [x] Add LFO2, LFO3, LFO4 params (rate, depth, waveform, shape, dest)
- [x] Add LFO1 waveform, shape, dest wiring (currently missing)
- [x] Add waveform choice-to-enum mapping helper

## 4. Add LFO destination fields to VoiceParams
- [x] Add lfo1Dest-lfo4Dest integer fields to VoiceParams struct

## 5. Process all 4 LFOs with destination routing in Voice.cpp
- [x] Process lfo3 and lfo4 (currently skipped)
- [x] Replace hardcoded LFO1→pitch with per-LFO destination routing
- [x] Route to: pitch, filter cutoff, tuning, oscillator gain, PWM, pan, amp gain, resonance

## 6. Create LogoComponent
- [x] Create LogoComponent.h/.cpp with graphical "GS" monogram logo
- [x] Add circuit-trace visual effect and gradient styling

## 7. Improve EnvDisplay
- [x] Rewrite EnvDisplay to have consistent point positions matching drawn path
- [x] Clean up drag logic and visual rendering

## 8. Integrate EnvDisplay into PluginEditor
- [x] Add two EnvDisplay components (VCA ENV and VCF ENV)
- [x] Wire knob-to-display (slider changes update display)
- [x] Wire display-to-knob (dragging points updates parameters)
- [x] Position EnvDisplay in layout alongside knobs

## 9. Add LFO2-LFO4 UI controls to PluginEditor
- [x] Add LFO2, LFO3, LFO4 panels with Waveform, Shape, Rate, Depth, Dest controls
- [x] Add LfoDisplay waveform visualization for each LFO
- [x] Add parameter attachments for all new controls
- [x] Redesign bottom row layout to accommodate 4 LFOs

## 10. Integrate LogoComponent into PluginEditor
- [x] Replace text title label with LogoComponent in header
- [x] Position logo in header bar

## 11. Update timerCallback for EnvDisplay sync
- [x] Sync EnvDisplay values from parameter changes in timer

## 12. Update README
- [x] Update controls documentation

## 13. Fix mode knob in tape delay section
- [x] Fix tapeDelayMode type mismatch: declared as juce::ComboBox in PluginEditor.h but used as LabeledKnob
- [x] Change tapeDelayMode declaration from juce::ComboBox to LabeledKnob { "Mode" }
- [x] Add snapping behavior to LabeledKnob::sliderValueChanged for discrete mode selection
- [x] Verify mode knob methods compile (setSnapToValues, setTextValues, setAutoCenterText, getSlider, setCenterText)

## 14. Build and verify
- [x] Build the project
- [x] Verify compilation succeeds (all targets: SharedCode, Standalone, AU, VST3)
