// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Diego Iastrubni

#pragma once

#include <QChildEvent>
#include <QPainter>
#include <QPen>
#include <QSplitter>
#include <QTabBar>
#include <QTabWidget>
#include <QWidget>

class QTabWidget;
class SplitTabWidget;

class DropIndicatorWidget : public QWidget {
    Q_OBJECT
  public:
    explicit DropIndicatorWidget(QWidget *parent = nullptr);
    void showAt(const QRect &rect, bool after);

  protected:
    void paintEvent(QPaintEvent *) override;

  private:
    QRect m_rect;
    bool m_after = false;
};

class DraggableTabBar : public QTabBar {
    Q_OBJECT

  public:
    explicit DraggableTabBar(QWidget *parent = nullptr);
    void dropEvent(QDropEvent *event) override;
    void setDragAndDropEnabled(bool enabled) { dragAndDropEnabled = enabled; }

  signals:
    void emptyAreaDoubleClicked(const QPoint &pos);

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

  private:
    QPoint dragStartPos;
    DropIndicatorWidget *dropIndicator;
    bool dragAndDropEnabled = false;
};

class DraggableTabWidget : public QTabWidget {
    Q_OBJECT
  public:
    explicit DraggableTabWidget(QWidget *parent = nullptr);
    void dropEvent(QDropEvent *event) override;

  signals:
    void tabWidgetRemoved();

  protected:
    // void mousePressEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void tabRemoved(int index) override;
};

class SplitterWithWidgetAdded : public QSplitter {
    Q_OBJECT

  public:
    SplitterWithWidgetAdded(Qt::Orientation orientation, QWidget *parent = nullptr);

  signals:
    void widgetAdded(QWidget *widget);

  protected:
    void childEvent(QChildEvent *event) override;
};

// Implementation

class ButtonsProvider {
  public:
    virtual QWidget *requestButton(bool first, int tabSize, SplitTabWidget *split) = 0;
};

class SplitTabWidget : public QWidget {
    Q_OBJECT

  public:
    explicit SplitTabWidget(QWidget *parent = nullptr);

    void addTab(QWidget *widget, const QString &label, const QString &tooltip = {});
    void splitHorizontally();
    void closeCurrentSplit();
    void closeSplitWithTabWidget(QTabWidget *tab);
    void addTabToCurrentSplit(QWidget *widget, const QString &label, const QString &tooltip = {});
    void addTabToSplit(int splitNumber, QWidget *widget, const QString &label,
                       const QString &tooltip = {});
    int findSplitIndex(QTabWidget *);
    void moveTabToNewSplit(QWidget *widget);
    void closeCurrentTab();
    void moveNextTab();
    void movePrevTab();
    inline QList<int> getSplitSizes() const { return splitter->sizes(); }
    QList<int> getSplitInternalCount() const;

    void setButtonProvider(ButtonsProvider *newProvider);
    inline ButtonsProvider *getButtonProvider() const { return buttonsProvider; }
    QWidget *getCurrentWidget();
    int getWigetsCountInCurrentSplit() const;
    inline int getSplitCount() const { return splitter->count(); }

    virtual void onTabFocusChanged(QWidget *widget, bool focused);
    virtual void onNewSplitCreated(QTabWidget *tabWidget, int count);

    bool closeSplitWhenEmpty = true;

    void updateCurrentTabWidget(QTabWidget *newCurrent);

  signals:
    void emptyAreaDoubleClicked(QTabWidget *tabWidget, const QPoint &pos);

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

    SplitterWithWidgetAdded *splitter = nullptr;
    QTabWidget *currentTabWidget = nullptr;
    ButtonsProvider *buttonsProvider = nullptr;
    void equalizeWidths();

  private slots:
    void onSplitCountMaybeChanged();

  public:
    QList<int> savedSplitCount;
    QList<int> savedSplitInternalSizes;
};
