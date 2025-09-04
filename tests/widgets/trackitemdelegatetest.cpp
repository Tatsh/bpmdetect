#include <QtGui/QStandardItemModel>
#include <QtTest>

#include "widgets/trackitemdelegate.h"

class TrackItemDelegateTest : public QObject {
    Q_OBJECT
public:
    explicit TrackItemDelegateTest(QObject *parent = nullptr);
    ~TrackItemDelegateTest() override;
private Q_SLOTS:
    void testCreateEditorReturnsNullForNonBpmColumn();
    void testCreateEditorReturnsEditorForBpmColumn();
};

TrackItemDelegateTest::TrackItemDelegateTest(QObject *parent) : QObject(parent) {
}

TrackItemDelegateTest::~TrackItemDelegateTest() {
}

void TrackItemDelegateTest::testCreateEditorReturnsNullForNonBpmColumn() {
    TrackItemDelegate delegate;
    QStandardItemModel model;
    model.setColumnCount(3);
    model.setRowCount(1);
    QModelIndex index = model.index(0, 1); // Not BPM column.
    auto editor = delegate.createEditor(nullptr, QStyleOptionViewItem(), index);
    QCOMPARE(editor, nullptr);
}

void TrackItemDelegateTest::testCreateEditorReturnsEditorForBpmColumn() {
    TrackItemDelegate delegate;
    QStandardItemModel model;
    model.setColumnCount(3);
    model.setRowCount(1);
    QModelIndex index = model.index(0, 0); // BPM column.
    auto editor = delegate.createEditor(nullptr, QStyleOptionViewItem(), index);
    QVERIFY(editor != nullptr);
    delete editor;
}

QTEST_MAIN(TrackItemDelegateTest)

#include "trackitemdelegatetest.moc"
