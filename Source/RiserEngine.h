#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <complex>
#include <vector>
#include <utility>

namespace riser
{
constexpr double kPi = 3.14159265358979323846;

// ---------- filters (shared by the DSP and by the EQ drawing) ----------
struct BiquadCoefs
{
    double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;

    static BiquadCoefs make (bool highpass, double sr, double freq, double q)
    {
        freq = juce::jlimit (5.0, sr * 0.49, freq);
        const double w0 = 2.0 * kPi * freq / sr;
        const double c = std::cos (w0), s = std::sin (w0);
        const double alpha = s / (2.0 * q);
        const double a0 = 1.0 + alpha;
        BiquadCoefs k;
        if (highpass)
        {
            k.b0 = (1.0 + c) * 0.5 / a0;
            k.b1 = -(1.0 + c) / a0;
            k.b2 = k.b0;
        }
        else
        {
            k.b0 = (1.0 - c) * 0.5 / a0;
            k.b1 = (1.0 - c) / a0;
            k.b2 = k.b0;
        }
        k.a1 = -2.0 * c / a0;
        k.a2 = (1.0 - alpha) / a0;
        return k;
    }

    double magnitude (double freq, double sr) const
    {
        const double w = 2.0 * kPi * freq / sr;
        const std::complex<double> z1 = std::polar (1.0, -w), z2 = std::polar (1.0, -2.0 * w);
        const std::complex<double> num = b0 + b1 * z1 + b2 * z2;
        const std::complex<double> den = 1.0 + a1 * z1 + a2 * z2;
        return std::abs (num / den);
    }
};

struct Biquad
{
    BiquadCoefs c;
    double z1 = 0, z2 = 0;

    float process (float x)
    {
        const double in = x;
        const double y = c.b0 * in + z1;
        z1 = c.b1 * in - c.a1 * y + z2;
        z2 = c.b2 * in - c.a2 * y;
        return (float) y;
    }
};

// Butterworth high/low-pass, order 2 / 4 / 8  (12 / 24 / 48 dB per octave)
inline const double* butterQ (int order, int& count)
{
    static const double q2[] = { 0.7071 };
    static const double q4[] = { 0.5412, 1.3066 };
    static const double q8[] = { 0.5098, 0.6013, 0.9000, 2.5629 };
    if (order >= 8) { count = 4; return q8; }
    if (order >= 4) { count = 2; return q4; }
    count = 1;
    return q2;
}

struct CutFilter
{
    Biquad s[4];
    int count = 2;

    void setup (bool highpass, double sr, double freq, int order = 4)
    {
        const double* q = butterQ (order, count);
        for (int i = 0; i < count; ++i)
        {
            s[i].c = BiquadCoefs::make (highpass, sr, freq, q[i]);
            s[i].z1 = s[i].z2 = 0.0;
        }
    }
    float process (float x)
    {
        for (int i = 0; i < count; ++i)
            x = s[i].process (x);
        return x;
    }
};

inline double cutMagnitude (bool highpass, double freq, double cutFreq, double sr, int order = 4)
{
    int count = 0;
    const double* q = butterQ (order, count);
    double m = 1.0;
    for (int i = 0; i < count; ++i)
        m *= BiquadCoefs::make (highpass, sr, cutFreq, q[i]).magnitude (freq, sr);
    return m;
}

// ---------- engine ----------
struct FxParams
{
    bool phone = false, chorus = false, drive = false, trem = false;
    float phoneMix = 1.0f;
    float chorusMix = 0.6f, chorusRate = 1.0f, chorusDepth = 0.6f;
    float driveAmt = 0.7f, driveMix = 0.5f;
    float tremRate = 5.0f, tremMix = 1.0f;
    float lowCut = 20.0f, highCut = 20000.0f;
    int cutOrder = 8;
};

juce::AudioBuffer<float> resampleBuffer (const juce::AudioBuffer<float>& in, double fromSr, double toSr);

// Which part of the captured audio is used to excite the reverb.
struct Excerpt { int start = 0; int length = 0; };

// Removes leading silence / noise so the phrase starts at sample 0.
void trimLeading (juce::AudioBuffer<float>& buf, double sr);

// Smart element picking: skips breaths, noise and a leading "sss"; for instrumentals locks on the first strong hit.
Excerpt chooseExcerpt (const juce::AudioBuffer<float>& src, double sr, double lengthSec, bool vox);

// Builds the raw "reverse reverb" swell (length = lengthSec, ends exactly where the chosen element starts).
juce::AudioBuffer<float> buildBase (const juce::AudioBuffer<float>& source, double sr, double lengthSec, bool vox, const Excerpt& ex);

// Applies the effect chain + cut filters + final fades/normalisation (in place).
void applyFx (juce::AudioBuffer<float>& buf, double sr, const FxParams& p);

// Sibilance regions in samples.
std::vector<std::pair<int, int>> detectSibilance (const juce::AudioBuffer<float>& src, double sr);

// Normalised (0..1) peak bins for drawing.
std::vector<float> computePeaks (const juce::AudioBuffer<float>& buf, int bins);
}
