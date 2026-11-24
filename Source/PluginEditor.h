#pragma once
#include "PluginProcessor.h"

namespace ui
{
inline juce::Font font (float h, bool bold = false)
{
    return juce::Font (juce::FontOptions (h, bold ? juce::Font::bold : juce::Font::plain));
}
inline const juce::Colour bg (0xff0b0b0e), panel (0xff17171b), panelDark (0xff0e0e11), purple (0xff7c5cff),
                          amber (0xffff9a1a), dim (0xff72727a), light (0xffd6d6dc), red (0xffee5555),
                          ice (0xff8fb4ff);

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

// a handful of faint, fixed "stars" inside a rectangle
inline void drawStars (juce::Graphics& g, juce::Rectangle<float> r, int count, float alpha, unsigned seed)
{
    unsigned s = seed;
    auto next = [&s] { s = s * 1664525u + 1013904223u; return (float) (s >> 8) / (float) (1u << 24); };
    for (int i = 0; i < count; ++i)
    {
        const float x = r.getX() + next() * r.getWidth(), y = r.getY() + next() * r.getHeight();
        const float a = alpha * (0.3f + 0.7f * next()), d = 0.8f + 1.4f * next();
        g.setColour (juce::Colours::white.withAlpha (a));
        g.fillEllipse (x, y, d, d);
    }
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
        setColour (juce::TooltipWindow::backgroundColourId, juce::Colour (0xff1c1c20));
        setColour (juce::TooltipWindow::textColourId, ui::light);
        setColour (juce::TooltipWindow::outlineColourId, juce::Colour (0xff3c3c44));
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
                           float startAngle, float endAngle, juce::Slider& slider) override
    {
        static const char* labels[6] = { "1/4", "1/2", "1", "2", "4", "8" };
        auto b = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height);
        const float cx = b.getCentreX(), cy = b.getCentreY();
        const float R = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f - 26.0f;
        const float angle = startAngle + sliderPos * (endAngle - startAngle);
        const float arcR = R + 11.0f;
        const int idx = juce::jlimit (0, 5, juce::roundToInt (slider.getValue()));

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

        // step dots + value labels
        g.setFont (ui::font (10.5f, true));
        for (int k = 0; k <= 5; ++k)
        {
            const float a = startAngle + (float) k / 5.0f * (endAngle - startAngle);
            const float rr = R + 26.0f;
            const float px = cx + rr * std::sin (a), py = cy - rr * std::cos (a);
            const bool lit = a <= angle + 0.02f;
            g.setColour (lit ? juce::Colour (0xffd9ceff) : juce::Colour (0xff4a4a52));
            g.fillEllipse (px - 2.5f, py - 2.5f, 5.0f, 5.0f);
            const float lr = R + 42.0f;
            g.setColour (k == idx ? juce::Colours::white : ui::dim);
            g.drawText (labels[k], juce::Rectangle<float> (cx + lr * std::sin (a) - 14.0f, cy - lr * std::cos (a) - 7.0f, 28.0f, 14.0f),
                        juce::Justification::centred, false);
        }

        // body
        g.setColour (juce::Colour (0x88000000));
        g.fillEllipse (cx - R - 3.0f, cy - R + 5.0f, 2.0f * R + 6.0f, 2.0f * R + 6.0f);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff45454b), cx, cy - R, juce::Colour (0xff0f0f11), cx, cy + R, false));
        g.fillEllipse (cx - R, cy - R, 2.0f * R, 2.0f * R);
        const float r2 = R * 0.92f;
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2e2e33), cx, cy - r2, juce::Colour (0xff17171a), cx, cy + r2, false));
        g.fillEllipse (cx - r2, cy - r2, 2.0f * r2, 2.0f * r2);
        g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.07f), cx - r2 * 0.3f, cy - r2 * 0.5f,
                                                 juce::Colours::white.withAlpha (0.0f), cx - r2 * 0.3f + r2 * 0.9f, cy - r2 * 0.5f, true));
        g.fillEllipse (cx - r2, cy - r2, 2.0f * r2, 2.0f * r2);

        // value readout in the middle of the knob
        g.setColour (juce::Colour (0xfff0eaff));
        g.setFont (ui::font (30.0f, true));
        g.drawText (labels[idx], juce::Rectangle<float> (cx - 50.0f, cy - 4.0f, 100.0f, 36.0f), juce::Justification::centred, false);
        g.setColour (ui::dim);
        g.setFont (ui::font (10.5f, true).withExtraKerningFactor (0.2f));
        g.drawText (idx >= 3 ? "BARS" : "BAR", juce::Rectangle<float> (cx - 50.0f, cy + 26.0f, 100.0f, 14.0f), juce::Justification::centred, false);

        // indicator
        juce::Path ind;
        ind.addRoundedRectangle (-2.0f, -r2 * 0.92f, 4.0f, r2 * 0.22f, 2.0f);
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

// static artwork (panels, shadows, titles). Rendered once and cached as an image => no lag.
class Backdrop : public juce::Component
{
public:
    Backdrop()
    {
        setInterceptsMouseClicks (false, false);
        setBufferedToImage (true);
    }
    void paint (juce::Graphics& g) override
    {
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff0e0e13), 0.0f, 0.0f, ui::bg, 0.0f, (float) getHeight(), false));
        g.fillAll();
        g.setGradientFill (juce::ColourGradient (ui::purple.withAlpha (0.10f), 320.0f, 60.0f, ui::purple.withAlpha (0.0f), 320.0f, 420.0f, true));
        g.fillRect (getLocalBounds());
        ui::drawStars (g, juce::Rectangle<float> (0.0f, 0.0f, 640.0f, 74.0f), 26, 0.30f, 7u);

        ui::drawMoon (g, juce::Rectangle<float> (28.0f, 20.0f, 28.0f, 28.0f), juce::Colour (0xffd9ceff), juce::Colour (0xff0d0d11));

        juce::DropShadow (juce::Colours::black.withAlpha (0.65f), 26, { 0, 8 }).drawForRectangle (g, juce::Rectangle<int> (14, 74, 612, 652));
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff1c1c21), 0.0f, 74.0f, juce::Colour (0xff131316), 0.0f, 726.0f, false));
        g.fillRoundedRectangle (14.0f, 74.0f, 612.0f, 652.0f, 20.0f);
        g.setColour (juce::Colour (0xff2c2c32));
        g.drawRoundedRectangle (14.5f, 74.5f, 611.0f, 651.0f, 20.0f, 1.0f);
        g.setColour (juce::Colours::white.withAlpha (0.05f));
        g.drawLine (34.0f, 75.5f, 606.0f, 75.5f, 1.0f);

        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff101014), 0.0f, 84.0f, juce::Colour (0xff09090c), 0.0f, 276.0f, false));
        g.fillRoundedRectangle (26.0f, 84.0f, 588.0f, 192.0f, 14.0f);
        ui::drawStars (g, juce::Rectangle<float> (30.0f, 120.0f, 580.0f, 116.0f), 34, 0.16f, 21u);
        g.setColour (juce::Colour (0xff202026));
        g.drawRoundedRectangle (26.5f, 84.5f, 587.0f, 191.0f, 14.0f, 1.0f);

        ui::drawMoon (g, juce::Rectangle<float> (42.0f, 97.0f, 14.0f, 14.0f), juce::Colour (0xffb9a8ff), juce::Colour (0xff0f0f13));
        juce::AttributedString title;
        title.append ("MOON", ui::font (15.0f).withExtraKerningFactor (0.30f), ui::dim);
        title.append (" RISER", ui::font (15.0f, true).withExtraKerningFactor (0.30f), ui::light);
        title.draw (g, juce::Rectangle<float> (66.0f, 92.0f, 200.0f, 24.0f));

        g.setGradientFill (juce::ColourGradient (ui::purple.withAlpha (0.10f), 320.0f, 526.0f, ui::purple.withAlpha (0.0f), 320.0f, 690.0f, true));
        g.fillRect (juce::Rectangle<int> (20, 400, 600, 300));

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

        g.setFont (ui::font (10.5f, true).withExtraKerningFactor (0.22f));
        g.setColour (ui::dim.withAlpha (0.8f));
        g.drawText ("MOON RISER   -   by Ernomoon", juce::Rectangle<float> (0.0f, 712.0f, 640.0f, 14.0f), juce::Justification::centred, false);
    }
};

//==============================================================================
// The cached picture of the waveform (bars, ruler, selection). Re-rendered only when the data changes.
class WaveBars : public juce::Component
{
public:
    explicit WaveBars (RiserProcessor& p) : proc (p)
    {
        setInterceptsMouseClicks (false, false);
        setBufferedToImage (true);
    }
    bool showRaw = false;

    static juce::Rectangle<float> plotArea (juce::Rectangle<float> b)
    {
        auto a = b.reduced (26.0f, 0.0f);
        a.removeFromBottom (18.0f);
        return a;
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        const auto& peaks = showRaw ? proc.sourcePeaks : proc.riserPeaks;
        if (peaks.empty())
        {
            if (proc.getRecState() == 0)
            {
                ui::drawMoon (g, juce::Rectangle<float> (b.getCentreX() - 14.0f, b.getCentreY() - 42.0f, 28.0f, 28.0f),
                              ui::purple.withAlpha (0.55f), juce::Colour (0xff0b0b0e));
                g.setColour (ui::dim.withAlpha (0.85f));
                g.setFont (ui::font (12.5f, true).withExtraKerningFactor (0.12f));
                g.drawText (showRaw ? "NOTHING RECORDED YET" : "PRESS REC, PLAY YOUR AUDIO, PRESS STOP   -   OR DROP A FILE",
                            b.withTrimmedTop (22.0f), juce::Justification::centred, false);
            }
            return;
        }

        auto area = plotArea (b);
        auto strip = b.reduced (26.0f, 0.0f).removeFromBottom (18.0f);
        const float cy = area.getCentreY() - 2.0f, maxH = area.getHeight() * 0.46f;
        const juce::Colour hi = showRaw ? juce::Colour (0xffe4edff) : juce::Colour (0xffc4b3ff);
        const juce::Colour mid = showRaw ? juce::Colour (0xff6f9dff) : juce::Colour (0xff6e48ff);
        const juce::Colour glowC = showRaw ? ui::ice : ui::purple;

        g.setColour (juce::Colour (0xff17171c));
        g.drawLine (area.getX(), cy, area.getRight(), cy, 1.0f);
        g.setGradientFill (juce::ColourGradient (glowC.withAlpha (0.22f), area.getCentreX(), cy,
                                                 glowC.withAlpha (0.0f), area.getX(), cy, true));
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
        g.setColour (glowC.withAlpha (0.12f));
        g.strokePath (bars, juce::PathStrokeType (6.0f));
        juce::ColourGradient grad (hi, 0.0f, cy - maxH, hi, 0.0f, cy + maxH, false);
        grad.addColour (0.5, mid);
        g.setGradientFill (grad);
        g.fillPath (bars);

        // caption: makes it obvious which waveform this is
        g.setFont (ui::font (10.5f, true).withExtraKerningFactor (0.12f));
        g.setColour (showRaw ? ui::ice : juce::Colour (0xffb9a8ff));
        g.drawText (showRaw ? "ORIGINAL AUDIO   " + juce::String (proc.getSourceSeconds(), 1) + " s"
                            : "RISER   " + juce::String (proc.getLengthSeconds(), 1) + " s",
                    juce::Rectangle<float> (b.getRight() - 200.0f, b.getY() + 5.0f, 186.0f, 14.0f), juce::Justification::centredRight, false);

        if (! showRaw)
        {
            // bar / beat ruler: shows where the riser lands (0 = the vocal enters)
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
            return;
        }

        // RAW: which part of the original is used for the riser
        if (proc.selEnd > proc.selStart)
        {
            const float x0 = area.getX() + proc.selStart * area.getWidth();
            const float x1 = area.getX() + proc.selEnd * area.getWidth();
            g.setColour (ui::purple.withAlpha (0.18f));
            g.fillRect (juce::Rectangle<float> (x0, b.getY() + 4.0f, juce::jmax (2.0f, x1 - x0), b.getHeight() - 8.0f));
            g.setColour (ui::purple);
            g.drawLine (x0, b.getY() + 4.0f, x0, b.getBottom() - 4.0f, 1.5f);
            g.drawLine (x1, b.getY() + 4.0f, x1, b.getBottom() - 4.0f, 1.5f);
            g.setFont (ui::font (10.5f, true).withExtraKerningFactor (0.1f));
            g.setColour (juce::Colour (0xffcfc6ff));
            g.drawText (proc.hasManualSelection() ? "PICKED BY YOU" : "AUTO PICK",
                        juce::Rectangle<float> (x0 + 4.0f, b.getY() + 5.0f, 120.0f, 14.0f), juce::Justification::centredLeft, false);
        }

        const bool vox = proc.getP ("vox") > 0.5f;
        if (vox)
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
        }
        g.setFont (ui::font (10.0f, true).withExtraKerningFactor (0.08f));
        g.setColour (ui::dim);
        g.drawText ("DRAG = PICK THE PART FOR THE RISER    DOUBLE-CLICK = AUTO", strip, juce::Justification::centredLeft, false);
        if (vox)
        {
            g.setColour (juce::Colour (0xffe05555).withAlpha (0.9f));
            g.drawText ("RED = SIBILANCE (REDUCED)", strip, juce::Justification::centredRight, false);
        }
    }

private:
    RiserProcessor& proc;
};

//==============================================================================
class WaveView : public juce::Component, public juce::FileDragAndDropTarget,
                 public juce::SettableTooltipClient, private juce::Timer
{
public:
    explicit WaveView (RiserProcessor& p) : proc (p), bars (p)
    {
        addAndMakeVisible (bars);
        setTooltip ("EDIT: drag the riser into your playlist.   RAW: drag over the original to choose the part that is used.");
    }
    ~WaveView() override { stopTimer(); }

    void setShowRaw (bool r)
    {
        showRaw = r;
        bars.showRaw = r;
        bars.repaint();
        repaint();
    }
    bool isRaw() const { return showRaw; }
    void refreshBars() { bars.repaint(); repaint(); }
    void startReveal()
    {
        revealStart = juce::Time::getMillisecondCounterHiRes();
        revealing = true;
        if (! isTimerRunning()) startTimerHz (60);
    }

    void resized() override { bars.setBounds (getLocalBounds()); }

    void paintOverChildren (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        auto area = WaveBars::plotArea (b);
        const int rec = proc.getRecState();
        const auto& peaks = showRaw ? proc.sourcePeaks : proc.riserPeaks;
        const juce::String recText = (rec == 2 ? "RECORDING   " : "CAPTURING   ") + juce::String (proc.getRecSeconds(), 1) + " s"
                                     + (rec == 2 ? "   -   press STOP to finish" : "");
        const float pulse = 0.5f + 0.5f * (float) std::sin (juce::Time::getMillisecondCounterHiRes() * 0.006);

        if (rec != 0)
        {
            if (peaks.empty())
            {
                g.setColour (ui::red.withAlpha (0.25f * pulse));
                g.fillEllipse (b.getCentreX() - 8.0f, b.getCentreY() - 40.0f, 16.0f, 16.0f);
                g.setColour (ui::red);
                g.fillEllipse (b.getCentreX() - 4.0f, b.getCentreY() - 36.0f, 8.0f, 8.0f);
                g.setFont (ui::font (12.5f, true).withExtraKerningFactor (0.12f));
                g.drawText (recText, b.withTrimmedTop (10.0f), juce::Justification::centred, false);
            }
            else
            {
                g.setColour (ui::red.withAlpha (0.3f * pulse));
                g.fillEllipse (b.getX() + 7.0f, b.getY() + 5.0f, 14.0f, 14.0f);
                g.setColour (ui::red);
                g.fillEllipse (b.getX() + 11.0f, b.getY() + 9.0f, 6.0f, 6.0f);
                g.setFont (ui::font (11.0f, true));
                g.drawText (recText, juce::Rectangle<float> (b.getX() + 26.0f, b.getY() + 4.0f, 280.0f, 16.0f), juce::Justification::centredLeft, false);
            }
        }
        if (peaks.empty())
        {
            if (fileHover)
            {
                g.setColour (juce::Colour (0xcc0b0b0e));
                g.fillRoundedRectangle (area.reduced (10.0f, 14.0f), 12.0f);
                g.setColour (juce::Colour (0xffd8d0ff));
                g.setFont (ui::font (15.0f, true));
                g.drawText ("DROP AUDIO FILE", area, juce::Justification::centred, false);
            }
            return;
        }

        if (! showRaw && proc.isListening())
        {
            const float x = area.getX() + proc.getListenProgress() * area.getWidth();
            g.setColour (ui::purple.withAlpha (0.10f));
            g.fillRect (juce::Rectangle<float> (area.getX(), b.getY() + 2.0f, juce::jmax (0.0f, x - area.getX()), b.getHeight() - 4.0f));
            g.setColour (juce::Colours::white.withAlpha (0.18f));
            g.drawLine (x, b.getY() + 6.0f, x, b.getBottom() - 6.0f, 5.0f);
            g.setColour (juce::Colours::white.withAlpha (0.9f));
            g.drawLine (x, b.getY() + 6.0f, x, b.getBottom() - 6.0f, 1.2f);
        }

        if (showRaw && selDragging)
        {
            const float x0 = area.getX() + juce::jmin (dragA, dragB) * area.getWidth();
            const float x1 = area.getX() + juce::jmax (dragA, dragB) * area.getWidth();
            g.setColour (ui::amber.withAlpha (0.16f));
            g.fillRect (juce::Rectangle<float> (x0, b.getY() + 4.0f, juce::jmax (1.0f, x1 - x0), b.getHeight() - 8.0f));
            g.setColour (ui::amber);
            g.drawLine (x0, b.getY() + 4.0f, x0, b.getBottom() - 4.0f, 1.5f);
            g.drawLine (x1, b.getY() + 4.0f, x1, b.getBottom() - 4.0f, 1.5f);
        }

        if (! showRaw && hover && ! dragging && rec == 0 && ! revealing)
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

        if (revealing)
        {
            const double t = juce::jlimit (0.0, 1.0, (juce::Time::getMillisecondCounterHiRes() - revealStart) / 520.0);
            const float eased = (float) (1.0 - std::pow (1.0 - t, 3.0));
            const float x = area.getX() + eased * area.getWidth();
            g.setColour (juce::Colour (0xf50b0b0e));
            g.fillRect (juce::Rectangle<float> (x, b.getY() + 1.0f, juce::jmax (0.0f, b.getRight() - x), b.getHeight() - 2.0f));
            g.setColour ((showRaw ? ui::ice : ui::purple).withAlpha (0.85f));
            g.drawLine (x, b.getY() + 6.0f, x, b.getBottom() - 18.0f, 1.5f);
        }
    }

    void mouseEnter (const juce::MouseEvent&) override { hover = true; repaint(); }
    void mouseExit (const juce::MouseEvent&) override { hover = false; repaint(); }

    void mouseDown (const juce::MouseEvent& e) override
    {
        dragging = false;
        dragConsumed = false;
        if (showRaw && ! proc.sourcePeaks.empty())
        {
            selDragging = true;
            dragA = dragB = xToNorm (e.position.x);
        }
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (showRaw)
        {
            if (selDragging)
            {
                dragB = xToNorm (e.position.x);
                repaint();
            }
            return;
        }
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

    void mouseUp (const juce::MouseEvent& e) override
    {
        if (selDragging)
        {
            selDragging = false;
            if (e.getDistanceFromDragStart() > 4 && std::abs (dragB - dragA) > 0.004f)
                proc.setManualSelection (dragA, dragB);
        }
        dragging = false;
        repaint();
    }

    void mouseDoubleClick (const juce::MouseEvent&) override
    {
        if (showRaw)
            proc.clearManualSelection();
    }

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
    void timerCallback() override
    {
        const double t = (juce::Time::getMillisecondCounterHiRes() - revealStart) / 520.0;
        if (t >= 1.0)
        {
            revealing = false;
            stopTimer();
        }
        repaint();
    }
    float xToNorm (float x) const
    {
        auto area = WaveBars::plotArea (getLocalBounds().toFloat());
        return juce::jlimit (0.0f, 1.0f, (x - area.getX()) / juce::jmax (1.0f, area.getWidth()));
    }

    RiserProcessor& proc;
    WaveBars bars;
    bool showRaw = false;
    bool hover = false, dragging = false, dragConsumed = false, fileHover = false;
    bool selDragging = false, revealing = false;
    float dragA = 0.0f, dragB = 0.0f;
    double revealStart = 0.0;
};

//==============================================================================
class EqView : public juce::Component, public juce::SettableTooltipClient
{
public:
    explicit EqView (juce::AudioProcessorValueTreeState& a) : apvts (a)
    {
        lowP = a.getParameter ("lowCut");
        highP = a.getParameter ("highCut");
        slopeP = a.getParameter ("cutSlope");
        eqP = a.getParameter ("eqOn");
        setTooltip ("Drag the circles to cut lows / highs of the riser.  Click EQ to bypass it (your settings are kept).  Click SLOPE to change the steepness.");
    }

    // called by the editor's single timer
    void refresh()
    {
        const double l = lowHz(), h = highHz();
        const int sl = slopeIndex();
        const bool on = eqIsOn();
        if (l != lastL || h != lastH || sl != lastSlope || on != lastOn)
        {
            lastL = l; lastH = h; lastSlope = sl; lastOn = on;
            repaint();
        }
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

        const bool eqOn = eqIsOn();
        const double lowF = lowHz(), highF = highHz();
        const bool lowSet = lowF > 25.0, highSet = highF < 19000.0;
        const bool lowOn = lowSet && eqOn, highOn = highSet && eqOn;

        // filter coefficients, computed once per paint
        int cnt = 0;
        const double* q = riser::butterQ (slopeOrder(), cnt);
        riser::BiquadCoefs lc[4], hc[4];
        for (int i = 0; i < cnt; ++i)
        {
            lc[i] = riser::BiquadCoefs::make (true, 48000.0, lowF, q[i]);
            hc[i] = riser::BiquadCoefs::make (false, 48000.0, highF, q[i]);
        }
        auto respDb = [&] (double f)
        {
            double m = 1.0;
            for (int i = 0; i < cnt; ++i)
            {
                if (lowSet)  m *= lc[i].magnitude (f, 48000.0);
                if (highSet) m *= hc[i].magnitude (f, 48000.0);
            }
            return 20.0 * std::log10 (juce::jmax (m, 1.0e-6));
        };

        // header
        {
            const auto accent = eqOn ? ui::purple : ui::dim;
            const float cx = eqRect.getX() + 10.0f, cy = eqRect.getCentreY();
            juce::Path arc;
            arc.addCentredArc (cx, cy + 0.5f, 5.0f, 5.0f, 0.0f, 0.6f, juce::MathConstants<float>::twoPi - 0.6f, true);
            g.setColour (accent);
            g.strokePath (arc, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.drawLine (cx, cy - 6.0f, cx, cy - 0.5f, 1.5f);
            g.setFont (ui::font (11.0f, true).withExtraKerningFactor (0.1f));
            g.drawText ("EQ", eqRect.withTrimmedLeft (22.0f), juce::Justification::centredLeft, false);
            if (eqHover)
            {
                g.setColour (juce::Colours::white.withAlpha (0.06f));
                g.fillRoundedRectangle (eqRect.reduced (0.0f, 1.0f), 8.0f);
            }
        }
        g.setFont (ui::font (11.0f, true).withExtraKerningFactor (0.1f));
        g.setColour (lowOn ? ui::amber : ui::dim);
        g.drawText ("LOW CUT  " + fmtHz (lowF, lowSet), head.withTrimmedLeft (56.0f), juce::Justification::centredLeft, false);
        g.setColour (highOn ? ui::amber : ui::dim);
        g.drawText ("HIGH CUT  " + fmtHz (highF, highSet), head, juce::Justification::centredRight, false);
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
        const float y0 = dbToY (0.0);
        g.setColour (juce::Colour (0xff2c2c32));
        g.drawLine (plot.getX(), y0, plot.getRight(), y0, 1.0f);

        // response curve
        juce::Path curve;
        const int w = (int) plot.getWidth();
        for (int px = 0; px <= w; ++px)
        {
            const float x = plot.getX() + (float) px;
            const float y = dbToY (respDb (xToFreq (x)));
            if (px == 0) curve.startNewSubPath (x, y);
            else         curve.lineTo (x, y);
        }

        // what is being removed: a soft amber wedge between the 0 dB line and the curve
        if (eqOn && (lowSet || highSet))
        {
            juce::Path removed (curve);
            removed.lineTo (plot.getRight(), y0);
            removed.lineTo (plot.getX(), y0);
            removed.closeSubPath();
            g.setGradientFill (juce::ColourGradient (ui::amber.withAlpha (0.02f), 0.0f, y0, ui::amber.withAlpha (0.16f), 0.0f, plot.getBottom(), false));
            g.fillPath (removed);
        }

        // what stays: gradient under the curve
        juce::Path fill (curve);
        fill.lineTo (plot.getRight(), plot.getBottom());
        fill.lineTo (plot.getX(), plot.getBottom());
        fill.closeSubPath();
        const auto lineCol = eqOn ? juce::Colour (0xff9a82ff) : juce::Colour (0xff5d5d6e);
        g.setGradientFill (juce::ColourGradient (lineCol.withAlpha (eqOn ? 0.34f : 0.12f), 0.0f, y0,
                                                 lineCol.withAlpha (0.02f), 0.0f, plot.getBottom(), false));
        g.fillPath (fill);

        if (eqOn)
        {
            g.setColour (ui::purple.withAlpha (0.20f));
            g.strokePath (curve, juce::PathStrokeType (6.0f));
        }
        g.setColour (lineCol);
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
        const bool s = slopeRect.contains (e.position), t = eqRect.contains (e.position);
        if (s != slopeHover || t != eqHover) { slopeHover = s; eqHover = t; repaint(); }
    }
    void mouseExit (const juce::MouseEvent&) override
    {
        if (slopeHover || eqHover) { slopeHover = eqHover = false; repaint(); }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        dragActive = false;
        if (eqRect.contains (e.position))
        {
            setBool (eqP, ! eqIsOn());
            return;
        }
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
        if (! eqIsOn())
            setBool (eqP, true); // touching a handle switches the EQ back on
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
    static void setBool (juce::RangedAudioParameter* p, bool v)
    {
        if (p == nullptr) return;
        p->beginChangeGesture();
        p->setValueNotifyingHost (v ? 1.0f : 0.0f);
        p->endChangeGesture();
    }
    void updatePlot()
    {
        auto full = getLocalBounds().toFloat();
        plot = full.reduced (16.0f, 10.0f);
        head = plot.removeFromTop (16.0f);
        plot.removeFromTop (4.0f);
        eqRect = juce::Rectangle<float> (head.getX(), head.getY(), 46.0f, head.getHeight());
        slopeRect = juce::Rectangle<float> (head.getCentreX() - 70.0f, head.getY(), 140.0f, head.getHeight());
    }
    double lowHz() const { return (double) apvts.getRawParameterValue ("lowCut")->load(); }
    double highHz() const { return (double) apvts.getRawParameterValue ("highCut")->load(); }
    bool eqIsOn() const { return apvts.getRawParameterValue ("eqOn")->load() > 0.5f; }
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

    juce::AudioProcessorValueTreeState& apvts;
    juce::RangedAudioParameter* lowP = nullptr;
    juce::RangedAudioParameter* highP = nullptr;
    juce::RangedAudioParameter* slopeP = nullptr;
    juce::RangedAudioParameter* eqP = nullptr;
    juce::Rectangle<float> plot, head, slopeRect, eqRect;
    bool dragLow = true, dragActive = false, slopeHover = false, eqHover = false, lastOn = true;
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

// effect pill: grows when the mouse is over it (the power icon grows even more)
class FxButton : public juce::Component, public juce::SettableTooltipClient, private juce::Timer
{
public:
    FxButton (juce::RangedAudioParameter* p, const juce::String& n, std::function<void (juce::uint32)> onName)
        : par (p), name (n), openPopup (std::move (onName))
    {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
        lastOn = isOn();
    }
    ~FxButton() override { stopTimer(); }

    bool isOn() const { return par != nullptr && par->getValue() > 0.5f; }
    void setOn (bool on)
    {
        if (par == nullptr) return;
        par->beginChangeGesture();
        par->setValueNotifyingHost (on ? 1.0f : 0.0f);
        par->endChangeGesture();
    }
    void refresh()
    {
        const bool on = isOn();
        if (on != lastOn) { lastOn = on; repaint(); }
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat().reduced (7.0f);
        const bool on = isOn();
        const auto col = on ? ui::amber : (hover ? juce::Colour (0xffb0b0b8) : ui::dim);
        const float rr = b.getHeight() * 0.5f;

        juce::Graphics::ScopedSaveState outer (g);
        g.addTransform (juce::AffineTransform::scale (scale, scale, b.getCentreX(), b.getCentreY()));
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
            g.setColour (hover ? juce::Colour (0xff3a3a42) : juce::Colour (0xff2c2c32));
            g.drawRoundedRectangle (b, rr, 1.0f);
        }
        const float cx = b.getX() + 20.0f, cy = b.getCentreY();
        {
            juce::Graphics::ScopedSaveState inner (g);
            g.addTransform (juce::AffineTransform::scale (iconScale, iconScale, cx, cy));
            if (overIcon)
            {
                g.setColour (col.withAlpha (0.18f));
                g.fillEllipse (cx - 11.0f, cy - 10.0f, 22.0f, 22.0f);
            }
            juce::Path arc;
            arc.addCentredArc (cx, cy + 1.0f, 6.0f, 6.0f, 0.0f, 0.6f, juce::MathConstants<float>::twoPi - 0.6f, true);
            g.setColour (col);
            g.strokePath (arc, juce::PathStrokeType (1.7f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.drawLine (cx, cy - 7.0f, cx, cy, 1.7f);
        }
        g.setColour (col);
        g.setFont (ui::font (13.0f, true).withExtraKerningFactor (0.12f));
        g.drawText (name, juce::Rectangle<float> (b.getX() + 36.0f, b.getY(), b.getWidth() - 40.0f, b.getHeight()),
                    juce::Justification::centredLeft, false);
    }

    void mouseEnter (const juce::MouseEvent& e) override
    {
        hover = true;
        overIcon = e.x < 43;
        kick();
    }
    void mouseMove (const juce::MouseEvent& e) override
    {
        const bool oi = e.x < 43;
        if (oi != overIcon) { overIcon = oi; kick(); }
    }
    void mouseExit (const juce::MouseEvent&) override
    {
        hover = overIcon = false;
        kick();
    }
    void mouseDown (const juce::MouseEvent&) override { downTime = juce::Time::getMillisecondCounter(); }
    void mouseUp (const juce::MouseEvent& e) override
    {
        if (! getLocalBounds().contains (e.getPosition())) return;
        if (e.x < 43)
            setOn (! isOn());
        else if (openPopup)
            openPopup (downTime);
        repaint();
    }

private:
    void kick() { if (! isTimerRunning()) startTimerHz (60); }
    void timerCallback() override
    {
        const float ts = hover ? 1.07f : 1.0f, ti = overIcon ? 1.32f : 1.0f;
        scale += (ts - scale) * 0.35f;
        iconScale += (ti - iconScale) * 0.35f;
        if (std::abs (ts - scale) < 0.003f && std::abs (ti - iconScale) < 0.003f)
        {
            scale = ts;
            iconScale = ti;
            stopTimer();
        }
        repaint();
    }

    juce::RangedAudioParameter* par;
    juce::String name;
    std::function<void (juce::uint32)> openPopup;
    juce::uint32 downTime = 0;
    bool hover = false, overIcon = false, lastOn = false;
    float scale = 1.0f, iconScale = 1.0f;
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

    Backdrop backdrop;
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
    LabelButton rawBtn { "RAW", LabelButton::None, ui::ice };
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
    int lastVer = -1, lastRecState = 0, lastCaptureId = 0;

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
    juce::TooltipWindow tooltipWindow { this, 700 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RiserEditor)
};
