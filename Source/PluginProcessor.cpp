#include "PluginProcessor.h"
#include "PluginEditor.h"

JCBSafetyFuseAudioProcessor::JCBSafetyFuseAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

void JCBSafetyFuseAudioProcessor::prepareToPlay(double sampleRate, int)
{
    currentSampleRate = sampleRate;

    // Calcular fade length en samples (mismo para in y out)
    float fadeMs = fadeTimeMs.load(std::memory_order_relaxed);
    fadeLength = static_cast<int>((fadeMs / 1000.0f) * sampleRate);

    // Calcular auto-recovery samples needed
    float recoveryMs = autoRecoveryTimeMs.load(std::memory_order_relaxed);
    autoRecoverySamplesNeeded = static_cast<int>((recoveryMs / 1000.0f) * sampleRate);
}

void JCBSafetyFuseAudioProcessor::releaseResources()
{
}

bool JCBSafetyFuseAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& mainInput  = layouts.getMainInputChannelSet();
    const auto& mainOutput = layouts.getMainOutputChannelSet();

    if (mainInput != mainOutput)
        return false;

    const int numChannels = mainInput.size();
    if (numChannels < 1 || numChannels > 8)
        return false;

    return true;
}

void JCBSafetyFuseAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                               juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();

    // === FASE 1: DETECCIÓN ===
    bool detectedNaN = false;
    bool detectedInf = false;
    bool detectedPosInf = false;
    bool detectedClipping = false;

    const float threshold = explosionThresholdLinear.load(std::memory_order_relaxed);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* samples = buffer.getWritePointer(ch);

        for (int i = 0; i < numSamples; ++i)
        {
            float& s = samples[i];

            // Detectar NaN
            if (std::isnan(s))
            {
                s = 0.0f;
                detectedNaN = true;
                continue;
            }

            // Detectar Inf
            if (std::isinf(s))
            {
                detectedInf = true;
                if (s > 0.0f)
                    detectedPosInf = true;
                s = 0.0f;
                continue;
            }

            // Detectar Clipping
            if (std::abs(s) > threshold)
            {
                detectedClipping = true;
            }
        }
    }

    // === FASE 2: DETECCIÓN DE INICIO DE FADE-OUT ===
    const bool isTripped = trippedClipping.load(std::memory_order_relaxed) ||
                          trippedNaN.load(std::memory_order_relaxed) ||
                          trippedInf.load(std::memory_order_relaxed);

    const bool shouldStartFadeOut = (detectedNaN || detectedInf || detectedClipping) && !isTripped && !isFadingOut;

    if (shouldStartFadeOut)
    {
        // Iniciar fade-out
        fadeOutSamples = 0;
        isFadingOut = true;
    }

    // === FASE 3: MUTE/FADE ===

    if (isFadingOut)
    {
        // Fade-out en progreso
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* samples = buffer.getWritePointer(ch);
            for (int i = 0; i < numSamples; ++i)
            {
                const int currentSample = fadeOutSamples + i;
                if (currentSample < fadeLength)
                {
                    const float gain = 1.0f - (static_cast<float>(currentSample) / static_cast<float>(fadeLength));
                    samples[i] *= gain;
                }
                else
                {
                    samples[i] = 0.0f;
                }
            }
        }

        fadeOutSamples += numSamples;

        // Si terminamos el fade-out, hacer trip
        if (fadeOutSamples >= fadeLength)
        {
            isFadingOut = false;

            // Hacer los trips correspondientes
            if (detectedNaN)
                tripNaN();
            if (detectedInf)
                tripInf(detectedPosInf);
            if (detectedClipping)
                tripClipping();
        }
    }
    else if (isTripped)
    {
        buffer.clear();
    }
    else if (fadeInSamples < fadeLength)
    {
        // Fade-in lineal (sample por sample)
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* samples = buffer.getWritePointer(ch);
            for (int i = 0; i < numSamples; ++i)
            {
                const int currentSample = fadeInSamples + i;
                if (currentSample < fadeLength)
                {
                    const float gain = static_cast<float>(currentSample) / static_cast<float>(fadeLength);
                    samples[i] *= gain;
                }
            }
        }

        fadeInSamples += numSamples;
    }

    // === FASE 4: AUTO-RECOVERY ===

    // NaN auto-recovery (condicional, 500ms fijo)
    if (trippedNaN.load(std::memory_order_relaxed))
    {
        if (autoRecoveryNaN.load(std::memory_order_relaxed))
        {
            if (!detectedNaN)
            {
                cleanSamplesNaN += numSamples;
                const int nanRecoverySamples = static_cast<int>(0.5 * currentSampleRate);  // 500ms
                if (cleanSamplesNaN >= nanRecoverySamples)
                {
                    resetNaN();
                    fadeInSamples = 0;  // Iniciar fade-in
                }
            }
            else
            {
                cleanSamplesNaN = 0;
            }
        }
        else
        {
            // Si auto-recovery está desactivado, resetear el contador
            cleanSamplesNaN = 0;
        }
    }

    // Inf auto-recovery (condicional, 500ms fijo)
    if (trippedInf.load(std::memory_order_relaxed))
    {
        if (autoRecoveryInf.load(std::memory_order_relaxed))
        {
            if (!detectedInf)
            {
                cleanSamplesInf += numSamples;
                const int infRecoverySamples = static_cast<int>(0.5 * currentSampleRate);  // 500ms
                if (cleanSamplesInf >= infRecoverySamples)
                {
                    resetInf();
                    fadeInSamples = 0;  // Iniciar fade-in
                }
            }
            else
            {
                cleanSamplesInf = 0;
            }
        }
        else
        {
            // Si auto-recovery está desactivado, resetear el contador
            cleanSamplesInf = 0;
        }
    }

    // Clipping auto-recovery (condicional, tiempo configurable)
    if (trippedClipping.load(std::memory_order_relaxed))
    {
        if (autoRecoveryClipping.load(std::memory_order_relaxed))
        {
            if (!detectedClipping)
            {
                cleanSamplesClipping += numSamples;
                if (cleanSamplesClipping >= autoRecoverySamplesNeeded)
                {
                    resetClipping();
                    fadeInSamples = 0;  // Iniciar fade-in
                }
            }
            else
            {
                cleanSamplesClipping = 0;
            }
        }
        else
        {
            // Si auto-recovery está desactivado, resetear el contador
            cleanSamplesClipping = 0;
        }
    }
}

// === FUNCIONES DE TRIP ===

void JCBSafetyFuseAudioProcessor::tripClipping() noexcept
{
    trippedClipping.store(true, std::memory_order_relaxed);
    cleanSamplesClipping = 0;
}

void JCBSafetyFuseAudioProcessor::tripNaN() noexcept
{
    trippedNaN.store(true, std::memory_order_relaxed);
    cleanSamplesNaN = 0;
}

void JCBSafetyFuseAudioProcessor::tripInf(bool positive) noexcept
{
    trippedInf.store(true, std::memory_order_relaxed);
    infSign.store(positive ? 1 : -1, std::memory_order_relaxed);
    cleanSamplesInf = 0;
}

// === FUNCIONES DE RESET ===

void JCBSafetyFuseAudioProcessor::resetClipping() noexcept
{
    trippedClipping.store(false, std::memory_order_relaxed);
    cleanSamplesClipping = 0;
    fadeInSamples = 0;
}

void JCBSafetyFuseAudioProcessor::resetNaN() noexcept
{
    trippedNaN.store(false, std::memory_order_relaxed);
    cleanSamplesNaN = 0;
    fadeInSamples = 0;  // Iniciar fade-in (previene clicks)
}

void JCBSafetyFuseAudioProcessor::resetInf() noexcept
{
    trippedInf.store(false, std::memory_order_relaxed);
    infSign.store(0, std::memory_order_relaxed);
    cleanSamplesInf = 0;
    fadeInSamples = 0;  // Iniciar fade-in (previene clicks)
}

// === GETTERS DE ESTADO ===

bool JCBSafetyFuseAudioProcessor::isTrippedClipping() const noexcept
{
    return trippedClipping.load(std::memory_order_relaxed);
}

bool JCBSafetyFuseAudioProcessor::isTrippedNaN() const noexcept
{
    return trippedNaN.load(std::memory_order_relaxed);
}

bool JCBSafetyFuseAudioProcessor::isTrippedInf() const noexcept
{
    return trippedInf.load(std::memory_order_relaxed);
}

int JCBSafetyFuseAudioProcessor::getInfSign() const noexcept
{
    return infSign.load(std::memory_order_relaxed);
}

// === AUTO-RECOVERY ===

bool JCBSafetyFuseAudioProcessor::getAutoRecoveryClipping() const noexcept
{
    return autoRecoveryClipping.load(std::memory_order_relaxed);
}

void JCBSafetyFuseAudioProcessor::setAutoRecoveryClipping(bool enabled) noexcept
{
    autoRecoveryClipping.store(enabled, std::memory_order_relaxed);
}

bool JCBSafetyFuseAudioProcessor::getAutoRecoveryNaN() const noexcept
{
    return autoRecoveryNaN.load(std::memory_order_relaxed);
}

void JCBSafetyFuseAudioProcessor::setAutoRecoveryNaN(bool enabled) noexcept
{
    autoRecoveryNaN.store(enabled, std::memory_order_relaxed);
}

bool JCBSafetyFuseAudioProcessor::getAutoRecoveryInf() const noexcept
{
    return autoRecoveryInf.load(std::memory_order_relaxed);
}

void JCBSafetyFuseAudioProcessor::setAutoRecoveryInf(bool enabled) noexcept
{
    autoRecoveryInf.store(enabled, std::memory_order_relaxed);
}

void JCBSafetyFuseAudioProcessor::setAutoRecoveryTimeMs(float ms) noexcept
{
    autoRecoveryTimeMs.store(ms, std::memory_order_relaxed);
    // Recalcular samples needed
    autoRecoverySamplesNeeded = static_cast<int>((ms / 1000.0f) * currentSampleRate);
}

float JCBSafetyFuseAudioProcessor::getAutoRecoveryTimeMs() const noexcept
{
    return autoRecoveryTimeMs.load(std::memory_order_relaxed);
}

void JCBSafetyFuseAudioProcessor::setFadeTimeMs(float ms) noexcept
{
    fadeTimeMs.store(ms, std::memory_order_relaxed);
    // Recalcular fade length
    fadeLength = static_cast<int>((ms / 1000.0f) * currentSampleRate);
}

float JCBSafetyFuseAudioProcessor::getFadeTimeMs() const noexcept
{
    return fadeTimeMs.load(std::memory_order_relaxed);
}

void JCBSafetyFuseAudioProcessor::setThresholdDB(float dBFS) noexcept
{
    float linear = std::pow(10.0f, dBFS / 20.0f);
    explosionThresholdLinear.store(linear, std::memory_order_relaxed);
}

float JCBSafetyFuseAudioProcessor::getThresholdDB() const noexcept
{
    float linear = explosionThresholdLinear.load(std::memory_order_relaxed);
    return 20.0f * std::log10(linear);
}

juce::AudioProcessorEditor* JCBSafetyFuseAudioProcessor::createEditor()
{
    return new JCBSafetyFuseAudioProcessorEditor(*this);
}

bool JCBSafetyFuseAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String JCBSafetyFuseAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool JCBSafetyFuseAudioProcessor::acceptsMidi() const
{
    return false;
}

bool JCBSafetyFuseAudioProcessor::producesMidi() const
{
    return false;
}

bool JCBSafetyFuseAudioProcessor::isMidiEffect() const
{
    return false;
}

double JCBSafetyFuseAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int JCBSafetyFuseAudioProcessor::getNumPrograms()
{
    return 1;
}

int JCBSafetyFuseAudioProcessor::getCurrentProgram()
{
    return 0;
}

void JCBSafetyFuseAudioProcessor::setCurrentProgram(int)
{
}

const juce::String JCBSafetyFuseAudioProcessor::getProgramName(int)
{
    return {};
}

void JCBSafetyFuseAudioProcessor::changeProgramName(int, const juce::String&)
{
}

void JCBSafetyFuseAudioProcessor::getStateInformation(juce::MemoryBlock&)
{
}

void JCBSafetyFuseAudioProcessor::setStateInformation(const void*, int)
{
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new JCBSafetyFuseAudioProcessor();
}
