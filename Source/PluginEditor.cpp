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

#if DEBUG
	setSize((int)(200.0f * 0.01f * SCALE * N_SLIDERS_COUNT), (int)(200.0f * 0.01f * SCALE) + (hasMenu * MENU_HEIGHT) + MENU_HEIGHT);

	startTimerHz(5);
#else
	setSize((int)(200.0f * 0.01f * SCALE * N_SLIDERS_COUNT), (int)(200.0f * 0.01f * SCALE) + (hasMenu * MENU_HEIGHT));
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
	g.fillAll(juce::Colours::darkslategrey);
}

void CompressorAudioProcessorEditor::resized()
{
	int width = getWidth() / N_SLIDERS_COUNT;

	bool hasMenu = (N_MENUS_COUNT > 0) ? true : false;
#if DEBUG
	int height = getHeight() - hasMenu * MENU_HEIGHT - MENU_HEIGHT;
#else
	int height = getHeight() - hasMenu * MENU_HEIGHT;
#endif

	

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
	const int menuPosY = (int)(height + MENU_HEIGHT * 0.3f);
	const int menuWidth = (int)(width * 0.9f);
	
	menuRectangle.setSize(menuWidth, (int)(MENU_HEIGHT * 0.4f));

	//1
	menuRectangle.setPosition((int)(0.05f * width), menuPosY);


	//2
	menuRectangle.setPosition((int)(1.05f * width), menuPosY);


	//3
	menuRectangle.setPosition((int)(2.05f * width), menuPosY);
	automationComboBox.setBounds(menuRectangle);

	//4
	menuRectangle.setPosition((int)(3.05f * width), menuPosY);
	ballisticTypeComboBox.setBounds(menuRectangle);

	//5
	menuRectangle.setPosition((int)(4.05f * width), menuPosY);
	architectureComboBox.setBounds(menuRectangle);

	//6
	menuRectangle.setPosition((int)(5.05f * width), menuPosY);
	

#if DEBUG
	// Debug menus

	juce::Rectangle<int> debugMenuRectangle;
	const int debugMenuPosY = (int)(height + MENU_HEIGHT * 1.3f);
	debugMenuRectangle.setSize(menuWidth, (int)(MENU_HEIGHT * 0.4f));

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