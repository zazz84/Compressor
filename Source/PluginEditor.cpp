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

	// Menus
	automationComboBox.addItem("Manual", CompressorAudioProcessor::automation::Manual);
	automationComboBox.addItem("Auto", CompressorAudioProcessor::automation::Auto);
	automationComboBox.setSelectedId(CompressorAudioProcessor::automation::Manual);
	automationComboBox.setJustificationType(juce::Justification::centred);
	addAndMakeVisible(automationComboBox);
	automationComboBoxAttachment.reset(new ComboBoxAttachment(valueTreeState, "automation", automationComboBox));

	ballisticTypeComboBox.addItem("Decoupled", EnvelopeFollower::ballisticType::Decoupled);
	ballisticTypeComboBox.addItem("Branching", EnvelopeFollower::ballisticType::Branching);
	ballisticTypeComboBox.addItem("SmoothDecoupled", EnvelopeFollower::ballisticType::SmoothDecoupled);
	ballisticTypeComboBox.addItem("SmoothBranching", EnvelopeFollower::ballisticType::SmoothBranching);
	ballisticTypeComboBox.setSelectedId(EnvelopeFollower::ballisticType::SmoothBranching);
	ballisticTypeComboBox.setJustificationType(juce::Justification::centred);
	addAndMakeVisible(ballisticTypeComboBox);
	ballisticTypeComboBoxAttachment.reset(new ComboBoxAttachment(valueTreeState, "ballisticType", ballisticTypeComboBox));

	architectureComboBox.addItem("ReturnToZero", CompressorAudioProcessor::architecture::ReturnToZero);
	architectureComboBox.addItem("ReturnToThreshold", CompressorAudioProcessor::architecture::ReturnToThreshold);
	architectureComboBox.addItem("LogDomain", CompressorAudioProcessor::architecture::LogDomain);
	architectureComboBox.setSelectedId(CompressorAudioProcessor::architecture::LogDomain);
	architectureComboBox.setJustificationType(juce::Justification::centred);
	addAndMakeVisible(architectureComboBox);
	architectureTypeComboBoxAttachment.reset(new ComboBoxAttachment(valueTreeState, "architecture", architectureComboBox));

	bool hasMenu = (N_MENUS_COUNT > 0) ? true : false;

	setSize((int)(200.0f * 0.01f * SCALE * N_SLIDERS_COUNT), (int)(200.0f * 0.01f * SCALE) + (hasMenu * MENU_HEIGHT));
}

CompressorAudioProcessorEditor::~CompressorAudioProcessorEditor()
{
}

//==============================================================================
void CompressorAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(juce::Colours::darkslategrey);
}

void CompressorAudioProcessorEditor::resized()
{
	int width = getWidth() / N_SLIDERS_COUNT;
	int height = getHeight() - MENU_HEIGHT;

	// Sliders + Menus
	juce::Rectangle<int> rectangles[N_SLIDERS_COUNT];

	//for (int i = (automationComboBox.getSelectedId() == (int)(CompressorAudioProcessor::automation::Auto)) ? 2 : 0; i < N_SLIDERS_COUNT; ++i)
	for (int i = 0; i < N_SLIDERS_COUNT; ++i)
	{
		rectangles[i].setSize(width, height);
		rectangles[i].setPosition(i * width, 0);
		m_sliders[i].setBounds(rectangles[i]);

		rectangles[i].removeFromBottom((int)(20.0f * 0.01f * SCALE));
		m_labels[i].setBounds(rectangles[i]);
	}

	// Menus
	juce::Rectangle<int> menuRectangle;
	const int menuHeight = (int)(height + MENU_HEIGHT * 0.3f);
	
	//2
	menuRectangle.setSize((int)(width * 0.9f), (int)(MENU_HEIGHT * 0.4f));
	menuRectangle.setPosition((int)(1.05f * width), menuHeight);
	//smoothingTypeLabel.setBounds(menuRectangle);
	automationComboBox.setBounds(menuRectangle);
	automationComboBox.setJustificationType(juce::Justification::centred);

	//3
	menuRectangle.setPosition((int)(2.05f * width), menuHeight);
	ballisticTypeComboBox.setBounds(menuRectangle);
	ballisticTypeComboBox.setJustificationType(juce::Justification::centred);

	//4
	menuRectangle.setPosition((int)(3.05f * width), menuHeight);
	detectionTypeLabel.setBounds(menuRectangle);

	//5
	menuRectangle.setPosition((int)(4.05f * width), menuHeight);
	architectureComboBox.setBounds(menuRectangle);
	architectureComboBox.setJustificationType(juce::Justification::centred);
}