#pragma once

#include "iplugin.h"
#include <QAbstractItemModel>
#include <QAbstractListModel>
#include <QCompleter>
#include <QList>

class QDockWidget;
class QTreeView;
class QCompleter;
class QSortFilterProxyModel;

class FoldersModel;
class DirectoryModel;
class FilterOutProxyModel;

struct ExecutableInfo {
    QString name;
    QHash<QString, QString> executables;
};

struct TaskInfo {
    QString name;
    QString command;
};

struct ProjectBuildConfig {
    QString sourceDir;
    QString buildDir;
    QList<ExecutableInfo> executables;
    QList<TaskInfo> tasksInfo;
    TaskInfo *buildTask;

    static std::shared_ptr<ProjectBuildConfig> buildFromDirectory(const QString directory);
    static std::shared_ptr<ProjectBuildConfig> buildFromFile(const QString jsonFileName);
};

class ProjectBuildModel : public QAbstractListModel {
    std::vector<std::shared_ptr<ProjectBuildConfig>> configs;

  public:
    void addConfig(std::shared_ptr<ProjectBuildConfig> config);
    void removeConfig(size_t index);
    std::shared_ptr<ProjectBuildConfig> getConfig(size_t index) const;
    std::shared_ptr<ProjectBuildConfig> findConfig(const QString dir);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
};

namespace Ui {
class ProjectManagerGUI;
} // namespace Ui

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
    void onItemClicked(const QModelIndex &index);
    void on_addProject_clicked(bool checked);
    void on_removeProject_clicked(bool checked);
    void on_newProjectSelected(int index);

    void on_runButton_clicked();
    void on_runTask_clicked();
    void on_clearProject_clicked();

  public:
    std::shared_ptr<ProjectBuildConfig> getCurrentConfig() const;

  private:
    int panelIndex = -1;
    Ui::ProjectManagerGUI *gui = nullptr;

    QString executableName;
    QString executablePath;
    QString taskName;
    QString taskCommand;

    ProjectBuildModel *projectModel = nullptr;
    DirectoryModel *directoryModel;
    FilterOutProxyModel *filesFilterModel;
};
