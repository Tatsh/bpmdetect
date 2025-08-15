#include "utils.h"

double stringToBpm(const QString &sBPM) {
    auto bpm = sBPM.toDouble();
    while (bpm > 300)
        bpm = bpm / 10;
    return bpm;
}

QString bpmToString(bpmtype dBPM, const QString &format) {
    static const auto zero = QChar::fromLatin1('0');
    if (format == QStringLiteral("0.0")) {
        return QString::number(dBPM, 'f', 1);
    } else if (format == QStringLiteral("0")) {
        return QString::number(dBPM, 'd', 0);
    } else if (format == QStringLiteral("000.00")) {
        return QString::number(dBPM, 'f', 2).rightJustified(6, zero);
    } else if (format == QStringLiteral("000.0")) {
        return QString::number(dBPM, 'f', 1).rightJustified(5, zero);
    } else if (format == QStringLiteral("000")) {
        return QString::number(dBPM, 'd', 0).rightJustified(3, zero);
    } else if (format == QStringLiteral("00000")) {
        return QString::number(dBPM, 'd', 0).rightJustified(5, zero);
    }
    // all other formats are converted to "0.00"
    return QString::number(dBPM, 'f', 2);
}
