// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QKeyEvent>
#include <QTreeWidget>

class QDropListView : public QTreeWidget {
    Q_OBJECT
public:
    QDropListView(QWidget *parent = nullptr);
    ~QDropListView() override;

public Q_SLOTS:
    /// Remove selected items from list
    void slotRemoveSelected();

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void dragLeaveEvent(QDragLeaveEvent *e) override;
    void dropEvent(QDropEvent *e) override;

    Q_SIGNAL void keyPress(QKeyEvent *e);
    Q_SIGNAL void keyRelease(QKeyEvent *e);
    Q_SIGNAL void dragEnter(QDragEnterEvent *e);
    Q_SIGNAL void dragMove(QDragMoveEvent *e);
    Q_SIGNAL void dragLeave(QDragLeaveEvent *e);
    Q_SIGNAL void drop(QDropEvent *e);
};
