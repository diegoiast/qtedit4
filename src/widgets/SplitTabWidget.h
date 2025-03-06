// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Diego Iastrubni

#pragma once

#include <QWidget>

class QSplitter;
class QTabWidget;

class SplitTabWidget : public QWidget {
    Q_OBJECT

  public:
    explicit SplitTabWidget(QWidget *parent = nullptr);

    void addTab(QWidget *widget, const QString &label);
    void splitHorizontally();
    void closeCurrentSplit();
    void addTabToCurrentSplit(QWidget *widget, const QString &label);
    void closeCurrentTab();
    void moveNextTab();
    void movePrevTab();

    virtual void onTabFocusChanged(QWidget *widget, bool focused);

    bool closeSplitWhenEmpty = true;

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

  protected:
    QSplitter *splitter;
    QTabWidget *currentTabWidget;

    void updateCurrentTabWidget(QTabWidget *newCurrent);
    void equalizeWidths();
};
