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
    void setFileName(const QString &fileName, bool readMetadata = true) override;
    void readTags() override;
    void readInfo() override;
    bpmtype detectBpm() override;
    void printBpm() const override;
    /** Get the progress amount. */
    double progress();
    void setBpm(bpmtype dBpm) override;
    bpmtype bpm() const override;
    void clearBpm() override;
    void saveBpm() override;
    QString formatted() const override;
    QString formatted(const QString &format) const override;
    QString fileName() const override;
    quint64 length() const override;
    QString formattedLength() const override;
    bool isValid() const override;
    bool isOpened() const override;
    QString artist() const override;
    QString title() const override;
    void setRedetect(bool redetect) override;
    bool redetect() const override;
    double progress() const override;
    void setFormat(const QString &format = QStringLiteral("0.00")) override;
    QString format() const override;
    void setConsoleProgress(bool enable = true) override;
    void stop() override;
    void setStartPos(quint64 ms) override;
    quint64 startPos() const override;
    void setEndPos(quint64 ms) override;
    quint64 endPos() const override;
    unsigned int sampleRate() const override;
    unsigned int sampleBytes() const override;
    unsigned int sampleBits() const override;
    unsigned int channels() const override;
    TrackType trackType() const override;

protected:
    void open() override;
    void close() override;
    void seek(quint64 ms) override;
    quint64 currentPos() override;
    int readSamples(QSpan<soundtouch::SAMPLETYPE> sp) override;
    void storeBpm(const QString &sBpm) override;
    void removeBpm() override;

private:
    Track *createTrack(const QString &fileName, bool readMetadata = true);
    Track *m_pTrack;
    bool m_bConsoleProgress;
};
