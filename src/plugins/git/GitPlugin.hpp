#pragma once

#include "iplugin.h"

namespace Ui {
class GitCommandsForm;
}

class GitPlugin : public IPlugin {
    Q_OBJECT

    enum class GitLog { File, Project };

  public:
    GitPlugin();
    ~GitPlugin();

    virtual void on_client_merged(qmdiHost *host) override;
    virtual void on_client_unmerged(qmdiHost *host) override;

  protected slots:
    void logFileHandler();
    void logProjectHandler();
    void logHandler(GitLog log, const QString &filename);
    void on_gitCommitClicked(const QModelIndex &mi);

  private:
    QAction *diffFile = nullptr;
    QAction *logFile = nullptr;
    QAction *logProject = nullptr;
    QAction *revert = nullptr;
    QAction *commit = nullptr;
    QAction *stash = nullptr;
    QAction *branches = nullptr;

    QDockWidget *gitDock = nullptr;
    Ui::GitCommandsForm *form = nullptr;
};
