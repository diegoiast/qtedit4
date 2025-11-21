#include "CommitDelegate.hpp"

#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QVariant>

QColor CommitDelegate::laneColor(int column) const {
    static QList<QColor> palette = {
        QColor("#d9534f"), // red
        QColor("#5bc0de"), // cyan
        QColor("#5cb85c"), // green
        QColor("#f0ad4e"), // orange
        QColor("#0275d8"), // blue
        QColor("#7952b3")  // purple
    };
    return palette[column % palette.size()];
}

CommitDelegate::CommitDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

QSize CommitDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &) const {
    auto h = qMax(20, m_columnWidth + m_nodeRadius * 2 + 4);
    return QSize(option.rect.width(), h);
}

void CommitDelegate::paintBezier(QPainter *p, QPoint start, QPoint end, const QColor &color) const {
    auto path = QPainterPath(start);
    auto c1 = QPoint(start.x(), (start.y() + end.y()) / 2);
    auto c2 = QPoint(end.x(), (start.y() + end.y()) / 2);
    path.cubicTo(c1, c2, end);
    p->setPen(QPen(color, 1.8));
    p->drawPath(path);
}

void CommitDelegate::paint(QPainter *p, const QStyleOptionViewItem &option,
                           const QModelIndex &index) const {
    // TODO: or compute max from model
    const int laneCount = 6;
    const int graphWidth = laneCount * m_columnWidth;

    auto o = QStyleOptionViewItem(option);
    o.rect = option.rect.adjusted(graphWidth + 6, 0, 0, 0);

    QStyledItemDelegate::paint(p, o, index);
    p->save();
    p->setRenderHint(QPainter::Antialiasing, true);

    auto col = index.data(Qt::UserRole + 1).toInt();
    auto parents = index.data(Qt::UserRole + 2).value<QVector<int>>();
    auto x0 = option.rect.x(); // left of the whole row
    auto xCenter = x0 + col * m_columnWidth + m_columnWidth / 2;
    auto yCenter = option.rect.center().y();
    p->setPen(QPen(laneColor(col).darker(140), 1.4));
    p->drawLine(QPoint(xCenter, option.rect.top()), QPoint(xCenter, option.rect.bottom()));
    for (int pc : parents) {
        auto px = x0 + pc * m_columnWidth + m_columnWidth / 2;
        paintBezier(p, QPoint(xCenter, yCenter + m_nodeRadius),
                    QPoint(px, option.rect.bottom() + m_nodeRadius), laneColor(pc));
    }

    p->setBrush(laneColor(col));
    p->setPen(Qt::NoPen);
    p->drawEllipse(QPoint(xCenter, yCenter), m_nodeRadius, m_nodeRadius);
    p->restore();
}
