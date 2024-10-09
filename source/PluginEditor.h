#pragma once

#include "PluginProcessor.h"
#include <zoo/swar/SWAR.h>

template <typename T, int N> constexpr auto decay_int_vibes(T in) {
  auto res = 0;
  auto i = N;
  while (i--) {
    in >>= 1;
    res += in;
  }
  return res;
}

template <typename S, int N> constexpr auto swar_decay_int_vibes(S in) {
  constexpr auto Lsbs = S{S::MostSignificantBit};
  auto i = N;
  auto res = S{0};
  while (i--) {
    in = in.shiftIntraLaneRight(1, ~Lsbs);
    res = S{res.value() + in.value()};
  }

  return res;
}

using S = zoo::swar::SWAR<8, uint32_t>;
static_assert(swar_decay_int_vibes<S, 0>(S{0x8}).value() == 0x0);
static_assert(swar_decay_int_vibes<S, 1>(S{0x8}).value() == 0x4);
static_assert(swar_decay_int_vibes<S, 2>(S{0x8}).value() == 0x6);
static_assert(swar_decay_int_vibes<S, 2>(S{0x08'08}).value() == 0x06'06);

struct LissDisplay final : juce::Component, juce::Timer {
  LissDisplay(MyPluginProcessor &p) : audio_processor(p) {
    setOpaque(true);
    startTimerHz(60);

    auto setup_slider = [this](juce::Slider &slider, juce::String name,
                               double init_value = 0.5) {
      slider.setSliderStyle(
          juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
      addAndMakeVisible(slider);
      slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
      slider.setPopupDisplayEnabled(true, false, this);
      slider.setNormalisableRange(
          juce::NormalisableRange<double>(0.0, 1.0, 0.01));
    };

    setup_slider(dot_size_slider, "Radius", 0.1);
    setup_slider(red_decay_slider, "Red Decay", 0.1);
    setup_slider(green_decay_slider, "Green Decay", 0.25);
    setup_slider(blue_decay_slider, "Blue Decay", 0.21);
    setup_slider(alpha_decay_slider, "Alpha Decay", 0.17);

    red_decay_slider.onValueChange = [this] {
      red_decay = red_decay_slider.getValue();
    };

    green_decay_slider.onValueChange = [this] {
      green_decay = green_decay_slider.getValue();
    };

    blue_decay_slider.onValueChange = [this] {
      blue_decay = blue_decay_slider.getValue();
    };

    alpha_decay_slider.onValueChange = [this] {
      alpha_decay = alpha_decay_slider.getValue();
    };

    dot_size_slider.onValueChange = [this] {
      dot_size = dot_size_slider.getValue();
    };
  }

  ~LissDisplay() override { stopTimer(); }

  float dot_size = 0.5f, red_decay = 0.5f, green_decay = 0.5f,
        blue_decay = 0.5f, alpha_decay = 0.5f;

  int num_samples = 1 << 12;

  juce::Slider dot_size_slider, red_decay_slider, green_decay_slider,
      blue_decay_slider, alpha_decay_slider, num_samples_slider;

  void resized() override {
    // a^2 + b^2 = c^2
    auto min_sz = std::min(getWidth(), getHeight());
    auto pow2 = std::pow(min_sz, 2);
    auto sz = std::sqrt(pow2 / 2);
    image = std::make_unique<juce::Image>(juce::Image::PixelFormat::RGB, sz, sz,
                                          true);

    juce::Rectangle<int> area = getLocalBounds();

    juce::Rectangle<int> left_slider_area =
        area.removeFromLeft(area.getWidth() / 10);
    // equally fill the slider area with the sliders

    dot_size_slider.setBounds(
        left_slider_area.removeFromTop(left_slider_area.getHeight() / 5));
    red_decay_slider.setBounds(
        left_slider_area.removeFromTop(left_slider_area.getHeight() / 4));
    green_decay_slider.setBounds(
        left_slider_area.removeFromTop(left_slider_area.getHeight() / 3));
    blue_decay_slider.setBounds(
        left_slider_area.removeFromTop(left_slider_area.getHeight() / 2));
    alpha_decay_slider.setBounds(left_slider_area);
  }

  void timerCallback() override { repaint(); }

  std::unique_ptr<juce::Image> image = nullptr;

  MyPluginProcessor &audio_processor;
  void paint(juce::Graphics &g) override {
    g.fillAll(juce::Colours::black);
    if (!image) {
      return;
    }

    float rows = image->getWidth();
    float cols = image->getHeight();
    auto ratio = 0.5f;

    // loop over all pixel and attenuate the colour
    auto data = juce::Image::BitmapData(
        *image, juce::Image::BitmapData::ReadWriteMode::writeOnly);

    constexpr auto swar_width = 8;
    for (int y = 0; y < data.height; ++y) {
      auto *p = data.getLinePointer(y);
      for (int x = 0; x < data.width; x += swar_width) {
        auto *pixel = p + x * data.pixelStride;
        typename S::type s;
        memcpy(&s, pixel, sizeof(s));
        auto result = swar_decay_int_vibes<S, 2>(S{s});
        memcpy(pixel, &result, sizeof(result));
      }
    }

    float current_rms{};
    for (int i = 0; i < audio_processor.num_samples_per_block; ++i) {
      auto l = audio_processor.left_buffer.read_back(i);
      auto r = audio_processor.right_buffer.read_back(i);
      auto rms = std::sqrt(l * l + r * r);
      current_rms += rms;
    }

    current_rms /= audio_processor.num_samples_per_block;
    current_rms = std::sqrt(
        current_rms); // just because make it more visible, pump those values up

    for (int i = 0; i < audio_processor.num_samples_per_block; ++i) {
      // draw a point for each sample using the left channel as x and the right
      // channel as y

      // TODO LINEARLIZE THIS READ
      auto l = audio_processor.left_buffer.read_back(i);
      auto r = audio_processor.right_buffer.read_back(i);

      auto x = l * (rows / 2) + (rows / 2);
      auto y = r * (cols / 2) + (cols / 2);

      auto v = static_cast<juce::uint8>(
          255.f * ((float)i / audio_processor.num_samples_per_block));

      auto radius = int(dot_size_slider.getValue() * 10);

      for (int dx = -radius; dx < radius; ++dx) {
        for (int dy = -radius; dy < radius; ++dy) {
          if (dx * dx + dy * dy < radius * radius) {
            auto
              px = x + dx,
              py = y + dy;

            if (px >= 0 && px < rows && py >= 0 && py < cols) {
              auto *pixel = data.getPixelPointer(px, py);
              pixel[0] = std::min<juce::uint8>(255, pixel[0] + v);
              pixel[1] = std::min<juce::uint8>(255, pixel[1] + v);
              pixel[2] = std::min<juce::uint8>(255, pixel[2] + v);
            }
          }
        }
      }
    }

    juce::AffineTransform transform;
    // rotate 45 degrees
    transform = transform.rotated(3.14159 / 4.0);
    // position in the middle of the screen
    transform = transform.translated(getWidth() / 2.0f, 0);

    g.drawImageTransformed(*image, transform);
  }
};

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor {
public:
  explicit PluginEditor(MyPluginProcessor &);
  ~PluginEditor() override;

  //==============================================================================
  void paint(juce::Graphics &) override;
  void resized() override;

private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  MyPluginProcessor &audio_processor;

  LissDisplay display;
  // juce::Slider left_freq_slider, right_freq_slider;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor)
};
