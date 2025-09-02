// SPDX-License-Identifier: GPL-3.0-or-later
/** @file */
#pragma once

#include <QtWidgets/QProgressBar>

class QMouseEvent;

/** Progress bar controlled by mouse. */
class ProgressBar : public QProgressBar {
    Q_OBJECT
#ifdef TESTING
    friend class ProgressBarTest;
#endif
public:
    explicit ProgressBar(QWidget *parent = nullptr);
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
    /** Set change state. */
    void setChange(bool change);

private:
    bool change_;
    bool enabled_;
};
