// SPDX-License-Identifier: GPL-3.0-or-later
/** @file */
#pragma once

#include <QtWidgets/QTreeWidget>

class QDragEnterEvent;
class QDragLeaveEvent;
class QDragMoveEvent;
class QDropEvent;
class QKeyEvent;

/** Custom widget to allow for dropping files. */
class QDropListView : public QTreeWidget {
    Q_OBJECT
#ifdef TESTING
    friend class QDropListViewTest;
#endif
public:
    /**
     * Constructor.
     * @param parent Parent widget.
     */
    explicit QDropListView(QWidget *parent = nullptr);
    ~QDropListView() override;
    /** Signal for key press event. */
    Q_SIGNAL void keyPress(QKeyEvent *e);
    /** Signal for key release event. */
    Q_SIGNAL void keyRelease(QKeyEvent *e);
    /** Signal for drag enter event. */
    Q_SIGNAL void dragEnter(QDragEnterEvent *e);
    /** Signal for drag move event. */
    Q_SIGNAL void dragMove(QDragMoveEvent *e);
    /** Signal for drag leave event. */
    Q_SIGNAL void dragLeave(QDragLeaveEvent *e);
    /** Signal for drop event. */
    Q_SIGNAL void drop(QDropEvent *e);

public Q_SLOTS:
    /** Remove selected items from list. */
    void slotRemoveSelected();

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void dragLeaveEvent(QDragLeaveEvent *e) override;
    void dropEvent(QDropEvent *e) override;
};
