#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace pond::math {

template <typename FloatType>
constexpr FloatType sin (FloatType x) noexcept
{
    auto x2 = x * x;
    auto numerator = -x * (-11511339840 + x2 * (1640635920 + x2 * (-52785432 + x2 * 479249)));
    auto denominator = 11511339840 + x2 * (277920720 + x2 * (3177720 + x2 * 18361));
    return numerator / denominator;
}

}


namespace pond::osc {
  template <typename T>
  struct Constants {
    constexpr static T pi = static_cast<T>(3.14159265358979323846);
    constexpr static T two_pi = T{2} * pi;
  };

  template<typename T>
  struct OscData {
    T phase = 0;
    T phase_increment = 0;

    constexpr static auto from_freq_fs(T freq, T fs) {
      return OscData {
        .phase = 0,
        .phase_increment = Constants<T>::two_pi * freq / fs
      };
    }
  };

  template<typename T>
  constexpr T next_phase(T current_phase, T phase_increment) {
    constexpr auto ModuloPoint = Constants<T>::two_pi;
    auto phase = current_phase + phase_increment;
    while (phase > ModuloPoint) {
      phase -= ModuloPoint;
    }
    return phase;
  }

  template<typename T>
  constexpr T next_phase(OscData<T> data) {
    return next_phase<T>(data.phase, data.phase_increment);
  }
}

static_assert(pond::osc::Constants<float>::two_pi == 6.28318530717958647693f);
static_assert(pond::osc::next_phase<int>(4, 1) == 5);
static_assert(pond::osc::next_phase<int>(4, 4) == 2);
static_assert(pond::osc::next_phase<int>({.phase = 4, .phase_increment = 4}) == 2);



template <typename T, size_t N>
struct CircularBufferBasic {
  void push(T value) {
    buffer[write_index] = value;
    write_index = (write_index + 1) % N;
  }

  T read_back(size_t index) {
    return buffer[(read_index - index) % N];
  }

  size_t write_index = 0, read_index = 0;
  std::array<T, N> buffer;
};

template <typename T>
struct LissajousFifo {

};

class MyPluginProcessor : public juce::AudioProcessor
{
public:
    MyPluginProcessor();
    ~MyPluginProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    juce::AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String &newName) override;

    void getStateInformation(juce::MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;


    using OscData = pond::osc::OscData<float>;
    OscData
      left_osc_data = OscData::from_freq_fs(440, 44100),
      right_osc_data = OscData::from_freq_fs(440, 44100);

    constexpr static size_t num_samples_per_block = 1 << 10;

    using GuiArray = CircularBufferBasic<float, num_samples_per_block>;
    GuiArray left_buffer{};
    GuiArray right_buffer{};



private:
//     std::atomic<float*> left_buffer_ptr = left_buffer.data();
//     std::atomic<float*> right_buffer_ptr = right_buffer.data();
//     static_assert(std::atomic<float*>::is_always_lock_free);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MyPluginProcessor)
};
