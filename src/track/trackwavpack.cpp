#include <iostream>

#ifdef HAVE_TAGLIB
#include <apetag.h>
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
    auto ape = f.APETag();
    TagLib::Tag *tag = f.tag();
    if (tag != NULL) {
        setArtist(tag->artist().toCString());
        setTitle(tag->title().toCString());
    }
    if (ape != NULL) {
        auto bpm = ape->itemListMap()["BPM"].toString().to8Bit();
        if (bpm.length() > 0) {
            sbpm = bpm;
        }
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
        fname.c_str(), nullptr, OPEN_2CH_MAX | OPEN_NORMALIZE | OPEN_EDIT_TAGS | OPEN_FILE_UTF8, 0);
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
    bool wasNull = wpc == nullptr;
    if (wasNull) {
        wpc = WavpackOpenFileInput(filename().c_str(),
                                   nullptr,
                                   OPEN_2CH_MAX | OPEN_NORMALIZE | OPEN_EDIT_TAGS | OPEN_FILE_UTF8,
                                   0);
    }
    int ret = WavpackAppendTagItem(wpc, "bpm", sBPM.c_str(), sBPM.length());
    cout << "WavpackAppendTagItem returned " << ret << endl;
    ret = WavpackWriteTag(wpc);
    cout << "WavpackWriteTag returned " << ret << endl;
    if (wasNull) {
        WavpackCloseFile(wpc);
        wpc = nullptr;
    }
}

void TrackWavpack::removeBPM() {
    bool wasNull = wpc == nullptr;
    if (wasNull) {
        wpc = WavpackOpenFileInput(filename().c_str(),
                                   nullptr,
                                   OPEN_2CH_MAX | OPEN_NORMALIZE | OPEN_EDIT_TAGS | OPEN_FILE_UTF8,
                                   0);
    }
    int ret = WavpackDeleteTagItem(wpc, "bpm");
    cout << "WavpackDeleteTagItem returned " << ret << endl;
    ret = WavpackWriteTag(wpc);
    cout << "WavpackWriteTag returned " << ret << endl;
    if (wasNull) {
        WavpackCloseFile(wpc);
        wpc = nullptr;
    }
}
