// SPDX-License-Identifier: GPL-3.0-or-later
/** @file */
#pragma once

#include <QtCore/QCommandLineParser>
#include <QtCore/QStringList>

#include "track/trackproxy.h"

/**
 * Console entry point.
 * @param parser Command line parser.
 * @param files List of files to process.
 */
void consoleMain(QCommandLineParser &parser,
                 const QStringList &files,
                 TrackProxyFactory proxyFactory = trackProxyFactory);
