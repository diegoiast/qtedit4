#pragma once

#include "iplugin.h"
#include <QFileSystemWatcher>
#include <QProcess>

class QDockWidget;
class QTreeView;
class QCompleter;
class QSortFilterProxyModel;

class ProjectBuildModel;
class FoldersModel;
class DirectoryModel;
class FilterOutProxyModel;

struct ProjectBuildConfig;
struct TaskInfo;
struct ExecutableInfo;

namespace Ui {
class ProjectManagerGUI;
class BuildRunOutput;
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

    std::shared_ptr<ProjectBuildConfig> getCurrentConfig() const;
    const QHash<QString, QString> getConfigHash() const;

  public slots:
    void onItemClicked(const QModelIndex &index);
    void on_addProject_clicked();
    void on_removeProject_clicked();
    void on_newProjectSelected(int index);

    void do_runExecutable(const ExecutableInfo *info);
    void do_runTask(const TaskInfo *task);
    void on_runButton_clicked();
    void on_runTask_clicked();
    void on_clearProject_clicked();
    void on_projectFile_modified(const QString &path);

  private:
    auto updateTasksUI(std::shared_ptr<ProjectBuildConfig> config) -> void;
    auto updateExecutablesUI(std::shared_ptr<ProjectBuildConfig> config) -> void;

    int panelIndex = -1;
    Ui::ProjectManagerGUI *gui = nullptr;
    Ui::BuildRunOutput *outputPanel = nullptr;

    QFileSystemWatcher configWatcher;
    ExecutableInfo *selectedTarget;
    TaskInfo *selectedTask;

    QProcess runProcess;

    ProjectBuildModel *projectModel = nullptr;
    DirectoryModel *directoryModel;
    FilterOutProxyModel *filesFilterModel;

    QAction *runAction = nullptr;
    QAction *buildAction = nullptr;
    QAction *clearAction = nullptr;
    QMenu *availableTasksMenu;
    QMenu *availableExecutablesMenu;
};
