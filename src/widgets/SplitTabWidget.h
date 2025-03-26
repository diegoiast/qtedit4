// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Diego Iastrubni

#pragma once

#include <QChildEvent>
#include <QSplitter>
#include <QWidget>

class QTabWidget;

class SplitterWithWidgetAdded : public QSplitter {
    Q_OBJECT

  public:
    SplitterWithWidgetAdded(Qt::Orientation orientation, QWidget *parent = nullptr)
        : QSplitter(orientation, parent) {}

  signals:
    void widgetAdded(QWidget *widget);

  protected:
    void childEvent(QChildEvent *event) override {
        if (event->type() == QEvent::ChildAdded) {
            QWidget *addedWidget = qobject_cast<QWidget *>(event->child());
            if (addedWidget) {
                emit widgetAdded(addedWidget);
            }
        }
        QSplitter::childEvent(event);
    }
};

class SplitTabWidget : public QWidget {
    Q_OBJECT

  public:
    explicit SplitTabWidget(QWidget *parent = nullptr);

    void addTab(QWidget *widget, const QString &label);
    void splitHorizontally();
    void closeCurrentSplit();
    void closeSplitWithTabWidget(QTabWidget *tab);
    void addTabToCurrentSplit(QWidget *widget, const QString &label);
    void moveTabToNewSplit(QWidget *widget);
    void closeCurrentTab();
    void moveNextTab();
    void movePrevTab();
    QWidget *getCurrentWidget();

    virtual void onTabFocusChanged(QWidget *widget, bool focused);
    virtual void onNewSplitCreated(QWidget *);

    bool closeSplitWhenEmpty = true;

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

  protected:
    SplitterWithWidgetAdded *splitter;
    QTabWidget *currentTabWidget = nullptr;

    void updateCurrentTabWidget(QTabWidget *newCurrent);
    void equalizeWidths();
};
