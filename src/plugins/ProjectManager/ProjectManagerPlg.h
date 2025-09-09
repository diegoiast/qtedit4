#pragma once

#include "iplugin.h"
#include "kitdefinitions.h"
#include <QAbstractItemModel>
#include <QFileSystemWatcher>
#include <QProcess>

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

class ProjectBuildModel : public QAbstractListModel {
    std::vector<std::shared_ptr<ProjectBuildConfig>> configs;

  public:
    void addConfig(std::shared_ptr<ProjectBuildConfig> config);
    void removeConfig(size_t index);
    std::shared_ptr<ProjectBuildConfig> getConfig(size_t index) const;
    int findConfigDirIndex(const QString &dir);
    std::shared_ptr<ProjectBuildConfig> findConfigDir(const QString &dir);
    std::shared_ptr<ProjectBuildConfig> findConfigFile(const QString &fileName);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;

    QStringList getAllOpenDirs() const;
};

class ProjectManagerPlugin : public IPlugin {

    struct Config {
        CONFIG_DEFINE(SaveBeforeTask, bool);
        CONFIG_DEFINE(BlackConsole, bool);
        CONFIG_DEFINE(ExtraPath, QStringList);
        CONFIG_DEFINE(OpenDirs, QStringList);
        CONFIG_DEFINE(SelectedDirectory, QString);
        CONFIG_DEFINE(FilterShow, QString);
        CONFIG_DEFINE(FilterOut, QString);
        CONFIG_DEFINE(SearchPath, QString);
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
    virtual void configurationHasBeenModified() override;
    virtual void loadConfig(QSettings &settings) override;
    virtual void saveConfig(QSettings &settings) override;

    virtual qmdiActionGroup *getContextMenuActions(const QString &menuId,
                                                   const QString &filePath) override;

    int canOpenFile(const QString &fileName) override;
    bool openFile(const QString &fileName, int = -1, int = -1, int = -1) override;
    std::shared_ptr<ProjectBuildConfig> getCurrentConfig() const;
    const KitDefinition *getCurrentKit() const;

  public slots:
    void onItemClicked(const QString &fileName);
    void addProject_clicked();
    void removeProject_clicked();
    void newProjectSelected(int index);

    void runCommand(const QString &workingDirectory, const QString &programOrCommand,
                    const QStringList &arguments, const QProcessEnvironment &env);
    void do_runExecutable(const ExecutableInfo *info);
    void do_runTask(const TaskInfo *task);
    void runButton_clicked();
    void runTask_clicked();
    void clearProject_clicked();
    void projectFile_modified(const QString &path);

  private:
    auto addProjectFromDir(const QString &dir) -> void;
    auto saveAllDocuments() -> bool;
    auto processBuildOutput(const QString &line) -> void;
    auto updateTasksUI(std::shared_ptr<ProjectBuildConfig> buildConfig) -> void;
    auto updateExecutablesUI(std::shared_ptr<ProjectBuildConfig> buildConfig) -> void;
    auto tryOpenProject(const QString &filename, const QString &dir) -> bool;
    auto tryScrollOutput(int line) -> bool;

    int panelIndex = -1;
    Ui::ProjectManagerGUI *gui = nullptr;
    Ui::BuildRunOutput *outputPanel = nullptr;
    QDockWidget *projectDock = nullptr;
    QDockWidget *outputDock = nullptr;
    QDockWidget *issuesDock = nullptr;
    ProjectIssuesWidget *projectIssues = nullptr;

    QFileSystemWatcher configWatcher;
    ExecutableInfo *selectedTarget = nullptr;
    int selectedTaskIndex = -1;

    QProcess runProcess;

    KitDefinitionModel *kitsModel = nullptr;
    ProjectBuildModel *projectModel = nullptr;
    CommandPalette *commandPalette = nullptr;
    ProjectSearch *searchPanelUI = nullptr;

    QAction *runAction = nullptr;
    QAction *buildAction = nullptr;
    QAction *clearAction = nullptr;
    QMenu *availableTasksMenu = nullptr;
    QMenu *availableExecutablesMenu = nullptr;
};
