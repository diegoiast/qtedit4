#pragma once

/**
 * \file qmdihost.h
 * \brief Definition of the qmdi host class
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL 2 or 3
 * \see qmdiHost
 */

// the reason for including this file, and not declare the classes
// is for the developer using this library - one single include
#include "qmdiactiongroup.h"

#include "qmdiactiongrouplist.h"

class QMainWindow;
class qmdiClient;

class qmdiHost {
  public:
    qmdiHost();
    virtual ~qmdiHost();

    qmdiActionGroupList menus;
    qmdiActionGroupList toolbars;

    virtual void updateGUI(QMainWindow *window = nullptr);
    void mergeClient(qmdiClient *client);
    void unmergeClient(qmdiClient *client);

    QList<QToolBar *> *toolBarList;

  protected:
    void addActionsToWidget(qmdiActionGroupList &agl, QWidget *w);
    void removeActionsFromWidget(qmdiActionGroupList &agl, QWidget *w);
    bool updateMenusAndToolBars;
};
