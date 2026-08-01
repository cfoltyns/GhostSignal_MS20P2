/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#include "Delay.h"
#include <cmath>

namespace fx
{
    void Delay::prepare (double newSampleRate)
    {
        params.sampleRate = newSampleRate;
        bufferSamples = (int) (newSampleRate * 5.0); // 5 seconds max
        buffer.setSize (2, bufferSamples + 1000);
        reset();
    }

    void Delay::reset()
    {
        buffer.clear();
        writePosition = 0;
        lfoPhase = 0.0f;
    }

    void Delay::setParameters (const DelayParams& p)
    {
        params = p;
    }

    float Delay::readInterpolated (int channel, double delayInSamples)
    {
        if (bufferSamples <= 0)
            return 0.0f;

        const int readPos = (int) (writePosition - (int) delayInSamples);
        const int wrappedPos = (readPos + bufferSamples) % bufferSamples;
        
        const float sample = buffer.getSample (channel, wrappedPos);
        return sample; // Linear interpolation simplified
    }

    void Delay::writeSample (int channel, int index, float sample)
    {
        if (bufferSamples <= 0)
            return;

        buffer.setSample (channel, index % bufferSamples, sample);
    }

    void Delay::process (juce::AudioBuffer<float>& buffer, int numSamples)
    {
        // Guard against uninitialized delay buffer
        if (bufferSamples <= 0)
            return;

        const float feedback = params.feedback;
        const float wetMix = params.mix;
        const float dryMix = 1.0f - wetMix;

        // Calculate delay time with modulation
        const double baseDelay = params.time * params.sampleRate;
        const double modAmount = params.modulation * 0.5 * params.sampleRate / 1000.0; // ms to samples
        const double modPhaseInc = params.sampleRate / (2.0 * params.sampleRate); // 0.5 Hz default

        for (int i = 0; i < numSamples; ++i)
        {
            // Update LFO phase
            lfoPhase += (float) modPhaseInc;
            if (lfoPhase > juce::MathConstants<float>::twoPi)
                lfoPhase -= juce::MathConstants<float>::twoPi;

            const double delayL = baseDelay + std::sin (lfoPhase) * modAmount;
            const double delayR = baseDelay + std::sin (lfoPhase + juce::MathConstants<float>::halfPi) * modAmount;

            for (int ch = 0; ch < juce::jmin (2, buffer.getNumChannels()); ++ch)
            {
                const float in = buffer.getSample (ch, i);
                
                // Read from delay line
                const float delayed = (ch == 0) 
                    ? readInterpolated (0, delayL) 
                    : readInterpolated (1, delayR);

                // Write to delay line with feedback
                writeSample (ch, writePosition, in + delayed * feedback);

                // Output
                buffer.setSample (ch, i, in * dryMix + delayed * wetMix);
            }

            writePosition++;
            if (writePosition >= bufferSamples)
                writePosition = 0;
        }
    }
}