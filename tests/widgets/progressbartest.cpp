#include <QtTest>

#include "widgets/progressbar.h"

class ProgressBarTest : public QObject {
    Q_OBJECT

public:
    explicit ProgressBarTest(QObject *parent = nullptr);
    ~ProgressBarTest() override;

private Q_SLOTS:
    void testInitialState();
    void testMouseMoveEvent();
    void testMousePressEvent();
    void testMouseReleaseEvent();
    void testSetAndGetChange();
    void testSetAndGetEnabled();
    void testSetAndGetLength();
    void testSetAndGetPosition();
    void testSetLength();
    void testSetPosition();
};

ProgressBarTest::ProgressBarTest(QObject *parent) : QObject(parent) {
}

ProgressBarTest::~ProgressBarTest() {
}

void ProgressBarTest::testInitialState() {
    ProgressBar bar;
    QCOMPARE(bar.change(), false);
    QCOMPARE(bar.enabled(), true);
    QCOMPARE(bar.length(), 100);
}

void ProgressBarTest::testSetAndGetEnabled() {
    ProgressBar bar;
    bar.setEnabled(false);
    QCOMPARE(bar.enabled(), false);
    bar.setEnabled(true);
    QCOMPARE(bar.enabled(), true);
}

void ProgressBarTest::testSetAndGetChange() {
    ProgressBar bar;
    bar.setChange(true);
    QCOMPARE(bar.change(), true);
    bar.setChange(false);
    QCOMPARE(bar.change(), false);
}

void ProgressBarTest::testSetAndGetLength() {
    ProgressBar bar;
    bar.setLength(250);
    QCOMPARE(bar.length(), 250);
}

void ProgressBarTest::testSetAndGetPosition() {
    ProgressBar bar;
    bar.setPosition(42);
    QCOMPARE(bar.value(), 42);
}

void ProgressBarTest::testSetLength() {
    ProgressBar bar;
    bar.setLength(500);
    QCOMPARE(bar.maximum(), 500);
}

void ProgressBarTest::testSetPosition() {
    ProgressBar bar;
    bar.setPosition(77);
    QCOMPARE(bar.value(), 77);
}

void ProgressBarTest::testMousePressEvent() {
    ProgressBar bar;
    bar.setEnabled(true);
    QMouseEvent event(QEvent::MouseButtonPress,
                      QPointF(50, 0), // localPos
                      QPointF(50, 0), // windowPos
                      QPointF(50, 0), // screenPos
                      Qt::LeftButton,
                      Qt::LeftButton,
                      Qt::NoModifier);
    bar.mousePressEvent(&event);
    QVERIFY(bar.change());

    QMouseEvent event2(QEvent::MouseButtonPress,
                       QPointF(150, 0), // localPos
                       QPointF(150, 0), // windowPos
                       QPointF(150, 0), // screenPos
                       Qt::RightButton,
                       Qt::RightButton,
                       Qt::NoModifier);
    bar.bChange = true;
    bar.mousePressEvent(&event2);
    QVERIFY(!bar.change());
}

void ProgressBarTest::testMouseMoveEvent() {
    ProgressBar bar;
    bar.setEnabled(true);
    QMouseEvent event(QEvent::MouseMove,
                      QPointF(100, 0), // localPos
                      QPointF(100, 0), // windowPos
                      QPointF(100, 0), // screenPos
                      Qt::NoButton,
                      Qt::NoButton,
                      Qt::NoModifier);
    bar.mouseMoveEvent(&event);
    QCOMPARE(bar.value(), -1);

    QMouseEvent event2(QEvent::MouseMove,
                       QPointF(0, 0), // localPos
                       QPointF(0, 0), // windowPos
                       QPointF(0, 0), // screenPos
                       Qt::NoButton,
                       Qt::NoButton,
                       Qt::NoModifier);
    bar.bChange = true;
    bar.mouseMoveEvent(&event2);
    QCOMPARE(bar.value(), 0);

    QMouseEvent event3(QEvent::MouseMove,
                       QPointF(bar.width() + 1, 0), // localPos
                       QPointF(bar.width() + 1, 0), // windowPos
                       QPointF(bar.width() + 1, 0), // screenPos
                       Qt::NoButton,
                       Qt::NoButton,
                       Qt::NoModifier);
    bar.bChange = true;
    bar.mouseMoveEvent(&event3);
    QCOMPARE(bar.value(), bar.maximum());
}

void ProgressBarTest::testMouseReleaseEvent() {
    ProgressBar bar;
    bar.setEnabled(true);
    bar.setChange(true);
    bar.setLength(100);
    QSignalSpy spy(&bar, &ProgressBar::positionChanged);
    QMouseEvent event(QEvent::MouseButtonPress,
                      QPointF(50, 0), // localPos
                      QPointF(50, 0), // windowPos
                      QPointF(50, 0), // screenPos
                      Qt::LeftButton,
                      Qt::LeftButton,
                      Qt::NoModifier);
    bar.mouseReleaseEvent(&event);
    QVERIFY(bar.change());
    QCOMPARE(spy.count(), 0);

    bar.bChange = true;
    bar.enable = true;
    QSignalSpy spy2(&bar, &ProgressBar::positionChanged);
    QMouseEvent event2(QEvent::MouseButtonRelease,
                       QPointF(75, 0), // localPos
                       QPointF(75, 0), // windowPos
                       QPointF(75, 0), // screenPos
                       Qt::LeftButton,
                       Qt::NoButton,
                       Qt::NoModifier);
    bar.mouseReleaseEvent(&event2);
    QVERIFY(!bar.change());
    QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(ProgressBarTest)
#include "progressbartest.moc"
