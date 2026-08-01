# Ghost Signal MS20P

A modern, fully polyphonic virtual analog synthesizer inspired by the Korg MS-20,
featuring four routable LFOs, interactive ADSR envelope displays, and a distinctive
logo identity.

Platforms: Windows x64, macOS Intel/Apple Silicon, Linux x64
Formats: VST3, AU, CLAP, Standalone
Framework: JUCE + CMake + C++20

## Prerequisites

- Visual Studio 2022 (Windows) / Xcode (macOS) / GCC 13+ or Clang 16+ (Linux)
- CMake >= 3.24
- Git
- JUCE 8 (vendored locally in ThirdParty/JUCE)

## Quick Start (Windows)

```powershell
cd c:\Users\Designer\Desktop\GhostSignalMS20P

# Configure
cmake -S . -B build

# Build Release
cmake --build build --config Release
```

## Outputs

- VST3: build\GhostSignalMS20P_artefacts\Release\VST3\Ghost Signal MS20P.vst3
- Standalone: build\GhostSignalMS20P_artefacts\Release\Standalone\Ghost Signal MS20P.exe

## Run the Standalone App

```powershell
cd build\GhostSignalMS20P_artefacts\Release\Standalone
. "Ghost Signal MS20P.exe"
```

## Install the VST3

Copy the `.vst3` folder to your DAW's VST3 directory, then rescan:
- Windows: C:\Program Files\Common Files\VST3\
- macOS: /Library/Audio/Plug-Ins/VST3/
- Linux: ~/.vst3/ or /usr/lib/vst3/

## Architecture

### Four Routable LFOs

The synth features **4 independent LFOs**, each with:

- **Waveform** selector (5 classic options: Sine, Triangle, Square, Sawtooth,
  Random) with real-time visual display
- **Rate** knob (0.01–20 Hz, logarithmic)
- **Depth** knob (0–100%)
- **Shape** knob for MS-20-style waveform morphing
- **Destination** selector routing the LFO to one of 13 modulation targets:

  | # | Destination    | Effect                                  |
  |---|----------------|-----------------------------------------|
  | 0 | Off             | LFO disabled                            |
  | 1 | VCO1 Pitch      | ±12 semitones on oscillator 1           |
  | 2 | VCO2 Pitch      | ±12 semitones on oscillator 2           |
  | 3 | VCO1 PWM        | ±30% pulse width on oscillator 1        |
  | 4 | VCO2 PWM        | ±30% pulse width on oscillator 2        |
  | 5 | VCO1 Tune       | ±2 semitones fine-tuning on osc 1       |
  | 6 | VCO2 Tune       | ±2 semitones fine-tuning on osc 2       |
  | 7 | VCO1 Level      | ±50% gain on oscillator 1               |
  | 8 | VCO2 Level      | ±50% gain on oscillator 2               |
  | 9 | Filter Cutoff   | ±1.5× cutoff multiplier on LPF          |
  | 10| Filter Res       | ±30% resonance on LPF                   |
  | 11| Amp Gain         | ±50% master amp gain                    |
  | 12| Pan              | ±30% stereo panning                     |

Multiple LFOs can be routed to the same destination simultaneously, with their
modulation values accumulated.

### Interactive ADSR Envelope Displays

Each envelope section (VCA ENV and VCF ENV) features a **visual EnvDisplay**
component with 4 draggable points:

- **Attack** — drag horizontally to adjust attack time
- **Decay** — drag to set decay time to sustain level
- **Sustain** — vertical drag sets the sustain plateau height
- **Release** — drag to adjust release time

The envelope shape updates in real time as you adjust the knobs, and dragging
the points on the visual display updates the corresponding parameter values.
The display uses a square-root time scale so that both short and long times
are easily editable.

### Ghost Signal Logo

The header features a custom **LogoComponent** rendering the "GHOST SIGNAL MS20P"
identity with a circuit-trace visual effect, LED accents, gradient styling,
and a dimmed "GS" monogram — replacing the previous plain text title label.

### Current Controls

- **VCO1/VCO2**: Waveform, Pulse Width, Octave, Gain, Tune
- **SUB**: Octave, Gain
- **NOISE**: Type, Gain
- **MIXER**: VCO1/VCO2/Sub Levels, Drive
- **FILTER**: HPF Cutoff/Res, LPF Cutoff/Res/Drive
- **SAT**: Drive
- **VCA ENV**: Attack, Decay, Sustain, Release + visual display
- **VCF ENV**: Attack, Decay, Sustain, Release + visual display
- **LFO1-4**: Waveform, Rate, Depth, Shape + visual display + Destination routing
- **AMP**: Gain, Pan
- **GLIDE**: Time, Voice Mode (Poly/Mono/Unison)
- **TAPE DELAY**: Enable, Time Mode, Time, Feedback, Mix, Age, Saturation, Wow, Flutter
- **MASTER**: Volume

## Troubleshooting

If you get build errors, delete the build folder and reconfigure:

```powershell
cmd /c rmdir /s /q build
cmake -S . -B build
```

## License

Proprietary. All rights reserved. Ghost Signal 2026.
