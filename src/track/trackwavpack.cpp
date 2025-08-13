// SPDX-License-Identifier: GPL-3.0-or-later
#include <QDebug>
#include <apetag.h>
#include <wavpackfile.h>

#include "trackwavpack.h"
#include "utils.h"

TrackWavpack::TrackWavpack(const QString &filename, bool readMetadata) : Track() {
    setFilename(filename, readMetadata);
}

TrackWavpack::~TrackWavpack() {
    close();
}

void TrackWavpack::readTags() {
    auto fname = filename();
    auto sBPM = QStringLiteral("000.00");
    TagLib::WavPack::File f(fname.toUtf8().constData(), false);
    auto ape = f.APETag();
    auto tag = f.tag();
    if (tag != nullptr) {
        setArtist(QString::fromUtf8(tag->artist().toCString(true)));
        setTitle(QString::fromUtf8(tag->title().toCString(true)));
    }
    if (ape != nullptr) {
        auto bpm = QString::fromUtf8(ape->itemListMap()["BPM"].toString().toCString(true));
        if (bpm.length() > 0) {
            sBPM = bpm;
        }
    }
    // set filename (without path) as title if the title is empty
    if (title().isEmpty())
        setTitle(fname.mid(fname.lastIndexOf(QStringLiteral("/")) + 1));
    setBPM(str2bpm(sBPM));
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
        qCritical() << "Cannot open file.";
#endif
        return;
    }

    setOpened(true);

    uint sRate = WavpackGetSampleRate(wpc);
    uint len = static_cast<unsigned int>((WavpackGetNumSamples64(wpc) * 1000) / sRate);
    setLength(len);
    setStartPos(0);
    setEndPos(len);
    setSampleRate(static_cast<int>(WavpackGetSampleRate(wpc)));
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
        auto pos = ms * sampleRate() / 1000;
        if (WavpackSeekSample64(wpc, static_cast<int64_t>(pos))) {
            m_iCurPosPCM = pos;
        } else {
            qCritical() << "Seek failed.";
        }
    }
}

qint64 TrackWavpack::currentPos() {
    if (wpc == nullptr)
        return 0;
    return (m_iCurPosPCM * 1000) / sampleRate();
}

int TrackWavpack::readSamples(QSpan<soundtouch::SAMPLETYPE> buffer) {
    const auto num = buffer.size();
    const auto sbytes = static_cast<unsigned int>(sampleBytes());
    if (!isValid() || wpc == nullptr || num < sbytes) {
        return -1;
    }
    const auto samplesRead = static_cast<unsigned int>(WavpackUnpackSamples(
        wpc, reinterpret_cast<int32_t *>(buffer.data()), static_cast<uint32_t>(num / sbytes)));
    // Handle non-float samples.
    if (!(WavpackGetMode(wpc) & MODE_FLOAT)) {
        const auto bufferSize = samplesRead * static_cast<unsigned int>(channels());
        const auto nativeBuffer =
            unsafe_forge_span(reinterpret_cast<int32_t *>(buffer.data()), num);
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
    auto wasNull = wpc == nullptr;
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
    auto wasNull = wpc == nullptr;
    if (wasNull) {
        wpc = WavpackOpenFileInput(filename().toUtf8().constData(),
                                   nullptr,
                                   OPEN_2CH_MAX | OPEN_NORMALIZE | OPEN_EDIT_TAGS | OPEN_FILE_UTF8,
                                   0);
    }
    WavpackDeleteTagItem(wpc, "bpm");
    WavpackWriteTag(wpc);
    if (wasNull) {
        WavpackCloseFile(wpc);
        wpc = nullptr;
    }
}
