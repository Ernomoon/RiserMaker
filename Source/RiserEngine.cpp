#include "RiserEngine.h"

namespace riser
{
juce::AudioBuffer<float> resampleBuffer (const juce::AudioBuffer<float>& in, double fromSr, double toSr)
{
    juce::AudioBuffer<float> out;
    const int inLen = in.getNumSamples();
    if (inLen < 2 || in.getNumChannels() < 1)
    {
        out.makeCopyOf (in);
        return out;
    }
    if (std::abs (fromSr - toSr) < 1.0)
    {
        out.makeCopyOf (in);
        return out;
    }
    const int outLen = (int) std::ceil ((double) inLen * toSr / fromSr);
    out.setSize (in.getNumChannels(), outLen);
    const double step = fromSr / toSr;
    for (int ch = 0; ch < in.getNumChannels(); ++ch)
    {
        const float* s = in.getReadPointer (ch);
        float* d = out.getWritePointer (ch);
        for (int i = 0; i < outLen; ++i)
        {
            const double pos = (double) i * step;
            int i0 = (int) pos;
            const float fr = (float) (pos - i0);
            i0 = juce::jmin (i0, inLen - 1);
            const int i1 = juce::jmin (i0 + 1, inLen - 1);
            d[i] = s[i0] * (1.0f - fr) + s[i1] * fr;
        }
    }
    return out;
}

static void deEss (juce::AudioBuffer<float>& b, double sr, int len)
{
    for (int ch = 0; ch < b.getNumChannels(); ++ch)
    {
        float* d = b.getWritePointer (ch);
        Biquad hp;
        hp.c = BiquadCoefs::make (true, sr, 5500.0, 0.7071);
        float eHi = 0.0f, eAll = 0.0f, gain = 1.0f;
        const float atk = (float) std::exp (-1.0 / (0.002 * sr));
        const float rel = (float) std::exp (-1.0 / (0.030 * sr));
        for (int i = 0; i < len; ++i)
        {
            const float x = d[i];
            const float h = hp.process (x);
            eHi += 0.01f * (std::abs (h) - eHi);
            eAll += 0.01f * (std::abs (x) - eAll);
            const bool sib = eAll > 0.003f && (eHi / (eAll + 1.0e-9f)) > 0.55f;
            const float target = sib ? 0.2f : 1.0f;
            if (target < gain) gain = target + (gain - target) * atk;
            else               gain = target + (gain - target) * rel;
            d[i] = x - h * (1.0f - gain);
        }
    }
}

juce::AudioBuffer<float> buildBase (const juce::AudioBuffer<float>& source, double sr, double lengthSec, bool vox)
{
    juce::AudioBuffer<float> work;
    const int n = (int) std::lround (lengthSec * sr);
    const int srcLen = source.getNumSamples();
    if (srcLen < 64 || n < 256 || source.getNumChannels() < 1)
        return work;

    work.setSize (2, n);
    work.clear();

    // excerpt = the beginning of the captured phrase
    const int exLen = juce::jmin (n, srcLen, (int) (juce::jlimit (0.4, 4.0, lengthSec * 0.6) * sr));
    for (int ch = 0; ch < 2; ++ch)
        work.copyFrom (ch, 0, source, juce::jmin (ch, source.getNumChannels() - 1), 0, exLen);

    const int fin = juce::jmin (exLen / 2, (int) (0.003 * sr));
    const int fout = juce::jmin (exLen / 2, (int) (0.03 * sr));
    for (int ch = 0; ch < 2; ++ch)
    {
        float* d = work.getWritePointer (ch);
        for (int i = 0; i < fin; ++i)  d[i] *= (float) i / (float) fin;
        for (int i = 0; i < fout; ++i) d[exLen - 1 - i] *= (float) i / (float) fout;
    }

    if (vox)
        deEss (work, sr, exLen);

    // long reverb, wet only
    juce::Reverb rv;
    juce::Reverb::Parameters rp;
    rp.roomSize = 1.0f;
    rp.damping = 0.25f;
    rp.wetLevel = 1.0f;
    rp.dryLevel = 0.0f;
    rp.width = 1.0f;
    rp.freezeMode = 0.0f;
    rv.setParameters (rp);
    rv.setSampleRate (sr);
    rv.reset();
    rv.processStereo (work.getWritePointer (0), work.getWritePointer (1), n);

    float* l = work.getWritePointer (0);
    float* r = work.getWritePointer (1);

    // trim the reverb pre-delay so the swell peaks right at the end
    const int scan = juce::jmin (n, (int) (0.5 * sr));
    float peak0 = 0.0f;
    for (int i = 0; i < scan; ++i)
        peak0 = juce::jmax (peak0, std::abs (l[i]), std::abs (r[i]));
    if (peak0 < 1.0e-9f)
    {
        work.clear();
        return work;
    }
    int lead = 0;
    for (int i = 0; i < scan; ++i)
        if (std::abs (l[i]) > 0.03f * peak0 || std::abs (r[i]) > 0.03f * peak0) { lead = i; break; }
    if (lead > 0 && lead < n)
    {
        std::memmove (l, l + lead, sizeof (float) * (size_t) (n - lead));
        std::memmove (r, r + lead, sizeof (float) * (size_t) (n - lead));
        std::memset (l + (n - lead), 0, sizeof (float) * (size_t) lead);
        std::memset (r + (n - lead), 0, sizeof (float) * (size_t) lead);
    }

    // force the tail to decay over exactly lengthSec (-43 dB at the end)
    const int bs = juce::jmax (64, (int) (0.02 * sr));
    const int nb = n / bs + 1;
    std::vector<float> e ((size_t) nb, 0.0f), g ((size_t) nb, 1.0f);
    for (int k = 0; k < nb; ++k)
    {
        const int s = k * bs, t = juce::jmin (n, s + bs);
        double acc = 0.0;
        for (int i = s; i < t; ++i)
            acc += (double) l[i] * l[i] + (double) r[i] * r[i];
        e[(size_t) k] = (float) std::sqrt (acc / (2.0 * juce::jmax (1, t - s)));
    }
    float ref = 0.0f;
    for (int k = 0; k < nb && (double) k * bs < 0.25 * sr; ++k)
        ref = juce::jmax (ref, e[(size_t) k]);
    if (ref < 1.0e-12f)
        ref = 1.0e-12f;
    for (int k = 0; k < nb; ++k)
    {
        const double tsec = ((double) k + 0.5) * bs / sr;
        const float d = ref * (float) std::exp (-5.0 * tsec / lengthSec);
        g[(size_t) k] = juce::jlimit (0.0f, 8.0f, d / (e[(size_t) k] + ref * 1.0e-4f));
    }
    // the last blocks only contain the trimmed (silent) end: hold the last valid gain there
    const int validBlocks = juce::jmax (1, (n - lead) / bs);
    for (int k = validBlocks; k < nb; ++k)
        g[(size_t) k] = g[(size_t) (validBlocks - 1)];
    for (int pass = 0; pass < 2; ++pass)
    {
        std::vector<float> c = g;
        for (int k = 0; k < nb; ++k)
        {
            const float a = c[(size_t) juce::jmax (0, k - 1)];
            const float bb = c[(size_t) k];
            const float cc = c[(size_t) juce::jmin (nb - 1, k + 1)];
            g[(size_t) k] = 0.25f * a + 0.5f * bb + 0.25f * cc;
        }
    }
    for (int i = 0; i < n; ++i)
    {
        const double pos = ((double) i + 0.5) / bs - 0.5;
        const int k0 = juce::jlimit (0, nb - 1, (int) std::floor (pos));
        const int k1 = juce::jmin (nb - 1, k0 + 1);
        const float fr = (float) juce::jlimit (0.0, 1.0, pos - k0);
        const float gain = g[(size_t) k0] * (1.0f - fr) + g[(size_t) k1] * fr;
        l[i] *= gain;
        r[i] *= gain;
    }

    // the magic step: reverse
    work.reverse (0, 0, n);
    work.reverse (1, 0, n);

    // fades + normalise to -6 dBFS
    const int fi = juce::jmin (n / 4, (int) (0.02 * sr));
    const int fo = juce::jmin (n / 4, (int) (0.002 * sr));
    float pk = 0.0f;
    for (int ch = 0; ch < 2; ++ch)
    {
        float* d = work.getWritePointer (ch);
        for (int i = 0; i < fi; ++i) d[i] *= (float) i / (float) juce::jmax (1, fi);
        for (int i = 0; i < fo; ++i) d[n - 1 - i] *= (float) i / (float) juce::jmax (1, fo);
        for (int i = 0; i < n; ++i) pk = juce::jmax (pk, std::abs (d[i]));
    }
    if (pk > 1.0e-9f)
        work.applyGain (0.5f / pk);
    return work;
}

void applyFx (juce::AudioBuffer<float>& b, double sr, const FxParams& p)
{
    const int n = b.getNumSamples();
    if (n < 2)
        return;
    const int nc = juce::jmin (2, b.getNumChannels());

    // PHONE
    if (p.phone)
    {
        for (int ch = 0; ch < nc; ++ch)
        {
            float* d = b.getWritePointer (ch);
            CutFilter hp, lp;
            hp.setup (true, sr, 450.0);
            lp.setup (false, sr, 3400.0);
            for (int i = 0; i < n; ++i)
            {
                const float x = d[i];
                float w = lp.process (hp.process (x));
                w = std::tanh (w * 2.5f) * 1.2f;
                d[i] = x * (1.0f - p.phoneMix) + w * p.phoneMix;
            }
        }
    }

    // CHORUS (2 modulated voices per channel)
    if (p.chorus)
    {
        const int dl = (int) (sr * 0.06) + 8;
        for (int ch = 0; ch < nc; ++ch)
        {
            float* d = b.getWritePointer (ch);
            std::vector<float> line ((size_t) dl, 0.0f);
            int w = 0;
            double ph0 = ch * 0.25, ph1 = ch * 0.25 + 0.5;
            const double inc = (double) p.chorusRate / sr;
            for (int i = 0; i < n; ++i)
            {
                const float x = d[i];
                line[(size_t) w] = x;
                float acc = 0.0f;
                for (int v = 0; v < 2; ++v)
                {
                    const double ph = (v == 0 ? ph0 : ph1);
                    const double del = (0.014 + 0.0045 * std::sin (2.0 * kPi * ph) + 0.003 * v) * sr;
                    double rp = (double) w - del;
                    if (rp < 0.0) rp += dl;
                    const int i0 = juce::jlimit (0, dl - 1, (int) rp);
                    const int i1 = (i0 + 1) % dl;
                    const float fr = (float) (rp - i0);
                    acc += line[(size_t) i0] * (1.0f - fr) + line[(size_t) i1] * fr;
                }
                const float wet = acc * 0.5f;
                d[i] = (x + p.chorusMix * wet) / (1.0f + 0.5f * p.chorusMix);
                w = (w + 1) % dl;
                ph0 += inc; ph1 += inc;
                if (ph0 > 1.0) ph0 -= 1.0;
                if (ph1 > 1.0) ph1 -= 1.0;
            }
        }
    }

    // DRIVE
    if (p.drive)
    {
        const float gain = 1.0f + 30.0f * p.driveAmt * p.driveAmt;
        for (int ch = 0; ch < nc; ++ch)
        {
            float* d = b.getWritePointer (ch);
            for (int i = 0; i < n; ++i)
            {
                const float x = d[i];
                const float w = std::tanh (gain * x) * 0.6f;
                d[i] = x * (1.0f - p.driveMix) + w * p.driveMix;
            }
        }
    }

    // TREMOLO
    if (p.trem)
    {
        const double inc = (double) p.tremRate / sr;
        for (int ch = 0; ch < nc; ++ch)
        {
            float* d = b.getWritePointer (ch);
            double ph = 0.0;
            for (int i = 0; i < n; ++i)
            {
                const float m = 1.0f - p.tremMix * 0.5f * (1.0f - (float) std::cos (2.0 * kPi * ph));
                d[i] *= m;
                ph += inc;
                if (ph > 1.0) ph -= 1.0;
            }
        }
    }

    // LOW / HIGH CUT
    const bool useLow = p.lowCut > 25.0f;
    const bool useHigh = p.highCut < 19000.0f;
    if (useLow || useHigh)
    {
        for (int ch = 0; ch < nc; ++ch)
        {
            float* d = b.getWritePointer (ch);
            CutFilter hp, lp;
            hp.setup (true, sr, p.lowCut);
            lp.setup (false, sr, p.highCut);
            for (int i = 0; i < n; ++i)
            {
                float x = d[i];
                if (useLow)  x = hp.process (x);
                if (useHigh) x = lp.process (x);
                d[i] = x;
            }
        }
    }

    // fades + normalise to -3 dBFS
    const int fi = juce::jmin (n / 4, (int) (0.01 * sr));
    const int fo = juce::jmin (n / 4, (int) (0.003 * sr));
    float pk = 0.0f;
    for (int ch = 0; ch < nc; ++ch)
    {
        float* d = b.getWritePointer (ch);
        for (int i = 0; i < fi; ++i) d[i] *= (float) i / (float) juce::jmax (1, fi);
        for (int i = 0; i < fo; ++i) d[n - 1 - i] *= (float) i / (float) juce::jmax (1, fo);
        for (int i = 0; i < n; ++i) pk = juce::jmax (pk, std::abs (d[i]));
    }
    if (pk > 1.0e-9f)
        b.applyGain (0.708f / pk);
}

std::vector<std::pair<int, int>> detectSibilance (const juce::AudioBuffer<float>& src, double sr)
{
    std::vector<std::pair<int, int>> out;
    const int n = src.getNumSamples();
    if (n < 512 || src.getNumChannels() < 1)
        return out;

    Biquad hp;
    hp.c = BiquadCoefs::make (true, sr, 5500.0, 0.7071);
    const int hop = juce::jmax (32, (int) (sr * 0.004));
    const float* a = src.getReadPointer (0);
    const float* bch = src.getReadPointer (juce::jmin (1, src.getNumChannels() - 1));
    float eHi = 0.0f, eAll = 0.0f;
    std::vector<char> flags;
    flags.reserve ((size_t) (n / hop + 1));
    for (int i = 0; i < n; ++i)
    {
        const float x = 0.5f * (a[i] + bch[i]);
        const float h = hp.process (x);
        eHi += 0.01f * (std::abs (h) - eHi);
        eAll += 0.01f * (std::abs (x) - eAll);
        if ((i % hop) == hop - 1)
            flags.push_back ((char) (eAll > 0.004f && (eHi / (eAll + 1.0e-9f)) > 0.55f));
    }

    std::vector<std::pair<int, int>> runs;
    const int F = (int) flags.size();
    int s = -1;
    for (int k = 0; k <= F; ++k)
    {
        const bool f = (k < F) && flags[(size_t) k] != 0;
        if (f && s < 0) s = k;
        if (! f && s >= 0) { runs.push_back ({ s, k }); s = -1; }
    }
    std::vector<std::pair<int, int>> merged;
    for (auto& r : runs)
    {
        if (! merged.empty() && r.first - merged.back().second <= 3)
            merged.back().second = r.second;
        else
            merged.push_back (r);
    }
    for (auto& m : merged)
        if (m.second - m.first >= 3)
            out.push_back ({ m.first * hop, m.second * hop });
    return out;
}

std::vector<float> computePeaks (const juce::AudioBuffer<float>& buf, int bins)
{
    std::vector<float> p ((size_t) juce::jmax (1, bins), 0.0f);
    const int n = buf.getNumSamples();
    if (n == 0)
        return p;
    float gmax = 0.0f;
    for (int b = 0; b < bins; ++b)
    {
        const int s = (int) ((juce::int64) b * n / bins);
        const int e = juce::jmax (s + 1, (int) ((juce::int64) (b + 1) * n / bins));
        float m = 0.0f;
        for (int ch = 0; ch < buf.getNumChannels(); ++ch)
        {
            const float* d = buf.getReadPointer (ch);
            for (int i = s; i < e && i < n; ++i)
                m = juce::jmax (m, std::abs (d[i]));
        }
        p[(size_t) b] = m;
        gmax = juce::jmax (gmax, m);
    }
    if (gmax > 1.0e-9f)
        for (auto& v : p) v /= gmax;
    return p;
}
}
