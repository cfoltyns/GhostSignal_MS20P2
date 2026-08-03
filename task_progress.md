# GhostSignal MS20P - VCO Layout & Waveform Icon Fixes

## 1. Add `setMinKnobSize` to LabeledKnob
- [x] Add `minKnobSize` member and `setMinKnobSize()` method to LabeledKnob.h
- [x] Use `minKnobSize` in LabeledKnob.cpp instead of hardcoded 48

## 2. Split VCO1/VCO2 into separate columns in layoutRow1
- [x] Replace shared oscW column with separate vco1W and vco2W columns
- [x] Add a gap between VCO1 and VCO2 sections
- [x] Update column widths proportionally (filterW reduced from 26% to 24%)

## 3. Increase top padding for VCO1/VCO2 knobs
- [x] Increase `topPad` from `titleH * 0.2` to `titleH * 0.35`
- [x] Reduce PW knob size (diameter: 40-48 → 32-40, widget height: 66 → 60)
- [x] Call `setMinKnobSize(40)` on PW knobs in constructor

## 4. Build and verify
- [x] Build the project (all targets: SharedCode, Standalone, AU, VST3)

## 5. Fix waveform icons (suppress numeric text)
- [x] Override `getTextFromValue()` in WaveformSlider to return empty string
- [x] This prevents the GhostSignalLookAndFeel from drawing numeric value text
  that was obscuring the waveform icon in the center of the knob

## 6. Move master volume to bottom right
- [x] Remove master volume from header bar (layoutHeaderBar)
- [x] Position master volume at bottom-right corner in resized()
- [x] Reduce Row 2 height to make room for the master volume knob
- [x] Update header comment to reflect new layout

## 7. Space out top row (reduce right spacer)
- [x] Increase column widths: vco1W 14%→15%, vco2W 14%→15%, subW 10%→11%,
      mixerW 16%→17%, filterW 24%→25%
- [x] This reduces the right-side spacer from ~22% to ~17%
- [x] Update paint() divider positions to 0.43f and 0.86f to match new boundaries

## 8. Final build and verify
- [x] Build the project (all targets: SharedCode, Standalone, AU, VST3)

## 9. VCO1/VCO2 layout redesign (waveform centered, octave/tune below)
- [x] Waveform knob centered at top of VCO1/VCO2 panels
- [x] When Pulse waveform selected, waveform shifts left and PW knob appears to the right
- [x] Octave and Tune moved to a second row below the waveform/PW row

## 10. Mixer layout redesign (2-2-1)
- [x] Top row: VCO1 Lvl, VCO2 Lvl (2 knobs)
- [x] Middle row: Sub Lvl, Drive (2 knobs)
- [x] Bottom row: LPF Drive (1 knob, centered)

## 11. Fix CMake build (VST3 + AU not being built)
- [x] Removed unsupported `CLAP` from `FORMATS` (not a valid format in JUCE 8.0.15)
- [x] Removed broken post-build copy section (`if (JucePlugin_Build_Standalone)` was always false — it's a compile definition, not a CMake variable; `JUCE_SIGNED_COPY_FILE_PATH` was not a real JUCE property)
- [x] Added default `CMAKE_BUILD_TYPE=Release` so artifacts always land in `GhostSignalMS20P_artefacts/Release/`
- [x] Added recommended JUCE link flags (`juce_recommended_config_flags`, `juce_recommended_lto_flags`, `juce_recommended_warning_flags`)
- [x] Removed stray `c` file (untracked duplicate of AudioEngine.cpp)
- [x] Verified clean build produces all three formats: Standalone, AU, VST3
- [x] Fixed standalone app "damaged or incomplete" error (executable was missing from app bundle — clean rebuild fixed it)
- [x] Added Gatekeeper first-launch note to README for standalone app

