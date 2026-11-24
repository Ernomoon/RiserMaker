#pragma once
#include "PluginProcessor.h"

namespace ui
{
inline juce::Font font (float h, bool bold = false)
{
    return juce::Font (juce::FontOptions (h, bold ? juce::Font::bold : juce::Font::plain));
}
inline const juce::Colour bg (0xff0b0b0e), panel (0xff17171b), panelDark (0xff0e0e11), purple (0xff7c5cff),
                          amber (0xffff9a1a), dim (0xff72727a), light (0xffd6d6dc), red (0xffee5555);

// small crescent-moon logo (the "hole" colour must match what is behind it)
inline void drawMoon (juce::Graphics& g, juce::Rectangle<float> r, juce::Colour c, juce::Colour hole)
{
    const float cx = r.getCentreX(), cy = r.getCentreY(), rad = r.getWidth() * 0.95f;
    g.setGradientFill (juce::ColourGradient (c.withAlpha (0.30f), cx, cy, c.withAlpha (0.0f), cx + rad, cy, true));
    g.fillEllipse (cx - rad, cy - rad, rad * 2.0f, rad * 2.0f);
    g.setColour (c);
    g.fillEllipse (r);
    g.setColour (hole);
    g.fillEllipse (r.translated (r.getWidth() * 0.30f, -r.getHeight() * 0.10f).reduced (r.getWidth() * 0.02f));
}
}

//==============================================================================
class RiserLNF : public juce::LookAndFeel_V4
{
public:
    RiserLNF()
    {
#if JUCE_WINDOWS
        setDefaultSansSerifTypefaceName ("Segoe UI");
#elif JUCE_MAC
        setDefaultSansSerifTypefaceName ("Helvetica Neue");
#endif
        setColour (juce::ResizableWindow::backgroundColourId, juce::Colour (0xff1c1c20));
        setColour (juce::ComboBox::textColourId, ui::light);
        setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff1c1c20));
        setColour (juce::PopupMenu::textColourId, ui::light);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff3a2f7a));
        setColour (juce::TextButton::textColourOffId, ui::light);
        setColour (juce::AlertWindow::backgroundColourId, juce::Colour (0xff1c1c20));
        setColour (juce::AlertWindow::textColourId, ui::light);
    }

    juce::Font getComboBoxFont (juce::ComboBox&) override { return ui::font (13.5f, true); }
    juce::Font getPopupMenuFont() override { return ui::font (14.0f); }
    juce::Font getTextButtonFont (juce::TextButton&, int) override { return ui::font (12.5f, true); }

    void drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool highlighted, bool down) override
    {
        auto r = b.getLocalBounds().toFloat().reduced (0.5f);
        g.setColour (down ? juce::Colour (0xff2a2a30) : highlighted ? juce::Colour (0xff222228) : ui::panelDark);
        g.fillRoundedRectangle (r, 9.0f);
        g.setColour (highlighted ? juce::Colour (0xff55555d) : juce::Colour (0xff34343a));
        g.drawRoundedRectangle (r, 9.0f, 1.0f);
    }

    void drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox& box) override
    {
        auto r = juce::Rectangle<float> (0.0f, 0.0f, (float) w, (float) h).reduced (0.5f);
        g.setColour (ui::panelDark);
        g.fillRoundedRectangle (r, 9.0f);
        g.setColour (box.isPopupActive() ? ui::purple : juce::Colour (0xff34343a));
        g.drawRoundedRectangle (r, 9.0f, 1.0f);
        juce::Path p;
        const float cx = (float) w - 18.0f, cy = (float) h * 0.5f;
        p.addTriangle (cx - 4.5f, cy - 2.0f, cx + 4.5f, cy - 2.0f, cx, cy + 3.5f);
        g.setColour (ui::dim);
        g.fillPath (p);
    }

    void drawCallOutBoxBackground (juce::CallOutBox&, juce::Graphics& g, const juce::Path& path, juce::Image&) override
    {
        g.setColour (juce::Colour (0xff1b1b1f));
        g.fillPath (path);
        g.setColour (juce::Colour (0xff3c3c44));
        g.strokePath (path, juce::PathStrokeType (1.0f));
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                           float startAngle, float endAngle, juce::Slider&) override
    {
        auto b = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height);
        const float cx = b.getCentreX(), cy = b.getCentreY();
        const float R = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f - 26.0f;
        const float angle = startAngle + sliderPos * (endAngle - startAngle);
        const float arcR = R + 11.0f;

        // value arc
        juce::Path track, val;
        track.addCentredArc (cx, cy, arcR, arcR, 0.0f, startAngle, endAngle, true);
        val.addCentredArc (cx, cy, arcR, arcR, 0.0f, startAngle, angle, true);
        g.setColour (juce::Colour (0xff25252a));
        g.strokePath (track, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (ui::purple.withAlpha (0.20f));
        g.strokePath (val, juce::PathStrokeType (10.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (ui::purple);
        g.strokePath (val, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // step dots
        for (int k = 0; k <= 5; ++k)
        {
            const float a = startAngle + (float) k / 5.0f * (endAngle - startAngle);
            const float rr = R + 26.0f;
            const float px = cx + rr * std::sin (a), py = cy - rr * std::cos (a);
            const bool lit = a <= angle + 0.02f;
            g.setColour (lit ? juce::Colour (0xffd9ceff) : juce::Colour (0xff4a4a52));
            g.fillEllipse (px - 2.5f, py - 2.5f, 5.0f, 5.0f);
        }

        // body
        g.setColour (juce::Colour (0x88000000));
        g.fillEllipse (cx - R - 3.0f, cy - R + 5.0f, 2.0f * R + 6.0f, 2.0f * R + 6.0f);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff45454b), cx, cy - R, juce::Colour (0xff0f0f11), cx, cy + R, false));
        g.fillEllipse (cx - R, cy - R, 2.0f * R, 2.0f * R);
        const float r2 = R * 0.92f;
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2e2e33), cx, cy - r2, juce::Colour (0xff17171a), cx, cy + r2, false));
        g.fillEllipse (cx - r2, cy - r2, 2.0f * r2, 2.0f * r2);
        // soft light on the face
        g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.07f), cx - r2 * 0.3f, cy - r2 * 0.5f,
                                                 juce::Colours::white.withAlpha (0.0f), cx - r2 * 0.3f + r2 * 0.9f, cy - r2 * 0.5f, true));
        g.fillEllipse (cx - r2, cy - r2, 2.0f * r2, 2.0f * r2);

        // indicator
        juce::Path ind;
        ind.addRoundedRectangle (-2.0f, -r2 * 0.90f, 4.0f, r2 * 0.30f, 2.0f);
        const auto tf = juce::AffineTransform::rotation (angle).translated (cx, cy);
        g.setColour (ui::purple.withAlpha (0.35f));
        g.strokePath (ind, juce::PathStrokeType (5.0f), tf);
        g.setColour (juce::Colour (0xffe6e0ff));
        g.fillPath (ind, tf);
    }

    int getSliderThumbRadius (juce::Slider&) override { return 9; }

    void drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float sliderPos, float, float,
                           juce::Slider::SliderStyle, juce::Slider&) override
    {
        const float cy = (float) y + (float) h * 0.5f;
        juce::Rectangle<float> track ((float) x + 9.0f, cy - 2.0f, (float) w - 18.0f, 4.0f);
        g.setColour (juce::Colour (0xff2a2a2e));
        g.fillRoundedRectangle (track, 2.0f);
        auto filled = track.withRight (juce::jmax (track.getX() + 1.0f, sliderPos));
        g.setColour (ui::amber.withAlpha (0.25f));
        g.fillRoundedRectangle (filled.expanded (0.0f, 3.0f), 5.0f);
        g.setColour (ui::amber);
        g.fillRoundedRectangle (filled, 2.0f);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3b3b41), 0.0f, cy - 9.0f, juce::Colour (0xff222226), 0.0f, cy + 9.0f, false));
        g.fillEllipse (sliderPos - 9.0f, cy - 9.0f, 18.0f, 18.0f);
        g.setColour (juce::Colour (0xff55555c));
        g.drawEllipse (sliderPos - 9.0f, cy - 9.0f, 18.0f, 18.0f, 1.0f);
    }
};

//==============================================================================
class LabelButton : public juce::Button
{
public:
    enum Icon { None, Play, Loop, Reset, Led };

    LabelButton (const juce::String& text, Icon ic = None, juce::Colour lit = ui::purple)
        : juce::Button (text), icon (ic), litColour (lit) {}

    void paintButton (juce::Graphics& g, bool over, bool down) override
    {
        auto b = getLocalBounds().toFloat().reduced (0.5f);
        const bool on = getToggleState();
        const float rr = b.getHeight() * 0.5f;
        if (on)
        {
            g.setColour (litColour.withAlpha (0.16f));
            g.fillRoundedRectangle (b, rr);
            g.setColour (litColour.withAlpha (0.55f));
            g.drawRoundedRectangle (b, rr, 1.0f);
        }
        else if (over || down)
        {
            g.setColour (juce::Colours::white.withAlpha (down ? 0.09f : 0.05f));
            g.fillRoundedRectangle (b, rr);
        }
        const auto col = on ? litColour : (over ? juce::Colour (0xffcfcfd6) : ui::dim);
        g.setColour (col);
        const float cy = (float) getHeight() * 0.5f, pad = 12.0f;
        float tx = 0.0f;
        if (icon == Play)
        {
            juce::Path p;
            p.addTriangle (pad, cy - 5.0f, pad, cy + 5.0f, pad + 9.0f, cy);
            g.fillPath (p);
            tx = pad + 16.0f;
        }
        else if (icon == Loop)
        {
            g.drawRoundedRectangle (pad, cy - 5.0f, 12.0f, 10.0f, 4.0f, 1.4f);
            tx = pad + 18.0f;
        }
        else if (icon == Reset)
        {
            juce::Path p;
            p.addCentredArc (pad + 6.0f, cy, 5.0f, 5.0f, 0.0f, 0.8f, 5.6f, true);
            g.strokePath (p, juce::PathStrokeType (1.4f));
            tx = pad + 18.0f;
        }
        else if (icon == Led)
        {
            if (on)
            {
                g.setColour (litColour.withAlpha (0.30f));
                g.fillEllipse (pad - 3.0f, cy - 6.5f, 13.0f, 13.0f);
                g.setColour (litColour);
                g.fillEllipse (pad, cy - 3.5f, 7.0f, 7.0f);
            }
            else
                g.drawEllipse (pad, cy - 3.5f, 7.0f, 7.0f, 1.3f);
            g.setColour (col);
            tx = pad + 14.0f;
        }
        g.setFont (ui::font (12.0f, true).withExtraKerningFactor (0.10f));
        if (icon == None)
            g.drawText (getButtonText(), getLocalBounds().toFloat(), juce::Justification::centred, false);
        else
            g.drawText (getButtonText(), juce::Rectangle<float> (tx, 0.0f, (float) getWidth() - tx - 6.0f, (float) getHeight()),
                        juce::Justification::centredLeft, false);
    }

private:
    Icon icon;
    juce::Colour litColour;
};

// the prominent record button
class RecButton : public juce::Button
{
public:
    RecButton() : juce::Button ("REC") {}
    void setRecording (bool r)
    {
        if (r != recording) { recording = r; repaint(); }
    }
    void paintButton (juce::Graphics& g, bool over, bool down) override
    {
        auto b = getLocalBounds().toFloat().reduced (3.0f);
        const float rr = b.getHeight() * 0.5f, cy = b.getCentreY();
        if (recording)
        {
            const float pulse = 0.5f + 0.5f * (float) std::sin (juce::Time::getMillisecondCounterHiRes() * 0.006);
            for (int i = 1; i <= 3; ++i)
            {
                g.setColour (ui::red.withAlpha ((0.22f - 0.05f * (float) i) * (0.4f + 0.6f * pulse)));
                g.drawRoundedRectangle (b.expanded ((float) i), rr + (float) i, 1.5f);
            }
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xffff6a6a), 0.0f, b.getY(), juce::Colour (0xffc22a2a), 0.0f, b.getBottom(), false));
            g.fillRoundedRectangle (b, rr);
            g.setColour (juce::Colours::white);
            g.fillRoundedRectangle (b.getX() + 14.0f, cy - 5.0f, 10.0f, 10.0f, 2.0f);
            g.setFont (ui::font (12.5f, true).withExtraKerningFactor (0.12f));
            g.drawText ("STOP", juce::Rectangle<float> (b.getX() + 30.0f, b.getY(), b.getWidth() - 34.0f, b.getHeight()),
                        juce::Justification::centredLeft, false);
        }
        else
        {
            g.setColour (over ? juce::Colour (0xff3a1c1f) : juce::Colour (0xff2a1618));
            g.fillRoundedRectangle (b, rr);
            g.setColour (ui::red.withAlpha (down ? 1.0f : 0.8f));
            g.drawRoundedRectangle (b, rr, 1.4f);
            g.fillEllipse (b.getX() + 14.0f, cy - 5.0f, 10.0f, 10.0f);
            g.setColour (juce::Colour (0xffff9a9a));
            g.setFont (ui::font (12.5f, true).withExtraKerningFactor (0.12f));
            g.drawText ("REC", juce::Rectangle<float> (b.getX() + 30.0f, b.getY(), b.getWidth() - 34.0f, b.getHeight()),
                        juce::Justification::centredLeft, false);
        }
    }

private:
    bool recording = false;
};

class MenuButton : public juce::Button
{
public:
    MenuButton() : juce::Button ("menu") {}
    void paintButton (juce::Graphics& g, bool over, bool) override
    {
        auto b = getLocalBounds().toFloat().reduced (1.0f);
        g.setColour (ui::panelDark);
        g.fillRoundedRectangle (b, 9.0f);
        g.setColour (over ? juce::Colour (0xff5a5a62) : juce::Colour (0xff34343a));
        g.drawRoundedRectangle (b, 9.0f, 1.0f);
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
            g.setFont (ui::font (12.5f, true).withExtraKerningFactor (0.12f));
            if (rec != 0)
            {
                const float pulse = 0.5f + 0.5f * (float) std::sin (juce::Time::getMillisecondCounterHiRes() * 0.006);
                g.setColour (ui::red.withAlpha (0.25f * pulse));
                g.fillEllipse (b.getCentreX() - 8.0f, b.getCentreY() - 40.0f, 16.0f, 16.0f);
                g.setColour (ui::red);
                g.fillEllipse (b.getCentreX() - 4.0f, b.getCentreY() - 36.0f, 8.0f, 8.0f);
                g.drawText (recText, b.withTrimmedTop (10.0f), juce::Justification::centred, false);
            }
            else
            {
                ui::drawMoon (g, juce::Rectangle<float> (b.getCentreX() - 14.0f, b.getCentreY() - 42.0f, 28.0f, 28.0f),
                              ui::purple.withAlpha (0.55f), ui::panelDark);
                g.setColour (ui::dim.withAlpha (0.85f));
                g.drawText (fileHover ? "DROP AUDIO FILE" : "PRESS REC, PLAY YOUR AUDIO, PRESS STOP   -   OR DROP A FILE",
                            b.withTrimmedTop (22.0f), juce::Justification::centred, false);
            }
            return;
        }

        auto area = b.reduced (26.0f, 0.0f);
        auto strip = area.removeFromBottom (18.0f);
        const float cy = area.getCentreY() - 2.0f, maxH = area.getHeight() * 0.46f;

        g.setColour (juce::Colour (0xff17171c));
        g.drawLine (area.getX(), cy, area.getRight(), cy, 1.0f);
        g.setGradientFill (juce::ColourGradient (ui::purple.withAlpha (0.26f), area.getCentreX(), cy,
                                                 ui::purple.withAlpha (0.0f), area.getX(), cy, true));
        g.fillRect (b);

        const int N = (int) peaks.size();
        const float step = area.getWidth() / (float) N;
        juce::Path bars;
        for (int i = 0; i < N; ++i)
        {
            const float h = juce::jmax (1.5f, peaks[(size_t) i] * maxH);
            const float x = area.getX() + ((float) i + 0.5f) * step;
            bars.addRoundedRectangle (x - 1.0f, cy - h, 2.0f, 2.0f * h, 1.0f);
        }
        g.setColour (ui::purple.withAlpha (0.13f));
        g.strokePath (bars, juce::PathStrokeType (6.0f));
        juce::ColourGradient grad (juce::Colour (0xffc4b3ff), 0.0f, cy - maxH, juce::Colour (0xffc4b3ff), 0.0f, cy + maxH, false);
        grad.addColour (0.5, juce::Colour (0xff6e48ff));
        g.setGradientFill (grad);
        g.fillPath (bars);

        // bar / beat ruler: shows where the riser lands (0 = the vocal enters)
        if (! showRaw)
        {
            static const double mult[6] = { 0.25, 0.5, 1.0, 2.0, 4.0, 8.0 };
            const int idx = juce::jlimit (0, 5, (int) std::lround (proc.getP ("length")));
            const int beats = juce::jmax (1, (int) std::lround (mult[idx] * 4.0));
            g.setFont (ui::font (9.5f, true));
            for (int i = 0; i <= beats; ++i)
            {
                const float x = area.getX() + area.getWidth() * (float) i / (float) beats;
                const bool isBar = (i % 4) == 0;
                g.setColour (ui::dim.withAlpha (isBar ? 0.75f : 0.35f));
                g.drawLine (x, strip.getY() + 1.0f, x, strip.getY() + (isBar ? 6.0f : 3.5f), 1.0f);
                if (isBar && (beats >= 4 || i == beats))
                {
                    const int left = (beats - i) / 4;
                    g.drawText (left == 0 ? "0" : "-" + juce::String (left),
                                juce::Rectangle<float> (x - 14.0f, strip.getY() + 5.0f, 28.0f, 12.0f), juce::Justification::centred, false);
                }
            }
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
            const float pulse = 0.5f + 0.5f * (float) std::sin (juce::Time::getMillisecondCounterHiRes() * 0.006);
            g.setColour (ui::red.withAlpha (0.3f * pulse));
            g.fillEllipse (b.getX() + 7.0f, b.getY() + 5.0f, 14.0f, 14.0f);
            g.setColour (ui::red);
            g.fillEllipse (b.getX() + 11.0f, b.getY() + 9.0f, 6.0f, 6.0f);
            g.setFont (ui::font (11.0f, true));
            g.drawText (recText, juce::Rectangle<float> (b.getX() + 26.0f, b.getY() + 4.0f, 280.0f, 16.0f), juce::Justification::centredLeft, false);
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
            g.setFont (ui::font (10.5f, true).withExtraKerningFactor (0.08f));
            g.drawText ("AUTOMATIC SIBILANCE DETECTION", strip, juce::Justification::centred, false);
        }

        if (! showRaw && proc.isListening())
        {
            const float x = area.getX() + proc.getListenProgress() * area.getWidth();
            g.setColour (juce::Colours::white.withAlpha (0.18f));
            g.drawLine (x, b.getY() + 6.0f, x, b.getBottom() - 6.0f, 5.0f);
            g.setColour (juce::Colours::white.withAlpha (0.9f));
            g.drawLine (x, b.getY() + 6.0f, x, b.getBottom() - 6.0f, 1.2f);
        }

        if (! showRaw && hover && ! dragging && rec == 0)
        {
            auto r = area.reduced (10.0f, 14.0f);
            g.setColour (juce::Colour (0xcc0b0b0e));
            g.fillRoundedRectangle (r, 12.0f);
            juce::Path rp, dashed;
            rp.addRoundedRectangle (r, 12.0f);
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
    void mouseDown (const juce::MouseEvent&) override { dragging = false; dragConsumed = false; }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (dragConsumed || e.getDistanceFromDragStart() < 8 || ! proc.hasRiser())
            return;
        auto f = proc.exportRiserToFile();
        if (f.existsAsFile())
        {
            dragConsumed = true; // one drag per mouse press
            dragging = true;
            juce::DragAndDropContainer::performExternalDragDropOfFiles (juce::StringArray (f.getFullPathName()), false, this);
            dragging = false;   // (blocking on Windows) the drag is over now
            repaint();
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
    bool hover = false, dragging = false, dragConsumed = false, fileHover = false;
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
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff111115), 0.0f, 0.0f, juce::Colour (0xff0c0c0f), 0.0f, full.getBottom(), false));
        g.fillRoundedRectangle (full, 12.0f);
        g.setColour (juce::Colour (0xff25252b));
        g.drawRoundedRectangle (full.reduced (0.5f), 12.0f, 1.0f);

        const double lowF = lowHz(), highF = highHz();
        const bool lowOn = lowF > 25.0, highOn = highF < 19000.0;
        g.setFont (ui::font (11.0f, true).withExtraKerningFactor (0.1f));
        g.setColour (lowOn ? ui::amber : ui::dim);
        g.drawText ("LOW CUT  " + fmtHz (lowF, lowOn), head, juce::Justification::centredLeft, false);
        g.setColour (highOn ? ui::amber : ui::dim);
        g.drawText ("HIGH CUT  " + fmtHz (highF, highOn), head, juce::Justification::centredRight, false);
        if (slopeHover)
        {
            g.setColour (juce::Colours::white.withAlpha (0.06f));
            g.fillRoundedRectangle (slopeRect.reduced (0.0f, 1.0f), 8.0f);
        }
        g.setColour (slopeHover ? ui::light : ui::dim);
        g.drawText ("SLOPE  " + juce::String (slopeDb()) + " dB/oct", slopeRect, juce::Justification::centred, false);

        // grid
        g.setFont (ui::font (9.5f));
        for (double f : { 50.0, 100.0, 500.0, 1000.0, 5000.0, 10000.0 })
        {
            const float x = freqToX (f);
            g.setColour (juce::Colour (0xff1e1e23));
            g.drawLine (x, plot.getY(), x, plot.getBottom(), 1.0f);
            g.setColour (ui::dim.withAlpha (0.7f));
            g.drawText (f >= 1000.0 ? juce::String ((int) (f / 1000.0)) + "k" : juce::String ((int) f),
                        juce::Rectangle<float> (x - 15.0f, plot.getBottom() - 12.0f, 30.0f, 12.0f), juce::Justification::centred, false);
        }
        g.setColour (juce::Colour (0xff2c2c32));
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
        g.setGradientFill (juce::ColourGradient (ui::purple.withAlpha (0.42f), 0.0f, plot.getY(),
                                                 ui::purple.withAlpha (0.03f), 0.0f, plot.getBottom(), false));
        g.fillPath (fill);

        // darken what is cut away
        g.setColour (juce::Colour (0x66000000));
        if (lowOn)
            g.fillRect (juce::Rectangle<float> (plot.getX(), plot.getY(), juce::jmax (0.0f, freqToX (lowF) - plot.getX()), plot.getHeight()));
        if (highOn)
            g.fillRect (juce::Rectangle<float> (freqToX (highF), plot.getY(), juce::jmax (0.0f, plot.getRight() - freqToX (highF)), plot.getHeight()));

        g.setColour (ui::purple.withAlpha (0.20f));
        g.strokePath (curve, juce::PathStrokeType (6.0f));
        g.setColour (juce::Colour (0xff9a82ff));
        g.strokePath (curve, juce::PathStrokeType (2.0f));

        // handles
        for (int k = 0; k < 2; ++k)
        {
            const double f = (k == 0) ? lowF : highF;
            const float x = freqToX (f), y = dbToY (respDb (f));
            const bool active = (k == 0) ? lowOn : highOn;
            if (active)
            {
                g.setColour (ui::amber.withAlpha (0.18f));
                g.fillEllipse (x - 13.0f, y - 13.0f, 26.0f, 26.0f);
            }
            g.setColour (juce::Colour (0xff17171a));
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
    enum Kind { Percent, Hertz, Choice };

    ParamRow (juce::AudioProcessorValueTreeState& apvts, const juce::String& id, const juce::String& title,
              Kind k, const juce::StringArray& names = {})
        : titleStr (title), kind (k), choiceNames (names)
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
        juce::String vs;
        if (kind == Percent)     vs = juce::String (juce::roundToInt (v * 100.0)) + "%";
        else if (kind == Hertz)  vs = juce::String (v, 1) + " Hz";
        else                     vs = choiceNames[juce::jlimit (0, juce::jmax (0, choiceNames.size() - 1), juce::roundToInt (v))];
        juce::AttributedString as;
        as.append (titleStr + "  ", ui::font (14.0f, true), juce::Colours::white);
        as.append (vs, ui::font (14.0f), kind == Choice ? ui::amber : ui::dim);
        as.setJustification (juce::Justification::centred);
        as.draw (g, juce::Rectangle<float> (0.0f, 0.0f, (float) getWidth(), 22.0f));
    }
    void resized() override { slider.setBounds (0, 24, getWidth(), 22); }

private:
    juce::String titleStr;
    Kind kind;
    juce::StringArray choiceNames;
    juce::Slider slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
};

class FxPopup : public juce::Component
{
public:
    FxPopup (juce::AudioProcessorValueTreeState& apvts, int which)
    {
        static const char* titles[4] = { "PHONE", "CHORUS", "DRIVE", "TREMOLO  -  synced to project tempo" };
        title = titles[juce::jlimit (0, 3, which)];
        auto add = [&] (const char* id, const char* t, ParamRow::Kind k, const juce::StringArray& names = {})
        {
            auto* r = rows.add (new ParamRow (apvts, id, t, k, names));
            addAndMakeVisible (r);
        };
        switch (which)
        {
            case 0: add ("phoneMix", "MIX", ParamRow::Percent); break;
            case 1: add ("chorusRate", "RATE", ParamRow::Hertz); add ("chorusDepth", "DEPTH", ParamRow::Percent); add ("chorusMix", "MIX", ParamRow::Percent); break;
            case 2: add ("driveAmt", "DRIVE", ParamRow::Percent); add ("driveMix", "MIX", ParamRow::Percent); break;
            default: add ("tremRate", "RATE", ParamRow::Choice, RiserProcessor::getTremDivisionNames()); add ("tremMix", "MIX", ParamRow::Percent); break;
        }
        setSize (250, rows.size() * 56 + 40);
    }
    void paint (juce::Graphics& g) override
    {
        g.setColour (ui::dim);
        g.setFont (ui::font (10.5f, true).withExtraKerningFactor (0.12f));
        g.drawText (title, juce::Rectangle<float> (0.0f, 8.0f, (float) getWidth(), 14.0f), juce::Justification::centred, false);
    }
    void resized() override
    {
        int y = 30;
        for (auto* r : rows)
        {
            r->setBounds (16, y, getWidth() - 32, 48);
            y += 56;
        }
    }

private:
    juce::String title;
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
        auto b = getLocalBounds().toFloat().reduced (1.0f);
        const bool on = isOn();
        const auto col = on ? ui::amber : (hover ? juce::Colour (0xffa0a0a8) : ui::dim);
        const float rr = b.getHeight() * 0.5f;
        if (on)
        {
            g.setColour (ui::amber.withAlpha (0.10f));
            g.fillRoundedRectangle (b.expanded (2.0f), rr + 2.0f);
            g.setColour (ui::amber.withAlpha (0.14f));
            g.fillRoundedRectangle (b, rr);
            g.setColour (ui::amber.withAlpha (0.6f));
            g.drawRoundedRectangle (b, rr, 1.2f);
        }
        else
        {
            g.setColour (hover ? juce::Colour (0xff1c1c21) : juce::Colour (0xff141417));
            g.fillRoundedRectangle (b, rr);
            g.setColour (juce::Colour (0xff2c2c32));
            g.drawRoundedRectangle (b, rr, 1.0f);
        }
        const float cx = b.getX() + 20.0f, cy = b.getCentreY();
        juce::Path arc;
        arc.addCentredArc (cx, cy + 1.0f, 6.0f, 6.0f, 0.0f, 0.6f, juce::MathConstants<float>::twoPi - 0.6f, true);
        g.setColour (col);
        g.strokePath (arc, juce::PathStrokeType (1.7f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.drawLine (cx, cy - 7.0f, cx, cy, 1.7f);
        g.setFont (ui::font (13.0f, true).withExtraKerningFactor (0.12f));
        g.drawText (name, juce::Rectangle<float> (b.getX() + 36.0f, b.getY(), b.getWidth() - 40.0f, b.getHeight()),
                    juce::Justification::centredLeft, false);
    }
    void mouseEnter (const juce::MouseEvent&) override { hover = true; repaint(); }
    void mouseExit (const juce::MouseEvent&) override { hover = false; repaint(); }
    void mouseDown (const juce::MouseEvent&) override { downTime = juce::Time::getMillisecondCounter(); }
    void mouseUp (const juce::MouseEvent& e) override
    {
        if (! getLocalBounds().contains (e.getPosition())) return;
        if (e.x < 36)
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
    bool hover = false;
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
    RecButton recBtn;
    LabelButton autoBtn { "AUTO REC", LabelButton::Led, ui::red };
    LabelButton exportBtn { "EXPORT", LabelButton::None, ui::light };
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
