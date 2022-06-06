//==============================================================================

#include "MainComponent.h"
#include "MainApplication.h"

MainComponent::MainComponent()
: deviceManager (MainApplication::getApp().audioDeviceManager), audioVisualizer(2) {
   addAndMakeVisible(playButton);
   playButton.addListener(this);
   drawPlayButton(playButton, true);

   addAndMakeVisible(freqLabel);
   freqLabel.setText("Frequency:", dontSendNotification);
   freqLabel.attachToComponent(&freqSlider, true);
   freqLabel.setJustificationType(1);


   addAndMakeVisible(freqSlider);
   freqSlider.setRange(0.0, 5000.0);
   freqSlider.setSliderStyle(Slider::LinearHorizontal);
   freqSlider.setTextBoxStyle(Slider::TextBoxLeft, true, 90, 22);
   freqSlider.addListener(this);
   freqSlider.setSkewFactorFromMidPoint(500);

   addAndMakeVisible(levelLabel);
   levelLabel.setText("Level:", dontSendNotification);
   levelLabel.attachToComponent(&levelSlider, true);
   levelLabel.setJustificationType(1);


   addAndMakeVisible(levelSlider);
   levelSlider.setRange(0.0, 1.0);
   levelSlider.setSliderStyle(Slider::LinearHorizontal);
   levelSlider.setTextBoxStyle(Slider::TextBoxLeft, true, 90, 22);
   levelSlider.addListener(this);

   addAndMakeVisible(settingsButton);
   settingsButton.addListener(this);

   // wavemenu
   addAndMakeVisible(waveformMenu);
   waveformMenu.setTextWhenNothingSelected("Waveforms");
//   StringArray noise {"White", "Brown", "Dust"};
   waveformMenu.addItemList(noise, WhiteNoise);
   waveformMenu.addSeparator();

   waveformMenu.addItem("Sine", SineWave);
   waveformMenu.addSeparator();

   StringArray LF {"LF Impulse", "LF Square", "LF Saw", "LF Triangle"};
   waveformMenu.addItemList(LF, LF_ImpulseWave);
   waveformMenu.addSeparator();

   StringArray BL {"BL Impulse", "BL Square", "BL Saw", "BL Triangle"};
   waveformMenu.addItemList(BL, BL_ImpulseWave);
   waveformMenu.addSeparator();


   StringArray WT {"WT SineWave", "WT Impulse", "WT Square", "WT Saw", "WT Triangle"};
   waveformMenu.addItemList(WT, WT_SineWave);
   waveformMenu.addSeparator();

   waveformMenu.addListener(this);

   addAndMakeVisible(audioVisualizer);
   this->deviceManager.addAudioCallback(&audioSourcePlayer);

}

MainComponent::~MainComponent() {
   audioSourcePlayer.setSource(nullptr);
   deviceManager.removeAudioCallback(&audioSourcePlayer);
}

//==============================================================================
// Component overrides
//==============================================================================

void MainComponent::paint (Graphics& g) {
   g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
//   g.setColour(Colours::red);
//   g.drawRect(playButton.getBounds());
}

void MainComponent::resized() {
   auto area = getLocalBounds();
   area.reduce(8,8);
   auto lineOne = area.removeFromTop(24);
   auto lineTwo = area.removeFromTop(32);

   settingsButton.setBounds(lineOne.removeFromLeft(118));
   lineOne.removeFromLeft(8);
   playButton.setBounds(lineOne.removeFromLeft(56));
   playButton.setSize(56, 56);

   lineTwo.removeFromTop(8);
   waveformMenu.setBounds(lineTwo.removeFromLeft(118));

   levelLabel.setBounds(lineOne.removeFromLeft(72));
   levelSlider.setBounds(lineOne);

   //to account for playbutton width
   lineTwo.removeFromLeft(8 + 56);
   lineTwo.removeFromLeft(8);
   freqLabel.setBounds(lineTwo.removeFromLeft(72));
   freqSlider.setBounds(lineTwo);

   auto insideArea = area.withTrimmedBottom(24).withTrimmedTop(8);
   audioVisualizer.setBounds(insideArea);
   auto bottomLine = area.removeFromBottom(24);
   bottomLine.removeFromRight(8);

}

void MainComponent::drawPlayButton(juce::DrawableButton& button, bool showPlay) {
  juce::Path path;
  if (showPlay) {
     path.addTriangle(0, 0, 0, 100, 86.6, 50);
  }
  else {
    // draw the two bars
     path.addRectangle(0.0, 0.0, 42, 100);
     path.addRectangle(100, 0.0, 42, 100);
  }
  juce::DrawablePath drawable;
  drawable.setPath(path);
  juce::FillType fill (Colours::white);
  drawable.setFill(fill);
  button.setImages(&drawable);
}

//==============================================================================
// Listener overrides
//==============================================================================

void MainComponent::buttonClicked (Button *button) {
   if (button == &playButton) {
//      std::cout << "play button" <<std::endl;
      if (isPlaying()) {
         audioSourcePlayer.setSource(nullptr);
         drawPlayButton(playButton, !isPlaying());
//         std::cout << "setting source to null" <<std::endl;
      } else {
         audioSourcePlayer.setSource(this);
//         std::cout << "setting source to this" <<std::endl;
         drawPlayButton(playButton, !isPlaying());
      }


   } else if (button == &settingsButton) {
      openAudioSettings();
//      std::cout << "settings button" <<std::endl;
   }
}

void MainComponent::sliderValueChanged (Slider *slider) {
   if (slider == &levelSlider) {
      level = levelSlider.getValue();
//      std::cout << "levelslider" <<std::endl;
   } else if (slider == &freqSlider) {
      freq = freqSlider.getValue();
      phaseDelta = freq/srate;
//      std::cout << "freqslider" <<std::endl;
      for (auto & index:oscillators) {
         index->setFrequency(freq, srate);
      }
   }
}

void MainComponent::comboBoxChanged (ComboBox *menu) {
   waveformId = static_cast<MainComponent::WaveformId>(menu->getSelectedId());
   if (menu == &waveformMenu) {
      if (waveformMenu.getSelectedIdAsValue() == WhiteNoise || waveformMenu.getSelectedIdAsValue() == BrownNoise) {
         freqSlider.setEnabled(false);
      } else {
         freqSlider.setEnabled(true);
      }
   }
}

//==============================================================================
// Timer overrides
//==============================================================================
void MainComponent::timerCallback() {
   cpuUsage.setText(String(deviceManager.getCpuUsage() * 100, 2, false) + " %", dontSendNotification);
}

//==============================================================================
// AudioSource overrides
//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate) {
   srate = sampleRate;
   phase = 0;
   phaseDelta = freq/srate;
   // specifically for sine wave
//   phaseDelta = srate * 2.0 * MathConstants<double>::pi;
   audioVisualizer.setBufferSize(samplesPerBlockExpected);
   //display 8 blocks concurrently
   audioVisualizer.setSamplesPerBlock(8);
   createWaveTables();
   for (auto & index:oscillators) {
      index->setFrequency(freq, srate);
   }
}

void MainComponent::releaseResources() {
//   std::cout << "releasing" <<std::endl;
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) {
  bufferToFill.clearActiveBufferRegion();

  switch (waveformId) {
    case WhiteNoise:      whiteNoise(bufferToFill);   break;
    case DustNoise:       dust(bufferToFill);         break;
    case BrownNoise:      brownNoise(bufferToFill);   break;
    case SineWave:        sineWave(bufferToFill);     break;
    case LF_ImpulseWave:  LF_impulseWave(bufferToFill);  break;
    case LF_SquareWave:   LF_squareWave(bufferToFill);   break;
    case LF_SawtoothWave: LF_sawtoothWave(bufferToFill); break;
    case LF_TriangeWave:  LF_triangleWave(bufferToFill); break;
    case BL_ImpulseWave:  BL_impulseWave(bufferToFill);  break;
    case BL_SquareWave:   BL_squareWave(bufferToFill);   break;
    case BL_SawtoothWave: BL_sawtoothWave(bufferToFill); break;
    case BL_TriangeWave:  BL_triangleWave(bufferToFill); break;
     case WT_SineWave:
    case WT_ImpulseWave:
    case WT_SquareWave:
    case WT_SawtoothWave:
    case WT_TriangleWave:
      WT_wave(bufferToFill);
      break;
    case Empty:
      break;
  }
  audioVisualizer.pushBuffer(bufferToFill);

}

//==============================================================================
// Audio Utilities
//==============================================================================

double MainComponent::phasor() {
  return fmod(phase + phaseDelta, 1);

}

float MainComponent::ranSamp() {
   // returns rand from 0 <->1
   return (random.Random::nextFloat()) *2 - 1;
}

float MainComponent::ranSamp(const float mul) {
  return (ranSamp() * mul);
}

float MainComponent::lowPass(const float value, const float prevout, const float alpha) {
   auto output = prevout + alpha * (value - prevout);
   return output;
}

bool MainComponent::isPlaying() {
   return audioSourcePlayer.getCurrentSource() != nullptr;
}

void MainComponent::openAudioSettings() {
   auto devComp = std::make_unique<AudioDeviceSelectorComponent>(this->deviceManager,0,2,0,2,true, false, true, false);
   DialogWindow::LaunchOptions dw;
   devComp->setSize(500, 270);
   dw.dialogTitle = "Audio Settings";
   dw.useNativeTitleBar = true;
   dw.resizable = false;
   dw.dialogBackgroundColour = this->getLookAndFeel().findColour(ResizableWindow::backgroundColourId);
   dw.DialogWindow::LaunchOptions::content.setOwned(devComp.release());
   dw.DialogWindow::LaunchOptions::launchAsync();

}

void MainComponent::createWaveTables() {
  createSineTable(sineTable);
  oscillators.push_back(std::make_unique<WavetableOscillator>(sineTable));
  createImpulseTable(impulseTable);
  oscillators.push_back(std::make_unique<WavetableOscillator>(impulseTable));
  createSquareTable(squareTable);
  oscillators.push_back(std::make_unique<WavetableOscillator>(squareTable));
  createSawtoothTable(sawtoothTable);
  oscillators.push_back(std::make_unique<WavetableOscillator>(sawtoothTable));
  createTriangleTable(triangleTable);
  oscillators.push_back(std::make_unique<WavetableOscillator>(triangleTable));
}

//==============================================================================
// Noise
//==============================================================================

// White Noise

void MainComponent::whiteNoise (const AudioSourceChannelInfo& bufferToFill) {
   // process every channel of data
   for (int chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan) {
      // get the pointer to the first sample in the channel to process
      float* const channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
      // iterate the pointer over all the samples in the channel data
      for (int i = 0; i < bufferToFill.numSamples ; ++i) {
         channelData[i] = ranSamp(level); // assign a value to the current sample
      }
   }
}


void MainComponent::brownNoise (const AudioSourceChannelInfo& bufferToFill) {
   // process every channel of data
   for (int chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan) {
      // get the pointer to the first sample in the channel to process
      float* const channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
      // iterate the pointer over all the samples in the channel data
      for (int i = 0; i < bufferToFill.numSamples ; ++i) {
         channelData[i] = ranSamp(level); // assign a value to the current sample
         //using white noise, use low pass filter
         channelData[i] = lowPass(channelData[i], channelData[i - 1], 0.5);
      }
   }
}
// Generates random uniform samples with a probability of freq/srate across all channels
void MainComponent::dust (const AudioSourceChannelInfo& bufferToFill) {
      // process every channel of data
      auto probability = freq/(srate * bufferToFill.buffer->getNumChannels());
      for (int chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan) {
         // get the pointer to the first sample in the channel to process
         float* const channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
         // iterate the pointer over all the samples in the channel data
         for (int i = 0; i < bufferToFill.numSamples ; ++i) {
            if (random.nextDouble() < probability) {
               channelData[i] = ranSamp(level); // assign a value to the current sample
            }
         }
      }

}

//==============================================================================
// Sine Wave
//==============================================================================

void MainComponent::sineWave (const AudioSourceChannelInfo& bufferToFill)
{
   double initialPhase = phase;
   for (int chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan)
   {
      phase = initialPhase;
      float* const channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
      for (int i = 0; i < bufferToFill.numSamples ; ++i)
      {
         channelData[i] = (float) std::sin(TwoPi * phase) * level;
         phase += phaseDelta;
      }
   }
}

//==============================================================================
// Low Frequency Waveforms
//==============================================================================

/// Impulse wave
void MainComponent::LF_impulseWave (const AudioSourceChannelInfo& bufferToFill) {
//current - prev -->neg --> started a new period
   double initialPhase = phase;
   for (int chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan)
   {
      phase = initialPhase;
      float* const channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
      for (int i = 0; i < bufferToFill.numSamples ; ++i) {
         auto prev = phasor();
         phase += phaseDelta;
         auto current = phasor();
         if (current - prev < 0) {
         // if phasor decrease -> output level
            channelData[i] = (float) level;
         }
      }
   }
}


/// Square wave

void MainComponent::LF_squareWave (const AudioSourceChannelInfo& bufferToFill) {
   double initialPhase = phase;
   for (int chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan)
   {
      phase = initialPhase;
      float* const channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
      for (int i = 0; i < bufferToFill.numSamples ; ++i) {

         if (phasor() < .5) {
            channelData[i] = (float) -1*level;
         } else {
            channelData[i] = (float) level;
         }
         phase += phaseDelta;
      }
   }
}

/// Sawtooth wave

void MainComponent::LF_sawtoothWave (const AudioSourceChannelInfo& bufferToFill) {
   double initialPhase = phase;
   for (int chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan)
   {
      phase = initialPhase;
      float* const channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
      for (int i = 0; i < bufferToFill.numSamples ; ++i) {
         //phasor is scaled by 2 and offset by 1
         channelData[i] = (float) (phasor() * 2 - 1) * level;
         phase += phaseDelta;
      }
   }
}

/// Triangle wave

void MainComponent::LF_triangleWave (const AudioSourceChannelInfo& bufferToFill) {
   double initialPhase = phase;
   for (int chan = 0; chan < bufferToFill.buffer->getNumChannels(); ++chan)
   {
      phase = initialPhase;
      float* const channelData = bufferToFill.buffer->getWritePointer(chan, bufferToFill.startSample);
      for (int i = 0; i < bufferToFill.numSamples ; ++i) {
            // phasor in first half
         if (phasor() < .5) {
            channelData[i] = (float) (phasor() * 4 - 1) * level;
         } else {
            channelData[i] = (float) ((phasor() * -1 * 4 + 3) *level);
         }
         phase += phaseDelta;
      }
   }
}

//==============================================================================
// Band Limited Waveforms
//==============================================================================

/// Impulse (pulse) wave
/// Synthesized by summing sin() over frequency and all its harmonics at equal
/// amplitude. To make it band limited only include harmonics that are at or
/// below the nyquist limit.
//nyquist limit = highest freq can represent
//sampling rate/ 2
void MainComponent::BL_impulseWave (const AudioSourceChannelInfo& bufferToFill) {
   double initialPhase = phase;
   phase = initialPhase;
   float* const chan0 = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
   float* const chan1 = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);
   for (int i = 0; i < bufferToFill.numSamples; ++i)
   {
      if (freq != 0) {
         auto numHarmonic = srate/2/freq;
         for (auto u = 1; u <= numHarmonic; ++u) {
            chan0[i] += (float) std::sin(TwoPi * phasor() * u) * (level/numHarmonic);
         }
         phase += phaseDelta;
      }
   }
   std::memcpy(chan1, chan0, bufferToFill.numSamples * sizeof(float));

}

/// Square wave
/// Synthesized by summing sin() over all ODD harmonics at 1/harmonic amplitude.
/// To make it band limited only include harmonics that are at or below the
/// nyquist limit.
void MainComponent::BL_squareWave (const AudioSourceChannelInfo& bufferToFill) {
   double initialPhase = phase;
   phase = initialPhase;
   float* const chan0 = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
   float* const chan1 = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);
   for (int i = 0; i < bufferToFill.numSamples; ++i)
   {
      if (freq != 0) {
         auto numHarmonic = srate/2/freq;
         for (auto u = 1; u <= numHarmonic; ++u) {
            if (u % 2 != 0) {
               chan0[i] += (float) std::sin(TwoPi * phasor() * u) * (level/u);
            }
         }
      }
      phase += phaseDelta;
   }
   std::memcpy(chan1, chan0, bufferToFill.numSamples * sizeof(float));

}

/// Sawtooth wave
/// Synthesized by summing sin() over all harmonics at 1/harmonic amplitude. To make
/// it band limited only include harmonics that are at or below the nyquist limit.
void MainComponent::BL_sawtoothWave (const AudioSourceChannelInfo& bufferToFill) {
   double initialPhase = phase;
   phase = initialPhase;
   float* const chan0 = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
   float* const chan1 = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);
   if (freq == 0) {
      return;
   }
   for (int i = 0; i < bufferToFill.numSamples; ++i)
   {
      if (freq != 0) {
         auto numHarmonic = srate/2/freq;
         for (auto u = 1; u <= numHarmonic; ++u) {
            chan0[i] += (float) std::sin(TwoPi * phasor() * u) * (level/u);
         }
         phase += phaseDelta;
      }
   }
   std::memcpy(chan1, chan0, bufferToFill.numSamples * sizeof(float));
}

/// Triangle wave
/// Synthesized by summing sin() over all ODD harmonics at 1/harmonic**2 amplitude.
/// To make it band limited only include harmonics that are at or below the
/// Nyquist limit.
void MainComponent::BL_triangleWave (const AudioSourceChannelInfo& bufferToFill) {
   double initialPhase = phase;
   phase = initialPhase;
   float* const chan0 = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
   float* const chan1 = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);
   if (freq != 0) {

      for (int i = 0; i < bufferToFill.numSamples; ++i)
      {
         auto numHarmonic = srate/2/freq;
         if (freq != 0) {
            for (auto u = 1; u <= numHarmonic; ++u) {
               if (u % 2 != 0) {
                  chan0[i] += (float) std::sin(TwoPi * phasor() * u) * (level/pow(u,2));
               }
            }
         }
         phase += phaseDelta;
      }
      std::memcpy(chan1, chan0, bufferToFill.numSamples * sizeof(float));
   }
}

//==============================================================================
// WaveTable Synthesis
//==============================================================================

// The audio block loop
void inline MainComponent::WT_wave(const AudioSourceChannelInfo& bufferToFill) {
   double initialPhase = phase;
   phase = initialPhase;
   float* const chan0 = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
   float* const chan1 = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

   for (int i = 0; i < bufferToFill.numSamples; ++i) {
      chan0[i] = oscillators[waveformId-WT_START]->getNextSample() *level;
   }
   std::memcpy(chan1, chan0, bufferToFill.numSamples * sizeof(float));
}


// Create a sine wave table
void MainComponent::createSineTable(AudioSampleBuffer& waveTable) {
  waveTable.setSize (1, tableSize + 1);
  waveTable.clear();
  auto* samples = waveTable.getWritePointer (0);
  auto phase = 0.0;
  auto phaseDelta = MathConstants<double>::twoPi / (double) (tableSize - 1);
  for (auto i = 0; i < tableSize; ++i) {
    samples[i] += std::sin(phase);
    phase += phaseDelta;
  }
  samples[tableSize] = samples[0];
}


void MainComponent::createImpulseTable(AudioSampleBuffer& waveTable) {
   waveTable.setSize (1, tableSize + 1);
   waveTable.clear();
   auto* samples = waveTable.getWritePointer (0);
   auto phase = 0.0;
   for (auto i = 0; i < tableSize; ++i) {
      auto numHarmonic = srate/2;
//      auto numHarmonic = srate/2/freq;
      for (auto u = 1; u <= numHarmonic; ++u) {
         samples[i] += std::sin(phase);
      }
      samples[i] *= 1/numHarmonic;
      phase += phaseDelta;
   }
   //keep this
   samples[tableSize] = samples[0];
}

// Create a square wave table
void MainComponent::createSquareTable(AudioSampleBuffer& waveTable) {
   waveTable.setSize (1, tableSize + 1);
   waveTable.clear();
   auto* samples = waveTable.getWritePointer (0);
   auto phase = 0.0;
   auto phaseDelta = MathConstants<double>::twoPi / (double) (tableSize - 1);
   for (auto i = 0; i < tableSize; ++i) {
         auto numHarmonic = srate/2;
         for (auto u = 1; u <= numHarmonic; ++u) {
            if (u % 2 != 0) {
               samples[i] += (float) std::sin(u * phase) / u;
            }
         }
         phase += phaseDelta;
   }
   //keep this
   samples[tableSize] = samples[0];
}

// Create a sawtooth wave table
void MainComponent::createSawtoothTable(AudioSampleBuffer& waveTable) {
   waveTable.setSize (1, tableSize + 1);
   waveTable.clear();
   auto* samples = waveTable.getWritePointer (0);
   auto phase = 0.0;
   auto phaseDelta = MathConstants<double>::twoPi / (double) (tableSize - 1);
   for (auto i = 0; i < tableSize; ++i) {
      auto numHarmonic = srate/2;
      for (auto u = 1; u <= numHarmonic; ++u) {
         samples[i] += (float) std::sin(u * phase) / u;
      }
      samples[i] = samples[i]/1.6;
      phase += phaseDelta;
      }
   //keep this
   samples[tableSize] = samples[0];
}

// Create a triagle wave table
void MainComponent::createTriangleTable(AudioSampleBuffer& waveTable) {
   waveTable.setSize (1, tableSize + 1);
   waveTable.clear();
   auto* samples = waveTable.getWritePointer (0);
   auto phase = 0.0;
   auto phaseDelta = MathConstants<double>::twoPi / (double) (tableSize - 1);
   for (auto i = 0; i < tableSize; ++i) {
      auto numHarmonic = srate/2;
      for (auto u = 1; u <= numHarmonic; ++u) {
         if (u % 2 != 0) {
            samples[i] += (float) std::sin(u * phase) / pow(u, 2);
         }
      }
      phase += phaseDelta;
   }
   //keep this
   samples[tableSize] = samples[0];
}
