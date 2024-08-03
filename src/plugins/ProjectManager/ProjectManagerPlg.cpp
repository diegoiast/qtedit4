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
#include <QStandardPaths>

#include "GenericItems.h"
#include "ProjectBuildConfig.h"
#include "ProjectManagerPlg.h"
#include "ProjectSearch.h"
#include "kitdefinitionmodel.h"
#include "pluginmanager.h"
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

#if defined(_WIN32)
constexpr auto EnvPathSeparator = ";";
#else
constexpr auto EnvPathSeparator = ":";
#endif

static auto findExecForPlatform(QHash<QString, QString> files) -> QString {
#if defined(__linux__)
    return files["linux"];
#elif defined(_WIN32)
    return files["windows"];
#else
    qDebug("Warning - unsupported platform, cannot find executable");
    return {};
#endif
}

static auto getConfigForPlatform(ProjectBuildConfig *project) -> PlatformConfig {
#if defined(__linux__)
    return project->platformConfig["linux"];
#elif defined(_WIN32)
    return project->platformConfig["windows"];
#else
    qDebug("Warning - unsupported platform, cannot find config");
    return {};
#endif
}

static auto expand(const QString &input, const QHash<QString, QString> &hashTable) -> QString {
    auto output = input;
    auto regex = QRegularExpression(R"(\$\{([a-zA-Z0-9_]+)\})");
    auto depth = 0;
    auto maxDepth = 10;

    while (depth < maxDepth) {
        auto it = regex.globalMatch(output);
        if (!it.hasNext()) {
            break;
        }
        while (it.hasNext()) {
            auto match = it.next();
            auto key = match.captured(1);
            auto replacement = hashTable.value(key, "");
            output.replace(match.captured(0), replacement);
        }
        depth++;
    }
    return output;
}

// Internal class
class ProjectBuildModel : public QAbstractListModel {
    std::vector<std::shared_ptr<ProjectBuildConfig>> configs;

  public:
    void addConfig(std::shared_ptr<ProjectBuildConfig> config);
    void removeConfig(size_t index);
    std::shared_ptr<ProjectBuildConfig> getConfig(size_t index) const;
    std::shared_ptr<ProjectBuildConfig> findConfigDir(const QString dir);
    std::shared_ptr<ProjectBuildConfig> findConfigFile(const QString fileName);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
};

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

std::shared_ptr<ProjectBuildConfig> ProjectBuildModel::findConfigDir(const QString dir) {
    for (auto v : configs) {
        if (v->sourceDir == dir) {
            return v;
        }
    }
    return {};
}

std::shared_ptr<ProjectBuildConfig> ProjectBuildModel::findConfigFile(const QString fileName) {
    for (auto v : configs) {
        if (v->fileName == fileName) {
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

ProjectManagerPlugin::ProjectManagerPlugin() {
    name = tr("Project manager");
    author = tr("Diego Iastrubni <diegoiast@gmail.com>");
    iVersion = 0;
    sVersion = "0.0.1";
    autoEnabled = true;
    alwaysEnabled = true;

    directoryModel = nullptr;
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
    gui->filterFiles->setPlaceholderText(tr("files to show"));
    gui->filterOutFiles->setClearButtonEnabled(true);
    gui->filterOutFiles->setPlaceholderText(tr("files to hide"));

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

    connect(gui->projectComboBox, &QComboBox::currentIndexChanged, this,
            &ProjectManagerPlugin::on_newProjectSelected);
    manager->createNewPanel(Panels::West, tr("Project"), w);

    auto *w2 = new QWidget;
    outputPanel = new Ui::BuildRunOutput;
    outputPanel->setupUi(w2);
    manager->createNewPanel(Panels::South, tr("Output"), w2);

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
    connect(&configWatcher, &QFileSystemWatcher::fileChanged, this,
            &ProjectManagerPlugin::on_projectFile_modified);

    connect(gui->cleanButton, &QToolButton::clicked,
            [this]() { this->outputPanel->commandOuput->clear(); });
    directoryModel = new DirectoryModel(this);
    filesFilterModel = new FilterOutProxyModel(this);
    filesFilterModel->setSourceModel(directoryModel);
    filesFilterModel->sort(0);
    gui->filesView->setModel(filesFilterModel);
    connect(gui->filterFiles, &QLineEdit::textChanged, [this](const QString &newText) {
        filesFilterModel->setFilterWildcards(newText);
        auto config = this->getCurrentConfig();
        if (config) {
            config->displayFilter = newText;
        }
    });
    connect(gui->filterOutFiles, &QLineEdit::textChanged, [this](const QString &newText) {
        filesFilterModel->setFilterOutWildcard(newText);
        auto config = this->getCurrentConfig();
        if (config) {
            config->hideFilter = newText;
        }
    });

    auto *searchPanelUI = new ProjectSearch(manager, directoryModel);
    auto seachID = manager->createNewPanel(Panels::West, tr("Search"), searchPanelUI);

    auto projectSearch = new QAction(tr("Search in project"));
    projectSearch->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_F));
    connect(projectSearch, &QAction::triggered, [seachID, manager, searchPanelUI]() {
        manager->showPanel(Panels::West, seachID);
        searchPanelUI->setFocusOnSearch();
    });
    manager->addAction(projectSearch);

    kitsModel = new KitDefinitionModel();
    gui->kitComboBox->setModel(kitsModel);

    auto dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto kits = findKitDefinitions(dataPath.toStdString());
    kitsModel->setKitDefinitions(kits);

    projectModel = new ProjectBuildModel();
    gui->projectComboBox->setModel(projectModel);

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

    settings.beginGroup("Loaded");
    foreach (auto s, settings.childKeys()) {
        if (!s.startsWith("dir")) {
            continue;
        }
        auto dirName = settings.value(s).toString();
        auto config = projectModel->findConfigDir(dirName);
        if (!config) {
            config = ProjectBuildConfig::buildFromDirectory(dirName);
            projectModel->addConfig(config);
            qDebug("adding %s to the watch dir", config->fileName.toStdString().c_str());
            configWatcher.addPath(config->fileName);
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

std::shared_ptr<ProjectBuildConfig> ProjectManagerPlugin::getCurrentConfig() const {
    auto currentIndex = gui->projectComboBox->currentIndex();
    if (currentIndex < 0) {
        return {};
    }
    return projectModel->getConfig(currentIndex);
}

const QHash<QString, QString> ProjectManagerPlugin::getConfigHash() const {
    auto hash = QHash<QString, QString>();
    auto project = getCurrentConfig();
    if (project) {
        hash["source_directory"] = QDir::toNativeSeparators(project->sourceDir);
        hash["build_directory"] = QDir::toNativeSeparators(project->buildDir);
    }
    return hash;
}

void ProjectManagerPlugin::onItemClicked(const QModelIndex &index) {
    auto i = filesFilterModel->mapToSource(index);
    auto s = directoryModel->getItem(i.row());
    getManager()->openFile(s);
}

void ProjectManagerPlugin::on_addProject_clicked() {
    QString dirName = QFileDialog::getExistingDirectory(gui->filesView, tr("Add directory"));
    if (dirName.isEmpty()) {
        return;
    }
    auto config = projectModel->findConfigDir(dirName);
    if (config) {
        return;
    }
    config = ProjectBuildConfig::buildFromDirectory(dirName);
    qDebug("adding %s to the watch dir", config->fileName.toStdString().c_str());

    configWatcher.addPath(config->fileName);
    projectModel->addConfig(config);
    directoryModel->addDirectory(dirName);
    getManager()->saveSettings();
}

void ProjectManagerPlugin::on_removeProject_clicked() {
    auto index = gui->projectComboBox->currentIndex();
    if (index < 0) {
        return;
    }
    auto path = projectModel->getConfig(index)->fileName;
    projectModel->removeConfig(index);
    qDebug("remove %s to the watch dir", path.toStdString().c_str());

    configWatcher.removePath(path);
    getManager()->saveSettings();
}

void ProjectManagerPlugin::on_newProjectSelected(int index) {
    std::shared_ptr<ProjectBuildConfig> config = {};
    if (index >= 0) {
        config = projectModel->getConfig(index);
    }

    this->directoryModel->removeAllDirs();
    if (!config) {
        this->gui->filterFiles->clear();
        this->gui->filterFiles->setEnabled(false);
        this->gui->filterOutFiles->clear();
        this->gui->filterOutFiles->setEnabled(false);
    } else {
        auto hash = getConfigHash();
        auto s1 = expand(config->displayFilter, hash);
        auto s2 = expand(config->hideFilter, hash);
        this->gui->filterFiles->setEnabled(true);
        this->gui->filterFiles->setText(s1);
        this->gui->filterOutFiles->setEnabled(true);
        this->gui->filterOutFiles->setText(s2);
        this->directoryModel->addDirectory(config->sourceDir);
    }

    updateTasksUI(config);
    updateExecutablesUI(config);
}

void ProjectManagerPlugin::do_runExecutable(const ExecutableInfo *info) {
    if (runProcess.processId() != 0) {
        runProcess.kill();
        return;
    }

    auto hash = getConfigHash();
    auto project = getCurrentConfig();
    auto executablePath = findExecForPlatform(info->executables);
    auto currentTask = expand(executablePath, hash);
    auto workingDirectory = expand(info->runDirectory, hash);
    if (workingDirectory.isEmpty()) {
        workingDirectory = project->buildDir;
    }
    outputPanel->commandOuput->clear();
    outputPanel->commandOuput->appendPlainText("cd " + QDir::toNativeSeparators(workingDirectory));
    outputPanel->commandOuput->appendPlainText(currentTask);

    auto command = QStringList();
    auto interpreter = QString();
#if defined(__linux__)
    interpreter = "/bin/sh";
    command.append("-c");
    command.append(currentTask);
#elif defined(_WIN32)
    interpreter = qgetenv("COMSPEC");
    command << "/k" << currentTask;
#else
    interpreter = "???";
#endif
    runProcess.start(interpreter, command);
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
    auto config = getConfigForPlatform(project.get());
    auto hash = getConfigHash();
    auto currentTask = expand(task->command, hash);
    auto workingDirectory = expand(task->runDirectory, hash);

    if (workingDirectory.isEmpty()) {
        workingDirectory = project->buildDir;
    }

    auto command = QStringList();
    auto interpreter = QString();
#if defined(__linux__)
    interpreter = "/bin/sh";
    command.append("-c");
    command.append(currentTask);
#elif defined(_WIN32)
    interpreter = qgetenv("COMSPEC");
    command << "/k" << currentTask;
#else
    interpreter = "???";
#endif
    auto env = QProcessEnvironment::systemEnvironment();
    QString currentPath = env.value("PATH");
    QString newPath =
        config.pathPrepend + EnvPathSeparator + currentPath + EnvPathSeparator + config.pathPrepend;
    env.insert("PATH", newPath);

    outputPanel->commandOuput->clear();
    outputPanel->commandOuput->appendPlainText(QString("PATH=%1").arg(newPath));
    outputPanel->commandOuput->appendPlainText("cd " + workingDirectory);
    outputPanel->commandOuput->appendPlainText(interpreter + " " + command.join(" "));

    runProcess.setWorkingDirectory(workingDirectory);
    runProcess.setProgram(interpreter);
    runProcess.setArguments(command);
    runProcess.setProcessEnvironment(env);
    runProcess.start();
    if (!runProcess.waitForStarted()) {
        qWarning() << "Process failed to start";
        this->outputPanel->commandOuput->appendPlainText("Process failed to start");
    }
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
    if (project == nullptr || project->buildDir.isEmpty()) {
        return;
    }

    auto hash = getConfigHash();
    auto projectBuildDir = expand(project->buildDir, hash);
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(tr("This will delete <b>%1</b> (%2). Do you want to proceed?")
                       .arg(project->buildDir)
                       .arg(projectBuildDir));
    msgBox.setWindowTitle(tr("Confirmation"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    int ret = msgBox.exec();
    switch (ret) {
    case QMessageBox::Yes: {
        // TODO - run this in a thread?
        auto outputDir = QDir(projectBuildDir);
        outputDir.removeRecursively();
        break;
    }
    case QMessageBox::No:
        break;
    default:
        break;
    }
}

void ProjectManagerPlugin::on_projectFile_modified(const QString &path) {
    auto onDiskConfig = ProjectBuildConfig::buildFromFile(path);
    auto inMemoryConfig = projectModel->findConfigFile(path);
    if (*onDiskConfig == *inMemoryConfig) {
        qDebug("Config file modified, content similar ignoring - %s", path.toStdString().data());
        return;
    }
    *inMemoryConfig = *onDiskConfig;
    emit on_newProjectSelected(gui->projectComboBox->currentIndex());

    // TODO  - new file created is not working yet.
    qDebug("Config file modified - %s", path.toStdString().data());
}

auto ProjectManagerPlugin::updateTasksUI(std::shared_ptr<ProjectBuildConfig> config) -> void {
    if (!config || config->tasksInfo.size() == 0) {
        this->selectedTask = nullptr;
        this->gui->taskButton->setText("...");
        this->gui->taskButton->setToolTip("...");
        this->gui->taskButton->setEnabled(false);
        this->availableTasksMenu->clear();

        this->buildAction->setEnabled(false);
        this->clearAction->setEnabled(false);
        this->availableTasksMenu->clear();
    } else {
        auto taskIndex = 0;
        if (!config->activeTaskName.isEmpty()) {
            taskIndex = config->findIndexOfTask(config->activeTaskName);
            if (taskIndex < 0) {
                taskIndex = 0;
            }
        }
        auto taskName = config->tasksInfo[taskIndex].name;
        auto taskCommand = config->tasksInfo[taskIndex].command;

        this->gui->taskButton->setEnabled(true);
        this->gui->taskButton->setText(taskName);
        this->gui->taskButton->setToolTip(taskCommand);
        this->selectedTask = &config->tasksInfo[taskIndex];

        this->buildAction->setEnabled(true);
        this->buildAction->setText(QString(tr("Action: %1")).arg(taskName));
        this->buildAction->setToolTip(taskCommand);

        this->clearAction->setEnabled(true);
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

                config->activeTaskName = taskName;
                this->gui->taskButton->setEnabled(true);
                this->gui->taskButton->setText(taskName);
                this->gui->taskButton->setToolTip(taskCommand);
                this->selectedTask = &config->tasksInfo[index];
                config->activeTaskName = this->selectedTask->name;

                this->buildAction->setEnabled(true);
                this->buildAction->setText(QString(tr("Action: %1")).arg(taskName));
                this->buildAction->setToolTip(taskCommand);
            });
        }
    }
}

auto ProjectManagerPlugin::updateExecutablesUI(std::shared_ptr<ProjectBuildConfig> config) -> void {
    if (!config || config->executables.size() == 0) {
        this->selectedTarget = nullptr;
        this->gui->runButton->setText("...");
        this->gui->runButton->setToolTip("...");
        this->gui->runButton->setEnabled(false);

        this->runAction->setEnabled(false);
        this->clearAction->setEnabled(false);
        this->availableExecutablesMenu->clear();
    } else {
        auto executableIndex = 0;
        if (!config->activeExecutableName.isEmpty()) {
            executableIndex = config->findIndexOfExecutable(config->activeExecutableName);
            if (executableIndex < 0) {
                executableIndex = 0;
            }
        }

        auto executableName = config->executables[executableIndex].name;
        auto executablePath = findExecForPlatform(config->executables[executableIndex].executables);

        this->gui->runButton->setEnabled(true);
        this->gui->runButton->setText(executableName);
        this->gui->runButton->setToolTip(executablePath);
        this->selectedTarget = &config->executables[executableIndex];

        this->runAction->setEnabled(true);
        this->runAction->setText(QString(tr("Run: %1")).arg(executableName));
        this->runAction->setToolTip(executablePath);

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
                config->activeExecutableName = this->selectedTarget->name;

                this->runAction->setEnabled(true);
                this->runAction->setText(QString(tr("Run: %1")).arg(executableName));
                this->runAction->setToolTip(executablePath);
            });
        }
    }
}
