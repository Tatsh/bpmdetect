#include <QtWidgets/QStyledItemDelegate>

class TrackItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    TrackItemDelegate(QObject *parent = nullptr);
    ~TrackItemDelegate() override;
    virtual QWidget *createEditor(QWidget *parent,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const override;

Q_SIGNALS:
    /** Event for when editing began. */
    void editingStarted(const QModelIndex &index) const;
};
