//==============================================================================
// MainComponent.h
// This file defines the class representing our main window content component.
//==============================================================================

#pragma once

#include "WavetableOscillator.h"

/// MainComponent provides the app's user controls and content. NOTE: this
/// must inherit from three listener classes to respond to user interactions
/// with its buttons, menus and sliders.
struct MainComponent : public Component, public AudioSource,
public Slider::Listener, public Button::Listener, public ComboBox::Listener, public Timer
{
  /// Constructor.
  /// * Add and make visible all subcomponents.
  /// * Add the main component as a listener to all the buttons and sliders.
  /// * The ComboBox (menu) should display "Waveforms" if nothing is selected in the menu.
  /// * The menu has 5 sections, use ComboBox::addItemList() to add each section.
  ///   After each section add a separator item (See ComboBox::addSeparator())
  /// - The first section contains the strings "White", "Brown", "Dust" and starts
  /// with the id WhiteNoise.
  /// - The second section contains just the string "Sine" with the id SineWave.
  /// - The third section contains "LF Impulse", "LF Square", "LF Saw", "LF Triangle" and
  /// its ids start with LF_ImpulseWave.
  /// - The fourth section contains "BL Impulse", "BL Square", "BL Saw", "BL Triangle"
  /// and starts with BL_ImpulseWave.
  /// - The fifth section contains "WT Sine", "WT Impulse", "WT Square", "WT Saw", "WT Triangle"
  ///  and starts with WT_SineWave.
  /// *  Add the level slider to MainComponent with proper text box style
  /// and range (0.0-1.0).
  /// * Both slider textboxes should be initilized to Slider::TextBoxLeft with a width of
  /// 90 and height of 22. The level slider should have a range of 0 to 1.
  /// * The frequecy slider should range from 0.0, 5000.0 and should be initially disabled
  /// (It will enabled whenever the menu selection has a frequency.) Set its "mid point
  /// skew factor" to 500 (See: Slider::setSkewFactorFromMidPoint()),
  /// * The play button should initially be disabled and draw the button in its initial Play mode.
  /// * Set the label justification of the cpu lable and display to Justification::centredRight.
  /// * Finally, add our audioSourcePlayer to the audio device manager for audio
  ///   streaming.  Since our component inherits from AudioSource, it will become the audio
  ///   source for the player to stream when we call audioSourcePlayer.setSource(this);
  MainComponent();

  /// Destructor.
  /// * Set audioSourcePlayer's source to nullptr.
  /// * Remove the audioSourcePlayer as the deviceManager's callback.
  /// * Close the deviceManager.
  ~MainComponent();
  
  //==============================================================================
  // Component overrides

  /// Called when the MainCompoent is painted. This should set MainComponent's
  /// background color to the default background color for DocumentWindow
  /// (see the paint function of the Basic GUI.app for more information.)
  void paint (Graphics&) override;

  /// Called when the MainComponent is resized. This method should take the
  /// following actions:
  /// * Unless otherwise stated the height of all components is 24 pixels.
  /// * All subcomponents except the CPU display line are inset from
  ///   MainComponent's top, left and right by 8 pixels
  /// * The visualizer is inset from the bottom by 24 pixels.
  /// * The width of the Audio Settings button and the Waveforms menu is 118 pixels.
  /// * There is an 8 pixel offset between the buttons and the transport button.
  /// * The width and height of the transport button is 56.
  /// * The Level and Frequency labels are right-justified and their width is 72.
  /// * The level and frequency sliders abut the labels and take the remainder
  ///   of the space on their lines.
  /// * The width of the cpu usage display is 66 pixels, its Y is 24 pixels from the bottom
  ///   and it is idented from the right by 8 pixels.
  /// * The cpu label is 36 pixels width and abuts the left side of the usage display.
  /// Look at Wave Lab.app image in the documentation to check your layout:
  /// http://cmp.music.illinois.edu/courses/taube/mus205/Projects/Wave%20Lab/WaveLabOutlines.png

  void resized() override;

  //==============================================================================
  // Listener overrides

  /// MainComponent's button callback. If the button is the settingsButton the
  /// then openAudioSettings() should be called. Otherwise the playButton was
  /// pressed and the following action should be taken:
  /// * If the mainComponent is playing then playback should stop by
  /// setting the source to nullptr and the playButton should be redrawn showing
  /// the play symbol (triangle).
  /// * Otherwise playButton should start playback by adding our component to the
  /// player and the button should redraw itself with the stop symbol (double
  /// vertical bars) showing.
  void buttonClicked (Button *button) override;

  /// MainComponent's slider callback. If slider is levelSlider the function
  /// should update the 'level' variable with the current slider
  /// value.  If slider is freqSlider it should update the 'freq' and
  /// 'phaseDelta' variables.
  void sliderValueChanged (Slider *slider) override;

  /// MainComponent's comboBoxChanged callback. The function should set
  /// the waveformId member with the selected waveform id. If the id
  /// is Empty then the playButton should be disabled otherwise the
  /// playButton should be enabled. If the id is WhiteNoise or BrownNoise
  /// then the frequency label and slider should be disabled otherwise
  /// they should be enabled.
  void comboBoxChanged (ComboBox *menu) override;
  
  //==============================================================================
  // Timer overrides
  /// The timer callback shuld get the AudioDeviceManager's cpu usage, convert
  /// it to percentage, and set the cpuUsage label to that value rounded
  /// to two digits (see String(int)) with the string " %" appended to it.
  void timerCallback() override;
  
  //==============================================================================
  // AudioSource overrides
  
  /// This function will be called (on the audio thread, not the GUI
  /// thread) when the audio device is started, or when its settings
  /// (i.e. sample rate, block size, etc) are changed.
  /// It should set the srate to the current sampling rate. set phase to 0, and
  /// set phaseDelta to the phase increment for the current frequency
  /// and srate. The visualizer's buffer size should be set to samplesPerBlockExpected
  /// and it should take 8 samples per block.
  void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override ;
  
  /// Your audio-processing code goes in this function.  This function
  /// will simply call one of the private inline waveform functions
  /// (e.g whiteNoise(), sineWave(), etc.)  according to what
  /// WaveFormId the user has selected.
  void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override ;
  
  /// This will be called when the audio device stops, or when it is
  /// being restarted due to a setting change.
  void releaseResources() override ;
  
  //==============================================================================
  // MainComponent members

  /// Returns true if the component is generating audio, otherwise
  /// false. If the player's source is null then the player is not
  /// generating audio, and if the player's source is not null then it
  /// is playing.
  bool isPlaying();
  
  /// Opens the application's audio settings panel.
  /// * use launchOptions to configure and launch the audio settings dialog
  /// as follows:
  ///    - it uses a native title bar,
  ///    - it is not resizable,
  ///    - it title is "Audio Settings",
  ///    - its background color is black,
  /// * use launchOptions.content.setOwned() to assign the component
  /// * call launchOptions.launchAsync() to open the dialog.
  void openAudioSettings();
  
  /// Draws the play button. Since the image will be scaled by the button use
  /// percentage coordinates (0-100) for x and y. If drawPlay is true the button
  /// displays an isoceles triangle pointing to the right. To do this
  /// call path.addTriangle() with base of 100 and the height of the triangle
  /// set to √3 / 2 * base. If drawPlay is false then the button displays
  /// two vertical bars. Call path.addRectangle()
  /// to add the vertical bars (rectangles) with width 42 and height 100
  /// draw first bar at x=0  and second bar at 100-width.
  void drawPlayButton(juce::DrawableButton& b, bool drawPlay) ;

  /// Called at startup to create the wavetables.
  void createWaveTables();

private:
   StringArray noise {"White", "Brown", "Dust"};
   
  /// Enumeration identifying all the different waveforms the app
  /// generates. The Empty value indicate that no waveform has
  /// been selected.
  enum WaveformId {
    Empty, WhiteNoise,  BrownNoise, DustNoise,
    SineWave,
    LF_ImpulseWave, LF_SquareWave, LF_SawtoothWave, LF_TriangeWave,
    BL_ImpulseWave, BL_SquareWave, BL_SawtoothWave, BL_TriangeWave,
    WT_SineWave,
    WT_ImpulseWave, WT_SquareWave, WT_SawtoothWave, WT_TriangleWave,
    WT_START = WT_SineWave
  };

  /// A variable holding the currently selected waveform to generate
  /// (see the WaveformId enum).  Its initial value should be Empty.
  WaveformId waveformId;

  /// A Random object for generating random numbers.
  Random random;

  /// A reference to the app's audio device manager.
  AudioDeviceManager& deviceManager;

  /// An object that plays an audio source. To stream audio the
  /// player must first be added to the device manager using
  /// deviceManager.addAudioCallback(), and then an audio source
  /// must be passed to the player using player.addSource().
  AudioSourcePlayer audioSourcePlayer;

  /// A specialized JUCE component that displays a wave form.
  AudioVisualiserComponent audioVisualizer;
  
  /// A button that opens the audio preferences window. Initialize
  /// the button to show "Audio Settings...".
   TextButton settingsButton {"Audio Settings..."};
  
  /// A transport button to start and stop audio output. When
  /// audio is not playing the transport should display the play
  /// symbol, which is a triangle pointing to the right. When
  /// audio is playing the button should display the pause symbol,
  /// which is two vertical stripes.
  DrawableButton playButton {"", juce::DrawableButton::ImageOnButtonBackground};

  /// A label that displays the text "Level:"
  Label levelLabel {"Level:"};

  /// A slider to control the gain (amplitude) of the output
  /// signal. It's range should be [0.0, 1.0]. Its style should be
  /// Slider::LinearHorizontal.  Its text box style should be
  /// Slider::TextBoxLeft and sized to a width of 90 and height of 22.
  Slider levelSlider;

  /// A label that displays the text "Frequency:"
   Label freqLabel {"Frequency"};

  /// A slider to control the frequency of the output signal. It's
  /// range should be [0.0, 5000.0] hz with a 500 hz midpoint. It's
  /// style should be Slider::LinearHorizontal, and its text box style
  /// should be Slider::TextBoxLeft, with a width 90 and height 22.
  Slider freqSlider;

  /// A label that displays the text "Waveforms:"
  Label waveformLabel;

  /// A menu for choosing waveforms. Each menu item holds a waveform
  /// name and its WaveformId. Consult the running app for more information.
  ComboBox waveformMenu;

  /// A label that displays the text "CPU:"
  Label cpuLabel {"", "CPU: "};

  /// A label that is updated by a timer to show the current cpu usage.
//  Label cpuUsage {"0.00", "%"};
   Label cpuUsage {"0.00", "%"};

  /// The current audio sample rate. Its initial value 0.0
  /// must be updated by prepareToPlay().
  double srate;

  /// The current audio amplitude level. Its initial value 0.0
  /// must updated by the levelSlider.
  double level = 0.0;

  /// The current audio frequency. Its initial value 0.0 must be updated
  /// by the freqSlider.
  double freq;

  /// The current phase position of the waveform. Its initial value
  /// 0.0 must be updated by the freqSlider.
  double phase;

  /// The phase increment per sample, e.g (freq/srate). The phase must be
  /// reset whenever the frequency or srate changes.
  double phaseDelta;

  /// 2pi as a double value.
  const double TwoPi {MathConstants<double>::pi * 2.0};

  //==============================================================================
  // Waveforms

  /// A periodic function ranging 0.0 to 1.0. It returns the current phase
  /// value and post-increments phase by phaseDelta. See: std::fmod().
  double inline phasor();

  /// Generates samples in a uniform random distribution.
  void inline whiteNoise(const AudioSourceChannelInfo& bufferToFill) ;

  /// Generates random uniform samples with a probability of freq/srate
  /// across all channels.
  void inline dust(const AudioSourceChannelInfo& bufferToFill) ;

  /// Generates samples in a 'brown' distribution (-6dB per octave).
  void inline brownNoise(const AudioSourceChannelInfo& bufferToFill) ;
  void inline brownNoiseFilt(const AudioSourceChannelInfo& bufferToFill) ;

  /// Generates a sine wave at a specified frequency and amplitude.
  void inline sineWave(const AudioSourceChannelInfo& bufferToFill) ;

  // Generators an inexpensive low frequency waves.
  void inline LF_impulseWave(const AudioSourceChannelInfo& bufferToFill);
  void inline LF_squareWave(const AudioSourceChannelInfo& bufferToFill);
  void inline LF_sawtoothWave(const AudioSourceChannelInfo& bufferToFill);
  void inline LF_triangleWave(const AudioSourceChannelInfo& bufferToFill);

  // Generators for band limited waves.
  void inline BL_impulseWave(const AudioSourceChannelInfo& bufferToFill);
  void inline BL_squareWave(const AudioSourceChannelInfo& bufferToFill);
  void inline BL_sawtoothWave(const AudioSourceChannelInfo& bufferToFill);
  void inline BL_triangleWave(const AudioSourceChannelInfo& bufferToFill);
  /// Generates samples using a wavetable oscillator.
  void inline WT_wave(const AudioSourceChannelInfo& bufferToFill);

  /// Returns a random value [-1.0, 1.0]
  float inline ranSamp();

  /// Returns a random value [-amp, amp]
  float inline ranSamp(const float amp) ;

  /// The change from one filter output to the next is proportional to the
  /// difference between the previous output and the next input.
  /// prevout + (alpha * (value - prevout)
  /// e.g. for i from 1 to n y[i] := y[i-1] + α * (x[i] - y[i-1])
  float lowPass(const float value, const float prevout, const float alpha) ;

  //==============================================================================
  // Wavetable support

  void createSineTable(AudioSampleBuffer& waveTable);
  void createSquareTable(AudioSampleBuffer& waveTable);
  void createImpulseTable(AudioSampleBuffer& waveTable);
  void createSawtoothTable(AudioSampleBuffer& waveTable);
  void createTriangleTable(AudioSampleBuffer& waveTable);
  void wavetablePrepareToPlay(int wttype);
  void wavetableSetFreq(float amp);
  /// Wavetable for sine waves.
  AudioSampleBuffer sineTable;
  /// Wavetable for square waves.
  AudioSampleBuffer squareTable;
  /// Wavetable for impulse waves.
  AudioSampleBuffer impulseTable;
  /// Wavetable for sawtooth ewaves.
  AudioSampleBuffer sawtoothTable;
  /// Wavetable for triangle waves.
  AudioSampleBuffer triangleTable;
  /// Size of wavetables
  int tableSize = 512;
  /// Array of wavetable oscillators
  std::vector<std::unique_ptr<WavetableOscillator>> oscillators;
  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
