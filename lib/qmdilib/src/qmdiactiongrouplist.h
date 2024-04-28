#pragma once

/**
 * \file qmdiactiongrouplist.h
 * \brief Definition of the action group list class
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL 2 or 3
 * \see qmdiActionGroupList
 */

#include <QList>
#include <QToolBar>

class QObject;
class QAction;
class QString;
class QMenuBar;
class QMenu;
class QMainWindow;

class qmdiActionGroup;

class qmdiActionGroupList {
    friend class qmdiHost;

  public:
    qmdiActionGroupList();
    ~qmdiActionGroupList();

    qmdiActionGroup *operator[](const QString name);
    qmdiActionGroup *getActionGroup(const QString &name);
    void mergeGroupList(qmdiActionGroupList *group);
    void unmergeGroupList(qmdiActionGroupList *group);

    QMenuBar *updateMenu(QMenuBar *menubar);
    QMenuBar *updateMenuBar(QMenuBar *menubar);
    QMenu *updatePopMenu(QMenu *popupMenu);
    QList<QToolBar *> *updateToolBar(QList<QToolBar *> *toolbars, QMainWindow *window);

  private:
    QList<qmdiActionGroup *> actionGroups;
};
