#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
const char* const kRenderIds[17] = { "length", "vox", "lowCut", "highCut", "phone", "chorus", "drive", "tremolo",
                                     "phoneMix", "chorusMix", "chorusRate", "driveAmt", "driveMix", "tremRate", "tremMix",
                                     "chorusDepth", "cutSlope" };
const int kCutOrders[3] = { 2, 4, 8 };
const double kLenMult[6] = { 0.25, 0.5, 1.0, 2.0, 4.0, 8.0 };
const char* const kLenName[6] = { "quarter", "half", "1", "2", "4", "8" };

struct FactoryPreset
{
    const char* name;
    std::vector<std::pair<const char*, float>> values;
};

const std::vector<FactoryPreset>& factoryPresets()
{
    static const std::vector<FactoryPreset> p = {
        { "Default", {} },
        { "Vocal Wash",    { { "chorus", 1.f }, { "chorusMix", 0.5f }, { "highCut", 12000.f } } },
        { "Dark Riser",    { { "lowCut", 120.f }, { "highCut", 3500.f }, { "drive", 1.f }, { "driveAmt", 0.5f }, { "driveMix", 0.4f } } },
        { "Telephone Build", { { "phone", 1.f }, { "tremolo", 1.f }, { "tremRate", 3.f }, { "tremMix", 0.7f } } },
        { "Wide Shimmer",  { { "chorus", 1.f }, { "chorusMix", 0.8f }, { "chorusRate", 1.2f }, { "lowCut", 200.f }, { "highCut", 14000.f } } },
        { "Hard Drive",    { { "drive", 1.f }, { "driveAmt", 0.85f }, { "driveMix", 0.7f }, { "tremolo", 1.f }, { "tremRate", 5.f }, { "tremMix", 0.6f } } },
        { "Short Snap (1/2 bar)", { { "length", 1.f }, { "chorus", 1.f }, { "chorusMix", 0.4f } } },
        { "Long Epic (8 bars)",   { { "length", 5.f }, { "lowCut", 150.f }, { "highCut", 9000.f } } },
    };
    return p;
}
} // namespace

juce::AudioProcessorValueTreeState::ParameterLayout RiserProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout l;

    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "length", 1 }, "Reverse Length",
                                                   StringArray { "1/4 Bar", "1/2 Bar", "1 Bar", "2 Bars", "4 Bars", "8 Bars" }, 4));
    l.add (std::make_unique<AudioParameterBool> (ParameterID { "vox", 1 }, "Vox Mode", true));
    l.add (std::make_unique<AudioParameterBool> (ParameterID { "loop", 1 }, "Loop", false));
    l.add (std::make_unique<AudioParameterBool> (ParameterID { "autoRec", 1 }, "Auto Rec", false));
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "cutSlope", 1 }, "Cut Slope",
                                                   StringArray { "12 dB/oct", "24 dB/oct", "48 dB/oct" }, 2));

    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "lowCut", 1 }, "Low Cut",
                                                  NormalisableRange<float> (20.0f, 5000.0f, 0.0f, 0.3f), 20.0f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "highCut", 1 }, "High Cut",
                                                  NormalisableRange<float> (500.0f, 20000.0f, 0.0f, 0.3f), 20000.0f));

    l.add (std::make_unique<AudioParameterBool> (ParameterID { "phone", 1 }, "Phone", false));
    l.add (std::make_unique<AudioParameterBool> (ParameterID { "chorus", 1 }, "Chorus", false));
    l.add (std::make_unique<AudioParameterBool> (ParameterID { "drive", 1 }, "Drive", false));
    l.add (std::make_unique<AudioParameterBool> (ParameterID { "tremolo", 1 }, "Tremolo", false));

    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "phoneMix", 1 }, "Phone Mix", NormalisableRange<float> (0.0f, 1.0f), 1.0f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "chorusMix", 1 }, "Chorus Mix", NormalisableRange<float> (0.0f, 1.0f), 0.6f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "chorusDepth", 1 }, "Chorus Depth", NormalisableRange<float> (0.0f, 1.0f), 0.6f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "chorusRate", 1 }, "Chorus Rate", NormalisableRange<float> (0.1f, 6.0f, 0.0f, 0.5f), 1.0f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "driveAmt", 1 }, "Drive Amount", NormalisableRange<float> (0.0f, 1.0f), 0.7f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "driveMix", 1 }, "Drive Mix", NormalisableRange<float> (0.0f, 1.0f), 0.5f));
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "tremRate", 1 }, "Tremolo Rate", getTremDivisionNames(), 3));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "tremMix", 1 }, "Tremolo Mix", NormalisableRange<float> (0.0f, 1.0f), 1.0f));
    return l;
}

juce::StringArray RiserProcessor::getTremDivisionNames()
{
    return juce::StringArray { "1/2", "1/4", "1/4T", "1/8", "1/8T", "1/16", "1/16T", "1/32" };
}

double RiserProcessor::getTremDivision (int index)
{
    static const double d[8] = { 2.0, 4.0, 6.0, 8.0, 12.0, 16.0, 24.0, 32.0 };
    return d[juce::jlimit (0, 7, index)];
}

RiserProcessor::RiserProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    for (auto& v : lastVals)
        v = -1.0e9f;
    startTimerHz (30);
}

RiserProcessor::~RiserProcessor()
{
    stopTimer();
}

bool RiserProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& o = layouts.getMainOutputChannelSet();
    if (o != juce::AudioChannelSet::mono() && o != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == o;
}

void RiserProcessor::prepareToPlay (double sampleRate, int)
{
    if (std::abs (hostSR.load() - sampleRate) > 0.5)
        renderDirty = true;
    hostSR = sampleRate;
    const int cap = (int) (sampleRate * 20.0);
    for (auto& b : capBuf)
        b.setSize (2, cap);
    capRecording = false;
    capWritePos = capSilence = 0;
    wasPlaying = true;
}

void RiserProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    const int numCh = buffer.getNumChannels();
    if (numCh < 1 || n < 1)
        return;

    bool playing = true;
    if (auto* ph = getPlayHead())
    {
        if (auto pos = ph->getPosition())
        {
            playing = pos->getIsPlaying();
            if (auto bpm = pos->getBpm())
            {
                const double b = *bpm;
                if (b > 20.0 && b < 400.0 && std::abs (b - hostBpm.load()) > 0.001)
                {
                    hostBpm = b;
                    renderDirty = true;
                }
            }
            if (auto ts = pos->getTimeSignature())
            {
                const double bars = (double) ts->numerator * 4.0 / (double) juce::jmax (1, ts->denominator);
                if (bars > 0.5 && std::abs (bars - hostBeatsPerBar.load()) > 0.001)
                {
                    hostBeatsPerBar = bars;
                    renderDirty = true;
                }
            }
        }
    }

    // ---------- capture the incoming audio ----------
    float peak = 0.0f;
    for (int ch = 0; ch < numCh; ++ch)
        peak = juce::jmax (peak, buffer.getMagnitude (ch, 0, n));

    const double sr = hostSR.load();
    const int capSize = capBuf[0].getNumSamples();
    const bool manual = manualRec.load();
    auto finish = [&] (int len)
    {
        if (len > (int) (0.2 * sr))
        {
            handoffIndex = capActive;
            handoffLength = len;
            handoffReady = true;
            capActive ^= 1;
        }
        capRecording = false;
        capManualActive = false;
        capWritePos = 0;
        capSilence = 0;
    };

    if (capSize > 0)
    {
        const float thr = 0.004f; // about -48 dBFS

        // REC button edges
        if (manual && ! prevManual)
        {
            capRecording = true;
            capManualActive = true;
            capWritePos = 0;
            capSilence = 0;
        }
        else if (! manual && prevManual && capRecording && capManualActive)
            finish (capWritePos);
        prevManual = manual;

        if (! capRecording && ! manual && getP ("autoRec") > 0.5f && peak > thr)
        {
            capRecording = true;
            capManualActive = false;
            capWritePos = 0;
            capSilence = 0;
        }

        if (capRecording)
        {
            auto& cb = capBuf[capActive];
            const int w = juce::jmin (n, capSize - capWritePos);
            for (int ch = 0; ch < 2; ++ch)
                cb.copyFrom (ch, capWritePos, buffer, juce::jmin (ch, numCh - 1), 0, w);
            capWritePos += w;

            if (capManualActive)
            {
                if (capWritePos >= capSize) // buffer full: stop by itself
                {
                    manualRec = false;
                    prevManual = false;
                    finish (capWritePos);
                }
            }
            else
            {
                capSilence = (peak < thr) ? capSilence + n : 0;
                const bool stoppedNow = wasPlaying && ! playing;
                if (capSilence > (int) (0.6 * sr) || stoppedNow || capWritePos >= capSize)
                    finish (juce::jmin (capWritePos, juce::jmax (0, capWritePos - capSilence) + (int) (0.05 * sr)));
            }
        }
        recState = capRecording ? (capManualActive ? 2 : 1) : 0;
        recSamples = capWritePos;
    }
    wasPlaying = playing;

    // ---------- preview playback (LISTEN) ----------
    if (listenRequest.exchange (false))
    {
        listenPos = 0;
        listening = true;
    }
    if (listening.load())
    {
        const juce::ScopedTryLock sl (riserLock);
        if (sl.isLocked())
        {
            const int len = riser.getNumSamples();
            if (len > 0)
            {
                const bool loop = getP ("loop") > 0.5f;
                int pos = listenPos.load();
                const float* rl = riser.getReadPointer (0);
                const float* rr = riser.getReadPointer (juce::jmin (1, riser.getNumChannels() - 1));
                float* o0 = buffer.getWritePointer (0);
                float* o1 = numCh > 1 ? buffer.getWritePointer (1) : nullptr;
                for (int i = 0; i < n; ++i)
                {
                    if (pos >= len)
                    {
                        if (loop) pos = 0;
                        else { listening = false; break; }
                    }
                    if (o1 != nullptr) { o0[i] += rl[pos]; o1[i] += rr[pos]; }
                    else               { o0[i] += 0.5f * (rl[pos] + rr[pos]); }
                    ++pos;
                }
                listenPos = pos;
            }
            else
                listening = false;
        }
    }
}

float RiserProcessor::getListenProgress() const
{
    const int len = riserLen.load();
    return len > 0 ? juce::jlimit (0.0f, 1.0f, (float) listenPos.load() / (float) len) : 0.0f;
}

// ---------------------------------------------------------------- message thread
riser::FxParams RiserProcessor::readFx() const
{
    riser::FxParams p;
    p.phone = getP ("phone") > 0.5f;
    p.chorus = getP ("chorus") > 0.5f;
    p.drive = getP ("drive") > 0.5f;
    p.trem = getP ("tremolo") > 0.5f;
    p.phoneMix = getP ("phoneMix");
    p.chorusMix = getP ("chorusMix");
    p.chorusRate = getP ("chorusRate");
    p.chorusDepth = getP ("chorusDepth");
    p.driveAmt = getP ("driveAmt");
    p.driveMix = getP ("driveMix");
    // tempo-synced: cycles per second = bpm * division / 240   (1/8 note at 120 bpm = 4 Hz)
    p.tremRate = (float) (hostBpm.load() * getTremDivision ((int) std::lround (getP ("tremRate"))) / 240.0);
    p.tremMix = getP ("tremMix");
    p.lowCut = getP ("lowCut");
    p.highCut = getP ("highCut");
    p.cutOrder = kCutOrders[juce::jlimit (0, 2, (int) std::lround (getP ("cutSlope")))];
    return p;
}

void RiserProcessor::timerCallback()
{
    if (handoffReady.exchange (false))
    {
        const int idx = handoffIndex.load();
        const int len = juce::jmin (handoffLength.load(), capBuf[idx].getNumSamples());
        if (len > 0)
        {
            source.setSize (2, len);
            for (int ch = 0; ch < 2; ++ch)
                source.copyFrom (ch, 0, capBuf[idx], ch, 0, len);
            sourceSR = hostSR.load();
            riser::trimLeading (source, sourceSR);
            ++sourceVersion;
            analyseSource();
            renderDirty = true;
        }
    }

    std::array<float, kNumRenderIds> vals;
    for (int i = 0; i < kNumRenderIds; ++i)
        vals[(size_t) i] = getP (kRenderIds[i]);
    if (vals != lastVals)
    {
        lastVals = vals;
        renderDirty = true;
    }

    if (renderDirty.exchange (false))
        rebuild();
}

void RiserProcessor::analyseSource()
{
    sourcePeaks = riser::computePeaks (source, 160);
    sibilance.clear();
    const int len = source.getNumSamples();
    if (len > 0)
        for (auto& r : riser::detectSibilance (source, sourceSR))
            sibilance.push_back ({ (float) r.first / (float) len, (float) r.second / (float) len });
    ++uiVersion;
}

void RiserProcessor::rebuild()
{
    if (source.getNumSamples() == 0)
        return;

    const double sr = hostSR.load();
    if (std::abs (sourceSR - sr) > 1.0)
    {
        source = riser::resampleBuffer (source, sourceSR, sr);
        sourceSR = sr;
        ++sourceVersion;
        analyseSource();
    }

    const double barSec = 60.0 / hostBpm.load() * hostBeatsPerBar.load();
    const int idx = juce::jlimit (0, 5, (int) std::lround (getP ("length")));
    const double lengthSec = barSec * kLenMult[idx];
    const bool vox = getP ("vox") > 0.5f;
    currentLengthSec = lengthSec;

    if (base.getNumSamples() == 0 || std::abs (baseLen - lengthSec) > 1.0e-6 || baseVox != vox
        || baseSrcVersion != sourceVersion || std::abs (baseSR - sr) > 0.5)
    {
        excerpt = riser::chooseExcerpt (source, sr, lengthSec, vox);
        const float srcLen = (float) juce::jmax (1, source.getNumSamples());
        selStart = (float) excerpt.start / srcLen;
        selEnd = (float) (excerpt.start + excerpt.length) / srcLen;
        base = riser::buildBase (source, sr, lengthSec, vox, excerpt);
        baseLen = lengthSec;
        baseVox = vox;
        baseSrcVersion = sourceVersion;
        baseSR = sr;
    }
    if (base.getNumSamples() == 0)
        return;

    juce::AudioBuffer<float> out;
    out.makeCopyOf (base);
    riser::applyFx (out, sr, readFx());

    {
        const juce::ScopedLock sl (riserLock);
        riser.makeCopyOf (out);
        riserLen = riser.getNumSamples();
        ++riserRevision;
    }
    riserPeaks = riser::computePeaks (out, 160);
    ++uiVersion;
}

void RiserProcessor::clearSource()
{
    stopListen();
    source.setSize (0, 0);
    base.setSize (0, 0);
    baseLen = -1.0;
    {
        const juce::ScopedLock sl (riserLock);
        riser.setSize (0, 0);
        riserLen = 0;
    }
    sourcePeaks.clear();
    riserPeaks.clear();
    sibilance.clear();
    selStart = selEnd = 0.0f;
    ++uiVersion;
}

void RiserProcessor::loadSourceFromFile (const juce::File& f)
{
    juce::AudioFormatManager fm;
    fm.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> r (fm.createReaderFor (f));
    if (r == nullptr || r->lengthInSamples < 256)
        return;
    const int len = (int) juce::jmin ((juce::int64) r->lengthInSamples, (juce::int64) (r->sampleRate * 30.0));
    juce::AudioBuffer<float> tmp (2, len);
    tmp.clear();
    r->read (&tmp, 0, len, 0, true, true);
    if (r->numChannels == 1)
        tmp.copyFrom (1, 0, tmp, 0, 0, len);

    source = riser::resampleBuffer (tmp, r->sampleRate, hostSR.load());
    sourceSR = hostSR.load();
    ++sourceVersion;
    analyseSource();
    renderDirty = true;
}

juce::File RiserProcessor::getRiserFolder() const
{
    return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
        .getChildFile ("MoonRiser").getChildFile ("Risers");
}

int RiserProcessor::countRiserFiles (juce::int64& bytes) const
{
    bytes = 0;
    int count = 0;
    auto dir = getRiserFolder();
    if (dir.isDirectory())
        for (auto& f : dir.findChildFiles (juce::File::findFiles, false, "*.wav"))
        {
            ++count;
            bytes += f.getSize();
        }
    return count;
}

int RiserProcessor::deleteAllRisers()
{
    int deleted = 0;
    auto wipe = [&] (const juce::File& dir)
    {
        if (dir.isDirectory())
            for (auto& f : dir.findChildFiles (juce::File::findFiles, false, "*.wav"))
                if (f.deleteFile())
                    ++deleted;
    };
    wipe (getRiserFolder());
    // folders used by earlier versions
    wipe (juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("RiserMaker").getChildFile ("Risers"));
    wipe (juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("RiserMaker"));
    exportedRevision = -1;
    exportedFile = juce::File();
    return deleted;
}

juce::File RiserProcessor::exportRiserToFile()
{
    if (exportedRevision == riserRevision && exportedFile.existsAsFile())
        return exportedFile; // nothing changed since the last export: reuse the same file

    juce::AudioBuffer<float> copy;
    int rev = 0;
    {
        const juce::ScopedLock sl (riserLock);
        copy.makeCopyOf (riser);
        rev = riserRevision;
    }
    if (copy.getNumSamples() == 0)
        return {};

    const int idx = juce::jlimit (0, 5, (int) std::lround (getP ("length")));
    auto dir = getRiserFolder();
    dir.createDirectory();
    auto name = "Riser_" + juce::String (kLenName[idx]) + "bar_" + juce::String (juce::roundToInt (hostBpm.load())) + "bpm";
    auto file = dir.getNonexistentChildFile (name, ".wav", false);

    juce::WavAudioFormat fmt;
    std::unique_ptr<juce::FileOutputStream> os (file.createOutputStream());
    if (os == nullptr)
        return {};
    std::unique_ptr<juce::AudioFormatWriter> w (fmt.createWriterFor (os.get(), hostSR.load(), 2, 24, {}, 0));
    if (w == nullptr)
        return {};
    os.release(); // the writer owns the stream now
    w->writeFromAudioSampleBuffer (copy, 0, copy.getNumSamples());
    w.reset();
    exportedFile = file;
    exportedRevision = rev;
    return file;
}

// ---------------------------------------------------------------- presets & state
void RiserProcessor::setParamPlain (const juce::String& id, float v)
{
    if (auto* p = apvts.getParameter (id))
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->convertTo0to1 (v));
        p->endChangeGesture();
    }
}

int RiserProcessor::getNumFactoryPresets() { return (int) factoryPresets().size(); }

juce::String RiserProcessor::getFactoryPresetName (int i)
{
    return factoryPresets()[(size_t) juce::jlimit (0, getNumFactoryPresets() - 1, i)].name;
}

void RiserProcessor::applyFactoryPreset (int index)
{
    if (index < 0 || index >= getNumFactoryPresets())
        return;
    for (auto* p : getParameters())
    {
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p))
        {
            const auto id = rp->paramID;
            if (id == "loop" || id == "autoRec")
                continue; // recording / playback settings are not part of a sound preset
        }
        p->setValueNotifyingHost (p->getDefaultValue());
    }
    for (auto& kv : factoryPresets()[(size_t) index].values)
        setParamPlain (kv.first, kv.second);
}

juce::File RiserProcessor::getUserPresetDir() const
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
        .getChildFile ("MoonRiser").getChildFile ("Presets");
}

juce::StringArray RiserProcessor::listUserPresets() const
{
    juce::StringArray names;
    auto dir = getUserPresetDir();
    if (dir.isDirectory())
        for (auto& f : dir.findChildFiles (juce::File::findFiles, false, "*.xml"))
            names.add (f.getFileNameWithoutExtension());
    names.sort (true);
    return names;
}

bool RiserProcessor::saveUserPreset (const juce::String& name)
{
    auto clean = juce::File::createLegalFileName (name.trim());
    if (clean.isEmpty())
        return false;
    auto dir = getUserPresetDir();
    dir.createDirectory();
    auto xml = apvts.copyState().createXml();
    return xml != nullptr && xml->writeTo (dir.getChildFile (clean + ".xml"));
}

bool RiserProcessor::loadUserPreset (const juce::String& name)
{
    auto f = getUserPresetDir().getChildFile (name + ".xml");
    if (! f.existsAsFile())
        return false;
    auto xml = juce::XmlDocument::parse (f);
    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType()))
        return false;
    const float keepLoop = getP ("loop"), keepAuto = getP ("autoRec");
    apvts.replaceState (juce::ValueTree::fromXml (*xml));
    setParamPlain ("loop", keepLoop);
    setParamPlain ("autoRec", keepAuto);
    return true;
}

void RiserProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    if (xml != nullptr)
    {
        xml->setAttribute ("uiW", editorWidth);
        copyXmlToBinary (*xml, destData);
    }
}

void RiserProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
    {
        editorWidth = juce::jlimit (480, 1120, xml->getIntAttribute ("uiW", 640));
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
    }
}

juce::AudioProcessorEditor* RiserProcessor::createEditor()
{
    return new RiserEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RiserProcessor();
}
