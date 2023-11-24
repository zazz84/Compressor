/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EnvelopeFollower::EnvelopeFollower()
{
}

void EnvelopeFollower::setCoef(float attackTimeMs, float releaseTimeMs)
{
	m_AttackCoef = exp(-1000.0f / (attackTimeMs * m_SampleRate));
	m_ReleaseCoef = exp(-1000.0f / (releaseTimeMs * m_SampleRate));
}

float EnvelopeFollower::process(float in)
{
	if (m_ballisticType == ballisticType::Decoupled)
	{
		m_Out1Last = fmax(fabs(in), m_ReleaseCoef * m_Out1Last);
		return m_OutLast = m_AttackCoef * m_OutLast + (1.0f - m_AttackCoef) * m_Out1Last;
	}
	else if (m_ballisticType == ballisticType::Branching)
	{
		const float inAbs = fabs(in);
		if (inAbs > m_OutLast)
		{
			return m_OutLast = m_AttackCoef * m_OutLast + (1.0f - m_AttackCoef) * inAbs;
		}
		else
		{
			return m_OutLast = m_ReleaseCoef * m_OutLast;
		}
	}
	else if (m_ballisticType == ballisticType::SmoothDecoupled)
	{
		const float inAbs = fabs(in);
		m_Out1Last = fmaxf(inAbs, m_ReleaseCoef * m_Out1Last + (1.0f - m_ReleaseCoef) * inAbs);
		return m_OutLast = m_AttackCoef * m_OutLast + (1.0f - m_AttackCoef) * m_Out1Last;
	}
	else if (m_ballisticType == ballisticType::SmoothBranching)
	{
		const float inAbs = fabs(in);
		if (inAbs > m_OutLast)
		{
			return m_OutLast = m_AttackCoef * m_OutLast + (1.0f - m_AttackCoef) * inAbs;
		}
		else
		{
			return m_OutLast = m_ReleaseCoef * m_OutLast + (1.0f - m_ReleaseCoef) * inAbs;
		}
	}

	return 0.0f;
}

const std::string CompressorAudioProcessor::paramsNames[] = { "Attack", "Release", "Ratio", "Threshold", "Mix", "Volume" };

//==============================================================================
CrestFactor::CrestFactor()
{
}

float CrestFactor::process(float in)
{
	const float inSQ = in * in;
	const float inFactor = (1.0f - m_Coef) * inSQ;

	m_PeakLastSQ = std::max(inSQ, m_Coef * m_PeakLastSQ + inFactor);
	m_RMSLastSQ = m_Coef * m_RMSLastSQ + inFactor;

	return std::sqrtf(m_PeakLastSQ / m_RMSLastSQ);
}
//==============================================================================
CompressorAudioProcessor::CompressorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
	automationParameter    = static_cast<juce::AudioParameterChoice*>(apvts.getParameter("automation"));
	ballisticTypeParameter = static_cast<juce::AudioParameterChoice*>(apvts.getParameter("ballisticType"));
	architectureParameter  = static_cast<juce::AudioParameterChoice*>(apvts.getParameter("architecture"));

	attackParameter    = apvts.getRawParameterValue(paramsNames[0]);
	releaseParameter   = apvts.getRawParameterValue(paramsNames[1]);
	ratioParameter     = apvts.getRawParameterValue(paramsNames[2]);
	thresholdParameter = apvts.getRawParameterValue(paramsNames[3]);
	mixParameter       = apvts.getRawParameterValue(paramsNames[4]);
	volumeParameter    = apvts.getRawParameterValue(paramsNames[5]);
}

CompressorAudioProcessor::~CompressorAudioProcessor()
{
}

//==============================================================================
const juce::String CompressorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CompressorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CompressorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CompressorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CompressorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CompressorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CompressorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CompressorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CompressorAudioProcessor::getProgramName (int index)
{
    return {};
}

void CompressorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CompressorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	m_envelopeFollower[0].init((int)(sampleRate));
	m_envelopeFollower[1].init((int)(sampleRate));

	m_crestFactor[0].init((int)(sampleRate));
	m_crestFactor[1].init((int)(sampleRate));

	m_crestFactor[0].setCoef(0.2f);
	m_crestFactor[1].setCoef(0.2f);
}

void CompressorAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CompressorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void CompressorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const CompressorAudioProcessor::automation automation = static_cast<CompressorAudioProcessor::automation>(automationParameter->getIndex() + 1);
	const EnvelopeFollower::ballisticType ballisticType = static_cast<EnvelopeFollower::ballisticType>(ballisticTypeParameter->getIndex() + 1);
	const CompressorAudioProcessor::architecture architecture = static_cast<CompressorAudioProcessor::architecture>(architectureParameter->getIndex() + 1);
	const auto attack = attackParameter->load();
	const auto release = releaseParameter->load();
	const auto ratio = ratioParameter->load();
	const auto threshold = thresholdParameter->load();
	const auto mix = mixParameter->load();
	const auto volume = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Mics constants
	const float mixInverse = 1.0f - mix;
	const float R_Inv_minus_One = (1.0f / ratio) - 1.0f;
	const int channels = getTotalNumOutputChannels();
	const int samples = buffer.getNumSamples();	
	
	for (int channel = 0; channel < channels; ++channel)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);
		
		// Envelope reference
		auto& envelopeFollower = m_envelopeFollower[channel];

		// CrestFactor
		auto& crestFactor = m_crestFactor[channel];
		
		// Set attack and release
		envelopeFollower.setCoef(attack, release);

		// Set ballistic type
		envelopeFollower.setBallisticType(ballisticType);

		// ReturToZero
		if (architecture == architecture::ReturnToZero)
		{
			for (int sample = 0; sample < samples; ++sample)
			{
				// Get input
				const float in = channelBuffer[sample];

				// Smooth
				const float smooth = envelopeFollower.process(in);

				// Convert input from gain to dB
				const float smoothdB = juce::Decibels::gainToDecibels(smooth + 0.000001f);

				//Get gain reduction, positive values
				const float attenuatedB = (smoothdB >= threshold) ? (smoothdB - threshold) * R_Inv_minus_One : 0.0f;

#ifdef DEBUG
				// Store gain reduction
				if (fabs(attenuatedB) > m_gainReductiondB)
					m_gainReductiondB = fabs(attenuatedB);
#endif

				// Apply gain reduction
				const float out = in * juce::Decibels::decibelsToGain(attenuatedB);

				// Apply volume, mix and send to output
				channelBuffer[sample] = volume * (mix * out + mixInverse * in);
			}
		}
		else if (architecture == architecture::ReturnToThreshold)
		{
			const float thresholdGain = juce::Decibels::decibelsToGain(threshold);

			for (int sample = 0; sample < samples; ++sample)
			{
				// Get input
				const float in = channelBuffer[sample];

				// In above threshold
				const float inOverThrehold = fmaxf(thresholdGain, in);

				// Smooth
				const float smooth = envelopeFollower.process(inOverThrehold);

				// Convert input from gain to dB
				const float smoothdB = juce::Decibels::gainToDecibels(smooth + 0.000001f);

				//Get gain reduction, positive values
				const float attenuatedB = (smoothdB >= threshold) ? (smoothdB - threshold) * R_Inv_minus_One : 0.0f;

#ifdef DEBUG
				// Store gain reduction
				if (fabs(attenuatedB) > m_gainReductiondB)
					m_gainReductiondB = fabs(attenuatedB);
#endif

				// Apply gain reduction
				const float out = in * juce::Decibels::decibelsToGain(attenuatedB);

				// Apply volume, mix and send to output
				channelBuffer[sample] = volume * (mix * out + mixInverse * in);
			}
		}
		else
		{
			const float factor = (ratio > 1.0f) ? -1.0f : 1.0f;

			for (int sample = 0; sample < samples; ++sample)
			{
				// Get input
				const float in = channelBuffer[sample];

				// Convert input from gain to dB
				const float indB = juce::Decibels::gainToDecibels(fabsf(in) + 0.000001f);

				//Get gain reduction, positive values
				const float attenuatedB = (indB >= threshold) ? (indB - threshold) * R_Inv_minus_One : 0.0f;

				//Automation
				if (automation == automation::Auto)
				{
					const float crest = crestFactor.process(in);
					const float crestSQ  = crest * crest;

					const float attackAuto = 4.0f * attack / crestSQ;
					float releaseAuto = 4.0f * release / crestSQ - attackAuto;
					if (releaseAuto <= 1.0f)
						releaseAuto = 1.0f;

					envelopeFollower.setCoef(attackAuto, releaseAuto);

#ifdef DEBUG
					// Values for meters
					if (attackAuto > m_attackTime)
						m_attackTime = attackAuto;

					if (releaseAuto > m_releaseTime)
						m_releaseTime = releaseAuto;

					if (crestSQ > m_crestFactorSQ)
						m_crestFactorSQ = crestSQ;
#endif
				}

				// Smooth
				const float smoothdB = factor * envelopeFollower.process(attenuatedB);

#ifdef DEBUG
				// Store gain reduction
				if (fabs(smoothdB) > m_gainReductiondB)
					m_gainReductiondB = fabs(smoothdB);
#endif

				// Apply gain reduction
				const float out = in * juce::Decibels::decibelsToGain(smoothdB);

				// Apply volume, mix and send to output
				channelBuffer[sample] = volume * (mix * out + mixInverse * in);
			}
		}
	}
}

//==============================================================================
bool CompressorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CompressorAudioProcessor::createEditor()
{
    return new CompressorAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void CompressorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void CompressorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout CompressorAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterChoice>("automation",    "Automation",     juce::StringArray{ "Manual", "Auto" }, 0));
	layout.add(std::make_unique<juce::AudioParameterChoice>("ballisticType", "BallisticType",  juce::StringArray{ "Decoupled", "Branching", "SmoothDecoupled", "SmoothBranching" }, 3));
	layout.add(std::make_unique<juce::AudioParameterChoice>("architecture",  "Architecture",   juce::StringArray{ "ReturnToZero", "ReturnToThreshold", "LogDomain" }, 2));

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(  0.1f,  80.0f,  0.1f, 0.5f),  10.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(  1.0f, 200.0f,  1.0f, 0.5f), 100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(  0.5f,   8.0f,  0.1f, 1.0f),   4.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(-60.0f,  12.0f,  1.0f, 1.0f), -12.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>(  0.0f,   1.0f, 0.05f, 1.0f),   1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[5], paramsNames[5], NormalisableRange<float>(-24.0f,  24.0f,  0.1f, 1.0f),   0.0f));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CompressorAudioProcessor();
}
