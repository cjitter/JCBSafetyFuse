//==============================================================================
//
//  Copyright 2025 Juan Carlos Blancas
//  This file is part of JCBSafetyFuse and is licensed under the GNU General Public License v3.0 or later.
//
//==============================================================================

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include <cmath>

class JCBSafetyFuseAudioProcessor : public juce::AudioProcessor
{
public:
    JCBSafetyFuseAudioProcessor();
    ~JCBSafetyFuseAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
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
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // API de estados independientes
    bool isTrippedClipping() const noexcept;
    bool isTrippedNaN() const noexcept;
    bool isTrippedInf() const noexcept;
    int getInfSign() const noexcept;  // 1=+Inf, -1=-Inf, 0=none

    void resetClipping() noexcept;
    void resetNaN() noexcept;
    void resetInf() noexcept;

    bool getAutoRecoveryClipping() const noexcept;
    void setAutoRecoveryClipping(bool enabled) noexcept;

    bool getAutoRecoveryNaN() const noexcept;
    void setAutoRecoveryNaN(bool enabled) noexcept;

    bool getAutoRecoveryInf() const noexcept;
    void setAutoRecoveryInf(bool enabled) noexcept;

    void setAutoRecoveryTimeMs(float ms) noexcept;
    float getAutoRecoveryTimeMs() const noexcept;

    void setFadeTimeMs(float ms) noexcept;
    float getFadeTimeMs() const noexcept;

    void setThresholdDB(float dBFS) noexcept;
    float getThresholdDB() const noexcept;

private:
    void tripClipping() noexcept;
    void tripNaN() noexcept;
    void tripInf(bool positive) noexcept;

    // Estados independientes de trip
    std::atomic<bool> trippedClipping{false};
    std::atomic<bool> trippedNaN{false};
    std::atomic<bool> trippedInf{false};
    std::atomic<int> infSign{0};  // 0=none, 1=+Inf, -1=-Inf

    // Auto-recovery
    std::atomic<bool> autoRecoveryClipping{true};
    std::atomic<bool> autoRecoveryNaN{true};
    std::atomic<bool> autoRecoveryInf{true};
    std::atomic<float> autoRecoveryTimeMs{500.0f};  // 500-2000 ms
    int autoRecoverySamplesNeeded = 0;  // Calculado en prepareToPlay
    int cleanSamplesClipping = 0;
    int cleanSamplesNaN = 0;
    int cleanSamplesInf = 0;

    // Fade-in/out (en samples)
    std::atomic<float> fadeTimeMs{40.0f};  // 0-40 ms
    int fadeInSamples = 0;
    int fadeOutSamples = 0;
    int fadeLength = 0;  // Calculado en prepareToPlay (mismo para in y out)
    bool isFadingOut = false;
    double currentSampleRate = 44100.0;

    std::atomic<float> explosionThresholdLinear{2.0f};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JCBSafetyFuseAudioProcessor)
};
