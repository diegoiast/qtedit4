#include <algorithm>

#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>
#include <QVariant>
#include <qnamespace.h>
#include <qtypes.h>

#include "CommitDelegate.hpp"
#include "CommitModel.hpp"

namespace {
void drawGraphLine(QPainter *p, const QString &graphString, int laneWidth, int leftMargin,
                   int yTop, int yBottom, int totalLines) {
    if (graphString.isEmpty()) {
        return;
    }

    auto circleSize = 10;
    auto dotRadius = circleSize / 2;
    auto yCenter = yTop + (yBottom - yTop) / 2;
    p->setRenderHint(QPainter::Antialiasing);
    for (auto i = 0; i < graphString.length(); ++i) {
        auto c = graphString.at(i);
        auto xCenter = leftMargin + i * laneWidth + laneWidth / 2.0;

        if (c == '*') {
            auto yPos = totalLines > 1 ? yTop : yCenter;
            p->setPen(Qt::NoPen);
            p->setBrush(CommitDelegate::laneColor(i));
            p->drawEllipse(QPointF(xCenter, yPos), dotRadius, dotRadius);
        } else if (c == '|') {
            p->setPen(QPen(CommitDelegate::laneColor(i), 2.4, Qt::SolidLine, Qt::RoundCap,
                           Qt::RoundJoin));
            p->drawLine(QPointF(xCenter, yTop), QPointF(xCenter, yBottom));
        } else if (c == '/') {
        } else if (c == '\\') {
        }
    }
}

} // namespace

QColor CommitDelegate::laneColor(int column) {
    static QList<QColor> palette = {
        QColor("#d9534f"), // red
        QColor("#5bc0de"), // cyan
        QColor("#5cb85c"), // green
        QColor("#f0ad4e"), // orange
        QColor("#0275d8"), // blue
        QColor("#7952b3"), // purple
    };
    return palette[column % palette.size()];
}

CommitDelegate::CommitDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

QSize CommitDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &) const {
    auto fm = QFontMetrics(option.font);
    auto rowHeight = fm.height() + 8;
    return QSize(option.rect.width(), rowHeight);
}

void CommitDelegate::paint(QPainter *p, const QStyleOptionViewItem &option,
                           const QModelIndex &index) const {
    p->save();

    auto style = option.widget ? option.widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, p, option.widget);

    auto graph = index.data(CommitModel::GraphRole).toStringList();
    if (graph.size() != 0) {
        auto text = index.data(Qt::DisplayRole).toString();
        auto rowRect = option.rect;
        auto fm = QFontMetrics(option.font);
        auto totalH = rowRect.height();
        auto lineCount = graph.size();
        auto lineHeight = totalH / std::max(qsizetype(1), lineCount); // avoid division by zero
        auto y = rowRect.y();
        auto maxGraphPixelWidth = 0;

        auto laneWidth = 10;
        auto leftMargin = 5;
        for (auto const &line : graph) {
            drawGraphLine(p, line, laneWidth, leftMargin, y, y + lineHeight, lineCount);
            auto linePixelWidth = int(line.size()) * laneWidth + leftMargin;
            maxGraphPixelWidth = std::max(maxGraphPixelWidth, linePixelWidth);
            y += lineHeight;
        }

        auto textRect = rowRect;
        textRect.setLeft(rowRect.left() + maxGraphPixelWidth + 5); // padding after graph

        auto textHeight = fm.height();
        auto yOffset = (textRect.height() - textHeight) / 2;
        textRect.setTop(textRect.top() + yOffset);
        textRect.setHeight(textHeight);

        auto cg = (option.state & QStyle::State_Enabled) ? QPalette::Normal : QPalette::Disabled;
        if (cg == QPalette::Normal && !(option.state & QStyle::State_Active)) {
            cg = QPalette::Inactive;
        }
        if (option.state & QStyle::State_Selected) {
            p->setPen(option.palette.color(cg, QPalette::HighlightedText));
        } else {
            p->setPen(option.palette.color(cg, QPalette::Text));
        }

        style->drawItemText(p, textRect, option.displayAlignment | Qt::AlignLeft, option.palette,
                            option.state & QStyle::State_Enabled, text, QPalette::Text);
    }

    if (option.state & QStyle::State_HasFocus) {
        QStyleOptionFocusRect fr;
        fr.rect = option.rect;
        fr.state = option.state;
        fr.backgroundColor = option.palette.color(QPalette::Window);
        style->drawPrimitive(QStyle::PE_FrameFocusRect, &fr, p, option.widget);
    }

    p->restore();
}
