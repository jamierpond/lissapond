#include "PluginEditor.h"
#include "PluginProcessor.h"

PluginEditor::PluginEditor(MyPluginProcessor &p)
    : AudioProcessorEditor(&p), display(p)
, audio_processor(p)
{
    setResizable(true, true);
    setResizeLimits(900, 450, 1920, 1080);
    addAndMakeVisible(display);
    display.setBounds(getLocalBounds());
}

PluginEditor::~PluginEditor()
{
}

void PluginEditor::paint(juce::Graphics &g) {
}

void PluginEditor::resized()
{
  display.setBounds(getLocalBounds());
}
