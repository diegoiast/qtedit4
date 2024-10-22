#include <QClipboard>
#include <QDesktopServices>
#include <QDockWidget>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>

#include <CommandPaletteWidget/CommandPalette>

#include "GenericItems.h"
#include "ProjectBuildConfig.h"
#include "ProjectManagerPlg.h"
#include "ProjectSearch.h"
#include "kitdefinitionmodel.h"
#include "kitdetector.h"
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

static auto expand(const QString &input, const QHash<QString, QString> &hashTable) -> QString {
    static auto regex = QRegularExpression(R"(\$\{([a-zA-Z0-9_]+)\})");
    auto output = input;
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

static auto regenerateKits(const std::filesystem::path &directoryPath) -> void {
    KitDetector::deleteOldKitFiles(directoryPath);

    auto tools = KitDetector::findCompilerTools(KitDetector::platformUnix);
    auto compilersFound = KitDetector::findCompilers(KitDetector::platformUnix);
    auto qtVersionsFound = std::vector<KitDetector::ExtraPath>();
    KitDetector::findQtVersions(KitDetector::platformUnix, qtVersionsFound, compilersFound);
    KitDetector::generateKitFiles(directoryPath, tools, compilersFound, qtVersionsFound,
                                  KitDetector::platformUnix);
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

    QStringList getAllOpenDirs() const;
};

void ProjectBuildModel::addConfig(std::shared_ptr<ProjectBuildConfig> config) {
    int row = configs.size();
    beginInsertRows(QModelIndex(), row, row);
    configs.push_back(config);
    endInsertRows();
}

void ProjectBuildModel::removeConfig(size_t index) {
    if (index >= configs.size()) {
        return;
    }
    beginRemoveRows(QModelIndex(), index, index);
    configs.erase(configs.begin() + index);
    endRemoveRows();
}

std::shared_ptr<ProjectBuildConfig> ProjectBuildModel::getConfig(size_t index) const {
    return this->configs.at(index);
}

std::shared_ptr<ProjectBuildConfig> ProjectBuildModel::findConfigDir(const QString dir) {
    for (const auto &v : configs) {
        if (v->sourceDir == dir) {
            return v;
        }
    }
    return {};
}

std::shared_ptr<ProjectBuildConfig> ProjectBuildModel::findConfigFile(const QString fileName) {
    for (const auto &v : configs) {
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
        return QDir::toNativeSeparators(config->sourceDir);
    case Qt::StatusTipRole:
        return QDir::toNativeSeparators(config->buildDir);
    default:
        break;
    }
    return {};
}

QStringList ProjectBuildModel::getAllOpenDirs() const {
    auto dirs = QStringList();
    for (const auto &v : configs) {
        dirs.append(v->sourceDir);
    }
    return dirs;
}

ProjectManagerPlugin::ProjectManagerPlugin() {
    name = tr("Project manager");
    author = tr("Diego Iastrubni <diegoiast@gmail.com>");
    iVersion = 0;
    sVersion = "0.0.1";
    autoEnabled = true;
    alwaysEnabled = true;

    directoryModel = nullptr;

    config.pluginName = "Project manager";
    config.description = "Add support for building using CMake/Cargo/Go";
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setDisplayName(tr("Extra paths"))
                                     .setDescription(tr("Add new paths for compilers/tools"))
                                     .setKey(Config::ExtraPathKey)
                                     .setType(qmdiConfigItem::StringList)
                                     .setDefaultValue(QStringList())
                                     .build());

    // project GUI
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setDisplayName(tr("Open directories"))
                                     .setKey(Config::OpenDirsKey)
                                     .setType(qmdiConfigItem::StringList)
                                     .setDefaultValue(QStringList())
                                     .setUserEditable(false)
                                     .build());
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setKey(Config::FilterShowKey)
                                     .setType(qmdiConfigItem::String)
                                     .setDefaultValue("")
                                     .setUserEditable(false)
                                     .build());
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setKey(Config::FilterOutKey)
                                     .setType(qmdiConfigItem::String)
                                     .setDefaultValue("")
                                     .setUserEditable(false)
                                     .build());

    // search GUI
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setKey(Config::SearchPatternKey)
                                     .setType(qmdiConfigItem::String)
                                     .setDefaultValue("")
                                     .setUserEditable(false)
                                     .build());
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setKey(Config::SearchIncludeKey)
                                     .setType(qmdiConfigItem::String)
                                     .setDefaultValue("")
                                     .setUserEditable(false)
                                     .build());
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setKey(Config::SearchExcludeKey)
                                     .setType(qmdiConfigItem::String)
                                     .setDefaultValue("")
                                     .setUserEditable(false)
                                     .build());
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
            &ProjectManagerPlugin::runButton_clicked);
    connect(gui->taskButton, &QAbstractButton::clicked, this,
            &ProjectManagerPlugin::runTask_clicked);
    connect(gui->cleanButton, &QAbstractButton::clicked, this,
            &ProjectManagerPlugin::clearProject_clicked);
    connect(gui->addDirectory, &QAbstractButton::clicked, this,
            &ProjectManagerPlugin::addProject_clicked);
    connect(gui->removeDirectory, &QAbstractButton::clicked, this,
            &ProjectManagerPlugin::removeProject_clicked);
    connect(gui->filesView, &QAbstractItemView::clicked, this,
            &ProjectManagerPlugin::onItemClicked);

    connect(gui->projectComboBox, &QComboBox::currentIndexChanged, this,
            &ProjectManagerPlugin::newProjectSelected);
    manager->createNewPanel(Panels::West, tr("Project"), w);

    auto *w2 = new QWidget;
    outputPanel = new Ui::BuildRunOutput;
    outputPanel->setupUi(w2);
    manager->createNewPanel(Panels::South, tr("Output"), w2);

    connect(outputPanel->clearOutput, &QAbstractButton::clicked, this,
            [this]() { this->outputPanel->commandOuput->clear(); });
    connect(outputPanel->copyOutput, &QAbstractButton::clicked, this, [this]() {
        auto text = this->outputPanel->commandOuput->document()->toPlainText();
        auto clipboard = QGuiApplication::clipboard();
        clipboard->setText(text);
    });
    connect(outputPanel->playButton, &QAbstractButton::clicked, this,
            [this]() { this->gui->runButton->animateClick(); });
    connect(outputPanel->buildButton, &QAbstractButton::clicked, this,
            [this]() { this->gui->taskButton->animateClick(); });
    connect(outputPanel->cancelButton, &QAbstractButton::clicked, this, [this]() {
        if (this->runProcess.processId() != 0) {
            this->runProcess.kill();
        }
    });
    connect(&runProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        auto output = this->runProcess.readAllStandardOutput();
        auto cursor = this->outputPanel->commandOuput->textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.insertText(output);
        this->outputPanel->commandOuput->setTextCursor(cursor);
    });
    connect(&runProcess, &QProcess::readyReadStandardError, this, [this]() {
        auto output = this->runProcess.readAllStandardError();
        auto cursor = this->outputPanel->commandOuput->textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.insertText(output);
        this->outputPanel->commandOuput->setTextCursor(cursor);
    });
    connect(&runProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
                auto output = QString("[code=%1, status=%2]").arg(exitCode).arg(str(exitStatus));
                this->outputPanel->commandOuput->appendPlainText(output);
            });
    connect(&runProcess, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        auto output = QString("[error: code=%1]").arg((int)error);
        this->outputPanel->commandOuput->appendPlainText(output);
        qWarning() << "Process error occurred:" << error;
        qWarning() << "Error string:" << runProcess.errorString();
    });
    connect(&configWatcher, &QFileSystemWatcher::fileChanged, this,
            &ProjectManagerPlugin::projectFile_modified);
    connect(gui->cleanButton, &QToolButton::clicked, this,
            [this]() { this->outputPanel->commandOuput->clear(); });
    directoryModel = new DirectoryModel(this);
    filesFilterModel = new FilterOutProxyModel(this);
    filesFilterModel->setSourceModel(directoryModel);
    filesFilterModel->sort(0);
    gui->filesView->setModel(filesFilterModel);
    connect(gui->filterFiles, &QLineEdit::textChanged, this, [this](const QString &newText) {
        filesFilterModel->setFilterWildcards(newText);
        auto config = this->getCurrentConfig();
        if (config) {
            config->displayFilter = newText;
        }
    });
    connect(gui->filterOutFiles, &QLineEdit::textChanged, this, [this](const QString &newText) {
        filesFilterModel->setFilterOutWildcard(newText);
        auto config = this->getCurrentConfig();
        if (config) {
            config->hideFilter = newText;
        }
    });

    auto menu = new QMenu(getManager());
    auto rescanKits = new QAction(tr("Rescan kits"), menu);
    auto recreateKits = new QAction(tr("Recreate kits"), menu);
    auto openKitsinFM = new QAction(tr("Open kits dir in file manager"), menu);
    auto editCurrentKit = new QAction(tr("Edit this kit"), menu);

    connect(rescanKits, &QAction::triggered, this, [this]() {
        auto dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        auto kits = findKitDefinitions(QDir::toNativeSeparators(dataPath).toStdString());
        kitsModel->setKitDefinitions(kits);
    });
    connect(recreateKits, &QAction::triggered, this, [rescanKits]() {
        auto dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        {
            auto d = QDir(dataPath);
            if (!d.exists()) {
                if (!d.mkpath(dataPath)) {
                    qDebug() << "Failed to create directory:" << dataPath;
                    return;
                }
            }
        }
        regenerateKits(std::filesystem::path(dataPath.toStdString()));
        rescanKits->trigger();
    });
    connect(openKitsinFM, &QAction::triggered, this, []() {
        auto dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        auto url = QUrl::fromLocalFile(dataPath);
        QDesktopServices::openUrl(url);
    });
    connect(editCurrentKit, &QAction::triggered, this, [this]() {
        auto kit = this->getCurrentKit();
        if (!kit) {
            return;
        }
        auto path = QString::fromStdString(kit->filePath);
        getManager()->openFile(path);
    });

    menu->addAction(rescanKits);
    menu->addAction(recreateKits);
    menu->addAction(openKitsinFM);
    menu->addAction(editCurrentKit);

    gui->toolkitToolButton->setMenu(menu);
    gui->toolkitToolButton->setPopupMode(QToolButton::InstantPopup);
    connect(gui->editBuildConfig, &QAbstractButton::clicked, this, [this]() {
        auto buildConfig = this->getCurrentConfig();
        if (buildConfig->fileName.isEmpty()) {
            qDebug("Should save this");
            auto path = buildConfig->sourceDir + "/" + "qtedit4.json";
            buildConfig->saveToFile(path);
            configWatcher.addPath(buildConfig->fileName);
        }
        getManager()->openFile(buildConfig->fileName);
    });

    searchPanelUI = new ProjectSearch(manager, directoryModel);
    auto seachID = manager->createNewPanel(Panels::West, tr("Search"), searchPanelUI);
    auto projectSearch = new QAction(tr("Search in project"));
    projectSearch->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_F));
    connect(projectSearch, &QAction::triggered, this, [seachID, manager, this]() {
        manager->showPanel(Panels::West, seachID);
        searchPanelUI->setFocusOnSearch();
    });
    manager->addAction(projectSearch);
    kitsModel = new KitDefinitionModel();
    gui->kitComboBox->setModel(kitsModel);

    auto dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto kits = findKitDefinitions(QDir::toNativeSeparators(dataPath).toStdString());
    kitsModel->setKitDefinitions(kits);

    projectModel = new ProjectBuildModel();
    gui->projectComboBox->setModel(projectModel);

    runAction = new QAction(QIcon::fromTheme("document-save"), tr("&Run"), this);
    buildAction = new QAction(QIcon::fromTheme("document-save-as"), tr("&Run task"), this);
    clearAction = new QAction(QIcon::fromTheme("edit-clear"), tr("&Delete build directory"), this);

    runAction->setEnabled(false);
    buildAction->setEnabled(false);
    clearAction->setEnabled(false);

    connect(runAction, &QAction::triggered, this, &ProjectManagerPlugin::runButton_clicked);
    connect(buildAction, &QAction::triggered, this, &ProjectManagerPlugin::runTask_clicked);
    connect(clearAction, &QAction::triggered, this, &ProjectManagerPlugin::clearProject_clicked);

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
    connect(addNewProjectIcon, &QAction::triggered, this,
            [this]() { this->gui->addDirectory->animateClick(); });
    connect(removeProjectIcon, &QAction::triggered, this,
            [this]() { this->gui->removeDirectory->animateClick(); });

    this->menus[tr("&Project")]->addSeparator();
    this->menus[tr("&Project")]->addAction(addNewProjectIcon);
    this->menus[tr("&Project")]->addAction(removeProjectIcon);

    auto quickOpen = new QAction(tr("Quick open file..."), this);
    quickOpen->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_P));
    this->menus[tr("&Project")]->addAction(quickOpen);

    commandPalette = new CommandPalette(manager);
    connect(quickOpen, &QAction::triggered, this, [this]() {
        if (commandPalette->isVisible()) {
            commandPalette->hide();
        } else {
            commandPalette->setDataModel(filesFilterModel);
            commandPalette->clearText();
            commandPalette->show();
        }
    });

    connect(commandPalette, &CommandPalette::didChooseItem, this,
            [this](const QModelIndex index, const QAbstractItemModel *model) {
                auto fname = model->data(index, Qt::UserRole).toString();
                this->getManager()->openFile(fname);
            });
}

void ProjectManagerPlugin::on_client_unmerged(qmdiHost *host) { Q_UNUSED(host); }

void ProjectManagerPlugin::loadConfig(QSettings &settings) {
    IPlugin::loadConfig(settings);

    searchPanelUI->setSearchPattern(getConfig().getSearchPattern());
    searchPanelUI->setSearchInclude(getConfig().getSearchInclude());
    searchPanelUI->setSearchExclude(getConfig().getSearchExclude());
    auto dirsToLoad = getConfig().getOpenDirs();
    QTimer::singleShot(0, [this, dirsToLoad]() {
        for (auto &dirName : dirsToLoad) {
            auto config = projectModel->findConfigDir(dirName);
            if (config) {
                continue;
            }
            config = ProjectBuildConfig::buildFromDirectory(dirName);
            projectModel->addConfig(config);
            if (!config->fileName.isEmpty()) {
                qDebug("loadConfig() - adding %s to the watch dir",
                       config->fileName.toStdString().c_str());
                configWatcher.addPath(config->fileName);
            }
        }
    });
}

void ProjectManagerPlugin::saveConfig(QSettings &settings) {
    getConfig().setOpenDirs(projectModel->getAllOpenDirs());
    getConfig().setSearchPattern(searchPanelUI->getSearchPattern());
    getConfig().setSearchInclude(searchPanelUI->getSearchInclude());
    getConfig().setSearchExclude(searchPanelUI->getSearchExclude());
    IPlugin::saveConfig(settings);
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

const KitDefinition *ProjectManagerPlugin::getCurrentKit() const {
    auto currentIndex = gui->kitComboBox->currentIndex();
    if (currentIndex < 0) {
        return nullptr;
    }
    return &kitsModel->getKit(currentIndex);
}

void ProjectManagerPlugin::onItemClicked(const QModelIndex &index) {
    auto i = filesFilterModel->mapToSource(index);
    auto s = directoryModel->getItem(i.row());
    getManager()->openFile(s);
}

void ProjectManagerPlugin::addProject_clicked() {
    QString dirName = QFileDialog::getExistingDirectory(gui->filesView, tr("Add directory"));
    if (dirName.isEmpty()) {
        return;
    }
    auto buildConfig = projectModel->findConfigDir(dirName);
    if (buildConfig) {
        return;
    }
    buildConfig = ProjectBuildConfig::buildFromDirectory(dirName);
    if (!buildConfig->fileName.isEmpty()) {
        // Auto generated config files have no filename
        qDebug("on_addProject_clicked() : adding %s to the watch dir",
               buildConfig->fileName.toStdString().c_str());
        configWatcher.addPath(buildConfig->fileName);
    }
    projectModel->addConfig(buildConfig);
    directoryModel->addDirectory(dirName);
    getManager()->saveSettings();
}

void ProjectManagerPlugin::removeProject_clicked() {
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

void ProjectManagerPlugin::newProjectSelected(int index) {
    // TODO - on startu this is called 2 times. I am unsure why yet.
    //        so this works around it. Its not the best solution.
    static auto lastProjectSelected = std::shared_ptr<ProjectBuildConfig>();
    auto buildConfig = std::shared_ptr<ProjectBuildConfig>();
    if (index >= 0) {
        buildConfig = projectModel->getConfig(index);
    }

    if (lastProjectSelected == buildConfig) {
        return;
    }

    lastProjectSelected = buildConfig;
    this->directoryModel->removeAllDirs();
    if (!buildConfig) {
        this->gui->filterFiles->clear();
        this->gui->filterFiles->setEnabled(false);
        this->gui->filterOutFiles->clear();
        this->gui->filterOutFiles->setEnabled(false);
    } else {
        auto hash = getConfigHash();
        auto s1 = expand(buildConfig->displayFilter, hash);
        auto s2 = expand(buildConfig->hideFilter, hash);
        this->gui->filterFiles->setEnabled(true);
        this->gui->filterFiles->setText(s1);
        this->gui->filterOutFiles->setEnabled(true);
        this->gui->filterOutFiles->setText(s2);
        this->directoryModel->addDirectory(buildConfig->sourceDir);
    }

    updateTasksUI(buildConfig);
    updateExecutablesUI(buildConfig);
}

void ProjectManagerPlugin::do_runExecutable(const ExecutableInfo *info) {
    if (runProcess.processId() != 0) {
        runProcess.kill();
        return;
    }

    auto hash = getConfigHash();
    auto project = getCurrentConfig();
    auto executablePath = QDir::toNativeSeparators(findExecForPlatform(info->executables));
    auto currentTask = expand(executablePath, hash);
    auto workingDirectory = info->runDirectory;
    if (workingDirectory.isEmpty()) {
        workingDirectory = project->buildDir;
    }
    workingDirectory = expand(workingDirectory, hash);
    outputPanel->commandOuput->clear();
    outputPanel->commandOuput->appendPlainText("cd " + QDir::toNativeSeparators(workingDirectory));
    outputPanel->commandOuput->appendPlainText(currentTask + "\n");

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

    auto kit = getCurrentKit();
    auto project = getCurrentConfig();
    auto hash = getConfigHash();
    auto currentTask = expand(task->command, hash);
    auto workingDirectory = expand(task->runDirectory, hash);

    if (workingDirectory.isEmpty()) {
        workingDirectory = project->buildDir;
    }

    outputPanel->commandOuput->clear();
    outputPanel->commandOuput->appendPlainText("cd " + workingDirectory);
    if (!kit) {
        auto command = QStringList();
        auto interpreter = QString();
#if defined(__linux__)
        interpreter = "/bin/sh";
        command << "-c" << currentTask;
#elif defined(_WIN32)
        interpreter = qgetenv("COMSPEC");
        command << "/k" << currentTask;
#else
        interpreter = "???";
#endif
        auto env = QProcessEnvironment::systemEnvironment();
        outputPanel->commandOuput->appendPlainText(interpreter + " " + command.join(" "));

        runProcess.setWorkingDirectory(workingDirectory);
        runProcess.setProgram(interpreter);
        runProcess.setArguments(command);
        runProcess.setProcessEnvironment(env);
    } else {
        auto env = QProcessEnvironment::systemEnvironment();
        env.insert("run_directory", expand(QDir::toNativeSeparators(workingDirectory), hash));
        env.insert("build_directory", expand(QDir::toNativeSeparators(project->buildDir), hash));
        env.insert("source_directory", QDir::toNativeSeparators(project->sourceDir));
        env.insert("task", currentTask);
        runProcess.setProcessEnvironment(env);
        runProcess.setProgram(QString::fromStdString(kit->filePath));
    }

    runProcess.start();
    if (!runProcess.waitForStarted()) {
        if (kit) {
            auto msg = QString("Failed to run kit %1, with task=%2")
                           .arg(QString::fromStdString(kit->filePath), currentTask);
            this->outputPanel->commandOuput->appendPlainText(msg);
        } else {
            this->outputPanel->commandOuput->appendPlainText("Process failed to start " +
                                                             currentTask);
        }
        qWarning() << "Process failed to start";
    }
}

void ProjectManagerPlugin::runButton_clicked() {
    assert(this->selectedTarget);
    do_runExecutable(this->selectedTarget);
}

void ProjectManagerPlugin::runTask_clicked() {
    assert(this->selectedTask);
    do_runTask(this->selectedTask);
}

void ProjectManagerPlugin::clearProject_clicked() {
    auto project = getCurrentConfig();
    if (project == nullptr || project->buildDir.isEmpty()) {
        return;
    }

    auto hash = getConfigHash();
    auto projectBuildDir = expand(project->buildDir, hash);
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(tr("This will delete <b>%1</b> (%2). Do you want to proceed?")
                       .arg(project->buildDir, projectBuildDir));
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

void ProjectManagerPlugin::projectFile_modified(const QString &path) {
    auto onDiskConfig = ProjectBuildConfig::buildFromFile(path);
    auto inMemoryConfig = projectModel->findConfigFile(path);
    if (*onDiskConfig == *inMemoryConfig) {
        qDebug("Config file modified, content similar ignoring - %s", path.toStdString().data());
        return;
    }
    *inMemoryConfig = *onDiskConfig;
    newProjectSelected(gui->projectComboBox->currentIndex());

    // TODO  - new file created is not working yet.
    qDebug("Config file modified - %s", path.toStdString().data());
}

auto ProjectManagerPlugin::updateTasksUI(std::shared_ptr<ProjectBuildConfig> buildConfig) -> void {
    if (!buildConfig || buildConfig->tasksInfo.size() == 0) {
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
        if (!buildConfig->activeTaskName.isEmpty()) {
            taskIndex = buildConfig->findIndexOfTask(buildConfig->activeTaskName);
            if (taskIndex < 0) {
                taskIndex = 0;
            }
        }
        auto taskName = buildConfig->tasksInfo[taskIndex].name;
        auto taskCommand = buildConfig->tasksInfo[taskIndex].command;

        this->gui->taskButton->setEnabled(true);
        this->gui->taskButton->setText(taskName);
        this->gui->taskButton->setToolTip(taskCommand);
        this->selectedTask = &buildConfig->tasksInfo[taskIndex];

        this->buildAction->setEnabled(true);
        this->buildAction->setText(QString(tr("Action: %1")).arg(taskName));
        this->buildAction->setToolTip(taskCommand);

        this->clearAction->setEnabled(true);
        this->availableTasksMenu->hide();
        this->availableTasksMenu->clear();
        this->mdiServer->mdiHost->updateGUI();
        if (buildConfig->tasksInfo.size() == 1) {
            gui->taskButton->setMenu(nullptr);
        } else {
            auto menu = new QMenu(gui->runButton);
            QList<QAction *> actions;
            for (const auto &taskInfo : std::as_const(buildConfig->tasksInfo)) {
                auto action = new QAction(taskInfo.name, this);
                menu->addAction(action);
                actions.append(action);

                action = new QAction(taskInfo.name, this);
                connect(action, &QAction::triggered, this,
                        [this, taskInfo]() { this->do_runTask(&taskInfo); });

                this->availableTasksMenu->addAction(action);
            }
            gui->taskButton->setMenu(menu);
            connect(menu, &QMenu::triggered, this, [this, actions, buildConfig](QAction *action) {
                auto index = actions.indexOf(action);
                auto taskName = buildConfig->tasksInfo[index].name;
                auto taskCommand = buildConfig->tasksInfo[index].command;

                buildConfig->activeTaskName = taskName;
                this->gui->taskButton->setEnabled(true);
                this->gui->taskButton->setText(taskName);
                this->gui->taskButton->setToolTip(taskCommand);
                this->selectedTask = &buildConfig->tasksInfo[index];
                buildConfig->activeTaskName = this->selectedTask->name;

                this->buildAction->setEnabled(true);
                this->buildAction->setText(QString(tr("Action: %1")).arg(taskName));
                this->buildAction->setToolTip(taskCommand);
            });
        }
    }
}

auto ProjectManagerPlugin::updateExecutablesUI(std::shared_ptr<ProjectBuildConfig> buildConfig)
    -> void {
    if (!buildConfig || buildConfig->executables.size() == 0) {
        this->selectedTarget = nullptr;
        this->gui->runButton->setText("...");
        this->gui->runButton->setToolTip("...");
        this->gui->runButton->setEnabled(false);

        this->runAction->setEnabled(false);
        this->clearAction->setEnabled(false);
        this->availableExecutablesMenu->clear();
    } else {
        auto executableIndex = 0;
        if (!buildConfig->activeExecutableName.isEmpty()) {
            executableIndex = buildConfig->findIndexOfExecutable(buildConfig->activeExecutableName);
            if (executableIndex < 0) {
                executableIndex = 0;
            }
        }

        auto executableName = buildConfig->executables[executableIndex].name;
        auto executablePath =
            findExecForPlatform(buildConfig->executables[executableIndex].executables);

        this->gui->runButton->setEnabled(true);
        this->gui->runButton->setText(executableName);
        this->gui->runButton->setToolTip(QDir::toNativeSeparators(executablePath));
        this->selectedTarget = &buildConfig->executables[executableIndex];

        this->runAction->setEnabled(true);
        this->runAction->setText(
            QString(tr("Run: %1")).arg(QDir::toNativeSeparators(executableName)));
        this->runAction->setToolTip(QDir::toNativeSeparators(executablePath));

        this->availableExecutablesMenu->hide();
        this->availableExecutablesMenu->clear();
        this->mdiServer->mdiHost->updateGUI();
        if (buildConfig->executables.size() == 1) {
            this->gui->runButton->setMenu(nullptr);
        } else {
            auto menu = new QMenu(gui->runButton);
            QList<QAction *> actions;
            for (const auto &target : std::as_const(buildConfig->executables)) {
                QAction *action = new QAction(target.name, this);
                menu->addAction(action);
                actions.append(action);

                action = new QAction(target.name, this);
                connect(action, &QAction::triggered, this,
                        [this, target]() { this->do_runExecutable(&target); });
                this->availableExecutablesMenu->addAction(action);
            }
            this->mdiServer->mdiHost->updateGUI();
            this->gui->runButton->setMenu(menu);
            connect(menu, &QMenu::triggered, this, [this, actions, buildConfig](QAction *action) {
                auto index = actions.indexOf(action);
                auto executableName = buildConfig->executables[index].name;
                auto executablePath =
                    findExecForPlatform(buildConfig->executables[index].executables);
                this->gui->runButton->setEnabled(true);
                this->gui->runButton->setText(executableName);
                this->gui->runButton->setToolTip(executablePath);
                this->selectedTarget = &buildConfig->executables[index];
                buildConfig->activeExecutableName = this->selectedTarget->name;

                this->runAction->setEnabled(true);
                this->runAction->setText(QString(tr("Run: %1")).arg(executableName));
                this->runAction->setToolTip(executablePath);
            });
        }
    }
}
