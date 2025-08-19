// SPDX-License-Identifier: GPL-3.0-or-later
/** @file */
#pragma once

#include <QtCore/QCommandLineParser>
#include <QtCore/QCoreApplication>
#include <QtCore/QStringList>

/**
 * Console entry point.
 * @param parser Command line parser.
 * @param files List of files to process.
 * @return Exit code of the application.
 */
int consoleMain(QCoreApplication &app, QCommandLineParser &parser, const QStringList &files);
