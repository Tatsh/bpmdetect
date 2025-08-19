#include "track/soundtouchbpmdetector.h"
#include "widgets/dlgbpmdetect.h"

void guiMain(const QApplication &app, const QStringList &files) {
    DlgBpmDetect mainWin;
    auto detector = new SoundTouchBpmDetector();
    mainWin.setDetector(detector);
    mainWin.slotAddFiles(files);
    mainWin.show();
    app.exec();
    delete detector;
}
