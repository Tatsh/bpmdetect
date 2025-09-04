#include <QtWidgets/QStyledItemDelegate>

class TrackItemDelegate : public QStyledItemDelegate {
public:
    TrackItemDelegate(QObject *parent = nullptr);
    ~TrackItemDelegate() override;
    virtual QWidget *createEditor(QWidget *parent,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const override;
};
