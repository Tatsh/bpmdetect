#include "track/soundtouchbpmdetector.h"
#include "widgets/dlgbpmdetect.h"

int guiMain(const QApplication &app, const QStringList &files) {
    DlgBpmDetect mainWin;
    auto detector = new SoundTouchBpmDetector();
    mainWin.setDetector(detector);
    mainWin.slotAddFiles(files);
    mainWin.show();
    auto ret = app.exec();
    delete detector;
    return ret;
}
