#pragma once

#include <QPainter>
#include <QPainterPath>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QTimer>
#include <QWidget>
#include <QtMath>

class BannerWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal animationProgress1 READ animationProgress1 WRITE setAnimationProgress1)
    Q_PROPERTY(qreal animationProgress2 READ animationProgress2 WRITE setAnimationProgress2)
    Q_PROPERTY(qreal animationProgress3 READ animationProgress3 WRITE setAnimationProgress3)
    Q_PROPERTY(qreal backgroundProgress READ backgroundProgress WRITE setBackgroundProgress)

  public:
    explicit BannerWidget(const QString &text, QWidget *parent = nullptr);

    qreal animationProgress1() const { return m_animationProgress1; }
    void setAnimationProgress1(qreal progress);

    qreal animationProgress2() const;
    void setAnimationProgress2(qreal progress);

    qreal animationProgress3() const { return m_animationProgress3; }
    void setAnimationProgress3(qreal progress);

    qreal backgroundProgress() const { return m_backgroundProgress; }
    void setBackgroundProgress(qreal progress);

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    void drawAnimatedLine(QPainter &painter, qreal yFactor, qreal progress, int direction);

    QString m_text;
    qreal m_animationProgress1;
    qreal m_animationProgress2;
    qreal m_animationProgress3;
    qreal m_backgroundProgress;
};
