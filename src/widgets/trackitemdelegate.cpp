#include <QtWidgets/QLineEdit>

#include "trackitemdelegate.h"

TrackItemDelegate::TrackItemDelegate(QObject *parent) : QStyledItemDelegate(parent) {
}

TrackItemDelegate::~TrackItemDelegate() = default;

QWidget *TrackItemDelegate::createEditor(QWidget *parent,
                                         const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const {
    Q_UNUSED(option)
    if (index.column() > 0) {
        return nullptr;
    }
    auto editor = new QLineEdit(parent);
    auto validator = new QDoubleValidator(0, 999.99, 2, editor);
    editor->setValidator(validator);
    emit editingStarted(index);
    return editor;
}
