#include <QtTest>

#include "widgets/qdroplistview.h"

class QDropListViewTest : public QObject {
    Q_OBJECT

public:
    explicit QDropListViewTest(QObject *parent = nullptr);
    ~QDropListViewTest() override;

private Q_SLOTS:
    void testDragEnterEvent();
    void testDragLeaveEvent();
    void testDragMoveEvent();
    void testDropEvent();
    void testKeyPressEvent();
    void testKeyPressEvent2();
    void testKeyReleaseEvent();
    void testRemoveSelected();
};

QDropListViewTest::QDropListViewTest(QObject *parent) : QObject(parent) {
}

QDropListViewTest::~QDropListViewTest() {
}

void QDropListViewTest::testRemoveSelected() {
    QDropListView view;
    auto *item1 = new QTreeWidgetItem();
    auto *item2 = new QTreeWidgetItem();
    view.addTopLevelItem(item1);
    view.addTopLevelItem(item2);
    item1->setSelected(true);
    item2->setSelected(true);
    view.slotRemoveSelected();
    QCOMPARE(view.topLevelItemCount(), 0);
}

void QDropListViewTest::testKeyPressEvent() {
    QDropListView view;
    QSignalSpy spy(&view, &QDropListView::keyPress);
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
    view.keyPressEvent(&event);
    QCOMPARE(spy.count(), 1);
}

// Only for coverage.
void QDropListViewTest::testKeyPressEvent2() {
    QDropListView view;
    QSignalSpy spy(&view, &QDropListView::keyPress);
    QKeyEvent event(QEvent::KeyPress, Qt::Key_J, Qt::NoModifier);
    view.keyPressEvent(&event);
    QCOMPARE(spy.count(), 1);
}

void QDropListViewTest::testKeyReleaseEvent() {
    QDropListView view;
    QSignalSpy spy(&view, &QDropListView::keyRelease);
    QKeyEvent event(QEvent::KeyRelease, Qt::Key_A, Qt::NoModifier);
    view.keyReleaseEvent(&event);
    QCOMPARE(spy.count(), 1);
}

void QDropListViewTest::testDragEnterEvent() {
    QDropListView view;
    QSignalSpy spy(&view, &QDropListView::dragEnter);
    QMimeData mime;
    QDragEnterEvent event(QPoint(0, 0), Qt::CopyAction, &mime, Qt::LeftButton, Qt::NoModifier);
    view.dragEnterEvent(&event);
    QCOMPARE(spy.count(), 1);
}

void QDropListViewTest::testDragMoveEvent() {
    QDropListView view;
    QSignalSpy spy(&view, &QDropListView::dragMove);
    QMimeData mime;
    QDragMoveEvent event(QPoint(0, 0), Qt::CopyAction, &mime, Qt::LeftButton, Qt::NoModifier);
    view.dragMoveEvent(&event);
    QCOMPARE(spy.count(), 1);
}

void QDropListViewTest::testDragLeaveEvent() {
    QDropListView view;
    QSignalSpy spy(&view, &QDropListView::dragLeave);
    QDragLeaveEvent event;
    view.dragLeaveEvent(&event);
    QCOMPARE(spy.count(), 1);
}

void QDropListViewTest::testDropEvent() {
    QDropListView view;
    QSignalSpy spy(&view, &QDropListView::drop);
    QMimeData mime;
    QDropEvent event(QPoint(0, 0), Qt::CopyAction, &mime, Qt::LeftButton, Qt::NoModifier);
    view.dropEvent(&event);
    QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(QDropListViewTest)

#include "qdroplistviewtest.moc"
