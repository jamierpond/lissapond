#include "PluginEditor.h"
#include "PluginProcessor.h"

PluginEditor::PluginEditor(MyPluginProcessor &p)
    : AudioProcessorEditor(&p),  display(p)
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

  // reseve 10% of the width on the left for the sliders

//   juce::Rectangle<int> left_slider_area = area.removeFromLeft(area.getWidth() / 10);
//   left_freq_slider.setBounds(left_slider_area);
//
//   juce::Rectangle<int> right_slider_area = area.removeFromRight(area.getWidth() / 10);
//
//   right_freq_slider.setBounds(right_slider_area);
}
