/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CompressorAudioProcessorEditor::CompressorAudioProcessorEditor (CompressorAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{
	juce::Colour light = juce::Colour::fromHSV(HUE * 0.01f, 0.5f, 0.6f, 1.0f);
	juce::Colour medium = juce::Colour::fromHSV(HUE * 0.01f, 0.5f, 0.5f, 1.0f);
	juce::Colour dark = juce::Colour::fromHSV(HUE * 0.01f, 0.5f, 0.4f, 1.0f);

	getLookAndFeel().setColour(juce::Slider::thumbColourId, dark);
	getLookAndFeel().setColour(juce::Slider::rotarySliderFillColourId, medium);
	getLookAndFeel().setColour(juce::Slider::rotarySliderOutlineColourId, light);

	for (int i = 0; i < N_SLIDERS_COUNT; i++)
	{
		auto& label = m_labels[i];
		auto& slider = m_sliders[i];

		//Lable
		label.setText(CompressorAudioProcessor::paramsNames[i], juce::dontSendNotification);
		label.setFont(juce::Font(24.0f * 0.01f * SCALE, juce::Font::bold));
		label.setJustificationType(juce::Justification::centred);
		addAndMakeVisible(label);

		//Slider
		slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
		slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
		addAndMakeVisible(slider);
		m_sliderAttachment[i].reset(new SliderAttachment(valueTreeState, CompressorAudioProcessor::paramsNames[i], slider));
	}

	//Label
	smoothingTypeLabel.setText("Smoothing Type :", juce::dontSendNotification);
	smoothingTypeLabel.setFont(juce::Font(22.0f * 0.01f * SCALE, juce::Font::plain));
	smoothingTypeLabel.setJustificationType(juce::Justification::centred);
	addAndMakeVisible(smoothingTypeLabel);

	detectionTypeLabel.setText("Detection Type :", juce::dontSendNotification);
	detectionTypeLabel.setFont(juce::Font(22.0f * 0.01f * SCALE, juce::Font::plain));
	detectionTypeLabel.setJustificationType(juce::Justification::centred);
	addAndMakeVisible(detectionTypeLabel);

#if DEBUG
	//Debug
	crestFactorLabel.setText("0", juce::dontSendNotification);
	crestFactorLabel.setFont(juce::Font(24.0f * 0.01f * SCALE, juce::Font::bold));
	crestFactorLabel.setJustificationType(juce::Justification::centred);
	addAndMakeVisible(crestFactorLabel);

	gainReductionLabel.setText("0", juce::dontSendNotification);
	gainReductionLabel.setFont(juce::Font(24.0f * 0.01f * SCALE, juce::Font::bold));
	gainReductionLabel.setJustificationType(juce::Justification::centred);
	addAndMakeVisible(gainReductionLabel);

	attackTimeLabel.setText("0", juce::dontSendNotification);
	attackTimeLabel.setFont(juce::Font(24.0f * 0.01f * SCALE, juce::Font::bold));
	attackTimeLabel.setJustificationType(juce::Justification::centred);
	addAndMakeVisible(attackTimeLabel);

	releaseTimeLabel.setText("0", juce::dontSendNotification);
	releaseTimeLabel.setFont(juce::Font(24.0f * 0.01f * SCALE, juce::Font::bold));
	releaseTimeLabel.setJustificationType(juce::Justification::centred);
	addAndMakeVisible(releaseTimeLabel);
#endif

	// Buttons
	addAndMakeVisible(typeAButton);
	addAndMakeVisible(typeBButton);
	addAndMakeVisible(typeCButton);
	addAndMakeVisible(typeDButton);

	typeAButton.setRadioGroupId(TYPE_BUTTON_GROUP);
	typeBButton.setRadioGroupId(TYPE_BUTTON_GROUP);
	typeCButton.setRadioGroupId(TYPE_BUTTON_GROUP);
	typeDButton.setRadioGroupId(TYPE_BUTTON_GROUP);

	typeAButton.setClickingTogglesState(true);
	typeBButton.setClickingTogglesState(true);
	typeCButton.setClickingTogglesState(true);
	typeDButton.setClickingTogglesState(true);

	buttonAAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "ButtonA", typeAButton));
	buttonBAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "ButtonB", typeBButton));
	buttonCAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "ButtonC", typeCButton));
	buttonDAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "ButtonD", typeDButton));

	typeAButton.setColour(juce::TextButton::buttonColourId, light);
	typeBButton.setColour(juce::TextButton::buttonColourId, light);
	typeCButton.setColour(juce::TextButton::buttonColourId, light);
	typeDButton.setColour(juce::TextButton::buttonColourId, light);

	typeAButton.setColour(juce::TextButton::buttonOnColourId, dark);
	typeBButton.setColour(juce::TextButton::buttonOnColourId, dark);
	typeCButton.setColour(juce::TextButton::buttonOnColourId, dark);
	typeDButton.setColour(juce::TextButton::buttonOnColourId, dark);

#if DEBUG
	setSize((int)(SLIDER_WIDTH * 0.01f * SCALE * N_SLIDERS_COUNT), (int)((SLIDER_WIDTH + BOTTOM_MENU_HEIGHT + BOTTOM_MENU_HEIGHT) * 0.01f * SCALE));

	startTimerHz(5);
#else
	setSize((int)(SLIDER_WIDTH * 0.01f * SCALE * N_SLIDERS_COUNT), (int)((SLIDER_WIDTH + BOTTOM_MENU_HEIGHT) * 0.01f * SCALE));
#endif
}

CompressorAudioProcessorEditor::~CompressorAudioProcessorEditor()
{
}

//==============================================================================
#ifdef DEBUG
void CompressorAudioProcessorEditor::timerCallback()
{
	const int attackTime = (int)audioProcessor.getAttackTime();
	attackTimeLabel.setText(juce::String(attackTime), juce::dontSendNotification);

	const int releaseTime = (int)audioProcessor.getReleaseTime();
	releaseTimeLabel.setText(juce::String(releaseTime), juce::dontSendNotification);

	const int crestFactorSQ = (int)audioProcessor.getCrestFactor();
	crestFactorLabel.setText(juce::String(crestFactorSQ), juce::dontSendNotification);

	const int gainReduction = (int)audioProcessor.getGainReduction();
	gainReductionLabel.setText(juce::String(gainReduction), juce::dontSendNotification);
	
	repaint();
}
#endif

void CompressorAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(juce::Colour::fromHSV(HUE * 0.01f, 0.5f, 0.7f, 1.0f));
}

void CompressorAudioProcessorEditor::resized()
{
	int width = getWidth() / N_SLIDERS_COUNT;
	int height = SLIDER_WIDTH * 0.01f * SCALE;

	// Sliders + Menus
	juce::Rectangle<int> rectangles[N_SLIDERS_COUNT];

	for (int i = 0; i < N_SLIDERS_COUNT; ++i)
	{
		rectangles[i].setSize(width, height);
		rectangles[i].setPosition(i * width, 0);
		m_sliders[i].setBounds(rectangles[i]);

		rectangles[i].removeFromBottom((int)(LABEL_OFFSET * 0.01f * SCALE));
		m_labels[i].setBounds(rectangles[i]);
	}

	// Buttons
	const int posY = height + (int)(BOTTOM_MENU_HEIGHT * 0.01f * SCALE * 0.25f);
	const int buttonHeight = (int)(BOTTOM_MENU_HEIGHT * 0.01f * SCALE * 0.5f);

	typeAButton.setBounds((int)(getWidth() * 0.5f - buttonHeight * 1.8f), posY, buttonHeight, buttonHeight);
	typeBButton.setBounds((int)(getWidth() * 0.5f - buttonHeight * 0.6f), posY, buttonHeight, buttonHeight);
	typeCButton.setBounds((int)(getWidth() * 0.5f + buttonHeight * 0.6f), posY, buttonHeight, buttonHeight);
	typeDButton.setBounds((int)(getWidth() * 0.5f + buttonHeight * 1.8f), posY, buttonHeight, buttonHeight);	

#if DEBUG
	// Debug menus
	const int menuWidth = (int)(width * 0.9f);
	juce::Rectangle<int> debugMenuRectangle;
	const int debugMenuPosY = (int)(height + BOTTOM_MENU_HEIGHT * 1.3f);
	debugMenuRectangle.setSize(menuWidth, (int)(BOTTOM_MENU_HEIGHT * 0.4f));

	//1
	debugMenuRectangle.setPosition((int)(0.05f * width), debugMenuPosY);
	attackTimeLabel.setBounds(debugMenuRectangle);

	//2
	debugMenuRectangle.setPosition((int)(1.05f * width), debugMenuPosY);
	releaseTimeLabel.setBounds(debugMenuRectangle);

	//3
	debugMenuRectangle.setPosition((int)(2.05f * width), debugMenuPosY);
	crestFactorLabel.setBounds(debugMenuRectangle);

	//4
	debugMenuRectangle.setPosition((int)(3.05f * width), debugMenuPosY);
	gainReductionLabel.setBounds(debugMenuRectangle);

	//5
	debugMenuRectangle.setPosition((int)(4.05f * width), debugMenuPosY);

	//6
#endif
}