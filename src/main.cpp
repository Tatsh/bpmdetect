// SPDX-License-Identifier: GPL-3.0-or-later
#include <cstdlib>
#include <iostream>
#include <string>

#include <getopt.h>

#include "track/trackproxy.h"

#ifndef NO_GUI
#include <QApplication>

#include "widgets/dlgbpmdetect.h"
#endif

using namespace std;

const char *version = "0.7.2"; // App version

void display_help() {
    printf("BPM Detect version %s\n\n", version);
    printf("Usage:\n bpmdetect [switches] [files]\n\n");
    printf("Switches:\n");
#ifndef NO_GUI
    printf("-c --console         - run in console mode\n");
#endif
    printf("-h --help            - show this help\n"
           "-s --save            - save BPMs to tags\n"
           "-d --detect          - redetect (do not print BPMs stored in tags)\n"
           "-r --remove          - remove stored BPMs from tags\n"
           "-p --noprogress      - disable progress display (console)\n"
           "-n --min <value>     - minimum BPM (default 80)\n"
           "-x --max <value>     - maximum BPM (default 185)\n"
           "-l --limit           - do not return BPM above the range <min, max>\n"
           "-f --format <format> - set BPM format (default is 0.00)\n"
           "\n");
}

int main(int argc, char *argv[]) {
    bool console = false, redetect = false, bpmsave = false, clear = false, bformat = false,
         progress = true;
    QString format;

    static struct option long_options[] = {
#ifndef NO_GUI
        {"console", no_argument, nullptr, 'c'},
#endif
        {"format", required_argument, nullptr, 'f'},
        {"save", no_argument, nullptr, 's'},
        {"detect", no_argument, nullptr, 'd'},
        {"remove", no_argument, nullptr, 'r'},
        {"noprogress", no_argument, nullptr, 'p'},
        {"min", required_argument, nullptr, 'n'},
        {"max", required_argument, nullptr, 'x'},
        {"limit", no_argument, nullptr, 'l'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, 0}};

    while (true) {
        int option_index = 0;
        int c, val;
        c = getopt_long(argc, argv, "csdrphf:n:x:l", long_options, &option_index);
        if (c < 0)
            break;
        switch (c) {
        case 's':
            bpmsave = true;
            break;
        case 'd':
            redetect = true;
            break;
        case 'h':
            display_help();
            return 0;
        case 'r':
            clear = true;
            // do not start GUI, just remove BPMs
            console = true;
            break;
        case 'p':
            progress = false;
            break;
        case 'f':
            bformat = true;
            format = QString::fromUtf8(optarg);
            break;
        case 'n':
            val = std::stoi(std::string(optarg));
            Track::setMinBPM(val);
#ifdef DEBUG
            cerr << "Min BPM set to " << Track::getMinBPM() << endl;
#endif
            break;
        case 'x':
            val = std::stoi(std::string(optarg));
            Track::setMaxBPM(val);
#ifdef DEBUG
            cerr << "Max BPM set to " << Track::getMaxBPM() << endl;
#endif
            break;
        case 'l':
            Track::setLimit(true);
            break;
#ifndef NO_GUI
        case 'c':
            console = true;
            break;
#endif
        case '?':
        default:
            display_help();
            return 0;
        }
    }

#ifdef NO_GUI
    console = true;
    if (argc - optind < 1)
#else
    QStringList filelist;
    if (console && argc - optind < 1)
#endif
    { // no files passed
        display_help();
        return 0;
    }

    for (int idx = optind; idx < argc; idx++) {
        if (console) {
            if (optind != argc - 1)
                qDebug() << "[" << idx + 1 - optind << "/" << argc - optind << "] " << argv[idx];
            TrackProxy track(QString::fromUtf8(argv[idx]));
            if (!clear) {
                track.enableConsoleProgress(progress);
                track.setRedetect(redetect);
                if (bformat)
                    track.setFormat(format);
                track.detectBPM();
                if (bpmsave)
                    track.saveBPM();
                track.printBPM();
            } else {
                track.clearBPM();
            }
        } else {
#ifndef NO_GUI
            filelist += QString::fromUtf8(argv[idx]);
#endif // NO_GUI
        }
    }

#ifndef NO_GUI
    if (!console) {
        QApplication app(argc, argv);
        app.setStyle(QStringLiteral("plastique"));
        DlgBPMDetect *mainWin = new DlgBPMDetect();
        mainWin->show();
        mainWin->slotAddFiles(filelist);
        app.exec();
        delete mainWin;
        mainWin = nullptr;
    }
#endif // NO_GUI
    return 0;
}
