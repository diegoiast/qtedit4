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
#include <QTimer>

SplitterWithWidgetAdded::SplitterWithWidgetAdded(Qt::Orientation orientation, QWidget *parent)
    : QSplitter(orientation, parent) {}

void SplitterWithWidgetAdded::childEvent(QChildEvent *event) {
    QSplitter::childEvent(event);
    if (event->type() == QEvent::ChildAdded) {
        auto addedWidget = qobject_cast<QWidget *>(event->child());
        if (addedWidget) {
            emit widgetAdded(addedWidget);
        }
    }
}

SplitTabWidget::SplitTabWidget(QWidget *parent)
    : QWidget(parent), splitter(new SplitterWithWidgetAdded(Qt::Horizontal, this)) {
    auto layout = new QHBoxLayout(this);
    connect(splitter, &SplitterWithWidgetAdded::widgetAdded, this, [this](QWidget *w) {
        auto tab = qobject_cast<QTabWidget *>(w);
        if (tab) {
            auto count = splitter->count();
            onNewSplitCreated(tab, count);
        }
    });

    layout->addWidget(splitter);
    layout->setContentsMargins(0, 0, 0, 0);

    // Whty a timer? without this - event listener does not work, and menus don't work
    QTimer::singleShot(0, splitter, [this]() { splitHorizontally(); });
}

void SplitTabWidget::addTab(QWidget *widget, const QString &label, const QString &tooltip) {
    if (!savedSplitCount.empty()) {
        auto currentTabIndex = findSplitIndex(currentTabWidget);
        auto widgetsInCurrentTab = currentTabWidget->count();
        auto maxWidgetsInCurrentTab = savedSplitCount[currentTabIndex];

        if (widgetsInCurrentTab >= maxWidgetsInCurrentTab) {

            if (currentTabIndex + 1 == savedSplitCount.count()) {
                savedSplitCount.clear();
            } else {
                splitHorizontally();
            }
        }
    }
    addTabToCurrentSplit(widget, label, tooltip);
}

void SplitTabWidget::splitHorizontally() {
    auto currentIndex = splitter->indexOf(currentTabWidget);
    auto newTabWidget = new QTabWidget(this);
    newTabWidget->setDocumentMode(true);
    newTabWidget->setMovable(true);
    newTabWidget->setObjectName(QString("QTabWidget#%1").arg(splitter->count()));
    splitter->insertWidget(currentIndex + 1, newTabWidget);
    updateCurrentTabWidget(newTabWidget);
    equalizeWidths();
}

void SplitTabWidget::closeCurrentSplit() {
    // We wlaways keep at last a single tab. Why? since it may contain a menu for the app.
    if (splitter->count() <= 1) {
        onTabFocusChanged(nullptr, true);
        return;
    }
    closeSplitWithTabWidget(currentTabWidget);
}

void SplitTabWidget::closeSplitWithTabWidget(QTabWidget *tab) {
    auto currentIndex = splitter->indexOf(tab);
    tab->deleteLater();
    if (currentIndex > 0) {
        currentIndex = currentIndex - 1;
    } else {
        currentTabWidget = nullptr;
        onTabFocusChanged(nullptr, false);
        return;
    }

    auto newWidget = splitter->widget(currentIndex);
    auto newTab = qobject_cast<QTabWidget *>(newWidget);

    updateCurrentTabWidget(newTab);
    equalizeWidths();
}

void SplitTabWidget::addTabToCurrentSplit(QWidget *widget, const QString &label,
                                          const QString &tooltip) {
    if (!currentTabWidget) {
        splitHorizontally();
    }
    auto index = currentTabWidget->addTab(widget, label);
    currentTabWidget->setCurrentIndex(index);
    currentTabWidget->setTabToolTip(index, tooltip);
    widget->setObjectName(label);
    widget->installEventFilter(this);
    widget->setFocus();
    onTabFocusChanged(currentTabWidget->currentWidget(), true);
}

void SplitTabWidget::addTabToSplit(int splitNumber, QWidget *widget, const QString &label,
                                   const QString &tooltip) {
    auto tabWidget = qobject_cast<QTabWidget *>(splitter->widget(splitNumber));
    if (!tabWidget) {
        return;
    }

    auto index = tabWidget->addTab(widget, label);
    tabWidget->setCurrentIndex(index);
    tabWidget->setTabToolTip(index, tooltip);
    widget->setObjectName(label);
    widget->installEventFilter(this);
    widget->setFocus();
    onTabFocusChanged(tabWidget->currentWidget(), true);
}

int SplitTabWidget::findSplitIndex(QTabWidget *w) {
    for (auto i = 0; i < splitter->count(); i++) {
        auto tabWidget = qobject_cast<QTabWidget *>(splitter->widget(i));
        if (!tabWidget) {
            continue;
        }
        if (w == tabWidget) {
            return i;
        }
    }
    return -1;
}

void SplitTabWidget::moveTabToNewSplit(QWidget *widget) {
    splitHorizontally();
    auto oldTab = qobject_cast<QTabWidget *>(widget->parentWidget()->parentWidget());
    auto index = oldTab->indexOf(widget);
    auto text = oldTab->tabText(index);
    currentTabWidget->addTab(widget, text);
    oldTab->removeTab(index);
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
    if (currentTabWidget == newCurrent) {
        return;
    }
    currentTabWidget = newCurrent;
    if (currentTabWidget && currentTabWidget->currentWidget()) {
        onTabFocusChanged(currentTabWidget->currentWidget(), true);
    }
}

void SplitTabWidget::equalizeWidths() {
    auto count = splitter->count();
    if (count <= 1) {
        return;
    }
    QList<int> sizes;
    auto width = splitter->width() / count;
    for (auto i = 0; i < count; ++i) {
        sizes << width;
    }
    splitter->setSizes(sizes);
}

bool SplitTabWidget::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::ShowToParent || event->type() == QEvent::HideToParent ||
        event->type() == QEvent::ChildRemoved) {

        // How can we detect that the widget is inside a QTabWidget?
        // Internally the QTabWidget has a QTabBar and a QStackedWidget.
        // When you request to add a new tab, internally you are adding a new widget
        // to the stacked widget (and also adding its name to the tab bar). So,
        // checking if the parent of a widget is a QTabWiget will always fail.
        // Theoretically - this is an internal implementation of Qt, but QtWidgets
        // is "done" meaning this will not change.
        auto widget = qobject_cast<QWidget *>(watched);
        auto parent = qobject_cast<QStackedWidget *>(watched->parent());
        if (widget && parent) {
            auto tabWidget = qobject_cast<QTabWidget *>(parent->parentWidget());
            if (tabWidget) {
                // Only close the split if it's a ChildRemoved event and we're down to 1 tab
                auto index = splitter->indexOf(tabWidget);
                if (index != -1) {
                    updateCurrentTabWidget(tabWidget);
                    switch (event->type()) {
                    // case QEvent::ChildRemoved:
                    case QEvent::HideToParent:
                        onTabFocusChanged(widget, false);
                        if (currentTabWidget->count() == 1) {
                            if (closeSplitWhenEmpty) {
                                closeCurrentSplit();
                            }
                        }
                        break;
                    case QEvent::ShowToParent:
                        onTabFocusChanged(widget, true);
                        break;
                    default:
                        break;
                    }
                    return false;
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

    currentTabWidget = tabWidget;
    currentTabWidget->setFocus(Qt::TabFocusReason);
    currentTabWidget->setCurrentIndex(widgetIndex);
    currentTabWidget->widget(widgetIndex)->setFocus();
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

    currentTabWidget = tabWidget;
    currentTabWidget->setFocus(Qt::TabFocusReason);
    currentTabWidget->setCurrentIndex(widgetIndex);
    currentTabWidget->widget(widgetIndex)->setFocus();
}

QList<int> SplitTabWidget::getSplitInternalCount() const {
    auto l = QList<int>();
    for (auto i = 0; i < splitter->count(); i++) {
        auto w = splitter->widget(i);
        auto t = qobject_cast<QTabWidget *>(w);
        if (t) {
            l.push_back(t->count());
        } else {
            l.push_back(0);
        }
    }
    return l;
}

void SplitTabWidget::setButtonProvider(ButtonsProvider *newProvider) {
    this->buttonsProvider = newProvider;
    // TODO - how do we modify existing tabs?
}

QWidget *SplitTabWidget::getCurrentWidget() {
    if (!currentTabWidget) {
        return nullptr;
    }
    return currentTabWidget->currentWidget();
}

int SplitTabWidget::getWigetsCountInCurrentSplit() const {
    if (!currentTabWidget) {
        return 0;
    }
    return currentTabWidget->count();
}

void SplitTabWidget::onTabFocusChanged(QWidget *widget, bool focused) {
    if (widget && focused) {
        widget->setFocus();
    }
}

void SplitTabWidget::onNewSplitCreated(QTabWidget *tabWidget, int count) {
    if (buttonsProvider) {
        auto left = buttonsProvider->requestButton(true, count, this);
        auto right = buttonsProvider->requestButton(false, count, this);
        tabWidget->setCornerWidget(left, Qt::TopLeftCorner);
        tabWidget->setCornerWidget(right, Qt::TopRightCorner);
    }
}
