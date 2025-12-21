#pragma once

#include "iplugin.h"

namespace Ui {
class GitCommandsForm;
}

class GitPlugin : public IPlugin {
    Q_OBJECT
    struct Config {
        CONFIG_DEFINE(GitBinary, QString)
        CONFIG_DEFINE(GitHomepage, QString)
        qmdiPluginConfig *config;
    };
    Config &getConfig() {
        static Config configObject{&this->config};
        return configObject;
    }

    enum class GitLog { File, Project };

  public:
    GitPlugin();
    ~GitPlugin();

    virtual void on_client_merged(qmdiHost *host) override;
    virtual void on_client_unmerged(qmdiHost *host) override;

  protected slots:
    void logFileHandler();
    void logProjectHandler();
    void diffFileHandler();
    void logHandler(GitPlugin::GitLog log, const QString &filename);
    void on_gitCommitClicked(const QModelIndex &mi);
    void on_gitCommitDoubleClicked(const QModelIndex &mi);

    QString runGit(const QStringList &args);
    QString detectRepoRoot(const QString &path);
    QString getDiff(const QString &path);
    QString getRawCommit(const QString &sha1);

  private:
    QAction *diffFile = nullptr;
    QAction *logFile = nullptr;
    QAction *logProject = nullptr;
    QAction *revert = nullptr;
    QAction *commit = nullptr;
    QAction *stash = nullptr;
    QAction *branches = nullptr;

    QString gitBinary = "git";
    QString repoRoot;

    QDockWidget *gitDock = nullptr;
    Ui::GitCommandsForm *form = nullptr;
};
