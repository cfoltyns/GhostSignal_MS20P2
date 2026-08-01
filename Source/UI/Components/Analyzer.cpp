/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#include "Analyzer.h"

using namespace juce;

Analyzer::Analyzer()
{
    fifo.setSize (1, fftSize * 2);
    waveformBuffer.setSize (1, waveformSize);
}

Analyzer::~Analyzer()
{
}

void Analyzer::paint (Graphics& g)
{
    g.setColour (bgColour);
    g.fillAll();

    const auto bounds = getLocalBounds().toFloat();
    const float width = bounds.getWidth();
    const float height = bounds.getHeight();
    const float centreY = height * 0.5f;

    // Draw grid lines
    g.setColour (lineColour.withAlpha (0.1f));
    for (int i = 0; i < 4; ++i)
    {
        float y = height * (i + 1) * 0.2f;
        g.drawHorizontalLine ((int) y, 0.0f, width);
    }
    for (int i = 0; i < 8; ++i)
    {
        float x = width * (i + 1) * 0.125f;
        g.drawVerticalLine ((int) x, 0.0f, height);
    }

    // Draw centre line
    g.setColour (lineColour.withAlpha (0.2f));
    g.drawHorizontalLine ((int) centreY, 0.0f, width);

    // Draw waveform
    g.setColour (lineColour);
    Path path;
    bool firstPoint = true;

    for (int i = 0; i < waveformSize; ++i)
    {
        float sample = waveformBuffer.getSample (0, i);
        float x = (float) i / (float) waveformSize * width;
        float y = centreY - (sample * height * 0.4f); // scale to 80% of height
        
        y = jlimit (0.0f, height, y);

        if (firstPoint)
        {
            path.startNewSubPath (x, y);
            firstPoint = false;
        }
        else
        {
            path.lineTo (x, y);
        }
    }

    g.strokePath (path, PathStrokeType (1.5f));
}

void Analyzer::resized()
{
}

void Analyzer::pushBuffer (const AudioBuffer<float>& buffer, int numSamples)
{
    if (buffer.getNumChannels() > 0 && numSamples > 0)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            // Get mono sample
            float mono = 0.0f;
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                mono += buffer.getSample (ch, i);
            mono /= (float) buffer.getNumChannels();
            
            // Write to waveform buffer
            waveformBuffer.setSample (0, waveformIndex, mono);
            waveformIndex++;
            
            // Write to FFT fifo
            fifo.setSample (0, fifoIndex, mono);
            fifoIndex++;

            if (waveformIndex >= waveformSize)
                waveformIndex = 0;

            if (fifoIndex >= fftSize * 2)
            {
                fifoIndex = 0;
                processFFT();
            }
        }
        
        repaint();
    }
}

void Analyzer::processFFT()
{
    // Simple magnitude calculation
    for (int i = 0; i < fftSize; ++i)
    {
        float window = (float) i / (float) fftSize;
        fftData[i] = fifo.getSample (0, i) * window;
    }

    // Calculate magnitude (simplified - just RMS per bin)
    const int numBins = 1024;
    const int binSize = fftSize / numBins;
    
    for (int bin = 0; bin < numBins; ++bin)
    {
        float sum = 0.0f;
        for (int i = 0; i < binSize; ++i)
        {
            float sample = fftData[bin * binSize + i];
            sum += sample * sample;
        }
        magnitude[bin] = std::sqrt (sum / (float) binSize) * 10.0f;
    }
}