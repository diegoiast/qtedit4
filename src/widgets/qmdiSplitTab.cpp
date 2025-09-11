/**
 * \file qmdiSplitTab.cpp
 * \brief Definition of qmdi enabled split tav
 * \author Diego Iastrubni diegoiast@gmail.com
 */

// SPDX-License-Identifier: MIT

#include <QApplication>
#include <QColor>
#include <QEvent>
#include <QFontMetrics>
#include <QGraphicsDropShadowEffect>
#include <QLinearGradient>
#include <QMainWindow>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QSize>
#include <QSplitter>
#include <QString>
#include <QStyle>
#include <QStyleOptionMenuItem>
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

class DecoratedButton : public QPushButton {
  public:
    explicit DecoratedButton(const QString &text, QWidget *parent = nullptr);
    QSize minimumSizeHint() const override;

  protected:
    bool hovering;
    bool pressed;
    virtual void enterEvent(QEnterEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;
};

class DecoratedMenu : public QMenu {
  public:
    explicit DecoratedMenu(const QString &sidebarText, QWidget *parent = nullptr);

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    QString m_sidebarText;
    int m_sidebarWidth;
};

DecoratedMenu::DecoratedMenu(const QString &sidebarText, QWidget *parent)
    : QMenu(parent), m_sidebarText(sidebarText) {
    m_sidebarWidth = style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, this);
}

void DecoratedMenu::paintEvent(QPaintEvent *event) {
    QMenu::paintEvent(event);

    auto p = QPainter(this);
    auto sidebarRect = QRect(0, 0, m_sidebarWidth, height());
    auto grad = QLinearGradient(sidebarRect.topLeft(), sidebarRect.bottomLeft());
    auto sidebarColor = palette().color(QPalette::Highlight);
    grad.setColorAt(0, sidebarColor);
    grad.setColorAt(1, sidebarColor.lighter(150));

    p.fillRect(sidebarRect, grad);
    p.save();
    p.translate(sidebarRect.center());
    p.rotate(-90);
    auto rotatedRect = QRect(-sidebarRect.height() / 2, -sidebarRect.width() / 2,
                             sidebarRect.height(), sidebarRect.width());

    p.setPen(Qt::darkGray);
    p.translate(-1, +1);
    p.drawText(rotatedRect, Qt::AlignCenter, m_sidebarText);
    p.translate(+2, -2);
    p.setPen(Qt::white);
    p.drawText(rotatedRect, Qt::AlignCenter, m_sidebarText);
    p.restore();
}

QWidget *DefaultButtonsProvider::requestButton(bool first, int tabIndex, SplitTabWidget *split) {
    Q_UNUSED(tabIndex);
    auto manager = dynamic_cast<PluginManager *>(split->parent());
    auto isMinimizedMode = manager->isInMinimizedMode();

    if (isMinimizedMode && tabIndex == 0) {
        return getFirstTabButtons(first, split);
    }
    return getNonFirstTabButtons(first, split);
}

QWidget *DefaultButtonsProvider::getFirstTabButtons(bool first, SplitTabWidget *split) {
    auto manager = dynamic_cast<PluginManager *>(split->parent());

    if (first) {
#if 0
        auto appMenuButton = new DecoratedButton(QApplication::applicationName(), split);
#else
        auto appMenuButton = new QToolButton(split);
        appMenuButton->setIcon(QApplication::windowIcon());
        appMenuButton->setIcon(QIcon(":qtedit4.ico"));
        appMenuButton->setText(QApplication::applicationName());
        appMenuButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        appMenuButton->setPopupMode(QToolButton::InstantPopup);
        appMenuButton->setAutoRaise(true);
        auto appHighlightColor = qApp->palette().color(QPalette::Highlight);
        QString highlightedStyle = QString("QToolButton { background-color: transparent; }"
                                           "QToolButton:hover { background-color: %1; }"
                                           "QToolButton:pressed { background-color: %1; }")
                                       .arg(appHighlightColor.name());
        appMenuButton->setStyleSheet(highlightedStyle);

#endif

        auto popup = new DecoratedMenu(QApplication::applicationName(), manager);
        auto menu = manager->menus.updatePopMenu(popup);
        appMenuButton->setMenu(menu);

        delete appMenuAction;
        appMenuAction = new QAction(menu);
        appMenuAction->setShortcuts({Qt::ALT | Qt::Key_M, Qt::ALT | Qt::Key_Atilde});
        appMenuAction->setShortcutContext(Qt::ApplicationShortcut);
        QObject::connect(appMenuAction, &QAction::triggered, [appMenuButton] {
            if (appMenuButton->menu()->isVisible()) {
                appMenuButton->menu()->close();
            } else {
                appMenuButton->showMenu();
            }
        });
        manager->addAction(appMenuAction);
        appMenuButton->setToolTip(
            manager->tr("Application menu, %1").arg(appMenuAction->shortcut().toString()));

        return appMenuButton;
    }

#if not CLOSABLE_TABS
    auto tabCloseBtn = new QToolButton(split);
    tabCloseBtn->setAutoRaise(true);
    tabCloseBtn->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::WindowClose));
    QObject::connect(tabCloseBtn, &QAbstractButton::clicked, manager, &PluginManager::closeClient);
    return tabCloseBtn;
#else
    return nullptr;
#endif
}

QWidget *DefaultButtonsProvider::getNonFirstTabButtons(bool first, SplitTabWidget *split) {
    auto manager = dynamic_cast<PluginManager *>(split->parent());

    if (first) {
        delete appMenuAction;
        appMenuAction = nullptr;

        auto tabNewBtn = new QToolButton(split);
        tabNewBtn->setAutoRaise(true);
        tabNewBtn->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::DocumentNew));
        tabNewBtn->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::DocumentNew));
        QObject::connect(tabNewBtn, &QAbstractButton::clicked, tabNewBtn,
                         [manager, tabNewBtn, split]() {
                             auto tab = qobject_cast<QTabWidget *>(tabNewBtn->parentWidget());
                             if (tab) {
                                 split->updateCurrentTabWidget(tab);
                             }
                             if (manager) {
                                 emit manager->newFileRequested(tabNewBtn);
                             }
                         });
        return tabNewBtn;
    }

#if not CLOSABLE_TABS
    auto tabCloseBtn = new QToolButton(split);
    tabCloseBtn->setAutoRaise(true);
    tabCloseBtn->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::WindowClose));
    QObject::connect(tabCloseBtn, &QAbstractButton::clicked, manager, &PluginManager::closeClient);
    return tabCloseBtn;
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

    if (auto firstTab = qobject_cast<QTabWidget *>(splitter->widget(0))) {
        if (auto button = qobject_cast<QToolButton *>(firstTab->cornerWidget(Qt::TopLeftCorner))) {
            auto menu = new DecoratedMenu(QApplication::applicationName(), this);
            mdiHost->menus.updatePopMenu(menu);
            button->setMenu(menu);
        };
    }
}

bool qmdiSplitTab::event(QEvent *ev) {
    if (ev->type() == QEvent::ParentChange) {
        // when minimized mode changes, modify buttons for each tab
        auto manager = dynamic_cast<PluginManager *>(parentWidget());
        if (manager) {
            connect(manager, &PluginManager::minimizedModeChanged, this, [this]() {
                if (!buttonsProvider) {
                    qDebug() << "No buttons provider";
                    return;
                }
                for (auto i = 0; i < splitter->count(); ++i) {
                    if (auto tabWidget = qobject_cast<DraggableTabWidget *>(splitter->widget(i))) {
                        auto left = buttonsProvider->requestButton(true, i, this);
                        auto right = buttonsProvider->requestButton(false, i, this);
                        left->setParent(tabWidget);
                        right->setParent(tabWidget);
                        left->show();
                        right->show();
                        tabWidget->setCornerWidget(left, Qt::TopLeftCorner);
                        tabWidget->setCornerWidget(right, Qt::TopRightCorner);
                        if (i == 0) {
                            // TODO - rebuild app menu - this is not working
                            if (auto button = qobject_cast<QToolButton *>(left)) {
                                auto menu =
                                    new DecoratedMenu(QApplication::applicationName(), this);
                                mdiHost->menus.updatePopMenu(menu);
                                button->setMenu(menu);
                            };
                        }
                    }
                }
            });
        }
    }
    return QWidget::event(ev);
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

DecoratedButton::DecoratedButton(const QString &text, QWidget *parent)
    : QPushButton(parent), hovering(false), pressed(false) {
    setCursor(Qt::PointingHandCursor);
    setText(text);
}

QSize DecoratedButton::minimumSizeHint() const {
    auto fm = QFontMetrics(font());
    auto textWidth = fm.horizontalAdvance(text()) + 10;
    auto textHeight = fm.height();

    // Minimum 120x40
    return QSize(qMax(120, textWidth), qMax(40, textHeight + 10));
}

void DecoratedButton::enterEvent(QEnterEvent *event) {
    hovering = true;
    update();
    QPushButton::enterEvent(event);
}

void DecoratedButton::leaveEvent(QEvent *event) {
    hovering = false;
    update();
    QPushButton::leaveEvent(event);
}

void DecoratedButton::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        pressed = true;
        update();
    }
    QPushButton::mousePressEvent(event);
}

void DecoratedButton::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        pressed = false;
        update();
    }
    QPushButton::mouseReleaseEvent(event);
}

void DecoratedButton::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // --- Background Gradient ---
    QColor startColor = QColor("#44ee44");
    QColor endColor = startColor.lighter();

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
    QFont font("Arial", 10);
    painter.setFont(font);
    QRect textRect(32, 0, width() - 32, height());
    painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text());
}
