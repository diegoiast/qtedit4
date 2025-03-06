// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Diego Iastrubni

#include "SplitTabWidget.h"
#include <QApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QSplitter>
#include <QStackedWidget>
#include <QTabBar>
#include <QTabWidget>

SplitTabWidget::SplitTabWidget(QWidget *parent)
    : QWidget(parent), splitter(new QSplitter(Qt::Horizontal, this)) {
    auto layout = new QHBoxLayout(this);
    layout->addWidget(splitter);
    layout->setContentsMargins(0, 0, 0, 0);

    currentTabWidget = new QTabWidget(this);
    currentTabWidget->installEventFilter(this);
    currentTabWidget->setDocumentMode(true);
    splitter->addWidget(currentTabWidget);
}

void SplitTabWidget::addTab(QWidget *widget, const QString &label) {
    addTabToCurrentSplit(widget, label);
}

void SplitTabWidget::splitHorizontally() {
    auto currentIndex = splitter->indexOf(currentTabWidget);
    auto newTabWidget = new QTabWidget(this);
    newTabWidget->installEventFilter(this);
    newTabWidget->setDocumentMode(true);
    splitter->insertWidget(currentIndex + 1, newTabWidget);
    updateCurrentTabWidget(newTabWidget);
    equalizeWidths();
}

void SplitTabWidget::closeCurrentSplit() {
    if (splitter->count() > 1) {
        auto currentIndex = splitter->indexOf(currentTabWidget);
        currentTabWidget->deleteLater();

        if (currentIndex > 0) {
            currentIndex = currentIndex - 1;
        } else {
            currentIndex = 0;
        }
        updateCurrentTabWidget(qobject_cast<QTabWidget *>(splitter->widget(currentIndex)));

        equalizeWidths();
    }
}

void SplitTabWidget::addTabToCurrentSplit(QWidget *widget, const QString &label) {

    widget->setObjectName(label);
    widget->installEventFilter(this);
    int index = currentTabWidget->addTab(widget, label);
    currentTabWidget->setCurrentIndex(index);
    widget->setFocus();
}

void SplitTabWidget::closeCurrentTab() {
    int index = currentTabWidget->currentIndex();
    if (index >= 0) {
        QWidget *widget = currentTabWidget->widget(index);
        widget->removeEventFilter(this);
        currentTabWidget->removeTab(index);
    }

    if (currentTabWidget->count() == 0) {
        if (closeSplitWhenEmpty) {
            closeCurrentSplit();
        }
    }
}

void SplitTabWidget::updateCurrentTabWidget(QTabWidget *newCurrent) {
    if (currentTabWidget != newCurrent) {
        currentTabWidget = newCurrent;
        if (currentTabWidget && currentTabWidget->currentWidget()) {
            onTabFocusChanged(currentTabWidget->currentWidget(), true);
        }
    }
}

void SplitTabWidget::equalizeWidths() {
    auto count = splitter->count();
    if (count > 1) {
        QList<int> sizes;
        auto width = splitter->width() / count;
        for (auto i = 0; i < count; ++i) {
            sizes << width;
        }
        splitter->setSizes(sizes);
    }
}

bool SplitTabWidget::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::FocusIn || event->type() == QEvent::ShowToParent) {
        // How can we detect that the widget is inside a QTabWidget?
        // Internally the QTabWidget has a QTabBar and a QStackedWidget.
        // When you request to add a new tab, internally you are adding a new widget
        // to the stacked widget (and also adding its name to the tab bar). So,
        // checking if the parent of a widget is a QTabWiget will always fail.
        // Theoritically - this is an internal implementation of Qt, but QtWidgets
        // is "done" meaning this will not change.
        auto widget = qobject_cast<QWidget *>(watched);
        auto parent = qobject_cast<QStackedWidget *>(watched->parent());
        if (widget && parent) {
            auto tabWidget = qobject_cast<QTabWidget *>(parent->parentWidget());
            if (tabWidget) {
                if (splitter->indexOf(tabWidget) != -1) {
                    updateCurrentTabWidget(tabWidget);
                    onTabFocusChanged(widget, true);
                    return true;
                }
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void SplitTabWidget::moveNextTab() {
    auto tabWidget = currentTabWidget;
    auto widgetIndex = tabWidget->currentIndex();

    widgetIndex++;
    if (widgetIndex >= tabWidget->count()) {
        auto currentSplitIndex = splitter->indexOf(currentTabWidget);
        currentSplitIndex++;
        if (currentSplitIndex >= splitter->count()) {
            currentSplitIndex = 0;
        }
        tabWidget = qobject_cast<QTabWidget *>(splitter->widget(currentSplitIndex));
        widgetIndex = 0;
    }

    // Update the visual state of all tab widgets
    for (int i = 0; i < splitter->count(); ++i) {
        auto tw = qobject_cast<QTabWidget *>(splitter->widget(i));
        if (tw) {
            tw->setStyleSheet(tw == tabWidget
                                  ? ""
                                  : "QTabBar::tab:selected { background-color: palette(window); }");
        }
    }

    // currentTabWidget->tabBar()->setEnabled(false);
    currentTabWidget = tabWidget;
    currentTabWidget->setFocus(Qt::TabFocusReason);
    currentTabWidget->setCurrentIndex(widgetIndex);
    currentTabWidget->widget(widgetIndex)->setFocus();
    // currentTabWidget->tabBar()->setEnabled(true);
}

void SplitTabWidget::movePrevTab() {
    auto tabWidget = currentTabWidget;
    auto widgetIndex = tabWidget->currentIndex();

    widgetIndex--;
    if (widgetIndex < 0) {
        auto currentSplitIndex = splitter->indexOf(currentTabWidget);
        currentSplitIndex--;
        if (currentSplitIndex < 0) {
            currentSplitIndex = splitter->count() - 1;
        }
        tabWidget = qobject_cast<QTabWidget *>(splitter->widget(currentSplitIndex));
        widgetIndex = tabWidget->count() - 1;
    }

    // currentTabWidget->tabBar()->setEnabled(false);
    currentTabWidget = tabWidget;
    currentTabWidget->setFocus(Qt::TabFocusReason);
    currentTabWidget->setCurrentIndex(widgetIndex);
    currentTabWidget->widget(widgetIndex)->setFocus();
    // currentTabWidget->tabBar()->setEnabled(true);
}

void SplitTabWidget::onTabFocusChanged(QWidget *widget, bool focused) {
    widget = widget->parentWidget()->parentWidget();
    for (int i = 0; i < splitter->count(); ++i) {
        auto tw = qobject_cast<QTabWidget *>(splitter->widget(i));
        if (tw) {
            tw->setStyleSheet(splitter->widget(i) == widget
                                  ? ""
                                  : "QTabBar::tab:selected { background-color: palette(window); }");
        }
    }
}
