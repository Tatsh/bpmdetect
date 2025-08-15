// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QProgressBar>

class QMouseEvent;

/** Progress bar controlled by mouse. */
class ProgressBar : public QProgressBar {
    Q_OBJECT

public:
    ProgressBar(QWidget *parent = nullptr);
    ~ProgressBar() override;
    /** Return change status. */
    bool change();
    /** Return enabled status. */
    bool enabled();
    /** Return total steps (track length). */
    int length();
    /** Current position changed signal. */
    Q_SIGNAL void positionChanged(int pos);

public Q_SLOTS:
    /** Set current position to @a pos. */
    void setPosition(int pos);
    /** Set total steps to @a len. */
    void setLength(int len);
    /** Set enabled status. */
    void setEnabled(bool s);

protected Q_SLOTS:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void setChange(bool s);

private:
    bool bChange;
    bool enable;
};
