#include <QDockWidget>
#include <QFileInfo>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QLabel>
#include <QPainter>
#include <QPlainTextEdit>
#include <QProcess>
#include <QResizeEvent>
#include <QScrollArea>
#include <QStringListModel>

#include "CommitDelegate.hpp"
#include "CommitModel.hpp"
#include "GitPlugin.hpp"
#include "GlobalCommands.hpp"
#include "ui_GitCommands.h"
#include "ui_GitCommit.h"
#include "widgets/AutoShrinkLabel.hpp"

QString shortGitSha1(const QString &fullSha1, int length = 7) {
    if (length <= 0) {
        return QString();
    }

    if (fullSha1.size() <= length) {
        return fullSha1;
    }

    return fullSha1.left(length);
}

class GitCommitDisplay : public QWidget {
  public:
    explicit GitCommitDisplay(QWidget *parent) : QWidget(parent) { ui.setupUi(this); }
    Ui::GitCommit ui;
};

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

    diffFile = new QAction(tr("git diff current file"), this);
    logFile = new QAction(tr("git log current file"), this);
    logProject = new QAction(tr("git log project/dir"), this);
    revert = new QAction(tr("git revert"), this);
    commit = new QAction(tr("git commit"), this);
    stash = new QAction(tr("git stash"), this);
    branches = new QAction(tr("git branch"), this);

    diffFile->setToolTip(tr("GIT: Show changes (current file)"));
    diffFile->setShortcut(QKeySequence("Ctrl+G, D"));
    logFile->setToolTip(tr("Show commits (current file)"));
    logFile->setShortcut(QKeySequence("Ctrl+G, F"));
    logProject->setToolTip(tr("Show commits (current project)"));
    logProject->setShortcut(QKeySequence("Ctrl+G, L"));
    revert->setToolTip(tr("Revert existing commits"));
    commit->setToolTip(tr("Record changes to the repository"));
    stash->setToolTip(tr("tash away changes to dirty working directory"));
    branches->setToolTip(tr("List, create, or delete branches"));

    connect(logFile, &QAction::triggered, this, &GitPlugin::logFileHandler);
    connect(logProject, &QAction::triggered, this, &GitPlugin::logProjectHandler);
    connect(diffFile, &QAction::triggered, this, &GitPlugin::diffFileHandler);

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
    form->listView->setAlternatingRowColors(true);

    auto delegate = new CommitDelegate(form->listView);
    form->listView->setItemDelegate(delegate);
    form->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(form->listView, &QAbstractItemView::clicked, this, &GitPlugin::on_gitCommitClicked);
    connect(form->listView, &QAbstractItemView::doubleClicked, this, &GitPlugin::on_gitCommitDoubleClicked);
    

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
void on_gitDiff(const QModelIndex &mi);

void GitPlugin::diffFileHandler() {
    auto manager = getManager();
    auto const model = static_cast<CommitModel *>(form->listView->model());
    auto client = manager->getMdiServer()->getCurrentClient();
    auto filename = client->mdiClientFileName();
    auto const diff = model->getCurrentDiff(filename);

    CommandArgs args = {
        {GlobalArguments::FileName, QString("%1.diff").arg(client->mdiClientName)},
        {GlobalArguments::Content, diff},
        {GlobalArguments::ReadOnly, true},
    };
    manager->handleCommandAsync(GlobalCommands::DisplayText, args);
}

void GitPlugin::logHandler(GitLog log, const QString &filename) {
    auto model = new CommitModel(this);

    auto res = false;
    switch (log) {
    case GitPlugin::GitLog::File:
        form->label->setText(QString("git log %1").arg(filename));
        res = model->loadFileHistory(filename);
        break;
    case GitPlugin::GitLog::Project:
        form->label->setText(QString("git log (repo)"));
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

void GitPlugin::on_gitCommitClicked(const QModelIndex &mi) {
    auto const *model = static_cast<CommitModel *>(form->listView->model());
    auto const sha1 = model->data(mi, CommitModel::Roles::HashRole).toString();
    auto const sha1Short = shortGitSha1(sha1);
    auto const fullCommit = model->getFullCommitInfo(sha1);
    auto widget = static_cast<GitCommitDisplay *>(form->container->widget(0));

    if (!widget) {
        widget = new GitCommitDisplay(form->container);
        form->container->addWidget(widget);
    }

    widget->ui.sha1->setPrimaryText(sha1);
    widget->ui.sha1->setFallbackText(sha1Short);
    widget->ui.commiter->setText(fullCommit.author.toString());
    widget->ui.commitDate->setText(fullCommit.date.toString());
    widget->ui.commit->setText(fullCommit.subject.toString());
    widget->ui.commitMessage->setVisible(!fullCommit.body.trimmed().isEmpty());
    widget->ui.commitMessage->setMarkdown(fullCommit.body);

    auto s = QStringList();
    for (auto ss : fullCommit.files) {
        s.push_back(ss.filename.toString());
    }
    widget->ui.commits->setModel(new QStringListModel(s));
    widget->ui.commits->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void GitPlugin::on_gitCommitDoubleClicked(const QModelIndex &mi) {
    auto const *model = static_cast<CommitModel *>(form->listView->model());
    auto const sha1 = model->data(mi, CommitModel::Roles::HashRole).toString();
    auto const fullCommit = model->getFullCommitInfo(sha1);

    auto manager = getManager();
    CommandArgs args = {
        {GlobalArguments::FileName, QString("%1.diff").arg(sha1)},
        {GlobalArguments::Content, *fullCommit.raw},
        {GlobalArguments::ReadOnly, true},
    };
    manager->handleCommandAsync(GlobalCommands::DisplayText, args);
}
