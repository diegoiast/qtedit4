#pragma once

#include <QLabel>

class AutoShrinkLabel : public QLabel {
  public:
    explicit AutoShrinkLabel(QWidget *parent = nullptr);
    void setPrimaryText(const QString &text);
    void setFallbackText(const QString &text);

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    QString m_primary;
    QString m_fallback;
};

class ElidedLabel final : public QLabel {
    Q_OBJECT
  public:
    explicit ElidedLabel(QWidget *parent = nullptr);
    ElidedLabel(const QString &text, QWidget *parent = nullptr);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

  protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    bool event(QEvent *e) override;

  private:
    QString m_cachedElided;
    QString m_cachedText;
    int m_cachedWidth = -1;
    bool m_cacheValid = false;
};
