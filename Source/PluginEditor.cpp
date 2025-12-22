#include "PluginEditor.h"

JCBSafetyFuseAudioProcessorEditor::JCBSafetyFuseAudioProcessorEditor(JCBSafetyFuseAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(750, 300);

    // Load background image
    normalBackground = juce::ImageCache::getFromMemory(
        BinaryData::fondo_png, BinaryData::fondo_pngSize);

    // === PANEL CLIPPING ===

    titleClipping.setText("CLIPPING", juce::dontSendNotification);
    titleClipping.setFont(juce::FontOptions(18.0f, juce::Font::bold));
    titleClipping.setColour(juce::Label::textColourId, juce::Colours::white);
    titleClipping.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleClipping);

    statusClipping.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    statusClipping.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(statusClipping);

    thresholdLabel.setText("Threshold:", juce::dontSendNotification);
    thresholdLabel.setFont(juce::FontOptions(14.0f));
    thresholdLabel.setJustificationType(juce::Justification::centredRight);
    thresholdLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.8f));
    addAndMakeVisible(thresholdLabel);

    thresholdSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    thresholdSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    thresholdSlider.setRange(-3.0, 15.0, 1.0);
    thresholdSlider.setValue(processor.getThresholdDB(), juce::dontSendNotification);
    thresholdSlider.setTextValueSuffix(" dBFS");
    thresholdSlider.setDoubleClickReturnValue(true, 6.0);

    const auto trackColour = juce::Colour(0xFF2E222E);
    thresholdSlider.setColour(juce::Slider::backgroundColourId, trackColour);
    thresholdSlider.setColour(juce::Slider::trackColourId, trackColour);
    auto thumbColour = juce::Colour(0xFF7C4DFF).interpolatedWith(juce::Colours::white, 0.55f);
    thresholdSlider.setColour(juce::Slider::thumbColourId, thumbColour);
    thresholdSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    thresholdSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);

    thresholdSlider.onValueChange = [this]() {
        processor.setThresholdDB(static_cast<float>(thresholdSlider.getValue()));
    };
    addAndMakeVisible(thresholdSlider);

    recoveryTimeLabel.setText("Recovery:", juce::dontSendNotification);
    recoveryTimeLabel.setFont(juce::FontOptions(14.0f));
    recoveryTimeLabel.setJustificationType(juce::Justification::centredRight);
    recoveryTimeLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.8f));
    addAndMakeVisible(recoveryTimeLabel);

    recoveryTimeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    recoveryTimeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    recoveryTimeSlider.setRange(500.0, 2000.0, 100.0);
    recoveryTimeSlider.setValue(processor.getAutoRecoveryTimeMs(), juce::dontSendNotification);
    recoveryTimeSlider.setTextValueSuffix(" ms");
    recoveryTimeSlider.setDoubleClickReturnValue(true, 2000.0);
    recoveryTimeSlider.setColour(juce::Slider::backgroundColourId, trackColour);
    recoveryTimeSlider.setColour(juce::Slider::trackColourId, trackColour);
    recoveryTimeSlider.setColour(juce::Slider::thumbColourId, thumbColour);
    recoveryTimeSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    recoveryTimeSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    recoveryTimeSlider.onValueChange = [this]() {
        processor.setAutoRecoveryTimeMs(static_cast<float>(recoveryTimeSlider.getValue()));
    };
    addAndMakeVisible(recoveryTimeSlider);

    fadeTimeLabel.setText("Fade:", juce::dontSendNotification);
    fadeTimeLabel.setFont(juce::FontOptions(14.0f));
    fadeTimeLabel.setJustificationType(juce::Justification::centredRight);
    fadeTimeLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.8f));
    addAndMakeVisible(fadeTimeLabel);

    fadeTimeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    fadeTimeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    fadeTimeSlider.setRange(0.0, 40.0, 1.0);
    fadeTimeSlider.setValue(processor.getFadeTimeMs(), juce::dontSendNotification);
    fadeTimeSlider.setTextValueSuffix(" ms");
    fadeTimeSlider.setDoubleClickReturnValue(true, 20.0);
    fadeTimeSlider.setColour(juce::Slider::backgroundColourId, trackColour);
    fadeTimeSlider.setColour(juce::Slider::trackColourId, trackColour);
    fadeTimeSlider.setColour(juce::Slider::thumbColourId, thumbColour);
    fadeTimeSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    fadeTimeSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    fadeTimeSlider.onValueChange = [this]() {
        processor.setFadeTimeMs(static_cast<float>(fadeTimeSlider.getValue()));
    };
    addAndMakeVisible(fadeTimeSlider);

    autoRecoveryToggle.setButtonText("Auto-Recovery");
    autoRecoveryToggle.setToggleState(processor.getAutoRecoveryClipping(), juce::dontSendNotification);
    autoRecoveryToggle.onClick = [this]() {
        processor.setAutoRecoveryClipping(autoRecoveryToggle.getToggleState());
    };
    addAndMakeVisible(autoRecoveryToggle);

    resetClippingButton.setButtonText("RESET | UNMUTE");
    resetClippingButton.onClick = [this]() { processor.resetClipping(); };
    addAndMakeVisible(resetClippingButton);

    // === PANEL NaN ===

    titleNaN.setText("NaN", juce::dontSendNotification);
    titleNaN.setFont(juce::FontOptions(18.0f, juce::Font::bold));
    titleNaN.setColour(juce::Label::textColourId, juce::Colours::white);
    titleNaN.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleNaN);

    statusNaN.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    statusNaN.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(statusNaN);

    recoveryTimeLabelNaN.setText("Recovery:", juce::dontSendNotification);
    recoveryTimeLabelNaN.setFont(juce::FontOptions(14.0f));
    recoveryTimeLabelNaN.setJustificationType(juce::Justification::centredRight);
    recoveryTimeLabelNaN.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.8f));
    recoveryTimeLabelNaN.setAlpha(0.4f);  // Siempre opaco
    addAndMakeVisible(recoveryTimeLabelNaN);

    recoveryTimeSliderNaN.setSliderStyle(juce::Slider::LinearHorizontal);
    recoveryTimeSliderNaN.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    recoveryTimeSliderNaN.setRange(500.0, 500.0, 100.0);  // Rango fijo a 500ms
    recoveryTimeSliderNaN.setValue(500.0, juce::dontSendNotification);
    recoveryTimeSliderNaN.setTextValueSuffix(" ms");
    recoveryTimeSliderNaN.setColour(juce::Slider::backgroundColourId, trackColour);
    recoveryTimeSliderNaN.setColour(juce::Slider::trackColourId, trackColour);
    recoveryTimeSliderNaN.setColour(juce::Slider::thumbColourId, thumbColour);
    recoveryTimeSliderNaN.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    recoveryTimeSliderNaN.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    recoveryTimeSliderNaN.setEnabled(false);  // Siempre deshabilitado
    recoveryTimeSliderNaN.setAlpha(0.4f);  // Siempre opaco
    addAndMakeVisible(recoveryTimeSliderNaN);

    fadeTimeLabelNaN.setText("Fade:", juce::dontSendNotification);
    fadeTimeLabelNaN.setFont(juce::FontOptions(14.0f));
    fadeTimeLabelNaN.setJustificationType(juce::Justification::centredRight);
    fadeTimeLabelNaN.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.8f));
    fadeTimeLabelNaN.setAlpha(0.4f);  // Siempre opaco
    addAndMakeVisible(fadeTimeLabelNaN);

    fadeTimeSliderNaN.setSliderStyle(juce::Slider::LinearHorizontal);
    fadeTimeSliderNaN.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    fadeTimeSliderNaN.setRange(40.0, 40.0, 1.0);  // Rango fijo a 40ms
    fadeTimeSliderNaN.setValue(40.0, juce::dontSendNotification);
    fadeTimeSliderNaN.setTextValueSuffix(" ms");
    fadeTimeSliderNaN.setColour(juce::Slider::backgroundColourId, trackColour);
    fadeTimeSliderNaN.setColour(juce::Slider::trackColourId, trackColour);
    fadeTimeSliderNaN.setColour(juce::Slider::thumbColourId, thumbColour);
    fadeTimeSliderNaN.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    fadeTimeSliderNaN.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    fadeTimeSliderNaN.setEnabled(false);  // Siempre deshabilitado
    fadeTimeSliderNaN.setAlpha(0.4f);  // Siempre opaco
    addAndMakeVisible(fadeTimeSliderNaN);

    autoRecoveryToggleNaN.setButtonText("Auto-Recovery");
    autoRecoveryToggleNaN.setToggleState(processor.getAutoRecoveryNaN(), juce::dontSendNotification);
    autoRecoveryToggleNaN.onClick = [this]() {
        processor.setAutoRecoveryNaN(autoRecoveryToggleNaN.getToggleState());
    };
    addAndMakeVisible(autoRecoveryToggleNaN);

    resetNaNButton.setButtonText("RESET | UNMUTE");
    resetNaNButton.onClick = [this]() { processor.resetNaN(); };
    addAndMakeVisible(resetNaNButton);

    // === PANEL Inf ===

    titleInf.setText("Inf", juce::dontSendNotification);
    titleInf.setFont(juce::FontOptions(18.0f, juce::Font::bold));
    titleInf.setColour(juce::Label::textColourId, juce::Colours::white);
    titleInf.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleInf);

    statusInf.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    statusInf.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(statusInf);

    infTypeLabel.setFont(juce::FontOptions(14.0f));
    infTypeLabel.setColour(juce::Label::textColourId, juce::Colours::orange);
    infTypeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(infTypeLabel);

    recoveryTimeLabelInf.setText("Recovery:", juce::dontSendNotification);
    recoveryTimeLabelInf.setFont(juce::FontOptions(14.0f));
    recoveryTimeLabelInf.setJustificationType(juce::Justification::centredRight);
    recoveryTimeLabelInf.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.8f));
    recoveryTimeLabelInf.setAlpha(0.4f);  // Siempre opaco
    addAndMakeVisible(recoveryTimeLabelInf);

    recoveryTimeSliderInf.setSliderStyle(juce::Slider::LinearHorizontal);
    recoveryTimeSliderInf.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    recoveryTimeSliderInf.setRange(500.0, 500.0, 100.0);  // Rango fijo a 500ms
    recoveryTimeSliderInf.setValue(500.0, juce::dontSendNotification);
    recoveryTimeSliderInf.setTextValueSuffix(" ms");
    recoveryTimeSliderInf.setColour(juce::Slider::backgroundColourId, trackColour);
    recoveryTimeSliderInf.setColour(juce::Slider::trackColourId, trackColour);
    recoveryTimeSliderInf.setColour(juce::Slider::thumbColourId, thumbColour);
    recoveryTimeSliderInf.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    recoveryTimeSliderInf.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    recoveryTimeSliderInf.setEnabled(false);  // Siempre deshabilitado
    recoveryTimeSliderInf.setAlpha(0.4f);  // Siempre opaco
    addAndMakeVisible(recoveryTimeSliderInf);

    fadeTimeLabelInf.setText("Fade:", juce::dontSendNotification);
    fadeTimeLabelInf.setFont(juce::FontOptions(14.0f));
    fadeTimeLabelInf.setJustificationType(juce::Justification::centredRight);
    fadeTimeLabelInf.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.8f));
    fadeTimeLabelInf.setAlpha(0.4f);  // Siempre opaco
    addAndMakeVisible(fadeTimeLabelInf);

    fadeTimeSliderInf.setSliderStyle(juce::Slider::LinearHorizontal);
    fadeTimeSliderInf.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    fadeTimeSliderInf.setRange(40.0, 40.0, 1.0);  // Rango fijo a 40ms
    fadeTimeSliderInf.setValue(40.0, juce::dontSendNotification);
    fadeTimeSliderInf.setTextValueSuffix(" ms");
    fadeTimeSliderInf.setColour(juce::Slider::backgroundColourId, trackColour);
    fadeTimeSliderInf.setColour(juce::Slider::trackColourId, trackColour);
    fadeTimeSliderInf.setColour(juce::Slider::thumbColourId, thumbColour);
    fadeTimeSliderInf.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    fadeTimeSliderInf.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    fadeTimeSliderInf.setEnabled(false);  // Siempre deshabilitado
    fadeTimeSliderInf.setAlpha(0.4f);  // Siempre opaco
    addAndMakeVisible(fadeTimeSliderInf);

    autoRecoveryToggleInf.setButtonText("Auto-Recovery");
    autoRecoveryToggleInf.setToggleState(processor.getAutoRecoveryInf(), juce::dontSendNotification);
    autoRecoveryToggleInf.onClick = [this]() {
        processor.setAutoRecoveryInf(autoRecoveryToggleInf.getToggleState());
    };
    addAndMakeVisible(autoRecoveryToggleInf);

    resetInfButton.setButtonText("RESET | UNMUTE");
    resetInfButton.onClick = [this]() { processor.resetInf(); };
    addAndMakeVisible(resetInfButton);

    // === VERSIÓN (con hipervínculo) ===

    titleLabel.setText("JCBSafetyFuse v1.0.1", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.7f));
    titleLabel.setFont(juce::FontOptions(21.0f));
    titleLabel.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    // Hacer que el label maneje sus propios clics
    titleLabel.addMouseListener(this, false);
    addAndMakeVisible(titleLabel);

    updateStatusDisplay();
    startTimerHz(20);
}

JCBSafetyFuseAudioProcessorEditor::~JCBSafetyFuseAudioProcessorEditor()
{
    stopTimer();
}

void JCBSafetyFuseAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Fondo siempre fondo.png
    g.drawImage(normalBackground, getLocalBounds().toFloat());

    // Overlay azul oscuro en cada panel si está MUTED
    const auto overlayColour = juce::Colour(20, 30, 80).withAlpha(0.85f);

    // Panel izquierdo: NaN
    if (processor.isTrippedNaN())
    {
        g.setColour(overlayColour);
        g.fillRect(0, 0, 250, 300);
    }

    // Panel centro: CLIPPING
    if (processor.isTrippedClipping())
    {
        g.setColour(overlayColour);
        g.fillRect(250, 0, 250, 300);
    }

    // Panel derecho: Inf
    if (processor.isTrippedInf())
    {
        g.setColour(overlayColour);
        g.fillRect(500, 0, 250, 300);
    }

    // Rectángulo de Auto-Recovery en panel Clipping
    if (!autoRecoveryRect.isEmpty())
    {
        bool autoRecoveryEnabled = processor.getAutoRecoveryClipping();

        // Color del rectángulo
        auto rectColour = juce::Colour(50, 50, 50).withAlpha(0.3f);

        // Si Auto-Recovery está desactivado, usar color más opaco
        if (!autoRecoveryEnabled)
            rectColour = juce::Colour(30, 30, 30).withAlpha(0.6f);

        g.setColour(rectColour);
        g.fillRoundedRectangle(autoRecoveryRect.toFloat(), 5.0f);

        // Borde del rectángulo
        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.drawRoundedRectangle(autoRecoveryRect.toFloat(), 5.0f, 1.0f);
    }

    // Rectángulo de Auto-Recovery en panel NaN
    if (!autoRecoveryRectNaN.isEmpty())
    {
        bool autoRecoveryEnabled = processor.getAutoRecoveryNaN();

        // Color del rectángulo
        auto rectColour = juce::Colour(50, 50, 50).withAlpha(0.3f);

        // Si Auto-Recovery está desactivado, usar color más opaco
        if (!autoRecoveryEnabled)
            rectColour = juce::Colour(30, 30, 30).withAlpha(0.6f);

        g.setColour(rectColour);
        g.fillRoundedRectangle(autoRecoveryRectNaN.toFloat(), 5.0f);

        // Borde del rectángulo
        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.drawRoundedRectangle(autoRecoveryRectNaN.toFloat(), 5.0f, 1.0f);
    }

    // Rectángulo de Auto-Recovery en panel Inf
    if (!autoRecoveryRectInf.isEmpty())
    {
        bool autoRecoveryEnabled = processor.getAutoRecoveryInf();

        // Color del rectángulo
        auto rectColour = juce::Colour(50, 50, 50).withAlpha(0.3f);

        // Si Auto-Recovery está desactivado, usar color más opaco
        if (!autoRecoveryEnabled)
            rectColour = juce::Colour(30, 30, 30).withAlpha(0.6f);

        g.setColour(rectColour);
        g.fillRoundedRectangle(autoRecoveryRectInf.toFloat(), 5.0f);

        // Borde del rectángulo
        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.drawRoundedRectangle(autoRecoveryRectInf.toFloat(), 5.0f, 1.0f);
    }
}

void JCBSafetyFuseAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    // Tres paneles de 250px cada uno
    auto leftPanel = area.removeFromLeft(250);    // NaN
    auto centerPanel = area.removeFromLeft(250);  // CLIPPING
    auto rightPanel = area.removeFromLeft(250);   // Inf

    // === PANEL NaN (izquierda) ===
    auto nanArea = leftPanel.reduced(10);

    titleNaN.setBounds(nanArea.removeFromTop(25));
    statusNaN.setBounds(nanArea.removeFromTop(25));
    nanArea.removeFromTop(8);  // Mismo aire que Clipping

    // Espacio para alinearse con threshold de Clipping
    nanArea.removeFromTop(25 + 8);

    // Botón RESET (alineado horizontalmente con Clipping, más estrecho)
    resetNaNButton.setBounds(nanArea.removeFromTop(30).reduced(55, 0));

    nanArea.removeFromTop(12);  // Más aire antes del rectángulo

    // Rectángulo de Auto-Recovery (guardar bounds para paint())
    autoRecoveryRectNaN = nanArea.removeFromTop(95).reduced(5);

    // Dentro del rectángulo:
    auto rectAreaNaN = autoRecoveryRectNaN.reduced(5);

    // Toggle Auto-Recovery
    autoRecoveryToggleNaN.setBounds(rectAreaNaN.removeFromTop(27).reduced(20, 0));

    rectAreaNaN.removeFromTop(2);  // Aire entre toggle y sliders

    // Recovery slider (deshabilitado, solo visual)
    auto recoveryRowNaN = rectAreaNaN.removeFromTop(27);
    recoveryTimeLabelNaN.setBounds(recoveryRowNaN.removeFromLeft(70));
    recoveryTimeSliderNaN.setBounds(recoveryRowNaN);

    rectAreaNaN.removeFromTop(2);  // Aire entre sliders

    // Fade slider (deshabilitado, solo visual)
    auto fadeRowNaN = rectAreaNaN.removeFromTop(27);
    fadeTimeLabelNaN.setBounds(fadeRowNaN.removeFromLeft(70));
    fadeTimeSliderNaN.setBounds(fadeRowNaN);

    // === PANEL CLIPPING (centro) ===
    auto clippingArea = centerPanel.reduced(10);

    // Título y estado
    titleClipping.setBounds(clippingArea.removeFromTop(25));
    statusClipping.setBounds(clippingArea.removeFromTop(25));
    clippingArea.removeFromTop(8);  // Más aire

    // Threshold (hacer más estrecho para que coincida con el rectángulo)
    auto thresholdRow = clippingArea.removeFromTop(25).reduced(5, 0);
    thresholdLabel.setBounds(thresholdRow.removeFromLeft(70));
    thresholdSlider.setBounds(thresholdRow);

    clippingArea.removeFromTop(8);  // Más aire

    // Botón RESET (nueva posición - más arriba, más estrecho)
    resetClippingButton.setBounds(clippingArea.removeFromTop(30).reduced(55, 0));

    clippingArea.removeFromTop(12);  // Más aire antes del rectángulo

    // Rectángulo de Auto-Recovery (guardar bounds para paint())
    autoRecoveryRect = clippingArea.removeFromTop(95).reduced(5);

    // Dentro del rectángulo:
    auto rectArea = autoRecoveryRect.reduced(5);

    // Toggle Auto-Recovery
    autoRecoveryToggle.setBounds(rectArea.removeFromTop(27).reduced(20, 0));

    rectArea.removeFromTop(2);  // Aire entre toggle y sliders

    // Recovery slider
    auto recoveryRow = rectArea.removeFromTop(27);
    recoveryTimeLabel.setBounds(recoveryRow.removeFromLeft(70));
    recoveryTimeSlider.setBounds(recoveryRow);

    rectArea.removeFromTop(2);  // Aire entre sliders

    // Fade slider
    auto fadeRow = rectArea.removeFromTop(27);
    fadeTimeLabel.setBounds(fadeRow.removeFromLeft(70));
    fadeTimeSlider.setBounds(fadeRow);

    // === PANEL Inf (derecha) ===
    auto infArea = rightPanel.reduced(10);

    titleInf.setBounds(infArea.removeFromTop(25));
    statusInf.setBounds(infArea.removeFromTop(25));
    infArea.removeFromTop(8);  // Mismo aire que Clipping
    infTypeLabel.setBounds(infArea.removeFromTop(20));  // Tipo de Inf

    // Espacio para alinear con Clipping y NaN
    infArea.removeFromTop(16);  // Ajustado para alineación perfecta

    // Botón RESET (alineado horizontalmente con Clipping y NaN, más estrecho)
    resetInfButton.setBounds(infArea.removeFromTop(30).reduced(55, 0));

    infArea.removeFromTop(12);  // Más aire antes del rectángulo

    // Rectángulo de Auto-Recovery (guardar bounds para paint())
    autoRecoveryRectInf = infArea.removeFromTop(95).reduced(5);

    // Dentro del rectángulo:
    auto rectAreaInf = autoRecoveryRectInf.reduced(5);

    // Toggle Auto-Recovery
    autoRecoveryToggleInf.setBounds(rectAreaInf.removeFromTop(27).reduced(20, 0));

    rectAreaInf.removeFromTop(2);  // Aire entre toggle y sliders

    // Recovery slider (deshabilitado, solo visual)
    auto recoveryRowInf = rectAreaInf.removeFromTop(27);
    recoveryTimeLabelInf.setBounds(recoveryRowInf.removeFromLeft(70));
    recoveryTimeSliderInf.setBounds(recoveryRowInf);

    rectAreaInf.removeFromTop(2);  // Aire entre sliders

    // Fade slider (deshabilitado, solo visual)
    auto fadeRowInf = rectAreaInf.removeFromTop(27);
    fadeTimeLabelInf.setBounds(fadeRowInf.removeFromLeft(70));
    fadeTimeSliderInf.setBounds(fadeRowInf);

    // === VERSIÓN (bottom, centrada en toda la ventana) ===
    // Calcular área del texto para que el click coincida exactamente
    auto bottomArea = getLocalBounds().removeFromBottom(55);
    auto font = juce::Font(
    juce::FontOptions()
            .withHeight(21.0f)
    );
    juce::GlyphArrangement glyphs;
    glyphs.addLineOfText(font, "JCBSafetyFuse v1.0.1", 0.0f, 0.0f);
    int textWidth = (int) glyphs.getBoundingBox(0, -1, true).getWidth();
    int textHeight = 25;

    // Centrar el label con el tamaño exacto del texto
    auto centeredBounds = bottomArea.withSizeKeepingCentre(textWidth + 20, textHeight);
    titleLabel.setBounds(centeredBounds);
}

void JCBSafetyFuseAudioProcessorEditor::mouseDown(const juce::MouseEvent& event)
{
    // Detectar clic en el título para abrir el enlace de GitHub
    if (event.eventComponent == &titleLabel)
    {
        juce::URL url("https://github.com/cjitter/JCBSafetyFuse");
        url.launchInDefaultBrowser();
    }
}

void JCBSafetyFuseAudioProcessorEditor::timerCallback()
{
    updateStatusDisplay();
    repaint();
}

void JCBSafetyFuseAudioProcessorEditor::updateStatusDisplay()
{
    // Panel Clipping
    if (processor.isTrippedClipping())
    {
        statusClipping.setText("MUTED", juce::dontSendNotification);
        statusClipping.setColour(juce::Label::textColourId, juce::Colours::red);
        resetClippingButton.setEnabled(true);
    }
    else
    {
        statusClipping.setText("SAFE", juce::dontSendNotification);
        statusClipping.setColour(juce::Label::textColourId, juce::Colours::limegreen);
        resetClippingButton.setEnabled(false);
    }

    // Panel NaN
    if (processor.isTrippedNaN())
    {
        statusNaN.setText("MUTED", juce::dontSendNotification);
        statusNaN.setColour(juce::Label::textColourId, juce::Colours::red);
        resetNaNButton.setEnabled(true);
    }
    else
    {
        statusNaN.setText("SAFE", juce::dontSendNotification);
        statusNaN.setColour(juce::Label::textColourId, juce::Colours::limegreen);
        resetNaNButton.setEnabled(false);
    }

    // Panel Inf
    if (processor.isTrippedInf())
    {
        statusInf.setText("MUTED", juce::dontSendNotification);
        statusInf.setColour(juce::Label::textColourId, juce::Colours::red);
        resetInfButton.setEnabled(true);

        // Mostrar tipo de Inf
        int sign = processor.getInfSign();
        if (sign > 0)
            infTypeLabel.setText("+Inf", juce::dontSendNotification);
        else if (sign < 0)
            infTypeLabel.setText("-Inf", juce::dontSendNotification);
    }
    else
    {
        statusInf.setText("SAFE", juce::dontSendNotification);
        statusInf.setColour(juce::Label::textColourId, juce::Colours::limegreen);
        resetInfButton.setEnabled(false);
        infTypeLabel.setText("", juce::dontSendNotification);
    }

    // Opacidad de controles de Auto-Recovery
    bool autoRecoveryEnabled = processor.getAutoRecoveryClipping();
    float alpha = autoRecoveryEnabled ? 1.0f : 0.4f;

    recoveryTimeLabel.setAlpha(alpha);
    recoveryTimeSlider.setAlpha(alpha);
    recoveryTimeSlider.setEnabled(autoRecoveryEnabled);

    fadeTimeLabel.setAlpha(alpha);
    fadeTimeSlider.setAlpha(alpha);
    fadeTimeSlider.setEnabled(autoRecoveryEnabled);
}
