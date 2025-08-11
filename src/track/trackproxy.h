// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "track.h"

/** Used to proxy between different track types. */
class TrackProxy : public Track {
public:
    /** Constructor.
     * @param filename Filename.
     * @param readtags If `true`, read tags from the file.
     */
    TrackProxy(const QString &filename, bool readtags = true);
    ~TrackProxy() override;

    void setFilename(const QString &filename, bool readtags = true) override;
    void readTags() override;
    void readInfo() override;
    double detectBPM() override;
    void printBPM() const override;
    /** Get the progress amount. */
    double progress();
    void setBPM(double dBPM) override;
    double getBPM() const override;
    void clearBPM() override;
    void saveBPM() override;
    QString strBPM() const override;
    QString strBPM(const QString &format) const override;
    QString filename() const override;

    unsigned int length() const override;
    QString strLength() override;
    bool isValid() const override;
    bool isOpened() const override;
    QString artist() const override;
    QString title() const override;
    void setRedetect(bool redetect) override;
    bool redetect() const override;
    double progress() const override;
    void setFormat(QString format = QStringLiteral("0.00")) override;
    QString format() const override;
    void enableConsoleProgress(bool enable = true) override;

    void stop() override;

    void setStartPos(qint64 ms) override;
    qint64 startPos() const override;
    void setEndPos(qint64 ms) override;
    qint64 endPos() const override;
    int samplerate() const override;
    int sampleBytes() const override;
    int sampleBits() const override;
    int channels() const override;
    int trackType() const override;

protected:
    /** Create the Track to proxy to. */
    Track *createTrack(const QString &filename, bool readtags = true);
    void open() override;
    void close() override;
    void seek(qint64 ms) override;
    qint64 currentPos() override;
    int readSamples(QSpan<soundtouch::SAMPLETYPE> sp) override;
    void storeBPM(const QString &sBPM) override;
    void removeBPM() override;

private:
    Track *m_pTrack;
    bool m_bConsoleProgress;
};
