/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
#if DEBUG
class CompressorAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
#else
class CompressorAudioProcessorEditor : public juce::AudioProcessorEditor
#endif
{
public:
    CompressorAudioProcessorEditor (CompressorAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~CompressorAudioProcessorEditor() override;

	// GUI setup
	static const int N_SLIDERS_COUNT = 6;
	static const int SCALE = 70;
	static const int LABEL_OFFSET = 25;
	static const int SLIDER_WIDTH = 200;

	static const int TYPE_BUTTON_GROUP = 1;
	static const int BOTTOM_MENU_HEIGHT = 50;

    //==============================================================================
#ifdef DEBUG
	void timerCallback() override;
#endif
	void paint (juce::Graphics&) override;
    void resized() override;

	typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
	typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    CompressorAudioProcessor& audioProcessor;

	juce::AudioProcessorValueTreeState& valueTreeState;

	juce::Label m_labels[N_SLIDERS_COUNT] = {};
	juce::Slider m_sliders[N_SLIDERS_COUNT] = {};
	std::unique_ptr<SliderAttachment> m_sliderAttachment[N_SLIDERS_COUNT] = {};

	juce::Label automationTLabel;
	juce::Label smoothingTypeLabel;
	juce::Label detectionTypeLabel;

	juce::TextButton typeAButton{ "A" };
	juce::TextButton typeBButton{ "B" };
	juce::TextButton typeCButton{ "C" };
	juce::TextButton typeDButton{ "D" };

	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonAAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonBAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonCAttachment;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> buttonDAttachment;

#ifdef DEBUG
	juce::Label crestFactorLabel;
	juce::Label gainReductionLabel;
	juce::Label attackTimeLabel;
	juce::Label releaseTimeLabel;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompressorAudioProcessorEditor)
};
