// SPDX-License-Identifier: GPL-3.0-or-later
#include <iostream>

#include <BPMDetect.h>
#include <QtCore/QDebug>
#include <fileref.h>
#include <tag.h>

#include "soundtouchbpmdetector.h"
#include "track.h"

bpmtype Track::_dMinBpm = 80.;
bpmtype Track::_dMaxBpm = 185.;

Track::Track(QObject *parent) : QObject(parent) {
}

Track::~Track() {
}

void Track::setMinimumBpm(bpmtype dMin) {
    if (dMin > 30. && dMin < 300.)
        _dMinBpm = dMin;
    // Swap min and max if min is greater than max.
    if (_dMinBpm > _dMaxBpm) {
        auto temp = _dMinBpm;
        _dMinBpm = _dMaxBpm;
        _dMaxBpm = temp;
    }
}

void Track::setMaximumBpm(bpmtype dMax) {
    if (dMax > 30. && dMax < 300.)
        _dMaxBpm = dMax;
    // Swap min and max if min is greater than max.
    if (_dMinBpm > _dMaxBpm) {
        auto temp = _dMinBpm;
        _dMinBpm = _dMaxBpm;
        _dMaxBpm = temp;
    }
}

bpmtype Track::minimumBpm() {
    return _dMinBpm;
}

bpmtype Track::maximumBpm() {
    return _dMaxBpm;
}

QString Track::formatted() const {
    return bpmToString(bpm(), format());
}

QString Track::formatted(const QString &format) const {
    return bpmToString(bpm(), format);
}

void Track::setFileName(const QString &fileName, bool readMetadata) {
    m_bOpened = false;
    m_bValid = false;
    m_dBpm = 0;
    m_iLength = 0;
    m_sFilename = fileName;
    if (!m_sFilename.isEmpty() && readMetadata) {
        readTags();
    }
}

QString Track::fileName() const {
    return m_sFilename;
}

void Track::setValid(bool bValid) {
    m_bValid = bValid;
}

bool Track::isValid() const {
    return m_bValid;
}

void Track::setOpened(bool opened) {
    m_bOpened = opened;
}

bool Track::isOpened() const {
    return m_bOpened;
}

void Track::setFormat(const QString &format) {
    m_sBpmFormat = format;
}

QString Track::format() const {
    return m_sBpmFormat;
}

void Track::setBpm(bpmtype dBpm) {
    m_dBpm = dBpm;
}

bpmtype Track::bpm() const {
    return m_dBpm;
}

void Track::setArtist(const QString &artist) {
    m_sArtist = artist;
}

QString Track::artist() const {
    return m_sArtist;
}

void Track::setTitle(const QString &title) {
    m_sTitle = title;
}

QString Track::title() const {
    return m_sTitle;
}

void Track::setRedetect(bool redetect) {
    m_bRedetect = redetect;
}

bool Track::redetect() const {
    return m_bRedetect;
}

quint64 Track::length() const {
    return m_iLength;
}

void Track::setLength(quint64 msec) {
    m_iLength = msec;
}

QString Track::formattedLength() const {
    auto len = length();

    auto csecs = len / 10;
    auto secs = csecs / 100;
    csecs = csecs % 100;
    auto mins = secs / 60;
    secs = secs % 60;
    static const auto zero = QChar::fromLatin1('0');

    return QStringLiteral("%1:%2").arg(mins, 2, 10, zero).arg(secs, 2, 10, zero);
}

void Track::saveBpm() {
    storeBpm(bpmToString(bpm(), format()));
}

void Track::clearBpm() {
    setBpm(0);
    removeBpm();
}

void Track::setDetector(AbstractBpmDetector *detector) {
    m_detector = detector;
}

AbstractBpmDetector *Track::detector() const {
    return m_detector;
}

bpmtype Track::correctBpm(bpmtype dBpm) const {
    auto min = minimumBpm();
    auto max = maximumBpm();

    if (dBpm < 1)
        return 0.;
    while (dBpm > max)
        dBpm /= 2.;
    while (dBpm < min)
        dBpm *= 2.;

    return dBpm;
}

void Track::printBpm() const {
    std::cout << fileName().toStdString() << ": " << bpmToString(bpm(), format()).toStdString()
              << " BPM" << std::endl;
}
