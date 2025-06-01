// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Diego Iastrubni

#pragma once

#include <QChildEvent>
#include <QSplitter>
#include <QTabBar>
#include <QTabWidget>
#include <QWidget>

class QTabWidget;
class SplitTabWidget;

class DraggableTabBar : public QTabBar {
    Q_OBJECT

  public:
    DraggableTabBar(QWidget *parent = nullptr);

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

  private:
    QPoint dragStartPos;
};

class DraggableTabWidget : public QTabWidget {
    Q_OBJECT
  public:
    DraggableTabWidget(QWidget *parent = nullptr);

  protected:
    // void mousePressEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
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

    virtual void onTabFocusChanged(QWidget *widget, bool focused);
    virtual void onNewSplitCreated(QTabWidget *tabWidget, int count);

    bool closeSplitWhenEmpty = true;

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

    SplitterWithWidgetAdded *splitter = nullptr;
    QTabWidget *currentTabWidget = nullptr;
    ButtonsProvider *buttonsProvider = nullptr;
    void updateCurrentTabWidget(QTabWidget *newCurrent);
    void equalizeWidths();

  public:
    QList<int> savedSplitCount;
    QList<int> savedSplitInternalSizes;
};
