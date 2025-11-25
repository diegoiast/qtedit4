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
    GitCommitDisplay(QWidget *parent) : QWidget(parent) {
        ui.setupUi(this);

        auto fnt = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        fnt.setFixedPitch(true);
        ui.rawGitCommit->setFont(fnt);
    }
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
    form->listView->setAlternatingRowColors(true);

    auto delegate = new CommitDelegate(form->listView);
    form->listView->setItemDelegate(delegate);
    connect(form->listView, &QAbstractItemView::clicked, this, &GitPlugin::on_gitCommitClicked);

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

    widget->ui.rawGitCommit->setPlainText(*fullCommit.raw);
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
}
