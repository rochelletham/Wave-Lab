
//==============================================================================
// WavetableOscillator.h
// Taken from JUCE's Wavetable tutorial
// URL https://docs.juce.com/master/tutorial_wavetable_synth.html
//==============================================================================

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

/// WavetableOscillator contains one period of a sampled waveform defined over
/// the number of samples in the table. The ending sample is set to be the same
/// as the starting sample.

class WavetableOscillator
{
public:
  WavetableOscillator (const AudioSampleBuffer& wavetableToUse)
  : wavetable (wavetableToUse),
  tableSize (wavetable.getNumSamples() - 1)
  {
    jassert (wavetable.getNumChannels() == 1);
  }
  
  void setFrequency (float frequency, float sampleRate)
  {
    /// For a one hertz tone we have to move over tableSize samples in one second. Since
    /// there are sampleRate samples per second, the table increment per sample would be
    /// tablesize/srate. For a two hertz tone we would have to move twice as fast, or
    /// 2 * tableSize/srate. In general, then, the table increment per sample will be
    /// frequency*(tableSize/srate).
    
    auto tableSizeOverSampleRate = tableSize / sampleRate;
    tableDelta = frequency * tableSizeOverSampleRate;
  }
  
  /// Uses linear interpolation to calculate the sample value for the (fractional) current index
  /// and table increment
  forcedinline float getNextSample() noexcept
  {
    /// Get current integer index (index0) and next index (index1)
    auto index0 = (unsigned int) currentIndex;
    auto index1 = index0 + 1;
    /// Calculate the difference between the floating point index (currentIndex) and the integer index
    auto frac = currentIndex - (float) index0;
    /// get the sample for the current index and for the next index
    auto* table = wavetable.getReadPointer (0);
    auto value0 = table[index0];
    auto value1 = table[index1];
    /// add to value 1 the proportional amount (frac) of the difference between the two samples.
    auto currentSample = value0 + frac * (value1 - value0);
    /// increment the currentIndex by the tableDelta (freq*sr/tableSize) and wrap if needed.
    if ((currentIndex += tableDelta) > tableSize)
      currentIndex -= tableSize;
    
    return currentSample;
  }
  
private:
  const AudioSampleBuffer& wavetable;
  const int tableSize;
  float currentIndex = 0.0f, tableDelta = 0.0f;
};
