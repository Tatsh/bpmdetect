#include "widgets/dlgbpmdetect.h"

void guiMain(const QApplication &app, const QStringList &files) {
    DlgBpmDetect mainWin;
    mainWin.show();
    mainWin.slotAddFiles(files);
    app.exec();
}
