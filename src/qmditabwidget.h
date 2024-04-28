#pragma once

/**
 * \file qmditabwidget.h
 * \brief Declaration of the qmdi tab widget
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL 2 or 3
 * \see qmdiServer, QTabWidget
 */

#include <QTabBar>
#include <QTabWidget>

#include "qmdiserver.h"

class QWidget;
class QPoint;
class QEvent;

class qmdiHost;

class qmdiTabWidget : public QTabWidget, public qmdiServer {
    Q_OBJECT
  public:
    qmdiTabWidget(QWidget *parent = nullptr, qmdiHost *host = nullptr);
    ~qmdiTabWidget();

  public slots:
    void tabChanged(int i);
    void on_middleMouse_pressed(int, QPoint);
    void on_rightMouse_pressed(int, QPoint);
    bool eventFilter(QObject *obj, QEvent *event);

  public:
    virtual void addClient(qmdiClient *client);
    virtual void deleteClient(qmdiClient *client);
    virtual int getClientsCount();
    virtual qmdiClient *getClient(int i);

  protected:
    void tabInserted(int index);
    void tabRemoved(int index);

  private:
    QWidget *activeWidget;
};
