#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
MyPluginProcessor::MyPluginProcessor()
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
                         )
{

}

MyPluginProcessor::~MyPluginProcessor()
{
}

//==============================================================================
const juce::String MyPluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MyPluginProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool MyPluginProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool MyPluginProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double MyPluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MyPluginProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
              // so this should be at least 1, even if you're not really implementing programs.
}

int MyPluginProcessor::getCurrentProgram()
{
    return 0;
}

void MyPluginProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String MyPluginProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void MyPluginProcessor::changeProgramName(int index, const juce::String &newName)
{
    juce::ignoreUnused(index, newName);
}

void MyPluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
  juce::ignoreUnused(samplesPerBlock, sampleRate);
}

void MyPluginProcessor::releaseResources()
{
}

bool MyPluginProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

        // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}

template <int Channel, typename BufferView, typename OscData>
constexpr auto update_channel(BufferView& buffer, OscData& osc_data) {
    auto* data = buffer.getWritePointer(Channel);
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        auto next_phase = pond::osc::next_phase<float>(osc_data);
        osc_data.phase = next_phase;
        data[sample] = std::sin(next_phase);
    }
}


void MyPluginProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                   juce::MidiBuffer &midiMessages)
{
//     update_channel<0>(buffer, left_osc_data);
//     update_channel<1>(buffer, right_osc_data);

    for (int i = 0; i < buffer.getNumSamples(); ++i) {
        left_buffer.push(buffer.getSample(0, i));
        right_buffer.push(buffer.getSample(1, i));
    }
}

//==============================================================================
bool MyPluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *MyPluginProcessor::createEditor()
{ // Use generic gui for editor for now
    return new PluginEditor(*this);
}

//==============================================================================
void MyPluginProcessor::getStateInformation(juce::MemoryBlock &destData)
{
}

void MyPluginProcessor::setStateInformation(const void *data, int sizeInBytes)
{
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new MyPluginProcessor();
}
