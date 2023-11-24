/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
class EnvelopeFollower
{
public:
	EnvelopeFollower();

	enum ballisticType
	{
		Decoupled = 1,
		Branching,
		SmoothDecoupled,
		SmoothBranching
	};

	void init(int sampleRate) { m_SampleRate = sampleRate; }
	void setCoef(float attackTime, float releaseTime);
	float process(float in);
	void setBallisticType(ballisticType ballisticType) { m_ballisticType = ballisticType; }

protected:
	ballisticType m_ballisticType = ballisticType::SmoothBranching;
	int  m_SampleRate = 48000;
	float m_AttackCoef = 0.0f;
	float m_ReleaseCoef = 0.0f;
	
	float m_OutLast = 0.0f;
	float m_Out1Last = 0.0f;
};

//==============================================================================
class CrestFactor
{
public:
	CrestFactor();

	void init(int sampleRate) { m_SampleRate = sampleRate; }
	void setCoef(float time) { m_Coef = exp(-1.0f / (m_SampleRate * time)); }
	float process(float in);

protected:
	int  m_SampleRate = 48000;
	float m_Coef = 0.0f;

	float m_PeakLastSQ = 0.0f;
	float m_RMSLastSQ = 0.0f;
};

//==============================================================================
class CompressorAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    CompressorAudioProcessor();
    ~CompressorAudioProcessor() override;

	enum architecture
	{
		ReturnToZero = 1,
		ReturnToThreshold,
		LogDomain,
	};

	enum automation
	{
		Manual = 1,
		Auto,
	};

	static const std::string paramsNames[];

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

#ifdef DEBUG
	float getCrestFactorSQ()
	{ 
		const float tmp = m_crestFactorSQ;
		m_crestFactorSQ = 0.0f;
		return tmp;
	}
	float getGainReduction()
	{
		const float tmp = m_gainReductiondB;
		m_gainReductiondB = 0.0f;
		return tmp;
	}
	float getAttackTime()
	{
		const float tmp = m_attackTime;
		m_attackTime = 0.0f;
		return tmp;
	}
	float getReleaseTime()
	{
		const float tmp = m_releaseTime;
		m_releaseTime = 0.0f;
		return tmp;
	}
#endif

	using APVTS = juce::AudioProcessorValueTreeState;
	static APVTS::ParameterLayout createParameterLayout();

	APVTS apvts{ *this, nullptr, "Parameters", createParameterLayout() };

private:	
	//==============================================================================

	juce::AudioParameterChoice* automationParameter = nullptr;
	juce::AudioParameterChoice* ballisticTypeParameter = nullptr;
	juce::AudioParameterChoice* architectureParameter = nullptr;

	std::atomic<float>* attackParameter = nullptr;
	std::atomic<float>* releaseParameter = nullptr;
	std::atomic<float>* ratioParameter = nullptr;
	std::atomic<float>* thresholdParameter = nullptr;
	std::atomic<float>* mixParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

	EnvelopeFollower m_envelopeFollower[2] = {};
	CrestFactor m_crestFactor[2] = {};

#ifdef DEBUG
	float m_attackTime = 0.0f;
	float m_releaseTime = 0.0f;
	float m_crestFactorSQ = 0.0f;
	float m_gainReductiondB = 0.0f;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompressorAudioProcessor)
};
