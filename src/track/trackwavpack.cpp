// SPDX-License-Identifier: GPL-3.0-or-later
#include <iostream>

#include <apetag.h>
#include <wavpackfile.h>

#include "trackwavpack.h"

using namespace std;
using namespace soundtouch;

TrackWavpack::TrackWavpack(const QString &filename, bool readtags) : Track() {
    setFilename(filename, readtags);
}

TrackWavpack::~TrackWavpack() {
    close();
}

void TrackWavpack::readTags() {
    auto fname = filename();
    auto sbpm = QString::fromUtf8("000.00");
    TagLib::WavPack::File f(fname.toUtf8().constData(), false);
    auto ape = f.APETag();
    TagLib::Tag *tag = f.tag();
    if (tag != NULL) {
        setArtist(QString::fromUtf8(tag->artist().toCString(true)));
        setTitle(QString::fromUtf8(tag->title().toCString(true)));
    }
    if (ape != NULL) {
        auto bpm = QString::fromUtf8(ape->itemListMap()["BPM"].toString().toCString(true));
        if (bpm.length() > 0) {
            sbpm = bpm;
        }
    }
    // set filename (without path) as title if the title is empty
    if (title().isEmpty())
        setTitle(fname.mid(fname.lastIndexOf(QLatin1Char('/')) + 1));
    setBPM(str2bpm(sbpm));
}

void TrackWavpack::open() {
    close();
    m_iCurPosPCM = 0;

    auto fname = filename();
    wpc = WavpackOpenFileInput(fname.toUtf8().constData(),
                               nullptr,
                               OPEN_2CH_MAX | OPEN_NORMALIZE | OPEN_EDIT_TAGS | OPEN_FILE_UTF8,
                               0);
    if (wpc == nullptr) {
#ifndef NDEBUG
        cerr << "TrackWavpack: can not open file" << endl;
#endif
        return;
    }

    setOpened(true);

    uint srate = WavpackGetSampleRate(wpc);
    uint len = static_cast<unsigned int>((WavpackGetNumSamples64(wpc) * 1000) / srate);
    setLength(len);
    setStartPos(0);
    setEndPos(len);
    setSamplerate(static_cast<int>(WavpackGetSampleRate(wpc)));
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

void TrackWavpack::seek(qint64 ms) {
    if (isValid() && wpc != nullptr) {
        auto pos = ms * samplerate() / 1000;
        if (WavpackSeekSample64(wpc, static_cast<int64_t>(pos))) {
            m_iCurPosPCM = pos;
        } else {
            cerr << "TrackWavpack: seek failed" << endl;
        }
    }
}

qint64 TrackWavpack::currentPos() {
    if (wpc == nullptr)
        return 0;
    return (m_iCurPosPCM * 1000) / samplerate();
}

int TrackWavpack::readSamples(QSpan<SAMPLETYPE> buffer) {
    const auto num = buffer.size();
    const auto sbytes = static_cast<unsigned int>(sampleBytes());
    if (!isValid() || wpc == nullptr || buffer.size() < sbytes) {
        return -1;
    }
    const auto samplesRead = static_cast<unsigned int>(WavpackUnpackSamples(
        wpc, reinterpret_cast<int32_t *>(buffer.data()), static_cast<uint32_t>(num / sbytes)));
    // Handle non-float samples.
    if (!(WavpackGetMode(wpc) & MODE_FLOAT)) {
        const auto nativeBuffer = reinterpret_cast<int32_t *>(buffer.data());
        const auto bufferSize = samplesRead * static_cast<unsigned int>(channels());
        const auto bitsPerSample = WavpackGetBytesPerSample(wpc) * 8;
        for (unsigned int index = 0; index < bufferSize; index++) {
            buffer[index] = static_cast<float>(nativeBuffer[index]) /
                            static_cast<float>(1 << (bitsPerSample - 1));
        }
    }
    m_iCurPosPCM += samplesRead;
    return static_cast<int>(samplesRead);
}

void TrackWavpack::storeBPM(const QString &sBPM) {
    bool wasNull = wpc == nullptr;
    if (wasNull) {
        wpc = WavpackOpenFileInput(filename().toUtf8().constData(),
                                   nullptr,
                                   OPEN_2CH_MAX | OPEN_NORMALIZE | OPEN_EDIT_TAGS | OPEN_FILE_UTF8,
                                   0);
    }
    WavpackAppendTagItem(wpc, "bpm", sBPM.toUtf8().constData(), static_cast<int>(sBPM.length()));
    WavpackWriteTag(wpc);
    if (wasNull) {
        WavpackCloseFile(wpc);
        wpc = nullptr;
    }
}

void TrackWavpack::removeBPM() {
    bool wasNull = wpc == nullptr;
    if (wasNull) {
        wpc = WavpackOpenFileInput(filename().toUtf8().constData(),
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
