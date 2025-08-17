// SPDX-License-Identifier: GPL-3.0-or-later
/** @file */
#pragma once

#include "track.h"

/** Used to proxy between different track types. */
class TrackProxy : public Track {
public:
    /** Constructor.
     * @param fileName Filename.
     * @param readMetadata If `true`, read tags from the file.
     */
    TrackProxy(const QString &fileName, bool readMetadata = true);
    ~TrackProxy() override;
    QString artist() const override;
    QString fileName() const override;
    QString format() const override;
    QString formatted() const override;
    QString formatted(const QString &format) const override;
    QString formattedLength() const override;
    QString title() const override;
    TrackType trackType() const override;
    bool isOpened() const override;
    bool isValid() const override;
    bool redetect() const override;
    bpmtype bpm() const override;
    bpmtype detectBpm() override;
    double progress() const override;
    /** Get the progress amount (non-const version). */
    double progress();
    quint64 endPos() const override;
    quint64 length() const override;
    quint64 startPos() const override;
    unsigned int channels() const override;
    unsigned int sampleBits() const override;
    unsigned int sampleBytes() const override;
    unsigned int sampleRate() const override;
    void clearBpm() override;
    void printBpm() const override;
    void readInfo() override;
    void readTags() override;
    void saveBpm() override;
    void setBpm(bpmtype dBpm) override;
    void setConsoleProgress(bool enable = true) override;
    void setEndPos(quint64 ms) override;
    void setFileName(const QString &fileName, bool readMetadata = true) override;
    void setFormat(const QString &format = QStringLiteral("0.00")) override;
    void setRedetect(bool redetect) override;
    void setStartPos(quint64 ms) override;
    void stop() override;

protected:
    Track *createTrack(const QString &fileName, bool readMetadata = true);
    int readSamples(QSpan<soundtouch::SAMPLETYPE> sp) override;
    quint64 currentPos() override;
    void close() override;
    void open() override;
    void removeBpm() override;
    void seek(quint64 ms) override;
    void storeBpm(const QString &sBpm) override;

private:
    Track *m_pTrack;
    bool m_bConsoleProgress;
};

/** Default TrackProxy factory. */
TrackProxy trackProxyFactory(const QString &fileName);
/** TrackProxy factory. */
typedef TrackProxy (*TrackProxyFactory)(const QString &fileName);
