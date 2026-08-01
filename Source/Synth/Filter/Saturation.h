/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#pragma once

#include <JuceHeader.h>

namespace dsp::filter
{
    struct SaturationParams
    {
        float drive { 0.0f };
        float bias { 0.0f };
        float mix { 1.0f };
        int   model { 0 }; // 0=off,1=MS20 input,2=tube,3=tape,4=germanium,5=diode,6=opamp,7=soft,8=hard,9=wavefolder
    };

    class Saturation
    {
    public:
        Saturation() = default;
        ~Saturation() = default;

        void prepare (double /*sampleRate*/) {}
        void reset() {}
        void setParameters (const SaturationParams& p) { params = p; }

        void process (juce::AudioBuffer<float>& buffer, int numSamples);

    private:
        float applyDrive (float x, float drive);
        float wavefolder (float x, float bias);

        SaturationParams params;
    };
}
