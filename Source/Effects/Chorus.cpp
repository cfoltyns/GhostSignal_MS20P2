/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#include "Chorus.h"
#include <cmath>

namespace fx
{
    void Chorus::prepare (double newSampleRate)
    {
        params.sampleRate = newSampleRate;
        // Max delay ~50ms should cover chorus needs
        bufferSamples = (int) (newSampleRate * 0.05);
        delayBuffer.setSize (2, bufferSamples + 100);
        reset();
    }

    void Chorus::reset()
    {
        delayBuffer.clear();
        writePos = 0;
        lfoPhase[0] = lfoPhase[1] = 0.0f;
    }

    void Chorus::setParameters (const ChorusParams& p)
    {
        params = p;
    }

    void Chorus::process (juce::AudioBuffer<float>& buffer, int numSamples)
    {
        const float rate = params.rate;
        const float depth = params.depth;
        const float centreDelay = params.centreDelay * params.sampleRate;
        const float maxDelay = depth * centreDelay;

        for (int i = 0; i < numSamples; ++i)
        {
            // Update LFO phases (stereo with 90-degree offset)
            lfoPhase[0] += (float) (rate * juce::MathConstants<float>::twoPi / params.sampleRate);
            lfoPhase[1] += (float) (rate * juce::MathConstants<float>::twoPi / params.sampleRate);
            
            if (lfoPhase[0] > juce::MathConstants<float>::twoPi)
                lfoPhase[0] -= juce::MathConstants<float>::twoPi;
            if (lfoPhase[1] > juce::MathConstants<float>::twoPi)
                lfoPhase[1] -= juce::MathConstants<float>::twoPi;

            const float delayOffsetL = std::sin (lfoPhase[0]) * maxDelay;
            const float delayOffsetR = std::sin (lfoPhase[1]) * maxDelay;

            for (int ch = 0; ch < juce::jmin (2, buffer.getNumChannels()); ++ch)
            {
                const float in = buffer.getSample (ch, i);
                
                // Calculate read position
                double delaySamples = centreDelay + (ch == 0 ? delayOffsetL : delayOffsetR);
                int readPos = writePos - (int) delaySamples;
                if (readPos < 0)
                    readPos += bufferSamples;
                
                // Ensure readPos is within valid range
                readPos = juce::jlimit (0, bufferSamples - 1, readPos);

                const float delayed = delayBuffer.getSample (ch, readPos);
                
                // Write to delay line
                delayBuffer.setSample (ch, writePos, in + delayed * params.feedback);
                
                // Output
                const float wet = params.mix;
                buffer.setSample (ch, i, in * (1.0f - wet) + delayed * wet);
            }

            writePos++;
            if (writePos >= bufferSamples)
                writePos = 0;
        }
    }
}