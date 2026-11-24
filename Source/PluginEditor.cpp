#include "PluginEditor.h"

//==============================================================================
RiserContent::RiserContent (RiserProcessor& p, juce::Component& pp, std::function<void()> resetSize)
    : proc (p), popupParent (pp), onResetSize (std::move (resetSize)),
      wave (p), eq (p.apvts),
      knobAtt (p.apvts, "length", knob),
      phoneBtn (p.apvts.getParameter ("phone"), "PHONE", [this] (juce::uint32 t) { openFxPopup (0, phoneBtn, t); }),
      chorusBtn (p.apvts.getParameter ("chorus"), "CHORUS", [this] (juce::uint32 t) { openFxPopup (1, chorusBtn, t); }),
      driveBtn (p.apvts.getParameter ("drive"), "DRIVE", [this] (juce::uint32 t) { openFxPopup (2, driveBtn, t); }),
      tremBtn (p.apvts.getParameter ("tremolo"), "TREMOLO", [this] (juce::uint32 t) { openFxPopup (3, tremBtn, t); })
{
    setSize (640, 740);

    addAndMakeVisible (presetBox);
    addAndMakeVisible (prevBtn);
    addAndMakeVisible (nextBtn);
    addAndMakeVisible (saveBtn);
    addAndMakeVisible (menuBtn);
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
    menuBtn.onClick = [this] { showMenu(); };
    refreshPresets();
    presetBox.setSelectedId (1, juce::dontSendNotification);

    juce::Component* controls[] = { &voxBtn, &instBtn, &resetBtn, &listenBtn, &loopBtn, &recBtn, &autoBtn, &exportBtn, &rawBtn, &editBtn };
    for (auto* b : controls)
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
    recBtn.onClick = [this] { proc.setManualRec (! proc.isManualRec()); };
    autoBtn.onClick = [this] { setBoolParam ("autoRec", proc.getP ("autoRec") < 0.5f); };
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

RiserContent::~RiserContent()
{
    stopTimer();
    if (popupPtr != nullptr)
        popupPtr->removeComponentListener (this);
}

void RiserContent::setBoolParam (const char* id, bool v)
{
    if (auto* p = proc.apvts.getParameter (id))
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (v ? 1.0f : 0.0f);
        p->endChangeGesture();
    }
}

void RiserContent::refreshPresets (const juce::String& selectUser)
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

void RiserContent::stepPreset (int delta)
{
    const int total = RiserProcessor::getNumFactoryPresets() + userNames.size();
    if (total <= 0) return;
    int cur = presetBox.getSelectedId();
    if (cur < 1) cur = 1;
    const int next = ((cur - 1 + delta + total) % total) + 1;
    presetBox.setSelectedId (next, juce::sendNotificationSync);
}

void RiserContent::savePresetDialog()
{
    auto* aw = new juce::AlertWindow ("Save preset", "Preset name:", juce::MessageBoxIconType::NoIcon);
    aw->addTextEditor ("name", "My Preset");
    aw->addButton ("Save", 1, juce::KeyPress (juce::KeyPress::returnKey));
    aw->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));
    juce::Component::SafePointer<RiserContent> safe (this);
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

void RiserContent::exportWav()
{
    if (! proc.hasRiser()) return;
    chooser = std::make_unique<juce::FileChooser> ("Save riser as WAV",
                                                   juce::File::getSpecialLocation (juce::File::userDesktopDirectory).getChildFile ("Riser.wav"),
                                                   "*.wav");
    juce::Component::SafePointer<RiserContent> safe (this);
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

void RiserContent::showMenu()
{
    juce::int64 bytes = 0;
    const int count = proc.countRiserFiles (bytes);
    juce::PopupMenu m;
    m.addItem (1, "Open risers folder");
    m.addItem (2, "Delete all created risers  (" + juce::String (count) + " files, "
                      + juce::String ((double) bytes / 1048576.0, 1) + " MB) ...", count > 0);
    m.addSeparator();
    m.addItem (3, "Reset window size");
    m.addSeparator();
    m.addItem (4, "About Moon Riser");
    juce::Component::SafePointer<RiserContent> safe (this);
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&menuBtn),
                     [safe] (int r)
                     {
                         if (safe == nullptr) return;
                         if (r == 1)
                         {
                             auto d = safe->proc.getRiserFolder();
                             d.createDirectory();
                             d.startAsProcess();
                         }
                         else if (r == 2)
                             safe->confirmDelete();
                         else if (r == 3 && safe->onResetSize)
                             safe->onResetSize();
                         else if (r == 4)
                             juce::AlertWindow::showAsync (juce::MessageBoxOptions::makeOptionsOk (
                                                               juce::MessageBoxIconType::InfoIcon, "Moon Riser",
                                                               "Moon Riser  v0.2\nReverse-reverb riser maker\n\nCreated by Ernomoon",
                                                               "OK", safe.getComponent()),
                                                           [] (int) {});
                     });
}

void RiserContent::confirmDelete()
{
    juce::int64 bytes = 0;
    const int count = proc.countRiserFiles (bytes);
    auto opts = juce::MessageBoxOptions::makeOptionsOkCancel (
        juce::MessageBoxIconType::WarningIcon, "Delete all risers",
        "This permanently deletes all " + juce::String (count) + " riser WAV file(s) that Moon Riser has created "
        "(" + proc.getRiserFolder().getFullPathName() + ").\n\nProjects that use these files will lose them. Continue?",
        "Delete", "Cancel", this);
    juce::Component::SafePointer<RiserContent> safe (this);
    juce::AlertWindow::showAsync (opts, [safe] (int r)
    {
        if (r != 1 || safe == nullptr) return;
        const int deleted = safe->proc.deleteAllRisers();
        juce::AlertWindow::showAsync (juce::MessageBoxOptions::makeOptionsOk (
                                          juce::MessageBoxIconType::InfoIcon, "Done",
                                          "Deleted " + juce::String (deleted) + " file(s).", "OK", safe.getComponent()),
                                      [] (int) {});
    });
}

void RiserContent::componentBeingDeleted (juce::Component& c)
{
    if (&c == popupPtr)
    {
        popupPtr = nullptr;
        lastDismissedWhich = popupWhich;
        lastDismissTime = juce::Time::getMillisecondCounter();
    }
}

void RiserContent::openFxPopup (int which, FxButton& anchor, juce::uint32 downTime)
{
    // 1) popup still open -> a click on the name closes it
    if (popupPtr != nullptr)
    {
        popupPtr->dismiss();
        return;
    }
    // 2) this very click already closed it (the callout dismisses itself on mouse-down) -> do not reopen
    if (which == lastDismissedWhich && (int) (downTime - lastDismissTime) < 150)
        return;

    if (! anchor.isOn())
        anchor.setOn (true);

    auto& box = juce::CallOutBox::launchAsynchronously (std::make_unique<FxPopup> (proc.apvts, which),
                                                         popupParent.getLocalArea (&anchor, anchor.getLocalBounds()),
                                                         &popupParent);
    popupPtr = &box;
    popupWhich = which;
    box.addComponentListener (this);
}

void RiserContent::timerCallback()
{
    const int v = proc.uiVersion.load();
    if (v != lastVer)
    {
        lastVer = v;
        wave.repaint();
    }
    const bool vox = proc.getP ("vox") > 0.5f;
    const bool manual = proc.isManualRec();
    voxBtn.setToggleState (vox, juce::dontSendNotification);
    instBtn.setToggleState (! vox, juce::dontSendNotification);
    listenBtn.setToggleState (proc.isListening(), juce::dontSendNotification);
    loopBtn.setToggleState (proc.getP ("loop") > 0.5f, juce::dontSendNotification);
    autoBtn.setToggleState (proc.getP ("autoRec") > 0.5f, juce::dontSendNotification);
    recBtn.setRecording (manual);
    if (manual)
        recBtn.repaint(); // pulsing glow
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
    if (proc.isListening() || proc.getRecState() != 0)
        wave.repaint();
}

void RiserContent::paint (juce::Graphics& g)
{
    // background with a soft moonlit glow
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff0e0e13), 0.0f, 0.0f, ui::bg, 0.0f, (float) getHeight(), false));
    g.fillAll();
    g.setGradientFill (juce::ColourGradient (ui::purple.withAlpha (0.10f), 320.0f, 60.0f, ui::purple.withAlpha (0.0f), 320.0f, 420.0f, true));
    g.fillRect (getLocalBounds());

    ui::drawMoon (g, juce::Rectangle<float> (28.0f, 20.0f, 28.0f, 28.0f), juce::Colour (0xffd9ceff), juce::Colour (0xff0d0d11));

    // main panel
    juce::DropShadow (juce::Colours::black.withAlpha (0.65f), 26, { 0, 8 }).drawForRectangle (g, juce::Rectangle<int> (14, 74, 612, 652));
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff1c1c21), 0.0f, 74.0f, juce::Colour (0xff131316), 0.0f, 726.0f, false));
    g.fillRoundedRectangle (14.0f, 74.0f, 612.0f, 652.0f, 20.0f);
    g.setColour (juce::Colour (0xff2c2c32));
    g.drawRoundedRectangle (14.5f, 74.5f, 611.0f, 651.0f, 20.0f, 1.0f);
    g.setColour (juce::Colours::white.withAlpha (0.05f));
    g.drawLine (34.0f, 75.5f, 606.0f, 75.5f, 1.0f);

    // waveform panel
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff101014), 0.0f, 84.0f, juce::Colour (0xff09090c), 0.0f, 276.0f, false));
    g.fillRoundedRectangle (26.0f, 84.0f, 588.0f, 192.0f, 14.0f);
    g.setColour (juce::Colour (0xff202026));
    g.drawRoundedRectangle (26.5f, 84.5f, 587.0f, 191.0f, 14.0f, 1.0f);

    // title
    ui::drawMoon (g, juce::Rectangle<float> (42.0f, 97.0f, 14.0f, 14.0f), juce::Colour (0xffb9a8ff), juce::Colour (0xff0f0f13));
    juce::AttributedString title;
    title.append ("MOON", ui::font (15.0f).withExtraKerningFactor (0.30f), ui::dim);
    title.append (" RISER", ui::font (15.0f, true).withExtraKerningFactor (0.30f), ui::light);
    title.draw (g, juce::Rectangle<float> (66.0f, 92.0f, 200.0f, 24.0f));

    // soft light behind the knob
    g.setGradientFill (juce::ColourGradient (ui::purple.withAlpha (0.10f), 320.0f, 526.0f, ui::purple.withAlpha (0.0f), 320.0f, 690.0f, true));
    g.fillRect (juce::Rectangle<int> (20, 400, 600, 300));

    juce::AttributedString lab;
    lab.append ("REVERSE  ", ui::font (17.0f, true).withExtraKerningFactor (0.15f), ui::light);
    lab.append (lengthText, ui::font (17.0f).withExtraKerningFactor (0.15f), ui::dim);
    lab.setJustification (juce::Justification::centred);
    lab.draw (g, juce::Rectangle<float> (0.0f, 646.0f, 640.0f, 26.0f));

    // corner screws
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3 - i; ++j)
        {
            const float dx = 26.0f + (float) i * 11.0f, dy = 690.0f + (float) j * 11.0f;
            for (float sx : { dx, 640.0f - dx - 6.0f })
            {
                g.setColour (juce::Colour (0xff111113));
                g.fillEllipse (sx, dy, 6.0f, 6.0f);
                g.setColour (juce::Colour (0xff2e2e33));
                g.drawEllipse (sx, dy, 6.0f, 6.0f, 1.0f);
            }
        }

    // brand footer
    g.setFont (ui::font (10.5f, true).withExtraKerningFactor (0.22f));
    g.setColour (ui::dim.withAlpha (0.8f));
    g.drawText ("MOON RISER   -   by Ernomoon", juce::Rectangle<float> (0.0f, 712.0f, 640.0f, 14.0f), juce::Justification::centred, false);
}

void RiserContent::resized()
{
    presetBox.setBounds (150, 20, 230, 36);
    prevBtn.setBounds (384, 20, 34, 36);
    nextBtn.setBounds (420, 20, 34, 36);
    saveBtn.setBounds (462, 20, 56, 36);
    menuBtn.setBounds (566, 20, 44, 36);

    voxBtn.setBounds (268, 92, 50, 26);
    instBtn.setBounds (322, 92, 50, 26);
    resetBtn.setBounds (510, 92, 94, 26);

    wave.setBounds (30, 120, 580, 118);
    const int ry = 245;
    listenBtn.setBounds (34, ry, 92, 26);
    loopBtn.setBounds (130, ry, 74, 26);
    recBtn.setBounds (210, ry - 3, 104, 32);
    autoBtn.setBounds (318, ry, 100, 26);
    exportBtn.setBounds (424, ry, 78, 26);
    rawBtn.setBounds (512, ry, 46, 26);
    editBtn.setBounds (560, ry, 50, 26);

    eq.setBounds (26, 286, 588, 118);
    knob.setBounds (200, 408, 240, 236);

    for (int i = 0; i < 4; ++i)
    {
        FxButton* b = (i == 0) ? &phoneBtn : (i == 1) ? &chorusBtn : (i == 2) ? &driveBtn : &tremBtn;
        b->setBounds (66 + 128 * i, 678, 124, 30);
    }
}

//==============================================================================
RiserEditor::RiserEditor (RiserProcessor& p)
    : juce::AudioProcessorEditor (&p), proc (p),
      content (p, *this, [this] { setSize (640, 740); })
{
    setLookAndFeel (&lnf);
    addAndMakeVisible (content);
    setResizable (true, true);
    setResizeLimits (480, 555, 1120, 1295);
    getConstrainer()->setFixedAspectRatio (640.0 / 740.0);
    const int w = juce::jlimit (480, 1120, p.editorWidth);
    setSize (w, (int) std::lround ((double) w * 740.0 / 640.0));
}

RiserEditor::~RiserEditor()
{
    setLookAndFeel (nullptr);
}

void RiserEditor::resized()
{
    content.setBounds (0, 0, 640, 740);
    content.setTransform (juce::AffineTransform::scale ((float) getWidth() / 640.0f));
    proc.editorWidth = getWidth();
}
