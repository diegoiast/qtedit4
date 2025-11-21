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

  private:
    struct ParentInfo {
        int column;
        QColor color;
    };

    int m_columnWidth = 18;
    int m_nodeRadius = 4;

    QColor laneColor(int column) const;
    void paintBezier(QPainter *p, QPoint start, QPoint end, const QColor &color) const;
};
