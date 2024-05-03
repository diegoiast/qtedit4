#pragma once

#include "iplugin.h"
#include <QAbstractItemModel>
#include <QCompleter>
#include <QList>

class QDockWidget;
class QTreeView;
class QCompleter;
class QSortFilterProxyModel;

class FoldersModel;
class DirectoryModel;
class FilterOutProxyModel;

namespace Ui {
class ProjectManagerGUI;
}

class ProjectManagerPlugin : public IPlugin {
    Q_OBJECT
  public:
    ProjectManagerPlugin();

    virtual void showAbout() override;
    virtual void on_client_merged(qmdiHost *host) override;
    virtual void on_client_unmerged(qmdiHost *host) override;
    virtual void loadConfig(QSettings &settings) override;
    virtual void saveConfig(QSettings &settings) override;

  public slots:
    // our code
    void onItemClicked(const QModelIndex &index);
    void on_addDirectory_clicked(bool checked);
    void on_removeDirectory_clicked(bool checked);

  private:
    QDockWidget *m_dockWidget;
    FilterOutProxyModel *filesFilterModel;
    DirectoryModel *directoryModel;
    Ui::ProjectManagerGUI *gui;
};
