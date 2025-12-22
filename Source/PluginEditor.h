//==============================================================================
//
//  Copyright 2025 Juan Carlos Blancas
//  This file is part of JCBSafetyFuse and is licensed under the GNU General Public License v3.0 or later.
//
//==============================================================================

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "BinaryData.h"

class JCBSafetyFuseAudioProcessorEditor : public juce::AudioProcessorEditor,
                                           private juce::Timer
{
public:
    explicit JCBSafetyFuseAudioProcessorEditor(JCBSafetyFuseAudioProcessor&);
    ~JCBSafetyFuseAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;

private:
    void timerCallback() override;
    void updateStatusDisplay();

    JCBSafetyFuseAudioProcessor& processor;

    // Panel Clipping
    juce::Label titleClipping;
    juce::Label statusClipping;
    juce::Label thresholdLabel;
    juce::Slider thresholdSlider;
    juce::Label recoveryTimeLabel;
    juce::Slider recoveryTimeSlider;
    juce::Label fadeTimeLabel;
    juce::Slider fadeTimeSlider;
    juce::ToggleButton autoRecoveryToggle;
    juce::TextButton resetClippingButton;

    // Panel NaN
    juce::Label titleNaN;
    juce::Label statusNaN;
    juce::Label recoveryTimeLabelNaN;
    juce::Slider recoveryTimeSliderNaN;
    juce::Label fadeTimeLabelNaN;
    juce::Slider fadeTimeSliderNaN;
    juce::ToggleButton autoRecoveryToggleNaN;
    juce::TextButton resetNaNButton;

    // Panel Inf
    juce::Label titleInf;
    juce::Label statusInf;
    juce::Label infTypeLabel;
    juce::Label recoveryTimeLabelInf;
    juce::Slider recoveryTimeSliderInf;
    juce::Label fadeTimeLabelInf;
    juce::Slider fadeTimeSliderInf;
    juce::ToggleButton autoRecoveryToggleInf;
    juce::TextButton resetInfButton;

    // Versión y fondo
    juce::Label titleLabel;
    juce::Image normalBackground;

    // Rectángulos visuales para Auto-Recovery
    juce::Rectangle<int> autoRecoveryRect;
    juce::Rectangle<int> autoRecoveryRectNaN;
    juce::Rectangle<int> autoRecoveryRectInf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JCBSafetyFuseAudioProcessorEditor)
};
