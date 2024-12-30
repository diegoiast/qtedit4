#include "bannerwidget.h"

BannerWidget::BannerWidget(const QString &text, QWidget *parent)
    : QWidget(parent), m_text(text), m_animationProgress1(0), m_animationProgress2(0),
      m_animationProgress3(0), m_backgroundProgress(0) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFixedHeight(100);

    QParallelAnimationGroup *animGroup = new QParallelAnimationGroup(this);

    QPropertyAnimation *anim1 = new QPropertyAnimation(this, "animationProgress1");
    anim1->setDuration(16500);
    anim1->setStartValue(0.0);
    anim1->setEndValue(1.0);
    anim1->setEasingCurve(QEasingCurve::OutBounce);

    QPropertyAnimation *anim2 = new QPropertyAnimation(this, "animationProgress2");
    anim2->setDuration(5000);
    anim2->setStartValue(1.0);
    anim2->setEndValue(0.0);
    anim2->setEasingCurve(QEasingCurve::InOutSine);

    QPropertyAnimation *anim3 = new QPropertyAnimation(this, "animationProgress3");
    anim3->setDuration(19000);
    anim3->setStartValue(0.0);
    anim3->setEndValue(1.0);
    anim3->setEasingCurve(QEasingCurve::OutInSine);

    QPropertyAnimation *backgroundAnim = new QPropertyAnimation(this, "backgroundProgress");
    backgroundAnim->setDuration(10000);
    backgroundAnim->setStartValue(0.0);
    backgroundAnim->setEndValue(1.0);
    backgroundAnim->setEasingCurve(QEasingCurve::InOutQuad);

    animGroup->addAnimation(anim1);
    animGroup->addAnimation(anim2);
    animGroup->addAnimation(anim3);
    animGroup->addAnimation(backgroundAnim);

    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [animGroup]() {
        animGroup->setDirection(animGroup->direction() == QAbstractAnimation::Forward
                                    ? QAbstractAnimation::Backward
                                    : QAbstractAnimation::Forward);
        animGroup->start();
    });
    timer->start(19100);
    animGroup->start();
}

void BannerWidget::setAnimationProgress1(qreal progress) {
    m_animationProgress1 = progress;
    update();
}

qreal BannerWidget::animationProgress2() const { return m_animationProgress2; }

void BannerWidget::setAnimationProgress2(qreal progress) {
    m_animationProgress2 = progress;
    update();
}

void BannerWidget::setAnimationProgress3(qreal progress) {
    m_animationProgress3 = progress;
    update();
}

void BannerWidget::setBackgroundProgress(qreal progress) {
    m_backgroundProgress = progress;
    update();
}

void BannerWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    float h, s, v, a;
    auto baseColor = palette().color(QPalette::Highlight);
    auto hueVariation = 0.1f;
    baseColor.getHsvF(&h, &s, &v, &a);
    auto color1 = QColor::fromHsvF(std::clamp(h + m_backgroundProgress * hueVariation, 0.0, 1.0),
                                   s * 0.7, std::min(v * 1.0, 1.0), a);
    auto color2 = QColor::fromHsvF(std::clamp(h - m_backgroundProgress * hueVariation, 0.0, 1.0),
                                   s * 0.7, std::max(v * 0.85, 0.0), a);
    auto gradient = QLinearGradient(0, 0, width(), height());
    auto font = QFont("Arial", 24, QFont::Bold);
    auto linePen = QPen(Qt::white);

    gradient.setColorAt(0, color1);
    gradient.setColorAt(1, color2);
    painter.fillRect(rect(), gradient);
    painter.setFont(font);

    painter.setPen(QColor(0, 0, 0, 0));
    painter.translate(-1, -1);
    painter.drawText(rect(), Qt::AlignCenter, m_text);

    painter.setPen(QColor(255, 255, 255, 230));
    painter.translate(+2, +2);
    painter.drawText(rect(), Qt::AlignCenter, m_text);

    linePen.setWidth(3);
    linePen.setCapStyle(Qt::RoundCap);
    painter.setPen(linePen);
    painter.setOpacity(0.3);

    drawAnimatedLine(painter, 0.8, m_animationProgress1, 1);
    drawAnimatedLine(painter, 0.9, m_animationProgress2, -1);
    drawAnimatedLine(painter, 1.0, m_animationProgress3, 1);
}

void BannerWidget::drawAnimatedLine(QPainter &painter, qreal yFactor, qreal progress,
                                    int direction) {
    auto y = height() * yFactor;
    auto amplitude = height() * 0.1;
    auto frequency = 2 * M_PI / width();

    QPainterPath path;
    path.moveTo(0, y);
    for (auto x = 0; x <= width(); ++x) {
        auto animProgress = progress * 2 * M_PI;
        auto yOffset = amplitude * qSin(frequency * x + direction * animProgress);
        path.lineTo(x, y + yOffset);
    }
    painter.drawPath(path);
}
