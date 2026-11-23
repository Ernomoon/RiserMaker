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

// 4th-order Butterworth high/low-pass (24 dB/oct)
struct CutFilter
{
    Biquad s[2];

    void setup (bool highpass, double sr, double freq)
    {
        s[0].c = BiquadCoefs::make (highpass, sr, freq, 0.5412);
        s[1].c = BiquadCoefs::make (highpass, sr, freq, 1.3066);
    }
    float process (float x) { return s[1].process (s[0].process (x)); }
};

inline double cutMagnitude (bool highpass, double freq, double cutFreq, double sr)
{
    return BiquadCoefs::make (highpass, sr, cutFreq, 0.5412).magnitude (freq, sr)
         * BiquadCoefs::make (highpass, sr, cutFreq, 1.3066).magnitude (freq, sr);
}

// ---------- engine ----------
struct FxParams
{
    bool phone = false, chorus = false, drive = false, trem = false;
    float phoneMix = 1.0f;
    float chorusMix = 0.5f, chorusRate = 0.8f;
    float driveAmt = 0.7f, driveMix = 0.5f;
    float tremRate = 5.0f, tremMix = 1.0f;
    float lowCut = 20.0f, highCut = 20000.0f;
};

juce::AudioBuffer<float> resampleBuffer (const juce::AudioBuffer<float>& in, double fromSr, double toSr);

// Builds the raw "reverse reverb" swell (length = lengthSec, ends exactly on the vocal onset).
juce::AudioBuffer<float> buildBase (const juce::AudioBuffer<float>& source, double sr, double lengthSec, bool vox);

// Applies the effect chain + cut filters + final fades/normalisation (in place).
void applyFx (juce::AudioBuffer<float>& buf, double sr, const FxParams& p);

// Sibilance regions in samples.
std::vector<std::pair<int, int>> detectSibilance (const juce::AudioBuffer<float>& src, double sr);

// Normalised (0..1) peak bins for drawing.
std::vector<float> computePeaks (const juce::AudioBuffer<float>& buf, int bins);
}
