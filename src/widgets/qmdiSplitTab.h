/**
 * \file qmdiSplitTab.h
 * \brief Definition of qmdi enabled split tav
 * \author Diego Iastrubni diegoiast@gmail.com
 */

// SPDX-License-Identifier: MIT

#pragma once

#include <QPainter>
#include <QPushButton>
#include <QSize>
#include <QSplitter>
#include <QTabBar>
#include <QTabWidget>
#include <QToolButton>

#include "SplitTabWidget.h"
#include "qmdiserver.h"

class CustomMenuButton : public QPushButton {
    Q_OBJECT

  public:
    explicit CustomMenuButton(const QString &text, QWidget *parent = nullptr);
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

class DefaultButtonsProvider : public ButtonsProvider {
  public:
    virtual QWidget *requestButton(bool first, int tabIndex, SplitTabWidget *split);
};

class qmdiSplitTab : public SplitTabWidget, public qmdiServer {
    Q_OBJECT

  public:
    // SplitTabWidget
    qmdiSplitTab(QWidget *parent = nullptr);
    virtual void onTabFocusChanged(QWidget *widget, bool focused) override;
    virtual bool eventFilter(QObject *obj, QEvent *event) override;
    virtual void onNewSplitCreated(QTabWidget *tabWidget, int count) override;

    // qmdiServer interface
    virtual void addClient(qmdiClient *client) override;
    virtual void deleteClient(qmdiClient *) override;
    virtual int getClientsCount() const override;
    virtual qmdiClient *getClient(int i) const override;
    virtual qmdiClient *getCurrentClient() const override;
    virtual int getCurrentClientIndex() const override;
    virtual void setCurrentClientIndex(int i) override;
    virtual void moveClient(int oldPosition, int newPosition) override;
    virtual void updateClientName(const qmdiClient *client) override;

  signals:
    void newClientAdded(qmdiClient *);

  private:
    virtual void mdiSelected(qmdiClient *client, int index) const override;
    void on_middleMouse_pressed(int, QPoint);
    void on_rightMouse_pressed(int, QPoint);
    int computeLeading(QTabWidget *);

    QTabWidget *tabWidgetFromIndex(int globalIndex, int &localIndex) const;
    QWidget *activeWidget = nullptr;
};
