// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#pragma once

#include <QMouseEvent>
#include <QProgressBar>
#include <QWidget>

/// @brief Progressbar controlled by mouse
class ProgressBar : public QProgressBar {
    Q_OBJECT

public:
    /// Constructor
    ProgressBar(QWidget *parent = nullptr);
    /// Destructor
    ~ProgressBar() override;
    /// Return change status
    bool change();
    /// Return enabled status
    bool enabled();
    /// Return total steps (track length)
    uint length();

public Q_SLOTS:
    /// Set current position
    void setPosition(uint pos);
    /// Set total steps
    void setLength(uint len);
    /// Set enabled status
    void setEnabled(bool s);

protected Q_SLOTS:
    /// Receiving mouse press events
    void mousePressEvent(QMouseEvent *e) override;
    /// Receiving mouse move events
    void mouseMoveEvent(QMouseEvent *e) override;
    /// Receiving mouse release events
    void mouseReleaseEvent(QMouseEvent *e) override;
    /// Set change status
    void setChange(bool s);

protected:
    /// Current position changed
    Q_SIGNAL void positionChanged(uint pos);

private:
    bool chng;   ///< true if left mouse button is down
    bool enable; ///< enable status
};
