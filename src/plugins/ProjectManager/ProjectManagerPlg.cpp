#include <QClipboard>
#include <QDockWidget>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QSortFilterProxyModel>

#include "GenericItems.h"
#include "ProjectManagerPlg.h"
#include "ProjectSearch.h"
#include "pluginmanager.h"
#include "qjsonobject.h"
#include "qmdihost.h"
#include "qmdiserver.h"
#include "ui_ProjectManagerGUI.h"
#include "ui_output.h"

static auto str(QProcess::ExitStatus e) -> QString {
    switch (e) {
    case QProcess::ExitStatus::NormalExit:
        return "Normal exit";
    case QProcess::ExitStatus::CrashExit:
        return "Crashed";
    }
    return "";
}

ProjectManagerPlugin::ProjectManagerPlugin() {
    name = tr("Project manager");
    author = tr("Diego Iastrubni <diegoiast@gmail.com>");
    iVersion = 0;
    sVersion = "0.0.1";
    autoEnabled = true;
    alwaysEnabled = true;

    directoryModel = NULL;
}

void ProjectManagerPlugin::showAbout() {
    QMessageBox::information(dynamic_cast<QMainWindow *>(mdiServer), "About",
                             "The project manager plugin");
}

void ProjectManagerPlugin::on_client_merged(qmdiHost *host) {
    auto manager = dynamic_cast<PluginManager *>(host);
    auto *w = new QWidget;

    gui = new Ui::ProjectManagerGUI;
    gui->setupUi(w);
    gui->addDirectory->setIcon(QIcon::fromTheme("list-add"));
    gui->removeDirectory->setIcon(QIcon::fromTheme("list-remove"));
    gui->filterFiles->setClearButtonEnabled(true);
    gui->filterFiles->setPlaceholderText("files to show");
    gui->filterOutFiles->setClearButtonEnabled(true);
    gui->filterOutFiles->setPlaceholderText("files to hide");

    connect(gui->runButton, &QAbstractButton::clicked, this,
            &ProjectManagerPlugin::on_runButton_clicked);
    connect(gui->taskButton, &QAbstractButton::clicked, this,
            &ProjectManagerPlugin::on_runTask_clicked);
    connect(gui->cleanButton, &QAbstractButton::clicked, this,
            &ProjectManagerPlugin::on_clearProject_clicked);
    connect(gui->addDirectory, &QAbstractButton::clicked, this,
            &ProjectManagerPlugin::on_addProject_clicked);
    connect(gui->removeDirectory, &QAbstractButton::clicked, this,
            &ProjectManagerPlugin::on_removeProject_clicked);
    connect(gui->filesView, &QAbstractItemView::clicked, this,
            &ProjectManagerPlugin::onItemClicked);

    connect(gui->comboBox, &QComboBox::currentIndexChanged, this,
            &ProjectManagerPlugin::on_newProjectSelected);
    manager->createNewPanel(Panels::West, QString("Project"), w);

    auto *w2 = new QWidget;
    outputPanel = new Ui::BuildRunOutput;
    outputPanel->setupUi(w2);
    manager->createNewPanel(Panels::South, QString("Output"), w2);

    connect(outputPanel->clearOutput, &QAbstractButton::clicked,
            [this]() { this->outputPanel->commandOuput->clear(); });
    connect(outputPanel->copyOutput, &QAbstractButton::clicked, [this]() {
        auto text = this->outputPanel->commandOuput->document()->toPlainText();
        auto clipboard = QGuiApplication::clipboard();
        clipboard->setText(text);
    });
    connect(outputPanel->playButton, &QAbstractButton::clicked,
            [this]() { this->gui->runButton->animateClick(); });

    connect(outputPanel->buildButton, &QAbstractButton::clicked,
            [this]() { this->gui->taskButton->animateClick(); });

    connect(outputPanel->cancelButton, &QAbstractButton::clicked, [this]() {
        if (this->runProcess.processId() != 0) {
            this->runProcess.kill();
        }
    });

    connect(&runProcess, &QProcess::readyReadStandardOutput, [this]() {
        auto output = this->runProcess.readAllStandardOutput();
        this->outputPanel->commandOuput->appendPlainText(output);
    });
    connect(&runProcess, &QProcess::readyReadStandardError, [this]() {
        auto output = this->runProcess.readAllStandardError();
        this->outputPanel->commandOuput->appendPlainText(output);
    });
    connect(&runProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
                auto output = QString("[code=%1, status=%2]").arg(exitCode).arg(str(exitStatus));
                this->outputPanel->commandOuput->appendPlainText(output);
            });
    connect(&runProcess, &QProcess::errorOccurred, [this](QProcess::ProcessError error) {
        auto output = QString("[error: code=%1]").arg((int)error);
        this->outputPanel->commandOuput->appendPlainText(output);
        qWarning() << "Process error occurred:" << error;
        qWarning() << "Error string:" << runProcess.errorString();
    });

    connect(gui->cleanButton, &QToolButton::clicked,
            [this]() { this->outputPanel->commandOuput->clear(); });
    directoryModel = new DirectoryModel(this);
    filesFilterModel = new FilterOutProxyModel(this);
    filesFilterModel->setSourceModel(directoryModel);
    filesFilterModel->sort(0);
    gui->filesView->setModel(filesFilterModel);
    connect(gui->filterFiles, &QLineEdit::textChanged,
            [this](const QString &newText) { filesFilterModel->setFilterWildcards(newText); });
    connect(gui->filterOutFiles, &QLineEdit::textChanged,
            [this](const QString &newText) { filesFilterModel->setFilterOutWildcard(newText); });

    auto *searchPanelUI = new ProjectSearch(manager, directoryModel);
    auto seachID = manager->createNewPanel(Panels::West, tr("Search"), searchPanelUI);

    auto projectSearch = new QAction(tr("Search in project"));
    projectSearch->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_F));
    connect(projectSearch, &QAction::triggered, [seachID, manager, searchPanelUI]() {
        manager->showPanel(Panels::West, seachID);
        searchPanelUI->setFocusOnSearch();
    });
    manager->addAction(projectSearch);

    projectModel = new ProjectBuildModel();
    gui->comboBox->setModel(projectModel);

    runAction = new QAction(QIcon::fromTheme("document-save"), tr("&Run"), this);
    buildAction = new QAction(QIcon::fromTheme("document-save-as"), tr("&Run task"), this);
    clearAction = new QAction(QIcon::fromTheme("edit-clear"), tr("&Delete build directory"), this);

    runAction->setEnabled(false);
    buildAction->setEnabled(false);
    clearAction->setEnabled(false);

    connect(runAction, SIGNAL(triggered()), this, SLOT(on_runButton_clicked()));
    connect(buildAction, SIGNAL(triggered()), this, SLOT(on_runTask_clicked()));
    connect(clearAction, SIGNAL(triggered()), this, SLOT(on_clearProject_clicked()));

    runAction->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_R));
    buildAction->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_B));

    this->menus[tr("&Project")]->addAction(runAction);
    this->menus[tr("&Project")]->addAction(buildAction);
    this->menus[tr("&Project")]->addAction(clearAction);

    this->availableTasksMenu = new QMenu(tr("Available tasks"));
    this->availableExecutablesMenu = new QMenu(tr("Available executables"));
    this->menus[tr("&Project")]->addMenu(availableTasksMenu);
    this->menus[tr("&Project")]->addMenu(availableExecutablesMenu);

    auto addNewProjectIcon = new QAction(tr("Add existing project.."));
    auto removeProjectIcon = new QAction(tr("Close project"));
    connect(addNewProjectIcon, &QAction::triggered,
            [this]() { this->gui->addDirectory->animateClick(); });
    connect(removeProjectIcon, &QAction::triggered,
            [this]() { this->gui->removeDirectory->animateClick(); });

    this->menus[tr("&Project")]->addSeparator();
    this->menus[tr("&Project")]->addAction(addNewProjectIcon);
    this->menus[tr("&Project")]->addAction(removeProjectIcon);
}

void ProjectManagerPlugin::on_client_unmerged(qmdiHost *host) { Q_UNUSED(host); }

void ProjectManagerPlugin::loadConfig(QSettings &settings) {
    settings.beginGroup("ProjectManager");
    gui->filterOutFiles->setText(settings.value("Filter-Out", "").toString());
    gui->filterFiles->setText(settings.value("Filter-In", "").toString());
    filesFilterModel->invalidate();

    settings.beginGroup("Loaded");
    foreach (auto s, settings.childKeys()) {
        if (!s.startsWith("dir")) {
            continue;
        }
        auto dirName = settings.value(s).toString();
        auto config = projectModel->findConfig(dirName);
        if (!config) {
            config = ProjectBuildConfig::buildFromDirectory(dirName);
            projectModel->addConfig(config);
        }
    }
    settings.endGroup();

    settings.endGroup();
}

void ProjectManagerPlugin::saveConfig(QSettings &settings) {
    settings.beginGroup("ProjectManager");
    settings.setValue("Filter-Out", gui->filterOutFiles->text());
    settings.setValue("Filter-In", gui->filterFiles->text());

    settings.remove("Loaded");
    settings.beginGroup("Loaded");
    for (auto i = 0; i < projectModel->rowCount(); i++) {
        auto config = projectModel->getConfig(i);
        settings.setValue(QString("dir%1").arg(i), config->sourceDir);
    }
    settings.endGroup();

    settings.endGroup();
}

void ProjectManagerPlugin::onItemClicked(const QModelIndex &index) {
    auto i = filesFilterModel->mapToSource(index);
    auto s = directoryModel->getItem(i.row());
    PluginManager *pluginManager = dynamic_cast<PluginManager *>(mdiServer->mdiHost);
    if (pluginManager) {
        pluginManager->openFile(s);
    }
}

void ProjectManagerPlugin::on_addProject_clicked(bool) {
    QString dirName = QFileDialog::getExistingDirectory(gui->filesView, tr("Add directory"));
    if (dirName.isEmpty()) {
        return;
    }
    auto config = projectModel->findConfig(dirName);
    if (config) {
        return;
    }
    config = ProjectBuildConfig::buildFromDirectory(dirName);
    projectModel->addConfig(config);
    directoryModel->addDirectory(dirName);
}

QString findExecForPlatform(QHash<QString, QString> files) {
    // TODO
    return files["linux"];
}

void ProjectManagerPlugin::on_newProjectSelected(int index) {
    auto p = projectModel->getConfig(index);
    directoryModel->removeAllDirs();
    directoryModel->addDirectory(p->sourceDir);

    auto config = getCurrentConfig();
    if (!config || config->executables.size() == 0) {
        this->selectedTarget = nullptr;
        this->gui->runButton->setText("...");
        this->gui->runButton->setToolTip("...");
        this->gui->runButton->setEnabled(false);

        this->runAction->setEnabled(false);
        this->buildAction->setEnabled(false);
        this->clearAction->setEnabled(false);
        this->availableExecutablesMenu->clear();
    } else {
        auto executableName = config->executables[0].name;
        auto executablePath = findExecForPlatform(config->executables[0].executables);
        this->gui->runButton->setEnabled(true);
        this->gui->runButton->setText(executableName);
        this->gui->runButton->setToolTip(executablePath);
        this->selectedTarget = &config->executables[0];

        this->runAction->setEnabled(true);
        this->runAction->setText(QString(tr("Run: %1")).arg(executableName));
        this->runAction->setToolTip(executablePath);

        this->clearAction->setEnabled(true);
        this->availableExecutablesMenu->hide();
        this->availableExecutablesMenu->clear();
        this->mdiServer->mdiHost->updateGUI();
        if (config->executables.size() == 1) {
            this->gui->runButton->setMenu(nullptr);
        } else {
            auto menu = new QMenu(gui->runButton);
            QList<QAction *> actions;
            for (auto target : config->executables) {
                QAction *action = new QAction(target.name, this);
                menu->addAction(action);
                actions.append(action);

                action = new QAction(target.name, this);
                connect(action, &QAction::triggered,
                        [this, target]() { this->do_runExecutable(&target); });
                this->availableExecutablesMenu->addAction(action);
            }
            this->mdiServer->mdiHost->updateGUI();
            this->gui->runButton->setMenu(menu);
            connect(menu, &QMenu::triggered, [this, actions, config](QAction *action) {
                auto index = actions.indexOf(action);
                auto executableName = config->executables[index].name;
                auto executablePath = findExecForPlatform(config->executables[index].executables);
                this->gui->runButton->setEnabled(true);
                this->gui->runButton->setText(executableName);
                this->gui->runButton->setToolTip(executablePath);
                this->selectedTarget = &config->executables[index];

                this->runAction->setEnabled(true);
                this->runAction->setText(QString(tr("Run: %1")).arg(executableName));
                this->runAction->setToolTip(executablePath);
            });
        }
    }

    if (!config || config->tasksInfo.size() == 0) {
        this->selectedTask = nullptr;
        this->gui->taskButton->setText("...");
        this->gui->taskButton->setToolTip("...");
        this->gui->taskButton->setEnabled(false);
        this->availableTasksMenu->clear();
    } else {
        auto taskName = config->tasksInfo[0].name;
        auto taskCommand = config->tasksInfo[0].command;
        this->gui->taskButton->setEnabled(true);
        this->gui->taskButton->setText(taskName);
        this->gui->taskButton->setToolTip(taskCommand);

        this->buildAction->setEnabled(true);
        this->buildAction->setText(QString(tr("Action: %1")).arg(taskName));
        this->buildAction->setToolTip(taskCommand);

        this->selectedTask = &config->tasksInfo[0];
        this->availableTasksMenu->hide();
        this->availableTasksMenu->clear();
        this->mdiServer->mdiHost->updateGUI();
        if (config->tasksInfo.size() == 1) {
            gui->taskButton->setMenu(nullptr);
        } else {
            auto menu = new QMenu(gui->runButton);
            QList<QAction *> actions;
            for (auto taskInfo : config->tasksInfo) {
                auto action = new QAction(taskInfo.name, this);
                menu->addAction(action);
                actions.append(action);

                action = new QAction(taskInfo.name, this);
                connect(action, &QAction::triggered,
                        [this, taskInfo]() { this->do_runTask(&taskInfo); });

                this->availableTasksMenu->addAction(action);
            }
            gui->taskButton->setMenu(menu);
            connect(menu, &QMenu::triggered, [this, actions, config](QAction *action) {
                auto index = actions.indexOf(action);
                auto taskName = config->tasksInfo[index].name;
                auto taskCommand = config->tasksInfo[index].command;
                this->gui->taskButton->setEnabled(true);
                this->gui->taskButton->setText(taskName);
                this->gui->taskButton->setToolTip(taskCommand);
                this->selectedTask = &config->tasksInfo[index];

                this->buildAction->setEnabled(true);
                this->buildAction->setText(QString(tr("Action: %1")).arg(taskName));
                this->buildAction->setToolTip(taskCommand);
            });
        }
    }
}

static auto expand(const QString &input, const QHash<QString, QString> &hashTable) -> QString {
    auto output = input;
    auto regex = QRegularExpression(R"(\$\{([a-zA-Z0-9_]+)\})");
    auto it = regex.globalMatch(input);

    while (it.hasNext()) {
        auto match = it.next();
        auto key = match.captured(1);
        auto replacement = hashTable.value(key, "");
        output.replace(match.captured(0), replacement);
    }
    return output;
}

void ProjectManagerPlugin::do_runExecutable(const ExecutableInfo *selectedTarget) {
    if (runProcess.processId() != 0) {
        runProcess.kill();
        return;
    }

    auto project = getCurrentConfig();
    auto hash = QHash<QString, QString>();
    hash["source_directory"] = project->sourceDir;
    hash["build_directory"] = project->buildDir;

    auto executablePath = findExecForPlatform(selectedTarget->executables);
    auto currentTask = expand(executablePath, hash);
    auto workingDirectory = expand(selectedTarget->runDirectory, hash);
    if (workingDirectory.isEmpty()) {
        workingDirectory = project->buildDir;
    }
    outputPanel->commandOuput->clear();
    outputPanel->commandOuput->appendPlainText("cd " + workingDirectory);
    outputPanel->commandOuput->appendPlainText(currentTask);

    // TODO - windows support
    auto command = QStringList();
    command.append("-c");
    command.append(currentTask);
    runProcess.start("/bin/bash", command);
    runProcess.setWorkingDirectory(workingDirectory);
    if (!runProcess.waitForStarted()) {
        qWarning() << "Process failed to start";
    }
}

void ProjectManagerPlugin::do_runTask(const TaskInfo *task) {
    if (runProcess.processId() != 0) {
        runProcess.kill();
        return;
    }

    auto project = getCurrentConfig();
    auto hash = QHash<QString, QString>();
    hash["source_directory"] = project->sourceDir;
    hash["build_directory"] = project->buildDir;

    auto currentTask = expand(task->command, hash);
    auto workingDirectory = expand(task->runDirectory, hash);

    outputPanel->commandOuput->clear();
    if (workingDirectory.isEmpty()) {
        workingDirectory = project->buildDir;
    }

    runProcess.setWorkingDirectory(workingDirectory);
    outputPanel->commandOuput->appendPlainText("cd " + workingDirectory);

    // TODO - windows support
    auto command = QStringList();
    command.append("-c");
    command.append(currentTask);
    outputPanel->commandOuput->appendPlainText(currentTask);
    runProcess.start("/bin/bash", command);
    if (!runProcess.waitForStarted()) {
        qWarning() << "Process failed to start";
        this->outputPanel->commandOuput->appendPlainText("Process failed to start");
    }
}

void ProjectManagerPlugin::on_removeProject_clicked() {
    // TODO
}

void ProjectManagerPlugin::on_runButton_clicked() {
    assert(this->selectedTarget);
    do_runExecutable(this->selectedTarget);
}

void ProjectManagerPlugin::on_runTask_clicked() {
    assert(this->selectedTask);
    do_runTask(this->selectedTask);
}

void ProjectManagerPlugin::on_clearProject_clicked() {
    auto project = getCurrentConfig();
    if (project->buildDir.isEmpty()) {
        return;
    }
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(
        tr("This will delete <b>%1</b>. Do you want to proceed?").arg(project->buildDir));
    msgBox.setWindowTitle(tr("Confirmation"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    int ret = msgBox.exec();
    switch (ret) {
    case QMessageBox::Yes: {
        // TODO - run this in a thread?
        auto outputDir = QDir(project->buildDir);
        outputDir.removeRecursively();
        break;
    }
    case QMessageBox::No:
        break;
    default:
        break;
    }
}

std::shared_ptr<ProjectBuildConfig> ProjectManagerPlugin::getCurrentConfig() const {
    auto currentIndex = gui->comboBox->currentIndex();
    if (currentIndex < 0) {
        return {};
    }
    return projectModel->getConfig(currentIndex);
}

std::shared_ptr<ProjectBuildConfig>
ProjectBuildConfig::buildFromDirectory(const QString directory) {
    auto configFileName = directory + "/" + "qtedit4.json";
    return buildFromFile(configFileName);
}

std::shared_ptr<ProjectBuildConfig> ProjectBuildConfig::buildFromFile(const QString jsonFileName) {
    auto value = std::make_shared<ProjectBuildConfig>();
    auto fi = QFileInfo(jsonFileName);
    value->sourceDir = fi.absolutePath();

    auto file = QFile();
    file.setFileName(jsonFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return value;
    }
    auto json = QJsonDocument::fromJson(file.readAll());
    file.close();

    auto toHash = [](QJsonValueRef v) -> QHash<QString, QString> {
        QHash<QString, QString> hash;
        if (v.isObject()) {
            auto jsonObj = v.toObject();
            for (auto vv : jsonObj.keys()) {
                hash[vv] = jsonObj[vv].toString();
            }
        }
        return hash;
    };
    auto parseExecutables = [&toHash](QJsonValue v) -> QList<ExecutableInfo> {
        QList<ExecutableInfo> info;
        if (v.isArray()) {
            for (auto vv : v.toArray()) {
                ExecutableInfo execInfo;
                execInfo.name = vv.toObject()["name"].toString();
                execInfo.executables = toHash(vv.toObject()["executables"]);
                execInfo.runDirectory = vv.toObject()["directory"].toString();
                info.push_back(execInfo);
            };
        }
        return info;
    };
    auto parseTasksInfo = [](QJsonValue v) -> QList<TaskInfo> {
        QList<TaskInfo> info;
        if (v.isArray()) {
            for (auto vv : v.toArray()) {
                TaskInfo taskInfo;
                taskInfo.name = vv.toObject()["name"].toString();
                taskInfo.command = vv.toObject()["command"].toString();
                taskInfo.runDirectory = vv.toObject()["directory"].toString();
                info.push_back(taskInfo);
            };
        }
        return info;
    };

    if (!json.isNull()) {
        value->buildDir = json["build_directory"].toString();
        value->executables = parseExecutables(json["executables"]);
        value->tasksInfo = parseTasksInfo(json["tasks"]);
    }
    return value;
}

void ProjectBuildModel::addConfig(std::shared_ptr<ProjectBuildConfig> config) {
    beginResetModel();
    this->configs.push_back(config);
    endResetModel();
}

void ProjectBuildModel::removeConfig(size_t index) {
    beginResetModel();
    this->configs.erase(this->configs.begin() + index);
    endResetModel();
}

std::shared_ptr<ProjectBuildConfig> ProjectBuildModel::getConfig(size_t index) const {
    return this->configs.at(index);
}

std::shared_ptr<ProjectBuildConfig> ProjectBuildModel::findConfig(const QString dir) {
    for (auto v : configs) {
        if (v->sourceDir == dir) {
            return v;
        }
    }
    return {};
}

int ProjectBuildModel::rowCount(const QModelIndex &) const { return configs.size(); }

QVariant ProjectBuildModel::data(const QModelIndex &index, int role) const {
    auto config = configs[index.row()];
    switch (role) {
    case Qt::DisplayRole:
        return config->sourceDir;
    case Qt::StatusTipRole:
        return config->buildDir;
    default:
        break;
    }
    return {};
}
