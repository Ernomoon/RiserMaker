#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <array>
#include <atomic>
#include <vector>
#include "RiserEngine.h"

class RiserProcessor : public juce::AudioProcessor,
                       private juce::Timer
{
public:
    RiserProcessor();
    ~RiserProcessor() override;

    // --- AudioProcessor ---
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Riser Maker"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // --- parameters ---
    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    float getP (const char* id) const { return apvts.getRawParameterValue (id)->load(); }
    void setParamPlain (const juce::String& id, float plainValue);

    // --- presets ---
    static int getNumFactoryPresets();
    static juce::String getFactoryPresetName (int index);
    void applyFactoryPreset (int index);
    juce::File getUserPresetDir() const;
    juce::StringArray listUserPresets() const;
    bool saveUserPreset (const juce::String& name);
    bool loadUserPreset (const juce::String& name);

    // --- UI-facing state (message thread) ---
    bool hasSource() const { return source.getNumSamples() > 0; }
    bool hasRiser() const { return ! riserPeaks.empty(); }
    double getLengthSeconds() const { return currentLengthSec; }
    std::vector<float> sourcePeaks, riserPeaks;
    std::vector<std::pair<float, float>> sibilance; // normalised 0..1 over the source
    std::atomic<int> uiVersion { 0 };

    // recording
    void setManualRec (bool on) { manualRec = on; }
    bool isManualRec() const { return manualRec.load(); }
    int getRecState() const { return recState.load(); }   // 0 idle, 1 auto, 2 manual
    float getRecSeconds() const { return (float) recSamples.load() / (float) hostSR.load(); }
    float selStart = 0.0f, selEnd = 0.0f;                 // chosen element, normalised over the source
    int editorWidth = 640;

    // exported risers
    juce::File getRiserFolder() const;
    int countRiserFiles (juce::int64& bytes) const;
    int deleteAllRisers();

    void requestListen() { listenRequest = true; }
    void stopListen() { listening = false; }
    bool isListening() const { return listening.load(); }
    float getListenProgress() const;
    juce::File exportRiserToFile();
    void loadSourceFromFile (const juce::File& f);
    void clearSource();

private:
    void timerCallback() override;
    void rebuild();
    void analyseSource();
    riser::FxParams readFx() const;

    static constexpr int kNumRenderIds = 17;
    std::array<float, kNumRenderIds> lastVals;

    // audio thread
    std::atomic<double> hostSR { 44100.0 };
    std::atomic<double> hostBpm { 120.0 };
    std::atomic<double> hostBeatsPerBar { 4.0 };
    std::atomic<bool> renderDirty { false };

    juce::AudioBuffer<float> capBuf[2];
    int capActive = 0, capWritePos = 0, capSilence = 0;
    bool capRecording = false, wasPlaying = true;
    std::atomic<int> handoffIndex { 0 }, handoffLength { 0 };
    std::atomic<bool> handoffReady { false };

    std::atomic<bool> manualRec { false };
    std::atomic<int> recState { 0 }, recSamples { 0 };
    bool capManualActive = false, prevManual = false;

    std::atomic<bool> listening { false }, listenRequest { false };
    std::atomic<int> listenPos { 0 };
    std::atomic<int> riserLen { 0 };

    // message thread
    juce::AudioBuffer<float> source, base;
    double sourceSR = 44100.0;
    int sourceVersion = 0;
    double baseLen = -1.0, baseSR = -1.0;
    bool baseVox = true;
    int baseSrcVersion = -1;
    double currentLengthSec = 0.0;
    riser::Excerpt excerpt;
    int riserRevision = 0, exportedRevision = -1;
    juce::File exportedFile;

    juce::CriticalSection riserLock;
    juce::AudioBuffer<float> riser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RiserProcessor)
};
