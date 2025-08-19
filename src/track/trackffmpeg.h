// SPDX-License-Identifier: GPL-3.0-or-later
/** @file */
#include "abstractbpmdetector.h"
#include "track.h"

class QAudioDecoder;

class TrackFfmpeg : public Track {
    Q_OBJECT
public:
    /**
     * Constructor.
     * @param fileName Filename.
     * @param readMetadata If `true`, read tags from the file.
     * @param parent Parent object.
     */
    TrackFfmpeg(const QString &fileName, bool readMetadata = true, QObject *parent = nullptr);
    ~TrackFfmpeg() override;
    void readTags() override;
    bpmtype detectBpm() override;
    /** Signal for when BPM has been detected (or when the BPM detection is finished). */
    Q_SIGNAL void hasBpm(bpmtype bpm);
    /** Signal for when length has been determined. */
    Q_SIGNAL void hasLength(quint64 ms);
    /**
     * Signal for progress updates.
     * @param pos Current position in milliseconds.
     * @param length Total length in milliseconds.
     */
    Q_SIGNAL void progress(qint64 pos, qint64 length);
    void stop() override;

protected:
    void open() override;
    void removeBpm() override;
    void storeBpm(const QString &sBpm) override;

private:
    QAudioDecoder *decoder_ = nullptr;
    bool startedDetection_ = false;
};
