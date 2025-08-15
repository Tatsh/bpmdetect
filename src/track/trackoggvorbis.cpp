// SPDX-License-Identifier: GPL-3.0-or-later
#include <climits>

#ifdef Q_OS_WIN
#include <fcntl.h>
#include <io.h>
#endif

#include <QtCore/QDebug>
#include <textidentificationframe.h>
#include <vorbisfile.h>
#include <xiphcomment.h>

#include "trackoggvorbis.h"

TrackOggVorbis::TrackOggVorbis(const QString &fname, bool readMetadata) : Track() {
    setFileName(fname, readMetadata);
}

TrackOggVorbis::~TrackOggVorbis() {
    close();
}

void TrackOggVorbis::open() {
    close();

    m_iCurPosPCM = 0;
    auto fname = fileName();
    // Try to open the file for reading
    fptr.setFileName(fname);
    if (!fptr.open(QFile::ReadOnly)) {
        qCritical() << "TrackOggVorbis: can not open file";
        return;
    }

    setOpened(true);
    if (ov_fopen(fptr.fileName().toUtf8().constData(), &vf) < 0) {
        qCritical() << "TrackOggVorbis: Input does not appear to be an Ogg bitstream";
        close();
        return;
    }

    // extract metadata
    auto vi = ov_info(&vf, -1);

    auto channels = vi->channels;
    auto sRate = vi->rate;
    auto numSamples = ov_pcm_total(&vf, -1);
    auto len = 1000 * numSamples / sRate;

    setLength(static_cast<quint64>(len));
    setStartPos(0);
    setEndPos(static_cast<quint64>(len));
    setSampleRate(static_cast<unsigned int>(sRate));
    setSampleBytes(2);
    setChannels(static_cast<unsigned int>(channels));
    setTrackType(Ogg);
    setValid(true);
}

void TrackOggVorbis::close() {
    if (isOpened())
        ov_clear(&vf);
    // note that fclose() is not needed, ov_clear() does this as well
    m_iCurPosPCM = 0;
    setOpened(false);
}

void TrackOggVorbis::seek(quint64 ms) {
    if (isValid()) {
        auto pos = (ms * sampleRate() /* * channels()*/) / 1000;
        if (ov_pcm_seek(&vf, static_cast<ogg_int64_t>(pos)) == 0) {
            m_iCurPosPCM = pos;
        } else {
            qCritical() << "seek failed: seek ERR.";
        }
    } else {
        qCritical() << "seek failed: track not valid";
    }
}

quint64 TrackOggVorbis::currentPos() {
    if (isValid()) {
        auto pos = 1000 * m_iCurPosPCM / sampleRate() /* *channels()*/;
        return pos;
    }
    return 0;
}

int TrackOggVorbis::readSamples(QSpan<soundtouch::SAMPLETYPE> buffer) {
    auto num = buffer.size();
    if (!isValid() || num < 2)
        return -1;

    QList<short> dest(num);
    unsigned int index = 0;
    auto needed = static_cast<long>(num);
    // loop until requested number of samples has been retrieved
    while (needed > 0) {
        // read samples into buffer
        auto ret = ov_read(&vf,
                           reinterpret_cast<char *>(&dest[index]),
                           static_cast<int>(needed),
                           0, // Little endian. Wait and see if anyone reports an issue.
                           2,
                           1,
                           &current_section);
        // if eof we fill the rest with zero
        if (ret == 0) {
            while (needed > 0) {
                dest[index] = 0;
                index++;
                needed--;
            }
        }
        index += ret;
        needed -= ret;
    }

    unsigned int nread = index / 2;
    for (qsizetype i = 0; i < nread; ++i) {
        buffer[i] = static_cast<float>(dest[i]) / SAMPLE_MAX_VALUE;
    }

    // return the number of samples in buffer
    m_iCurPosPCM += nread;
    return static_cast<int>(nread);
}

void TrackOggVorbis::storeBPM(const QString &format) {
    auto fname = fileName();
    auto sBPM = bpmToString(bpm(), format);
    TagLib::Ogg::Vorbis::File f(fname.toUtf8().constData(), false);
    auto tag = f.tag();
    if (tag == nullptr) {
        qCritical() << "Failed to save BPM.";
        return;
    }
    tag->addField("TBPM", sBPM.toUtf8().constData(), true); // add new BPM field (replace existing)
    f.save();
}

void TrackOggVorbis::readTags() {
    auto fname = fileName();
    auto sBPM = QStringLiteral("000.00");
    TagLib::Ogg::Vorbis::File f(fname.toUtf8().constData(), false);
    auto tag = f.tag();
    if (tag != nullptr) {
        setArtist(QString::fromUtf8(tag->artist().toCString(true)));
        setTitle(QString::fromUtf8(tag->title().toCString(true)));
        auto flMap = tag->fieldListMap();
        auto strl = flMap["TBPM"];
        if (!strl.isEmpty())
            sBPM = QString::fromUtf8(strl[0].toCString(true));
    }
    // set fileName (without path) as title if the title is empty
    if (title().isEmpty())
        setTitle(fname.mid(fname.lastIndexOf(QStringLiteral("/")) + 1));
    setBpm(stringToBpm(sBPM));
}

void TrackOggVorbis::removeBpm() {
    auto fname = fileName();
    //close();
    TagLib::Ogg::Vorbis::File f(fname.toUtf8().constData(), false);
    auto tag = f.tag();
    if (tag == nullptr) {
        return;
    }
    tag->removeFields("TBPM");
    f.save();
    //open();
}
