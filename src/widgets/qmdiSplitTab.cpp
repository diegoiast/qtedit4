#include <QMainWindow>
#include <QMouseEvent>
#include <QSplitter>
#include <QTabBar>
#include <QTabWidget>

#include <QEvent>
#include <qmdiclient.h>
#include <qmdihost.h>

#include "widgets/qmdiSplitTab.h"

void qmdiSplitTab::onTabFocusChanged(QWidget *widget, bool focused) {
    SplitTabWidget::onTabFocusChanged(widget, focused);

    // nothing to do, if the same tab has been selected twice
    if (widget == activeWidget) {
        return;
    }
    if (!focused) {
        return;
    }

    auto client = dynamic_cast<qmdiClient *>(activeWidget);
    if (client) {
        mdiHost->unmergeClient(client);
    }

    activeWidget = widget;

    client = dynamic_cast<qmdiClient *>(activeWidget);
    if (activeWidget) {
        mdiHost->mergeClient(client);
    }

    auto m = dynamic_cast<QMainWindow *>(mdiHost);
    auto index = getCurrentClientIndex();
    mdiHost->updateGUI(m);
    mdiSelected(client, index);
    splitter->installEventFilter(this);
}

bool qmdiSplitTab::eventFilter(QObject *obj, QEvent *event) {
    if (!obj->inherits("QTabBar")) {
        return SplitTabWidget::eventFilter(obj, event);
    }

    if (event->type() != QEvent::MouseButtonPress) {
        return SplitTabWidget::eventFilter(obj, event);
    }

    auto tabBar = qobject_cast<QTabBar *>(obj);
    auto mouseEvent = static_cast<QMouseEvent *>(event);
    auto position = mouseEvent->pos();
    auto clickedItem = tabBar->tabAt(position);

    // just in case
    if (clickedItem == -1) {
        return QObject::eventFilter(obj, event);
    }

    auto tabWidget = qobject_cast<QTabWidget *>(tabBar->parent());
    auto globalPosition = tabBar->mapToGlobal(position);
    auto parentPosition = this->mapFromGlobal(globalPosition);
    clickedItem += computeLeading(tabWidget);

    switch (mouseEvent->button()) {
    case Qt::LeftButton:
        return QObject::eventFilter(obj, event);
        break;

    case Qt::RightButton:
        on_rightMouse_pressed(clickedItem, parentPosition);
        break;

    case Qt::MiddleButton:
        on_middleMouse_pressed(clickedItem, parentPosition);
        break;

    default:
        return QObject::eventFilter(obj, event);
    }

    return true;
}

void qmdiSplitTab::onNewSplitCreated(QWidget *w) {
    auto tabWidget = qobject_cast<QTabWidget *>(w);
    if (tabWidget) {
        tabWidget->tabBar()->installEventFilter(this);
    }
}

void qmdiSplitTab::addClient(qmdiClient *client) {
    auto w = dynamic_cast<QWidget *>(client);

    if (w == nullptr) {
        qDebug("%s %s %d: warning trying to add a qmdiClient which does not derive "
               "QWidget",
               __FILE__, __FUNCTION__, __LINE__);
        return;
    }

    addTabToCurrentSplit(w, client->mdiClientName);
    client->mdiServer = this;
    w->setFocus();
}

void qmdiSplitTab::deleteClient(qmdiClient *client) {
    if (client == nullptr) {
        return;
    }
    if (mdiHost == nullptr) {
        return;
    }
    if (dynamic_cast<qmdiClient *>(activeWidget) != client) {
        return;
    }

    mdiHost->unmergeClient(client);
    mdiHost->updateGUI(dynamic_cast<QMainWindow *>(mdiHost));
    activeWidget = nullptr;
}

int qmdiSplitTab::getClientsCount() const {
    auto total = 0;
    for (auto i = 0; i < splitter->count(); i++) {
        auto w = splitter->widget(i);
        auto t = qobject_cast<QTabWidget *>(w);
        if (!w) {
            continue;
        }
        total += t->count();
    }
    return total;
}

qmdiClient *qmdiSplitTab::getClient(int i) const {
    if (i < 0) {
        return nullptr;
    }
    auto currentIndex = 0;
    for (auto tabIndex = 0; tabIndex < splitter->count(); tabIndex++) {
        auto tabWidget = qobject_cast<QTabWidget *>(splitter->widget(tabIndex));
        if (!tabWidget) {
            continue;
        }
        for (auto innerIndex = 0; innerIndex < tabWidget->count(); innerIndex++) {
            if (currentIndex == i) {
                return dynamic_cast<qmdiClient *>(tabWidget->widget(innerIndex));
            }
            currentIndex++;
        }
    }
    return nullptr;
}

qmdiClient *qmdiSplitTab::getCurrentClient() const {
    if (!currentTabWidget) {
        return nullptr;
    }
    auto currentIndex = currentTabWidget->currentIndex();
    if (currentIndex < 0) {
        return nullptr;
    }
    return dynamic_cast<qmdiClient *>(currentTabWidget->widget(currentIndex));
}

int qmdiSplitTab::getCurrentClientIndex() const {
    if (!currentTabWidget) {
        return -1;
    }
    auto currentWidgetIndex = currentTabWidget->currentIndex();
    if (currentWidgetIndex < 0) {
        return -1;
    }
    auto globalIndex = 0;
    for (auto i = 0; i < splitter->count(); i++) {
        auto tabWidget = qobject_cast<QTabWidget *>(splitter->widget(i));
        if (!tabWidget) {
            continue;
        }
        if (tabWidget == currentTabWidget) {
            return globalIndex + currentWidgetIndex;
        }
        globalIndex += tabWidget->count();
    }

    return -1;
}

void qmdiSplitTab::setCurrentClientIndex(int i) {
    if (i < 0) {
        return;
    }

    int currentCount = 0;
    for (auto tabIndex = 0; tabIndex < splitter->count(); tabIndex++) {
        auto tabWidget = qobject_cast<QTabWidget *>(splitter->widget(tabIndex));
        if (!tabWidget) {
            continue;
        }

        int tabCount = tabWidget->count();
        if (i < currentCount + tabCount) {
            // The requested index is in this tab widget
            tabWidget->setCurrentIndex(i - currentCount);
            updateCurrentTabWidget(tabWidget);
            return;
        }

        currentCount += tabCount;
    }
}

void qmdiSplitTab::updateClientName(const qmdiClient *client) {
    for (auto tabIndex = 0; tabIndex < splitter->count(); tabIndex++) {
        auto tabWidget = qobject_cast<QTabWidget *>(splitter->widget(tabIndex));
        if (!tabWidget) {
            continue;
        }
        for (auto innerIndex = 0; innerIndex < tabWidget->count(); innerIndex++) {
            auto w = tabWidget->widget(innerIndex);
            auto c = dynamic_cast<qmdiClient *>(w);
            if (!c) {
                return;
            }
            if (c == client) {
                tabWidget->setTabText(innerIndex, c->mdiClientName);
                return;
            }
        }
    }
}

void qmdiSplitTab::mdiSelected(qmdiClient *client, int index) const {
    if (onMdiSelected) {
        onMdiSelected(client, index);
    }
}

void qmdiSplitTab::on_middleMouse_pressed(int i, QPoint) { tryCloseClient(i); }

void qmdiSplitTab::on_rightMouse_pressed(int i, QPoint p) { showClientMenu(i, p); }

int qmdiSplitTab::computeLeading(QTabWidget *w) {
    if (!w) {
        return 0;
    }
    auto leadingIndex = 0;
    for (auto tabIndex = 0; tabIndex < splitter->count(); tabIndex++) {
        auto tabWidget = qobject_cast<QTabWidget *>(splitter->widget(tabIndex));
        if (!tabWidget) {
            continue;
        }
        if (tabWidget == w) {
            return leadingIndex;
        }
        leadingIndex += tabWidget->count();
    }
    return leadingIndex;
}
