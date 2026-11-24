#pragma once
#include "PluginProcessor.h"

namespace ui
{
inline juce::Font font (float h, bool bold = false)
{
    return juce::Font (juce::FontOptions (h, bold ? juce::Font::bold : juce::Font::plain));
}
inline const juce::Colour bg (0xff0c0c0e), panel (0xff17171a), panelDark (0xff0f0f11), purple (0xff7c5cff),
                          amber (0xffff9a1a), dim (0xff72727a), light (0xffd0d0d6);
}

//==============================================================================
class RiserLNF : public juce::LookAndFeel_V4
{
public:
    RiserLNF()
    {
        setColour (juce::ResizableWindow::backgroundColourId, juce::Colour (0xff1c1c20));
        setColour (juce::ComboBox::backgroundColourId, ui::panelDark);
        setColour (juce::ComboBox::outlineColourId, juce::Colour (0xff3a3a40));
        setColour (juce::ComboBox::textColourId, ui::light);
        setColour (juce::ComboBox::arrowColourId, ui::dim);
        setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff1c1c20));
        setColour (juce::PopupMenu::textColourId, ui::light);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff3a2f7a));
        setColour (juce::TextButton::buttonColourId, ui::panelDark);
        setColour (juce::TextButton::textColourOffId, ui::light);
        setColour (juce::AlertWindow::backgroundColourId, juce::Colour (0xff1c1c20));
        setColour (juce::AlertWindow::textColourId, ui::light);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                           float startAngle, float endAngle, juce::Slider&) override
    {
        auto b = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height);
        const float cx = b.getCentreX(), cy = b.getCentreY();
        const float R = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f - 22.0f;
        const float angle = startAngle + sliderPos * (endAngle - startAngle);

        // tick dots
        for (int j = 0; j <= 25; ++j)
        {
            const float a = startAngle + (float) j / 25.0f * (endAngle - startAngle);
            const bool step = (j % 5) == 0;
            const float rr = R + 16.0f;
            const float px = cx + rr * std::sin (a), py = cy - rr * std::cos (a);
            const bool active = step && std::abs (a - angle) < 0.05f;
            g.setColour (active ? juce::Colours::white : (step ? juce::Colour (0xffb0b0b8) : juce::Colour (0xff55555c)));
            const float d = step ? 4.0f : 2.6f;
            g.fillEllipse (px - d * 0.5f, py - d * 0.5f, d, d);
        }

        // body
        g.setColour (juce::Colour (0x66000000));
        g.fillEllipse (cx - R - 4.0f, cy - R + 4.0f, 2.0f * R + 8.0f, 2.0f * R + 8.0f);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3a3a3f), cx, cy - R, juce::Colour (0xff111113), cx, cy + R, false));
        g.fillEllipse (cx - R, cy - R, 2.0f * R, 2.0f * R);
        const float r2 = R * 0.93f;
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2a2a2d), cx, cy - r2, juce::Colour (0xff1b1b1d), cx, cy + r2, false));
        g.fillEllipse (cx - r2, cy - r2, 2.0f * r2, 2.0f * r2);

        // indicator
        juce::Path ind;
        ind.addRoundedRectangle (-1.8f, -r2 * 0.88f, 3.6f, r2 * 0.30f, 1.8f);
        g.setColour (juce::Colour (0xffc8c8ce));
        g.fillPath (ind, juce::AffineTransform::rotation (angle).translated (cx, cy));
    }

    int getSliderThumbRadius (juce::Slider&) override { return 9; }

    void drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float sliderPos, float, float,
                           juce::Slider::SliderStyle, juce::Slider&) override
    {
        const float cy = (float) y + (float) h * 0.5f;
        juce::Rectangle<float> track ((float) x + 9.0f, cy - 2.0f, (float) w - 18.0f, 4.0f);
        g.setColour (juce::Colour (0xff2a2a2e));
        g.fillRoundedRectangle (track, 2.0f);
        g.setColour (ui::amber);
        g.fillRoundedRectangle (track.withRight (juce::jmax (track.getX() + 1.0f, sliderPos)), 2.0f);
        g.setColour (juce::Colour (0xff2c2c30));
        g.fillEllipse (sliderPos - 9.0f, cy - 9.0f, 18.0f, 18.0f);
        g.setColour (juce::Colour (0xff4a4a50));
        g.drawEllipse (sliderPos - 9.0f, cy - 9.0f, 18.0f, 18.0f, 1.0f);
    }
};

//==============================================================================
class LabelButton : public juce::Button
{
public:
    enum Icon { None, Play, Loop, Reset, Rec };

    LabelButton (const juce::String& text, Icon ic = None, juce::Colour lit = ui::purple)
        : juce::Button (text), icon (ic), litColour (lit) {}

    void paintButton (juce::Graphics& g, bool over, bool) override
    {
        const bool on = getToggleState();
        auto col = on ? litColour : (over ? juce::Colour (0xffbdbdc4) : ui::dim);
        g.setColour (col);
        const float cy = (float) getHeight() * 0.5f;
        float tx = 0.0f;
        if (icon == Play)
        {
            juce::Path p;
            p.addTriangle (1.0f, cy - 5.0f, 1.0f, cy + 5.0f, 10.0f, cy);
            g.fillPath (p);
            tx = 16.0f;
        }
        else if (icon == Loop)
        {
            g.drawRoundedRectangle (1.0f, cy - 5.0f, 12.0f, 10.0f, 4.0f, 1.4f);
            tx = 18.0f;
        }
        else if (icon == Reset)
        {
            juce::Path p;
            p.addCentredArc (7.0f, cy, 5.0f, 5.0f, 0.0f, 0.8f, 5.6f, true);
            g.strokePath (p, juce::PathStrokeType (1.4f));
            tx = 18.0f;
        }
        else if (icon == Rec)
        {
            g.fillEllipse (1.0f, cy - 5.0f, 10.0f, 10.0f);
            tx = 16.0f;
        }
        g.setFont (ui::font (12.5f, true).withExtraKerningFactor (0.15f));
        g.drawText (getButtonText(), juce::Rectangle<float> (tx, 0.0f, (float) getWidth() - tx, (float) getHeight()),
                    icon == None ? juce::Justification::centred : juce::Justification::centredLeft, false);
    }

private:
    Icon icon;
    juce::Colour litColour;
};

class MenuButton : public juce::Button
{
public:
    MenuButton() : juce::Button ("menu") {}
    void paintButton (juce::Graphics& g, bool over, bool) override
    {
        auto b = getLocalBounds().toFloat().reduced (1.0f);
        g.setColour (ui::panelDark);
        g.fillRoundedRectangle (b, 8.0f);
        g.setColour (over ? juce::Colour (0xff5a5a62) : juce::Colour (0xff3a3a40));
        g.drawRoundedRectangle (b, 8.0f, 1.0f);
        g.setColour (over ? ui::light : ui::dim);
        const float cx = b.getCentreX(), cy = b.getCentreY();
        for (int i = -1; i <= 1; ++i)
            g.drawLine (cx - 8.0f, cy + (float) i * 5.0f, cx + 8.0f, cy + (float) i * 5.0f, 1.8f);
    }
};

//==============================================================================
class WaveView : public juce::Component, public juce::FileDragAndDropTarget
{
public:
    explicit WaveView (RiserProcessor& p) : proc (p) {}
    bool showRaw = false;

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        const auto& peaks = showRaw ? proc.sourcePeaks : proc.riserPeaks;
        const int rec = proc.getRecState();
        const juce::String recText = (rec == 2 ? "RECORDING   " : "CAPTURING   ") + juce::String (proc.getRecSeconds(), 1) + " s"
                                     + (rec == 2 ? "   -   press STOP to finish" : "");
        if (peaks.empty())
        {
            g.setFont (ui::font (13.0f, true).withExtraKerningFactor (0.12f));
            if (rec != 0)
            {
                g.setColour (juce::Colour (0xffe05555));
                g.fillEllipse (b.getCentreX() - 130.0f, b.getCentreY() - 4.0f, 8.0f, 8.0f);
                g.drawText (recText, b.withTrimmedLeft (16.0f), juce::Justification::centred, false);
            }
            else
            {
                g.setColour (ui::dim.withAlpha (0.8f));
                g.drawText (fileHover ? "DROP AUDIO FILE" : "PRESS REC OR PLAY AUDIO THROUGH THE PLUGIN  /  DROP A FILE",
                            b, juce::Justification::centred, false);
            }
            return;
        }
        auto area = b.reduced (26.0f, 0.0f);
        const float cy = b.getCentreY(), maxH = b.getHeight() * 0.46f;

        juce::ColourGradient glow (ui::purple.withAlpha (0.28f), area.getCentreX(), cy,
                                   ui::purple.withAlpha (0.0f), area.getX(), cy, true);
        g.setGradientFill (glow);
        g.fillRect (b);

        const int N = (int) peaks.size();
        const float step = area.getWidth() / (float) N;
        for (int i = 0; i < N; ++i)
        {
            const float pk = peaks[(size_t) i];
            const float h = juce::jmax (1.5f, pk * maxH);
            const float x = area.getX() + ((float) i + 0.5f) * step;
            g.setColour (ui::purple.withAlpha (0.35f + 0.65f * pk));
            g.fillRoundedRectangle (x - 1.0f, cy - h, 2.0f, 2.0f * h, 1.0f);
        }

        if (showRaw && proc.selEnd > proc.selStart)
        {
            const float x0 = area.getX() + proc.selStart * area.getWidth();
            const float x1 = area.getX() + proc.selEnd * area.getWidth();
            g.setColour (ui::purple.withAlpha (0.16f));
            g.fillRect (juce::Rectangle<float> (x0, b.getY() + 4.0f, juce::jmax (2.0f, x1 - x0), b.getHeight() - 8.0f));
            g.setColour (ui::purple);
            g.drawLine (x0, b.getY() + 4.0f, x0, b.getBottom() - 4.0f, 1.5f);
            g.drawLine (x1, b.getY() + 4.0f, x1, b.getBottom() - 4.0f, 1.5f);
            g.setFont (ui::font (10.5f, true).withExtraKerningFactor (0.1f));
            g.setColour (juce::Colour (0xffcfc6ff));
            g.drawText ("USED FOR RISER", juce::Rectangle<float> (x0 + 4.0f, b.getY() + 4.0f, 120.0f, 14.0f), juce::Justification::centredLeft, false);
        }

        if (rec != 0)
        {
            g.setColour (juce::Colour (0xffe05555));
            g.fillEllipse (b.getX() + 10.0f, b.getY() + 8.0f, 8.0f, 8.0f);
            g.setFont (ui::font (11.0f, true));
            g.drawText (recText, juce::Rectangle<float> (b.getX() + 24.0f, b.getY() + 4.0f, 260.0f, 16.0f), juce::Justification::centredLeft, false);
        }

        if (showRaw && proc.getP ("vox") > 0.5f)
        {
            int count = 0;
            for (auto& r : proc.sibilance)
            {
                if (++count > 40) break;
                for (float pos : { r.first, r.second })
                {
                    const float x = area.getX() + pos * area.getWidth();
                    g.setColour (juce::Colour (0xffe05555));
                    g.drawLine (x, cy - 24.0f, x, cy + 24.0f, 1.5f);
                    g.fillEllipse (x - 2.5f, cy - 26.5f, 5.0f, 5.0f);
                    g.fillEllipse (x - 2.5f, cy + 21.5f, 5.0f, 5.0f);
                }
            }
            g.setColour (juce::Colour (0xffe05555).withAlpha (0.9f));
            g.setFont (ui::font (11.0f, true));
            g.drawText ("AUTOMATIC SIBILANCE DETECTION", b.removeFromBottom (16.0f), juce::Justification::centred, false);
        }

        if (! showRaw && proc.isListening())
        {
            const float x = area.getX() + proc.getListenProgress() * area.getWidth();
            g.setColour (juce::Colours::white.withAlpha (0.8f));
            g.drawLine (x, b.getY() + 6.0f, x, b.getBottom() - 6.0f, 1.2f);
        }

        if (! showRaw && hover && ! dragging && rec == 0)
        {
            auto r = area.reduced (10.0f, 14.0f);
            g.setColour (juce::Colour (0xaa0f0f11));
            g.fillRoundedRectangle (r, 10.0f);
            juce::Path rp, dashed;
            rp.addRoundedRectangle (r, 10.0f);
            const float dash[] = { 5.0f, 4.0f };
            juce::PathStrokeType (1.5f).createDashedStroke (dashed, rp, dash, 2);
            g.setColour (ui::purple);
            g.fillPath (dashed);
            g.setColour (juce::Colour (0xffd8d0ff));
            g.setFont (ui::font (15.0f, true));
            g.drawText ("Drag & Drop into DAW", r, juce::Justification::centred, false);
        }
    }

    void mouseEnter (const juce::MouseEvent&) override { hover = true; repaint(); }
    void mouseExit (const juce::MouseEvent&) override { hover = false; repaint(); }
    void mouseDown (const juce::MouseEvent&) override { dragging = false; }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (dragging || e.getDistanceFromDragStart() < 8 || ! proc.hasRiser())
            return;
        auto f = proc.exportRiserToFile();
        if (f.existsAsFile())
        {
            dragging = true;
            juce::DragAndDropContainer::performExternalDragDropOfFiles (juce::StringArray (f.getFullPathName()), false, this);
        }
    }
    void mouseUp (const juce::MouseEvent&) override { dragging = false; repaint(); }

    bool isInterestedInFileDrag (const juce::StringArray& files) override
    {
        for (auto& f : files)
            if (juce::File (f).hasFileExtension ("wav;mp3;flac;aif;aiff;ogg"))
                return true;
        return false;
    }
    void fileDragEnter (const juce::StringArray&, int, int) override { fileHover = true; repaint(); }
    void fileDragExit (const juce::StringArray&) override { fileHover = false; repaint(); }
    void filesDropped (const juce::StringArray& files, int, int) override
    {
        fileHover = false;
        if (files.size() > 0)
            proc.loadSourceFromFile (juce::File (files[0]));
        repaint();
    }

private:
    RiserProcessor& proc;
    bool hover = false, dragging = false, fileHover = false;
};

//==============================================================================
class EqView : public juce::Component, private juce::Timer
{
public:
    explicit EqView (juce::AudioProcessorValueTreeState& a) : apvts (a)
    {
        lowP = a.getParameter ("lowCut");
        highP = a.getParameter ("highCut");
        slopeP = a.getParameter ("cutSlope");
        startTimerHz (30);
    }

    void resized() override { updatePlot(); }

    void paint (juce::Graphics& g) override
    {
        updatePlot();
        auto full = getLocalBounds().toFloat();
        g.setColour (ui::panelDark);
        g.fillRoundedRectangle (full, 10.0f);
        g.setColour (juce::Colour (0xff26262a));
        g.drawRoundedRectangle (full.reduced (0.5f), 10.0f, 1.0f);

        const double lowF = lowHz(), highF = highHz();
        g.setFont (ui::font (11.0f, true).withExtraKerningFactor (0.1f));
        g.setColour (lowF > 25.0 ? ui::amber : ui::dim);
        g.drawText ("LOW CUT  " + fmtHz (lowF, lowF > 25.0), head, juce::Justification::centredLeft, false);
        g.setColour (highF < 19000.0 ? ui::amber : ui::dim);
        g.drawText ("HIGH CUT  " + fmtHz (highF, highF < 19000.0), head, juce::Justification::centredRight, false);
        g.setColour (slopeHover ? ui::light : ui::dim);
        g.drawText ("SLOPE  " + juce::String (slopeDb()) + " dB/oct", slopeRect, juce::Justification::centred, false);

        // grid
        g.setFont (ui::font (9.5f));
        for (double f : { 50.0, 100.0, 500.0, 1000.0, 5000.0, 10000.0 })
        {
            const float x = freqToX (f);
            g.setColour (juce::Colour (0xff222226));
            g.drawLine (x, plot.getY(), x, plot.getBottom(), 1.0f);
            g.setColour (ui::dim.withAlpha (0.7f));
            g.drawText (f >= 1000.0 ? juce::String ((int) (f / 1000.0)) + "k" : juce::String ((int) f),
                        juce::Rectangle<float> (x - 15.0f, plot.getBottom() - 12.0f, 30.0f, 12.0f), juce::Justification::centred, false);
        }
        g.setColour (juce::Colour (0xff2e2e33));
        g.drawLine (plot.getX(), dbToY (0.0), plot.getRight(), dbToY (0.0), 1.0f);

        // response curve (the real filter response, same maths as the audio)
        juce::Path curve;
        const int w = (int) plot.getWidth();
        for (int px = 0; px <= w; ++px)
        {
            const float x = plot.getX() + (float) px;
            const float y = dbToY (respDb (xToFreq (x)));
            if (px == 0) curve.startNewSubPath (x, y);
            else         curve.lineTo (x, y);
        }
        juce::Path fill (curve);
        fill.lineTo (plot.getRight(), plot.getBottom());
        fill.lineTo (plot.getX(), plot.getBottom());
        fill.closeSubPath();
        g.setGradientFill (juce::ColourGradient (ui::purple.withAlpha (0.38f), 0.0f, plot.getY(),
                                                 ui::purple.withAlpha (0.02f), 0.0f, plot.getBottom(), false));
        g.fillPath (fill);
        g.setColour (ui::purple);
        g.strokePath (curve, juce::PathStrokeType (2.0f));

        // handles
        for (int k = 0; k < 2; ++k)
        {
            const double f = (k == 0) ? lowF : highF;
            const float x = freqToX (f), y = dbToY (respDb (f));
            const bool active = (k == 0) ? lowF > 25.0 : highF < 19000.0;
            g.setColour (juce::Colour (0xff1a1a1d));
            g.fillEllipse (x - 8.0f, y - 8.0f, 16.0f, 16.0f);
            g.setColour (active ? ui::amber : ui::dim);
            g.drawEllipse (x - 8.0f, y - 8.0f, 16.0f, 16.0f, 2.0f);
        }
    }

    void mouseMove (const juce::MouseEvent& e) override
    {
        const bool h = slopeRect.contains (e.position);
        if (h != slopeHover) { slopeHover = h; repaint(); }
    }
    void mouseExit (const juce::MouseEvent&) override
    {
        if (slopeHover) { slopeHover = false; repaint(); }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        dragActive = false;
        if (slopeRect.contains (e.position))
        {
            if (slopeP != nullptr)
            {
                const int next = (slopeIndex() + 1) % 3;
                slopeP->beginChangeGesture();
                slopeP->setValueNotifyingHost (slopeP->convertTo0to1 ((float) next));
                slopeP->endChangeGesture();
            }
            return;
        }
        const float dl = std::abs (e.position.x - freqToX (lowHz()));
        const float dh = std::abs (e.position.x - freqToX (highHz()));
        dragLow = dl < dh;
        if (juce::jmin (dl, dh) > 26.0f)
            return; // clicked on empty space: do not move anything
        auto* p = dragLow ? lowP : highP;
        if (p == nullptr) return;
        dragActive = true;
        p->beginChangeGesture();
    }
    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (! dragActive) return;
        auto* p = dragLow ? lowP : highP;
        const auto& r = p->getNormalisableRange();
        const float f = juce::jlimit (r.start, r.end, (float) xToFreq (e.position.x));
        p->setValueNotifyingHost (p->convertTo0to1 (f));
        repaint();
    }
    void mouseUp (const juce::MouseEvent&) override
    {
        if (! dragActive) return;
        if (auto* p = dragLow ? lowP : highP)
            p->endChangeGesture();
        dragActive = false;
    }
    void mouseDoubleClick (const juce::MouseEvent& e) override
    {
        const float dl = std::abs (e.position.x - freqToX (lowHz()));
        const float dh = std::abs (e.position.x - freqToX (highHz()));
        if (juce::jmin (dl, dh) > 40.0f) return;
        if (auto* p = (dl < dh) ? lowP : highP)
        {
            p->beginChangeGesture();
            p->setValueNotifyingHost (p->getDefaultValue());
            p->endChangeGesture();
        }
    }

private:
    void timerCallback() override
    {
        const double l = lowHz(), h = highHz();
        const int sl = slopeIndex();
        if (l != lastL || h != lastH || sl != lastSlope) { lastL = l; lastH = h; lastSlope = sl; repaint(); }
    }
    void updatePlot()
    {
        auto full = getLocalBounds().toFloat();
        plot = full.reduced (16.0f, 10.0f);
        head = plot.removeFromTop (16.0f);
        plot.removeFromTop (4.0f);
        slopeRect = juce::Rectangle<float> (head.getCentreX() - 70.0f, head.getY(), 140.0f, head.getHeight());
    }
    double lowHz() const { return (double) apvts.getRawParameterValue ("lowCut")->load(); }
    double highHz() const { return (double) apvts.getRawParameterValue ("highCut")->load(); }
    int slopeIndex() const { return juce::jlimit (0, 2, (int) std::lround (apvts.getRawParameterValue ("cutSlope")->load())); }
    int slopeDb() const { static const int v[3] = { 12, 24, 48 }; return v[slopeIndex()]; }
    int slopeOrder() const { static const int v[3] = { 2, 4, 8 }; return v[slopeIndex()]; }
    static juce::String fmtHz (double f, bool active)
    {
        if (! active) return "OFF";
        return f >= 1000.0 ? juce::String (f / 1000.0, 1) + " kHz" : juce::String ((int) f) + " Hz";
    }
    double xToFreq (float x) const
    {
        return 20.0 * std::pow (1000.0, juce::jlimit (0.0, 1.0, (double) (x - plot.getX()) / (double) plot.getWidth()));
    }
    float freqToX (double f) const
    {
        return plot.getX() + (float) (std::log (f / 20.0) / std::log (1000.0)) * plot.getWidth();
    }
    float dbToY (double db) const
    {
        const double lo = -48.0, hi = 6.0;
        return plot.getBottom() - (float) ((juce::jlimit (lo, hi, db) - lo) / (hi - lo)) * plot.getHeight();
    }
    double respDb (double f) const
    {
        double m = 1.0;
        const double lo = lowHz(), hi = highHz();
        const int order = slopeOrder();
        if (lo > 25.0)    m *= riser::cutMagnitude (true, f, lo, 48000.0, order);
        if (hi < 19000.0) m *= riser::cutMagnitude (false, f, hi, 48000.0, order);
        return 20.0 * std::log10 (juce::jmax (m, 1.0e-6));
    }

    juce::AudioProcessorValueTreeState& apvts;
    juce::RangedAudioParameter* lowP = nullptr;
    juce::RangedAudioParameter* highP = nullptr;
    juce::RangedAudioParameter* slopeP = nullptr;
    juce::Rectangle<float> plot, head, slopeRect;
    bool dragLow = true, dragActive = false, slopeHover = false;
    double lastL = -1.0, lastH = -1.0;
    int lastSlope = -1;
};

//==============================================================================
class ParamRow : public juce::Component
{
public:
    ParamRow (juce::AudioProcessorValueTreeState& apvts, const juce::String& id, const juce::String& title, bool percent)
        : titleStr (title), isPercent (percent)
    {
        addAndMakeVisible (slider);
        slider.setSliderStyle (juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, id, slider);
        slider.onValueChange = [this] { repaint(); };
    }
    void paint (juce::Graphics& g) override
    {
        const double v = slider.getValue();
        const juce::String vs = isPercent ? juce::String (juce::roundToInt (v * 100.0)) + "%" : juce::String (v, 1) + " Hz";
        juce::AttributedString as;
        as.append (titleStr + "  ", ui::font (14.0f, true), juce::Colours::white);
        as.append (vs, ui::font (14.0f), ui::dim);
        as.setJustification (juce::Justification::centred);
        as.draw (g, juce::Rectangle<float> (0.0f, 0.0f, (float) getWidth(), 22.0f));
    }
    void resized() override { slider.setBounds (0, 24, getWidth(), 22); }

private:
    juce::String titleStr;
    bool isPercent;
    juce::Slider slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
};

class FxPopup : public juce::Component
{
public:
    FxPopup (juce::AudioProcessorValueTreeState& apvts, int which)
    {
        auto add = [&] (const char* id, const char* title, bool pct)
        {
            auto* r = rows.add (new ParamRow (apvts, id, title, pct));
            addAndMakeVisible (r);
        };
        switch (which)
        {
            case 0: add ("phoneMix", "MIX", true); break;
            case 1: add ("chorusRate", "RATE", false); add ("chorusDepth", "DEPTH", true); add ("chorusMix", "MIX", true); break;
            case 2: add ("driveAmt", "DRIVE", true); add ("driveMix", "MIX", true); break;
            default: add ("tremRate", "RATE", false); add ("tremMix", "MIX", true); break;
        }
        setSize (230, rows.size() * 56 + 14);
    }
    void resized() override
    {
        int y = 8;
        for (auto* r : rows)
        {
            r->setBounds (14, y, getWidth() - 28, 48);
            y += 56;
        }
    }

private:
    juce::OwnedArray<ParamRow> rows;
};

class FxButton : public juce::Component
{
public:
    FxButton (juce::RangedAudioParameter* p, const juce::String& n, std::function<void (juce::uint32)> onName)
        : par (p), name (n), openPopup (std::move (onName))
    {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }
    bool isOn() const { return par != nullptr && par->getValue() > 0.5f; }
    void setOn (bool on)
    {
        if (par == nullptr) return;
        par->beginChangeGesture();
        par->setValueNotifyingHost (on ? 1.0f : 0.0f);
        par->endChangeGesture();
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        const bool on = isOn();
        const auto col = on ? ui::amber : ui::dim;
        if (on)
        {
            g.setColour (ui::amber.withAlpha (0.14f));
            g.fillRoundedRectangle (b.reduced (0.0f, 2.0f), 10.0f);
        }
        const float cx = b.getX() + 14.0f, cy = b.getCentreY();
        juce::Path arc;
        arc.addCentredArc (cx, cy + 1.0f, 6.0f, 6.0f, 0.0f, 0.6f, juce::MathConstants<float>::twoPi - 0.6f, true);
        g.setColour (col);
        g.strokePath (arc, juce::PathStrokeType (1.7f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.drawLine (cx, cy - 7.0f, cx, cy, 1.7f);
        g.setFont (ui::font (13.5f, true).withExtraKerningFactor (0.12f));
        g.drawText (name, juce::Rectangle<float> (b.getX() + 28.0f, b.getY(), b.getWidth() - 28.0f, b.getHeight()),
                    juce::Justification::centredLeft, false);
    }
    void mouseDown (const juce::MouseEvent&) override { downTime = juce::Time::getMillisecondCounter(); }
    void mouseUp (const juce::MouseEvent& e) override
    {
        if (! getLocalBounds().contains (e.getPosition())) return;
        if (e.x < 28)
            setOn (! isOn());
        else if (openPopup)
            openPopup (downTime);
        repaint();
    }

private:
    juce::RangedAudioParameter* par;
    juce::String name;
    std::function<void (juce::uint32)> openPopup;
    juce::uint32 downTime = 0;
};

//==============================================================================
// All the UI lives here, laid out for a fixed 640x740 design; the editor scales it.
class RiserContent : public juce::Component, private juce::Timer, private juce::ComponentListener
{
public:
    RiserContent (RiserProcessor&, juce::Component& popupParent, std::function<void()> resetSize);
    ~RiserContent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void componentBeingDeleted (juce::Component& c) override;
    void refreshPresets (const juce::String& selectUser = {});
    void stepPreset (int delta);
    void savePresetDialog();
    void exportWav();
    void openFxPopup (int which, FxButton& anchor, juce::uint32 downTime);
    void showMenu();
    void confirmDelete();
    void setBoolParam (const char* id, bool v);

    RiserProcessor& proc;
    juce::Component& popupParent;
    std::function<void()> onResetSize;

    juce::ComboBox presetBox;
    juce::TextButton prevBtn { "<" }, nextBtn { ">" }, saveBtn { "SAVE" };
    MenuButton menuBtn;
    juce::StringArray userNames;

    LabelButton voxBtn { "VOX" }, instBtn { "INST", LabelButton::None, ui::light };
    LabelButton resetBtn { "RESET", LabelButton::Reset, ui::light };
    LabelButton listenBtn { "LISTEN", LabelButton::Play, ui::purple };
    LabelButton loopBtn { "LOOP", LabelButton::Loop, ui::purple };
    LabelButton recBtn { "REC", LabelButton::Rec, juce::Colour (0xffe05555) };
    LabelButton autoBtn { "AUTO", LabelButton::None, ui::purple };
    LabelButton exportBtn { "EXPORT WAV", LabelButton::None, ui::light };
    LabelButton rawBtn { "RAW", LabelButton::None, ui::purple };
    LabelButton editBtn { "EDIT", LabelButton::None, ui::purple };

    WaveView wave;
    EqView eq;
    juce::Slider knob;
    juce::AudioProcessorValueTreeState::SliderAttachment knobAtt;
    FxButton phoneBtn, chorusBtn, driveBtn, tremBtn;

    // popup bookkeeping (second click on the same effect closes it)
    juce::CallOutBox* popupPtr = nullptr;
    int popupWhich = -1, lastDismissedWhich = -1;
    juce::uint32 lastDismissTime = 0;

    std::unique_ptr<juce::FileChooser> chooser;
    juce::String lengthText = "4 BARS";
    int lastVer = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RiserContent)
};

//==============================================================================
class RiserEditor : public juce::AudioProcessorEditor
{
public:
    explicit RiserEditor (RiserProcessor&);
    ~RiserEditor() override;
    void resized() override;

private:
    RiserProcessor& proc;
    RiserLNF lnf;
    RiserContent content;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RiserEditor)
};
