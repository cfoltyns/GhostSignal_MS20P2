/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#pragma once

#include <JuceHeader.h>
#include <array>

enum class ArpMode
{
    off,
    up,
    down,
    upDown,
    random,
    chord
};

struct ArpeggiatorParams
{
    double sampleRate { 44100.0 };
    ArpMode mode { ArpMode::off };
    int octaveRange { 1 };
    float gate { 0.9f };
    bool sync { true };
    float tempo { 120.0f };
    int direction { 0 }; // 0=up, 1=down, etc
};

class Arpeggiator
{
public:
    Arpeggiator() = default;
    ~Arpeggiator() = default;

    void prepare (double sampleRate, const ArpeggiatorParams& p);
    void reset();
    void setParameters (const ArpeggiatorParams& p);

    void noteOn (int noteNumber);
    void noteOff (int noteNumber);
    void allNotesOff();

    // Returns next note to trigger (or -1 if none)
    int getNextNote();

    bool isPlaying() const { return playing; }

private:
    ArpeggiatorParams params;
    
    std::array<int, 128> heldNotes;
    int heldNoteCount { 0 };
    int sortedNotes[128];
    
    int currentNoteIndex { 0 };
    int sequencePosition { 0 };
    bool playing { false };
    bool goingUp { true };
    
    double samplesPerStep { 0.0 };
    double stepCounter { 0.0 };
};