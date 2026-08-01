/*
 * This file is part of Ghost Signal MS20P.
 *
 * (c) 2026 Ghost Signal
 */

#pragma once

namespace dsp::filter
{
    enum class FilterModel
    {
        k35Vintage,
        ota,
        ladder,
        stateVariable,
        cleanDigital
    };

    enum class FilterMode { lowpass, highpass, bandpass };
}
