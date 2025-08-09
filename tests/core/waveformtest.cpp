// SPDX-License-Identifier: GPL-3.0-or-later
#include <QtCore/QDebug>
#include <QtNetwork/QNetworkInterface>
#include <QtTest/QTest>

#include "core/waveform.h"

class WaveformTest : public QObject {
    Q_OBJECT

public:
    WaveformTest(QObject *parent = nullptr) {
        Q_UNUSED(parent);
    };
    ~WaveformTest() override {};

private Q_SLOTS:
    void testInit();
};

void WaveformTest::testInit() {
    Waveform waveform(44100, 4);
    QVERIFY(waveform.getMaxValue() >= 0.0f);
    QVERIFY(waveform.getMinValue() >= 0.0f);
    QVERIFY(waveform.getAverageValue() >= 0.0f);
}

QTEST_MAIN(WaveformTest)

#include "waveformtest.moc"
