// SPDX-License-Identifier: GPL-3.0-or-later
/** @file */
#pragma once

#include <QtCore/QStringList>
#include <QtWidgets/QApplication>

/**
 * GUI entry point.
 * @param app Application instance.
 * @param files List of files to add to the view.
 * @return Exit code of the application.
 */
int guiMain(const QApplication &app, const QStringList &files);
