#pragma once

#include <QPainter>
#include <QStyledItemDelegate>

class CommitDelegate : public QStyledItemDelegate {
    Q_OBJECT
  public:
    explicit CommitDelegate(QObject *parent = nullptr);

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    static QColor laneColor(int column);

  private:
};
