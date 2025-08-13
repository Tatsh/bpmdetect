// SPDX-License-Identifier: GPL-3.0-or-later
#include <climits>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif
#ifdef __APPLE__
#ifdef __i386
#define OV_ENDIAN_ARG 0
#else
#define OV_ENDIAN_ARG 1
#endif
#endif
#ifdef __linux__
#include <endian.h>
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define OV_ENDIAN_ARG 0
#else
#define OV_ENDIAN_ARG 1
#endif
#else
#define OV_ENDIAN_ARG 0
#endif

#include <QDebug>
#include <textidentificationframe.h>
#include <vorbisfile.h>
#include <xiphcomment.h>

#include "trackoggvorbis.h"

TrackOggVorbis::TrackOggVorbis(const QString &fname, bool readMetadata) : Track() {
    setFilename(fname, readMetadata);
}

TrackOggVorbis::~TrackOggVorbis() {
    close();
}

void TrackOggVorbis::open() {
    close();

    m_iCurPosPCM = 0;
    auto fname = filename();
    // Try to open the file for reading
    fptr.setFileName(fname);
    if (!fptr.open(QFile::ReadOnly)) {
#ifdef DEBUG
        std::cerr << "TrackOggVorbis: can not open file" << endl;
#endif
        return;
    }

    setOpened(true);
    if (ov_fopen(fptr.fileName().toUtf8().constData(), &vf) < 0) {
#ifdef DEBUG
        cerr << "TrackOggVorbis: Input does not appear to be an Ogg bitstream" << endl;
#endif
        close();
        return;
    }

    // extract metadata
    auto vi = ov_info(&vf, -1);

    auto channels = vi->channels;
    auto sRate = vi->rate;
    auto numSamples = ov_pcm_total(&vf, -1);
    auto len = 1000 * numSamples / sRate;

    setLength(static_cast<unsigned int>(len));
    setStartPos(0);
    setEndPos(len);
    setSampleRate(static_cast<int>(sRate));
    setSampleBytes(2);
    setChannels(channels);
    setTrackType(TYPE_OGGVORBIS);
    setValid(true);
}

void TrackOggVorbis::close() {
    if (isOpened())
        ov_clear(&vf);
    // note that fclose() is not needed, ov_clear() does this as well
    m_iCurPosPCM = 0;
    setOpened(false);
}

void TrackOggVorbis::seek(qint64 ms) {
    if (isValid()) {
        auto pos = (ms * sampleRate() /* * channels()*/) / 1000;
        if (ov_pcm_seek(&vf, static_cast<ogg_int64_t>(pos)) == 0) {
            m_iCurPosPCM = pos;
        }
#ifdef DEBUG
        else {
            cerr << "seek failed: seek ERR.";
        }
    } else {
        cerr << "seek failed: track not valid" << endl;
#endif
    }
}

qint64 TrackOggVorbis::currentPos() {
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
                           OV_ENDIAN_ARG,
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
    m_iCurPosPCM += static_cast<qint64>(nread);
    return static_cast<int>(nread);
}

void TrackOggVorbis::storeBPM(const QString &format) {
    auto fname = filename();
    auto sBPM = bpm2str(getBPM(), format);
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
    auto fname = filename();
    auto sBPM = QStringLiteral("000.00");
    TagLib::Ogg::Vorbis::File f(fname.toUtf8().constData(), false);
    TagLib::Ogg::XiphComment *tag = f.tag();
    if (tag != NULL) {
        setArtist(QString::fromUtf8(tag->artist().toCString(true)));
        setTitle(QString::fromUtf8(tag->title().toCString(true)));
        TagLib::Ogg::FieldListMap flMap = tag->fieldListMap();
        TagLib::StringList strl = flMap["TBPM"];
        if (!strl.isEmpty())
            sBPM = QString::fromUtf8(strl[0].toCString(true));
    }
    // set filename (without path) as title if the title is empty
    if (title().isEmpty())
        setTitle(fname.mid(fname.lastIndexOf(QStringLiteral("/")) + 1));
    setBPM(str2bpm(sBPM));
}

void TrackOggVorbis::removeBPM() {
    auto fname = filename();
    //close();
    TagLib::Ogg::Vorbis::File f(fname.toUtf8().constData(), false);
    TagLib::Ogg::XiphComment *tag = f.tag();
    if (tag == NULL) {
        return;
    }
    tag->removeFields("TBPM");
    f.save();
    //open();
}
