#include "AutoShrinkLabel.hpp"

#include <QEvent>
#include <QPainter>
#include <QStringListModel>
#include <QStyleOption>

AutoShrinkLabel::AutoShrinkLabel(QWidget *parent) : QLabel(parent) {}

void AutoShrinkLabel::setPrimaryText(const QString &text) {
    m_primary = text;
    update();
}

void AutoShrinkLabel::setFallbackText(const QString &text) {
    m_fallback = text;
    update();
}

void AutoShrinkLabel::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setFont(font());
    QFontMetrics fm(font());
    QString toDraw = (fm.horizontalAdvance(m_primary) <= width()) ? m_primary : m_fallback;
    QRect r = rect();
    painter.drawText(r, alignment(), toDraw);
}

ElidedLabel::ElidedLabel(QWidget *parent) : QLabel(parent) {
    setAttribute(Qt::WA_WState_OwnSizePolicy);
}

ElidedLabel::ElidedLabel(const QString &text, QWidget *parent) : QLabel(text, parent) {
    setAttribute(Qt::WA_WState_OwnSizePolicy);
}

QSize ElidedLabel::sizeHint() const {
    if (wordWrap() || textInteractionFlags() != Qt::NoTextInteraction) {
        return QLabel::sizeHint();
    }

    QFontMetrics fm(font());
    int w = fm.horizontalAdvance(text());
    w += contentsMargins().left() + contentsMargins().right();
    return QSize(w, QLabel::sizeHint().height());
}

QSize ElidedLabel::minimumSizeHint() const {
    if (wordWrap() || textInteractionFlags() != Qt::NoTextInteraction) {
        return QLabel::minimumSizeHint();
    }

    // Allow shrinking down to ~20 pixels (or whatever you want)
    return QSize(20, QLabel::minimumSizeHint().height());
}

void ElidedLabel::paintEvent(QPaintEvent *) {
    if (wordWrap() || textInteractionFlags() != Qt::NoTextInteraction) {
        QLabel::paintEvent(nullptr);
        return;
    }

    const int w = width() - contentsMargins().left() - contentsMargins().right();
    if (m_cacheValid == false || m_cachedWidth != w || m_cachedText != text()) {
        QFontMetrics fm(font());
        m_cachedElided = fm.elidedText(text(), Qt::ElideRight, std::max(0, w));
        m_cachedWidth = w;
        m_cachedText = text();
        m_cacheValid = true;
    }

    QPainter painter(this);
    QStyleOption opt;
    opt.initFrom(this);
    style()->drawItemText(&painter, contentsRect(), alignment(), palette(), isEnabled(),
                          m_cachedElided, QPalette::WindowText);
}

void ElidedLabel::resizeEvent(QResizeEvent *) {
    m_cacheValid = false;
    QLabel::resizeEvent(nullptr);
}

bool ElidedLabel::event(QEvent *e) {
    if (e->type() == QEvent::FontChange || e->type() == QEvent::StyleChange ||
        e->type() == QEvent::PaletteChange || e->type() == QEvent::ApplicationPaletteChange) {
        m_cacheValid = false;
    }
    return QLabel::event(e);
}
