/**
 * \file qmdiSplitTab.cpp
 * \brief Definition of qmdi enabled split tab
 * \author Diego Iastrubni diegoiast@gmail.com
 */

// SPDX-License-Identifier: MIT

#include <QApplication>
#include <QEvent>
#include <QFontMetrics>
#include <QGraphicsDropShadowEffect>
#include <QLinearGradient>
#include <QMainWindow>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QSize>
#include <QSplitter>
#include <QTabBar>
#include <QTabWidget>
#include <QToolButton>

#include <pluginmanager.h>
#include <qmdiclient.h>
#include <qmdihost.h>

#include "widgets/qmdiSplitTab.h"

// Should we have a close button on each tab? or at the sides of the widget? On the widget
// This is not default, as close button on each tab takes space. Mid button click exists, right
// button click brings a menu to close.
#define CLOSABLE_TABS 0

QWidget *DefaultButtonsProvider::requestButton(bool first, int tabIndex, SplitTabWidget *split) {
    Q_UNUSED(tabIndex);

    if (first) {
<<<<<<< HEAD
        auto addbutton = new QToolButton(split);
        addbutton->setAutoRaise(true);
        addbutton->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::DocumentNew));
        QObject::connect(addbutton, &QAbstractButton::clicked, addbutton, [addbutton, split]() {
            auto manager = dynamic_cast<PluginManager *>(split->parent());
            auto tab = qobject_cast<QTabWidget *>(addbutton->parentWidget());
            if (tab) {
                split->updateCurrentTabWidget(tab);
            }
            if (manager) {
                emit manager->newFileRequested(addbutton);
            }
        });
        return addbutton;
=======
        auto addNewMdiClient = new CustomMenuButton(QApplication::applicationName(), split);
        // addNewMdiClient->setAutoRaise(true);
        addNewMdiClient->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::DocumentNew));
        QObject::connect(addNewMdiClient, &QAbstractButton::clicked, addNewMdiClient,
                         [manager, addNewMdiClient, split]() {
                             auto tab = qobject_cast<QTabWidget *>(addNewMdiClient->parentWidget());
                             if (tab) {
                                 split->updateCurrentTabWidget(tab);
                             }
                             if (manager) {
                                 emit manager->newFileRequested(addNewMdiClient);
                             }
                         });
        return addNewMdiClient;
>>>>>>> 4ee4931 (Initial code for the new button)
    }

#if !CLOSABLE_TABS
    auto closeButton = new QToolButton(split);
    closeButton->setAutoRaise(true);
    closeButton->setIcon(QIcon::fromTheme("document-close"));
    QObject::connect(closeButton, &QAbstractButton::clicked, closeButton, [closeButton, split]() {
        auto manager = dynamic_cast<PluginManager *>(split->parent());
        auto tab = qobject_cast<QTabWidget *>(closeButton->parentWidget());
        if (tab) {
            split->updateCurrentTabWidget(tab);
        }
        manager->closeClient();
    });
    return closeButton;
#else
    return nullptr;
#endif
}

qmdiSplitTab::qmdiSplitTab(QWidget *parent) : SplitTabWidget(parent) {
    connect(this, &SplitTabWidget::emptyAreaDoubleClicked, this,
            [this](QTabWidget *tabWidget, const QPoint &) {
                auto manager = dynamic_cast<PluginManager *>(this->parent());
                if (manager) {
                    emit manager->newFileRequested(tabWidget->tabBar());
                }
            });
}

void qmdiSplitTab::onTabFocusChanged(QWidget *widget, bool focused) {
    SplitTabWidget::onTabFocusChanged(widget, focused);

    // nothing to do, if the same tab has been selected twice
    if (widget == activeWidget) {
        return;
    }
    if (!focused) {
        if (widget == nullptr) {
            activeWidget = nullptr;
        }
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

void qmdiSplitTab::onNewSplitCreated(QTabWidget *tabWidget, int count) {
    SplitTabWidget::onNewSplitCreated(tabWidget, count);
    tabWidget->tabBar()->installEventFilter(this);

#if CLOSABLE_TABS
    tabWidget->setTabsClosable(true);
    connect(tabWidget, &QTabWidget::tabCloseRequested, tabWidget, [this, tabWidget](int index) {
        auto widget = tabWidget->widget(index);
        auto client = dynamic_cast<qmdiClient *>(widget);
        if (!client) {
            delete widget;
            return;
        }
        client->closeClient();
    });
#endif
}

void qmdiSplitTab::addClient(qmdiClient *client) {
    auto w = dynamic_cast<QWidget *>(client);

    if (w == nullptr) {
        qDebug("%s %s %d: warning trying to add a qmdiClient which does not derive QWidget",
               __FILE__, __FUNCTION__, __LINE__);
        return;
    }

    addTab(w, client->mdiClientName, client->mdiClientFileName());
    client->mdiServer = this;
    w->setFocus();
    emit newClientAdded(client);
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
    auto localIndex = -1;
    auto tabWidget = tabWidgetFromIndex(i, localIndex);
    if (tabWidget && localIndex >= 0) {
        return dynamic_cast<qmdiClient *>(tabWidget->widget(localIndex));
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

void qmdiSplitTab::moveClient(int oldPosition, int newPosition) {
    if (oldPosition == newPosition || oldPosition < 0 || newPosition < 0) {
        qDebug() << "qmdiSplitTab::moveClient Invalid old/new position";
        return;
    }

    auto oldLocalIndex = -1;
    auto oldTabWidget = tabWidgetFromIndex(oldPosition, oldLocalIndex);
    if (!oldTabWidget || oldLocalIndex < 0) {
        qDebug() << "qmdiSplitTab::moveClient Old tab/position not found";
        return;
    }

    auto newLocalIndex = -1;
    auto newTabWidget = tabWidgetFromIndex(newPosition, newLocalIndex);
    if (!newTabWidget || newLocalIndex < 0) {
        qDebug() << "qmdiSplitTab::moveClient New tab/position not found";
        return;
    }

    if (oldTabWidget == newTabWidget) {
        newTabWidget->tabBar()->moveTab(oldLocalIndex, newLocalIndex);
        newTabWidget->setCurrentIndex(newLocalIndex);
        return;
    }

    if (oldPosition > newPosition) {
        ++newLocalIndex;
    }

    auto widget = oldTabWidget->widget(oldLocalIndex);
    if (!widget) {
        qDebug() << "qmdiSplitTab::moveClient Widget not found at oldLocalIndex";
        return;
    }

    auto text = oldTabWidget->tabText(oldLocalIndex);
    auto tooltip = oldTabWidget->tabToolTip(oldLocalIndex);
    oldTabWidget->removeTab(oldLocalIndex);
    newTabWidget->insertTab(newLocalIndex, widget, text);
    newTabWidget->setTabToolTip(newLocalIndex, tooltip);
    newTabWidget->setCurrentIndex(newLocalIndex);
    updateCurrentTabWidget(newTabWidget);
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
                tabWidget->setTabToolTip(innerIndex, c->mdiClientFileName());
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

QTabWidget *qmdiSplitTab::tabWidgetFromIndex(int globalIndex, int &localIndex) const {
    auto currentIndex = 0;
    for (auto i = 0; i < splitter->count(); ++i) {
        auto tabWidget = qobject_cast<QTabWidget *>(splitter->widget(i));
        if (!tabWidget) {
            continue;
        }

        auto count = tabWidget->count();
        if (globalIndex < currentIndex + count) {
            localIndex = globalIndex - currentIndex;
            return tabWidget;
        }
        currentIndex += count;
    }

    localIndex = -1;
    return nullptr;
}

CustomMenuButton::CustomMenuButton(const QString &text, QWidget *parent)
    : QPushButton(parent), hovering(false), pressed(false) {
    setCursor(Qt::PointingHandCursor);
    setText(text); // We'll draw the text manually based on the button's current text
}

QSize CustomMenuButton::minimumSizeHint() const {
    // Get font metrics for the current text
    QFontMetrics fm(font());
    int textWidth = fm.horizontalAdvance(text()) + 10; // Add padding to the text width
    int textHeight = fm.height();

    // Button height should accommodate the text height + padding
    return QSize(qMax(120, textWidth), qMax(40, textHeight + 10)); // Minimum 120x40
}

void CustomMenuButton::enterEvent(QEnterEvent *event) {
    hovering = true;
    update();
    QPushButton::enterEvent(event);
}

void CustomMenuButton::leaveEvent(QEvent *event) {
    hovering = false;
    update();
    QPushButton::leaveEvent(event);
}

void CustomMenuButton::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        pressed = true;
        update();
    }
    QPushButton::mousePressEvent(event);
}

void CustomMenuButton::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        pressed = false;
        update();
    }
    QPushButton::mouseReleaseEvent(event);
}

void CustomMenuButton::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // --- Background Gradient ---
    QColor startColor = QColor("#41cd52");
    QColor endColor = QColor("#41cd52");

    if (pressed) {
        startColor = startColor.darker(140); // 40% darker
        endColor = endColor.darker(140);
    } else if (hovering) {
        startColor = startColor.darker(115); // 15% darker
        endColor = endColor.darker(115);
    }

    QLinearGradient bgGradient(0, 0, width(), height());
    bgGradient.setColorAt(0.0, startColor);
    bgGradient.setColorAt(1.0, endColor);

    QRectF bgRect = rect();
    QPainterPath path;
    path.addRoundedRect(bgRect, 4, 4);
    painter.fillPath(path, bgGradient);

    // --- Shadow Effect (when hovered) ---
    if (hovering) {
        QGraphicsDropShadowEffect shadowEffect;
        shadowEffect.setOffset(0, 3); // Make the shadow drop a little lower
        shadowEffect.setBlurRadius(8);
        shadowEffect.setColor(QColor(0, 0, 0, 80)); // Slightly transparent shadow
        painter.setOpacity(1); // Draw without transparency for proper shadow effect
        // shadowEffect.drawSource(&painter);
    }

    // --- Hamburger Icon ---
    QPen pen;
    pen.setCapStyle(Qt::FlatCap);
    pen.setWidth(4);

    // Line 1: white + light gray split
    pen.setColor(Qt::white);
    painter.setPen(pen);
    painter.drawLine(8, 12, 10, 12);

    pen.setColor(QColor("#eeeeee"));
    painter.setPen(pen);
    painter.drawLine(15, 12, 26, 12);

    // Lines 2 and 3: green
    pen.setColor(QColor("#4ee44e"));
    painter.setPen(pen);
    painter.drawLine(8, 20, 26, 20);
    painter.drawLine(8, 28, 26, 28);

    // --- Text (from QPushButton) ---
    painter.setPen(Qt::white);
    QFont font("Arial", 14);
    painter.setFont(font);
    QRect textRect(32, 0, width() - 32, height());
    painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text());
}
