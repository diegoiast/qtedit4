#pragma once

#include "iplugin.h"
#include "kitdefinitions.h"
#include <QFileSystemWatcher>
#include <QProcess>

class ProjectBuildModel;
class ProjectIssuesWidget;
class FoldersModel;
class DirectoryModel;
class FilterOutProxyModel;
class KitDefinitionModel;

class CommandPalette;
class ProjectSearch;

struct ProjectBuildConfig;
struct TaskInfo;
struct ExecutableInfo;

namespace Ui {
class ProjectManagerGUI;
class BuildRunOutput;
} // namespace Ui

class ProjectManagerPlugin : public IPlugin {

    struct Config {
        CONFIG_DEFINE(ExtraPath, QStringList);
        CONFIG_DEFINE(OpenDirs, QStringList);
        CONFIG_DEFINE(FilterShow, QString);
        CONFIG_DEFINE(FilterOut, QString);
        CONFIG_DEFINE(SearchPattern, QString);
        CONFIG_DEFINE(SearchInclude, QString);
        CONFIG_DEFINE(SearchExclude, QString);
        qmdiPluginConfig *config;
    };
    Config &getConfig() {
        static Config configObject{&this->config};
        return configObject;
    }

    Q_OBJECT
  public:
    ProjectManagerPlugin();

    virtual void showAbout() override;
    virtual void on_client_merged(qmdiHost *host) override;
    virtual void loadConfig(QSettings &settings) override;
    virtual void saveConfig(QSettings &settings) override;

    std::shared_ptr<ProjectBuildConfig> getCurrentConfig() const;
    const QHash<QString, QString> getConfigDictionary() const;
    const KitDefinition *getCurrentKit() const;

  public slots:
    void onItemClicked(const QModelIndex &index);
    void addProject_clicked();
    void removeProject_clicked();
    void newProjectSelected(int index);

    void do_runExecutable(const ExecutableInfo *info);
    void do_runTask(const TaskInfo *task);
    void runButton_clicked();
    void runTask_clicked();
    void clearProject_clicked();
    void projectFile_modified(const QString &path);

  private:
    auto updateTasksUI(std::shared_ptr<ProjectBuildConfig> buildConfig) -> void;
    auto updateExecutablesUI(std::shared_ptr<ProjectBuildConfig> buildConfig) -> void;

    int panelIndex = -1;
    Ui::ProjectManagerGUI *gui = nullptr;
    Ui::BuildRunOutput *outputPanel = nullptr;
    QDockWidget* outputDock;
    QDockWidget* issuesDock;
    ProjectIssuesWidget *projectIssues = nullptr;

    QFileSystemWatcher configWatcher;
    ExecutableInfo *selectedTarget = nullptr;
    int selectedTaskIndex = -1;

    QProcess runProcess;

    KitDefinitionModel *kitsModel = nullptr;
    ProjectBuildModel *projectModel = nullptr;
    DirectoryModel *directoryModel = nullptr;
    FilterOutProxyModel *filesFilterModel = nullptr;
    CommandPalette *commandPalette = nullptr;
    ProjectSearch *searchPanelUI = nullptr;

    QAction *runAction = nullptr;
    QAction *buildAction = nullptr;
    QAction *clearAction = nullptr;
    QMenu *availableTasksMenu = nullptr;
    QMenu *availableExecutablesMenu = nullptr;
};
