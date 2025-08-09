#include <iostream>

#ifdef HAVE_TAGLIB
#include <wavpackfile.h>
#endif

#include "trackwavpack.h"

using namespace std;
using namespace soundtouch;

TrackWavpack::TrackWavpack(const char *filename, bool readtags) : Track() {
    setFilename(filename, readtags);
}

TrackWavpack::~TrackWavpack() {
    close();
}

void TrackWavpack::readTags() {
    string fname = filename();
    string sbpm = "000.00";
#ifdef HAVE_TAGLIB
    TagLib::WavPack::File f(fname.c_str(), false);
    TagLib::Tag *tag = f.tag();
    if (tag != NULL) {
        setArtist(tag->artist().toCString());
        setTitle(tag->title().toCString());
    }
#endif
    // set filename (without path) as title if the title is empty
    if (title().empty())
        setTitle(fname.substr(fname.find_last_of("/") + 1));
    setBPM(str2bpm(sbpm));
}

void TrackWavpack::open() {
    close();
    m_iCurPosPCM = 0;

    string fname = filename();
    wpc = WavpackOpenFileInput(
        fname.c_str(), nullptr, OPEN_2CH_MAX | OPEN_NORMALIZE | OPEN_FILE_UTF8, 0);
    if (wpc == nullptr) {
#ifndef NDEBUG
        cerr << "TrackWavpack: can not open file" << endl;
#endif
        return;
    }

    setOpened(true);

    uint srate = WavpackGetSampleRate(wpc);
    uint len = (unsigned int)((WavpackGetNumSamples64(wpc) * 1000) / srate);
    setLength(len);
    setStartPos(0);
    setEndPos(len);
    setSamplerate(WavpackGetSampleRate(wpc));
    setSampleBytes(WavpackGetBytesPerSample(wpc));
    setChannels(WavpackGetReducedChannels(wpc));
    setTrackType(TYPE_WAVPACK);
    setValid(true);
}

void TrackWavpack::close() {
    if (isOpened()) {
        WavpackCloseFile(wpc);
        wpc = nullptr;
        setOpened(false);
        m_iCurPosPCM = 0;
    }
}

void TrackWavpack::seek(uint ms) {
    if (isValid() && wpc != nullptr) {
        unsigned long long pos = (unsigned long long)((ms * samplerate()) / 1000);
        if (WavpackSeekSample64(wpc, pos)) {
            m_iCurPosPCM = pos;
        } else {
            cerr << "TrackWavpack: seek failed" << endl;
        }
    }
}

uint TrackWavpack::currentPos() {
    if (wpc == nullptr)
        return 0;
    return (uint)((m_iCurPosPCM * 1000) / samplerate());
}

int TrackWavpack::readSamples(SAMPLETYPE *buffer, unsigned int num) {
    const auto sbytes = sampleBytes();
    if (!isValid() || wpc == nullptr || num < sbytes) {
        return -1;
    }
    auto samplesRead = WavpackUnpackSamples(wpc, reinterpret_cast<int32_t *>(buffer), num / sbytes);
    // Handle non-float samples.
    if (!(WavpackGetMode(wpc) & MODE_FLOAT)) {
        int32_t *nativeBuffer = reinterpret_cast<int32_t *>(buffer);
        const long bufferSize = samplesRead * channels();
        const auto bitsPerSample = WavpackGetBytesPerSample(wpc) * 8;
        for (long index = 0; index < bufferSize; index++) {
            buffer[index] = static_cast<float>(nativeBuffer[index]) / (1 << (bitsPerSample - 1));
        }
    }
    m_iCurPosPCM += samplesRead;
    return samplesRead;
}

void TrackWavpack::storeBPM(string sBPM) {
    if (wpc == nullptr)
        return;
    WavpackAppendTagItem(wpc, "bpm", sBPM.c_str(), 0);
}

void TrackWavpack::removeBPM() {
    if (wpc == nullptr)
        return;
    WavpackDeleteTagItem(wpc, "bpm");
}
