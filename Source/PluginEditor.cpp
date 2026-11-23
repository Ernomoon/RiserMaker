#include "PluginEditor.h"

RiserEditor::RiserEditor (RiserProcessor& p)
    : juce::AudioProcessorEditor (&p), proc (p), wave (p), eq (p.apvts),
      knobAtt (p.apvts, "length", knob),
      phoneBtn (p.apvts.getParameter ("phone"), "PHONE", [this] { openFxPopup (0, phoneBtn); }),
      chorusBtn (p.apvts.getParameter ("chorus"), "CHORUS", [this] { openFxPopup (1, chorusBtn); }),
      driveBtn (p.apvts.getParameter ("drive"), "DRIVE", [this] { openFxPopup (2, driveBtn); }),
      tremBtn (p.apvts.getParameter ("tremolo"), "TREMOLO", [this] { openFxPopup (3, tremBtn); })
{
    setLookAndFeel (&lnf);
    setSize (640, 740);

    addAndMakeVisible (presetBox);
    addAndMakeVisible (prevBtn);
    addAndMakeVisible (nextBtn);
    addAndMakeVisible (saveBtn);
    presetBox.setJustificationType (juce::Justification::centredLeft);
    presetBox.onChange = [this]
    {
        const int id = presetBox.getSelectedId();
        const int F = RiserProcessor::getNumFactoryPresets();
        if (id >= 1 && id <= F)
            proc.applyFactoryPreset (id - 1);
        else if (id > F && (id - F - 1) < userNames.size())
            proc.loadUserPreset (userNames[id - F - 1]);
    };
    prevBtn.onClick = [this] { stepPreset (-1); };
    nextBtn.onClick = [this] { stepPreset (1); };
    saveBtn.onClick = [this] { savePresetDialog(); };
    refreshPresets();
    presetBox.setSelectedId (1, juce::dontSendNotification);

    for (auto* b : { &voxBtn, &instBtn, &resetBtn, &listenBtn, &loopBtn, &exportBtn, &rawBtn, &editBtn })
        addAndMakeVisible (b);
    voxBtn.onClick = [this] { setBoolParam ("vox", true); };
    instBtn.onClick = [this] { setBoolParam ("vox", false); };
    resetBtn.onClick = [this] { proc.clearSource(); };
    listenBtn.onClick = [this]
    {
        if (proc.isListening()) proc.stopListen();
        else                    proc.requestListen();
    };
    loopBtn.onClick = [this] { setBoolParam ("loop", proc.getP ("loop") < 0.5f); };
    exportBtn.onClick = [this] { exportWav(); };
    rawBtn.onClick = [this] { wave.showRaw = true; wave.repaint(); };
    editBtn.onClick = [this] { wave.showRaw = false; wave.repaint(); };

    addAndMakeVisible (wave);
    addAndMakeVisible (eq);

    addAndMakeVisible (knob);
    knob.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    knob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    knob.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
    knob.setMouseDragSensitivity (150);

    for (auto* f : { &phoneBtn, &chorusBtn, &driveBtn, &tremBtn })
        addAndMakeVisible (f);

    startTimerHz (30);
}

RiserEditor::~RiserEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void RiserEditor::setBoolParam (const char* id, bool v)
{
    if (auto* p = proc.apvts.getParameter (id))
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (v ? 1.0f : 0.0f);
        p->endChangeGesture();
    }
}

void RiserEditor::refreshPresets (const juce::String& selectUser)
{
    presetBox.clear (juce::dontSendNotification);
    const int F = RiserProcessor::getNumFactoryPresets();
    for (int i = 0; i < F; ++i)
        presetBox.addItem (RiserProcessor::getFactoryPresetName (i), i + 1);
    userNames = proc.listUserPresets();
    if (userNames.size() > 0)
    {
        presetBox.addSeparator();
        for (int i = 0; i < userNames.size(); ++i)
            presetBox.addItem (userNames[i], F + 1 + i);
    }
    if (selectUser.isNotEmpty())
    {
        const int idx = userNames.indexOf (selectUser);
        if (idx >= 0)
            presetBox.setSelectedId (F + 1 + idx, juce::dontSendNotification);
    }
}

void RiserEditor::stepPreset (int delta)
{
    const int total = RiserProcessor::getNumFactoryPresets() + userNames.size();
    if (total <= 0) return;
    int cur = presetBox.getSelectedId();
    if (cur < 1) cur = 1;
    const int next = ((cur - 1 + delta + total) % total) + 1;
    presetBox.setSelectedId (next, juce::sendNotificationSync);
}

void RiserEditor::savePresetDialog()
{
    auto* aw = new juce::AlertWindow ("Save preset", "Preset name:", juce::MessageBoxIconType::NoIcon);
    aw->addTextEditor ("name", "My Preset");
    aw->addButton ("Save", 1, juce::KeyPress (juce::KeyPress::returnKey));
    aw->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));
    juce::Component::SafePointer<RiserEditor> safe (this);
    aw->enterModalState (true,
                         juce::ModalCallbackFunction::create ([safe, aw] (int result)
                         {
                             if (result == 1 && safe != nullptr)
                             {
                                 const auto nm = aw->getTextEditorContents ("name");
                                 if (safe->proc.saveUserPreset (nm))
                                     safe->refreshPresets (juce::File::createLegalFileName (nm.trim()));
                             }
                         }),
                         true);
}

void RiserEditor::exportWav()
{
    if (! proc.hasRiser()) return;
    chooser = std::make_unique<juce::FileChooser> ("Save riser as WAV",
                                                   juce::File::getSpecialLocation (juce::File::userDesktopDirectory).getChildFile ("Riser.wav"),
                                                   "*.wav");
    juce::Component::SafePointer<RiserEditor> safe (this);
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                              | juce::FileBrowserComponent::warnAboutOverwriting,
                          [safe] (const juce::FileChooser& fc)
                          {
                              if (safe == nullptr) return;
                              const auto dest = fc.getResult();
                              if (dest == juce::File()) return;
                              const auto tmp = safe->proc.exportRiserToFile();
                              if (tmp.existsAsFile())
                                  tmp.copyFileTo (dest);
                          });
}

void RiserEditor::openFxPopup (int which, juce::Component& anchor)
{
    auto content = std::make_unique<FxPopup> (proc.apvts, which);
    juce::CallOutBox::launchAsynchronously (std::move (content), getLocalArea (&anchor, anchor.getLocalBounds()), this);
}

void RiserEditor::timerCallback()
{
    const int v = proc.uiVersion.load();
    if (v != lastVer)
    {
        lastVer = v;
        wave.repaint();
    }
    const bool vox = proc.getP ("vox") > 0.5f;
    voxBtn.setToggleState (vox, juce::dontSendNotification);
    instBtn.setToggleState (! vox, juce::dontSendNotification);
    listenBtn.setToggleState (proc.isListening(), juce::dontSendNotification);
    loopBtn.setToggleState (proc.getP ("loop") > 0.5f, juce::dontSendNotification);
    rawBtn.setToggleState (wave.showRaw, juce::dontSendNotification);
    editBtn.setToggleState (! wave.showRaw, juce::dontSendNotification);

    static const char* names[6] = { "1/4 BAR", "1/2 BAR", "1 BAR", "2 BARS", "4 BARS", "8 BARS" };
    juce::String t = names[juce::jlimit (0, 5, (int) std::lround (proc.getP ("length")))];
    if (proc.getLengthSeconds() > 0.0)
        t += "   " + juce::String (proc.getLengthSeconds(), 1) + " s";
    if (t != lengthText)
    {
        lengthText = t;
        repaint (0, 640, getWidth(), 40);
    }
    for (auto* f : { &phoneBtn, &chorusBtn, &driveBtn, &tremBtn })
        f->repaint();
    if (proc.isListening())
        wave.repaint();
}

void RiserEditor::paint (juce::Graphics& g)
{
    g.fillAll (ui::bg);

    g.setColour (ui::light);
    g.setFont (juce::Font (juce::FontOptions (30.0f, juce::Font::italic)));
    g.drawText ("R", 24, 16, 40, 44, juce::Justification::centred, false);

    g.setColour (ui::panel);
    g.fillRoundedRectangle (14.0f, 74.0f, 612.0f, 652.0f, 18.0f);
    g.setColour (juce::Colour (0xff2a2a2e));
    g.drawRoundedRectangle (14.5f, 74.5f, 611.0f, 651.0f, 18.0f, 1.0f);

    g.setColour (ui::panelDark);
    g.fillRoundedRectangle (26.0f, 84.0f, 588.0f, 192.0f, 12.0f);

    juce::AttributedString title;
    title.append ("RISER", ui::font (15.0f).withExtraKerningFactor (0.3f), ui::dim);
    title.append (" MAKER", ui::font (15.0f, true).withExtraKerningFactor (0.3f), ui::light);
    title.draw (g, juce::Rectangle<float> (42.0f, 92.0f, 220.0f, 24.0f));

    juce::AttributedString lab;
    lab.append ("REVERSE  ", ui::font (17.0f, true).withExtraKerningFactor (0.15f), ui::light);
    lab.append (lengthText, ui::font (17.0f).withExtraKerningFactor (0.15f), ui::dim);
    lab.setJustification (juce::Justification::centred);
    lab.draw (g, juce::Rectangle<float> (0.0f, 646.0f, 640.0f, 26.0f));

    g.setColour (juce::Colour (0xff2c2c30));
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3 - i; ++j)
        {
            const float dx = 24.0f + (float) i * 11.0f, dy = 690.0f + (float) j * 11.0f;
            g.fillEllipse (dx, dy, 6.0f, 6.0f);
            g.fillEllipse (640.0f - dx - 6.0f, dy, 6.0f, 6.0f);
        }
}

void RiserEditor::resized()
{
    presetBox.setBounds (150, 20, 230, 36);
    prevBtn.setBounds (380, 20, 34, 36);
    nextBtn.setBounds (414, 20, 34, 36);
    saveBtn.setBounds (462, 20, 56, 36);

    voxBtn.setBounds (272, 92, 46, 24);
    instBtn.setBounds (322, 92, 46, 24);
    resetBtn.setBounds (510, 92, 90, 24);

    wave.setBounds (30, 120, 580, 120);
    listenBtn.setBounds (42, 246, 90, 22);
    loopBtn.setBounds (140, 246, 80, 22);
    exportBtn.setBounds (270, 246, 110, 22);
    rawBtn.setBounds (480, 246, 50, 22);
    editBtn.setBounds (538, 246, 56, 22);

    eq.setBounds (26, 286, 588, 118);
    knob.setBounds (200, 408, 240, 236);

    for (int i = 0; i < 4; ++i)
    {
        FxButton* b = (i == 0) ? &phoneBtn : (i == 1) ? &chorusBtn : (i == 2) ? &driveBtn : &tremBtn;
        b->setBounds (70 + 125 * i, 680, 120, 28);
    }
}
