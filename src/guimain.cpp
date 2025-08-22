#include "track/soundtouchbpmdetector.h"
#include "widgets/dlgbpmdetect.h"

int guiMain(const QApplication &app, const QStringList &files) {
    DlgBpmDetect mainWin;
    mainWin.setDetector(new SoundTouchBpmDetector(qApp));
    mainWin.slotAddFiles(files);
    mainWin.show();
    return app.exec();
}
