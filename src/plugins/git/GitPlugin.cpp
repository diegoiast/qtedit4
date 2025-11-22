#include "GitPlugin.hpp"

#include "CommitDelegate.hpp"
#include "CommitModel.hpp"
#include "ui_GitCommands.h"

#include <QDockWidget>
#include <QFileInfo>
#include <QProcess>

GitPlugin::GitPlugin() {
    name = tr("Help system browser");
    author = tr("Diego Iastrubni <diegoiast@gmail.com>");
    iVersion = 0;
    sVersion = "0.0.1";
    autoEnabled = true;
    alwaysEnabled = false;
}

GitPlugin::~GitPlugin() {}

void GitPlugin::on_client_merged(qmdiHost *host) {
    IPlugin::on_client_merged(host);

    diffFile = new QAction(tr("Diff current file"), this);
    logFile = new QAction(tr("Log current file"), this);
    logProject = new QAction(tr("Log project/dir"), this);
    revert = new QAction(tr("Revert"), this);
    commit = new QAction(tr("Commit"), this);
    stash = new QAction(tr("Stash"), this);
    branches = new QAction(tr("Branches"), this);

    diffFile->setToolTip(tr("Show changes (current file)"));
    diffFile->setShortcut(QKeySequence("Altl+G, D"));
    logFile->setToolTip(tr("Show commits (current file)"));
    logFile->setShortcut(QKeySequence("Alt+G, F"));
    logProject->setToolTip(tr("Show commits (current project)"));
    logProject->setShortcut(QKeySequence("Alt+G, L"));
    revert->setToolTip(tr("Revert existing commits"));
    commit->setToolTip(tr("Record changes to the repository"));
    stash->setToolTip(tr("tash away changes to dirty working directory"));
    branches->setToolTip(tr("List, create, or delete branches"));

    connect(logFile, &QAction::triggered, this, &GitPlugin::logFileHandler);
    connect(logProject, &QAction::triggered, this, &GitPlugin::logProjectHandler);

    auto menuName = "&Git";
    host->menus.addActionGroup(menuName, "&Project");
    menus[menuName]->addAction(diffFile);
    menus[menuName]->addAction(logFile);
    menus[menuName]->addAction(logProject);
    menus[menuName]->addAction(revert);
    menus[menuName]->addAction(commit);
    menus[menuName]->addAction(stash);
    menus[menuName]->addAction(branches);

    auto manager = dynamic_cast<PluginManager *>(host);
    auto w = new QWidget;
    form = new Ui::GitCommandsForm();
    form->setupUi(w);
    form->listView->setViewMode(QListView::ListMode);
    form->listView->setFlow(QListView::TopToBottom);
    form->listView->setAlternatingRowColors(true);

    auto delegate = new CommitDelegate(form->listView);
    form->listView->setItemDelegate(delegate);
    form->listView->setItemDelegate(new CommitDelegate(form->listView));
    form->listView->setViewMode(QListView::ListMode);
    form->listView->setUniformItemSizes(true);
    form->listView->setResizeMode(QListView::Adjust);
    form->listView->setWrapping(false);

    gitDock = manager->createNewPanel(Panels::West, "gitpanel", tr("Git"), w);
}

void GitPlugin::on_client_unmerged(qmdiHost *host) {
    IPlugin::on_client_unmerged(host);
    delete gitDock;
}

void GitPlugin::logFileHandler() {
    auto manager = getManager();
    auto client = manager->getMdiServer()->getCurrentClient();
    auto filename = client->mdiClientFileName();
    logHandler(GitLog::File, filename);
}

void GitPlugin::logProjectHandler() {
    auto manager = getManager();
    auto client = manager->getMdiServer()->getCurrentClient();
    auto filename = client->mdiClientFileName();
    logHandler(GitLog::Project, filename);
}

void GitPlugin::logHandler(GitLog log, const QString &filename) {
    auto model = new CommitModel(this);
    form->label->setText(tr("git log"));

    auto res = false;
    switch (log) {
    case GitPlugin::GitLog::File:
        res = model->loadFileHistory(filename);
        break;
    case GitPlugin::GitLog::Project:
        res = model->loadProjectHistory(filename);
        break;
    }

    if (!res) {
        form->label->setText(tr("No commits or not a git repo"));
        delete model;
        return;
    }

    form->listView->setModel(model);
    gitDock->raise();
    gitDock->show();
}
