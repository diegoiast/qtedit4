#pragma once

/**
 * \file qmdiactiongroup.h
 * \brief Definition of the action group class
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL 2 or 3
 * \see qmdiActionGroup
 */

#include <QList>
#include <QString>

class QAction;
class QObject;
class QWidget;
class QMenu;
class QToolBar;
class QActionGroup;

class qmdiActionGroup {
    friend class qmdiHost;

  public:
    qmdiActionGroup(QString name);
    ~qmdiActionGroup();

    void setName(const QString &newName);
    QString getName();
    void addAction(QAction *action, int location = -1);
    void addActions(QActionGroup *actions, int location = -1);
    void addWidget(QWidget *widget, int location = -1);
    void addMenu(QMenu *menu, int location = -1);
    void addSeparator(int location = -1);
    bool containsAction(QAction *action);
    void removeAction(QAction *action);
    void removeActions(QActionGroup *actions);
    void removeMenu(QMenu *menu);
    void removeWidget(QWidget *widget);
    void setMergePoint();
    int getMergePoint();

    void mergeGroup(qmdiActionGroup *group);
    void unmergeGroup(qmdiActionGroup *group);

    QMenu *updateMenu(QMenu *menu = nullptr);
    QToolBar *updateToolBar(QToolBar *toolbar);

    bool breakAfter;

  private:
    QString name;
    QList<QObject *> actionGroupItems;
    QList<qmdiActionGroup *> actionGroups;

    int breakCount;
    int mergeLocation;
};
