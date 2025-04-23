#include <QClipboard>
#include <QDesktopServices>
#include <QDockWidget>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QScrollBar>
#include <QSettings>
#include <QSocketNotifier>
#include <QStandardPaths>
#include <QTimer>

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#endif

#if defined(WIN32)
#include <io.h>
#define read _read
#endif

#include <CommandPaletteWidget/CommandPalette>
#include <qmdihost.h>
#include <qmdiserver.h>
#include <qmditabwidget.h>

#include "AnsiToHTML.hpp"
#include "GenericItems.h"
#include "GlobalCommands.hpp"
#include "ProjectBuildConfig.h"
#include "ProjectIssuesWidget.h"
#include "ProjectManagerPlg.h"
#include "ProjectSearch.h"
#include "kitdefinitionmodel.h"
#include "kitdetector.h"
#include "pluginmanager.h"
#include "ui_BuildRunOutput.h"
#include "ui_ProjectManagerGUI.h"
#include "widgets/qmdieditor.h"

#define USE_TTY_FOR_TASKS

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

static auto getCommandInterpreter(const QString &externalCommand)
    -> std::tuple<QStringList, QString> {
    QString interpreter;
    QStringList command;

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    interpreter = "/bin/sh";
    command << "-c" << externalCommand;
#elif defined(_WIN32)
    interpreter = qgetenv("COMSPEC");
    command << "/k" << externalCommand;
#else
    interpreter = "???"; // Default fallback
#endif

    return {command, interpreter};
}

static auto setupPty(QProcess &process, int &masterFd) -> bool {
#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    masterFd = posix_openpt(O_RDWR | O_NOCTTY);
    if (masterFd < 0) {
        return false;
    }

    if (grantpt(masterFd) < 0) {
        close(masterFd);
        return false;
    }

    if (unlockpt(masterFd) < 0) {
        close(masterFd);
        return false;
    }

    char slaveName[512];
    if (ptsname_r(masterFd, slaveName, sizeof(slaveName)) != 0) {
        close(masterFd);
        return false;
    }

    int slaveFd = open(slaveName, O_RDWR | O_NOCTTY);
    if (slaveFd < 0) {
        close(masterFd);
        return false;
    }

    process.setStandardInputFile(QString::fromUtf8(slaveName));
    process.setStandardOutputFile(QString::fromUtf8(slaveName));
    process.setStandardErrorFile(QString::fromUtf8(slaveName));

    close(slaveFd);

    QObject::connect(&process, &QProcess::finished, [&]() {
        if (masterFd >= 0) {
            close(masterFd);
            masterFd = -1;
        }
    });
    return true;
#else
    Q_UNUSED(process);
    Q_UNUSED(masterFd);
    return false;
#endif
}

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

int ProjectBuildModel::findConfigDirIndex(const QString &dir) {
    auto i = 0;
    for (const auto &v : configs) {
        if (v->sourceDir == dir) {
            return i;
        }
        i++;
    }
    return -1;
}

std::shared_ptr<ProjectBuildConfig> ProjectBuildModel::findConfigDir(const QString &dir) {
    for (const auto &v : configs) {
        if (v->sourceDir == dir) {
            return v;
        }
    }
    return {};
}

std::shared_ptr<ProjectBuildConfig> ProjectBuildModel::findConfigFile(const QString &fileName) {
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

    config.pluginName = tr("Project manager");
    config.description = tr("Add support for building using CMake/Cargo/Go");
    config.configItems.push_back(
        qmdiConfigItem::Builder()
            .setDisplayName(tr("Save before running tasks (build, config etc)"))
            .setDescription(tr("If checked, files are saved before running any task"))
            .setKey(Config::SaveBeforeTaskKey)
            .setType(qmdiConfigItem::Bool)
            .setDefaultValue(true)
            .build());
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     .setDisplayName(tr("Extra paths"))
                                     .setDescription(tr("Add new paths for compilers/tools"))
                                     .setKey(Config::ExtraPathKey)
                                     .setType(qmdiConfigItem::StringList)
                                     .setDefaultValue(QStringList())
                                     .build());

    // project GUI
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     // .setDisplayName(tr("Open directories"))
                                     .setKey(Config::OpenDirsKey)
                                     .setType(qmdiConfigItem::StringList)
                                     .setDefaultValue(QStringList())
                                     .setUserEditable(false)
                                     .build());
    config.configItems.push_back(qmdiConfigItem::Builder()
                                     // .setDisplayName(tr("Currently open directory"))
                                     .setKey(Config::SelectedDirectoryKey)
                                     .setType(qmdiConfigItem::String)
                                     .setDefaultValue(QString{})
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
    QMessageBox::information(dynamic_cast<QMainWindow *>(mdiServer), tr("About"),
                             tr("The project manager plugin"));
}

void ProjectManagerPlugin::on_client_merged(qmdiHost *host) {
    IPlugin::on_client_merged(host);
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
    manager->createNewPanel(Panels::West, "projectmamager", tr("Project"), w);

    projectIssues = new ProjectIssuesWidget(manager);
    issuesDock =
        manager->createNewPanel(Panels::South, "projectissues", tr("Issues"), projectIssues);

    auto *w2 = new QWidget;
    outputPanel = new Ui::BuildRunOutput;
    outputPanel->setupUi(w2);
    outputDock = manager->createNewPanel(Panels::South, "buildoutput", tr("Output"), w2);
    outputPanel->commandOuput->setLineWrapMode(QTextEdit::LineWrapMode::NoWrap);
    outputPanel->commandOuput->viewport()->setCursor(Qt::CursorShape::IBeamCursor);
    outputPanel->commandOuput->setOpenExternalLinks(false);
    outputPanel->commandOuput->setOpenLinks(false);

    auto font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    outputPanel->commandOuput->setFont(font);
    outputPanel->commandOuput->setAcceptRichText(true);
    outputPanel->commandOuput->viewport()->setCursor(Qt::CursorShape::IBeamCursor);
    outputPanel->commandOuput->setLineWrapMode(QTextEdit::NoWrap);
    connect(outputPanel->commandOuput, &QTextBrowser::anchorClicked, outputPanel->commandOuput,
            [this](const QUrl &link) {
                if (!link.isLocalFile()) {
                    QDesktopServices::openUrl(link);
                    return;
                }
                auto fileName = QFileInfo(link.toLocalFile()).filePath();
                auto row = -1;
                auto col = -1;
                auto fragment = link.fragment();

                if (!fragment.isEmpty()) {
                    // line/rows are 1 based on compiler output, internally they are 0 based
                    auto parts = fragment.split(',');
                    auto ok = false;
                    if (!parts.isEmpty()) {
                        row = parts[0].toInt(&ok);
                        if (!ok) {
                            row = -1;
                        } else {
                            row--;
                        }
                    }
                    if (parts.size() > 1) {
                        col = parts[1].toInt(&ok);
                        if (!ok) {
                            col = -1;
                        } else {
                            col--;
                        }
                    }
                }
                getManager()->openFile(fileName, row, col);
            });

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
        processBuildOutput(output);
    });
    connect(&runProcess, &QProcess::readyReadStandardError, this, [this]() {
        auto output = this->runProcess.readAllStandardError();
        processBuildOutput(output);
    });
    connect(&runProcess, &QProcess::finished, this,
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
                auto output = QString("[code=%1, status=%2]").arg(exitCode).arg(str(exitStatus));
                appendAnsiHtml(outputPanel->commandOuput, output);
                getManager()->showPanels(Qt::BottomDockWidgetArea);
                outputDock->raise();
                outputDock->show();

                auto process = sender();
                auto var1 = process->property("runningTask").value<quintptr>();
                auto *runningTask = reinterpret_cast<TaskInfo *>(var1);
                
                auto project = process->property("runningProject").value<std::shared_ptr<ProjectBuildConfig>>();
                auto buildDirectory = process->property("buildDirectory").toString();
                auto sourceDirectory = process->property("sourceDirectory").toString();
                auto projectName = QString{};
                {
                    auto d = QDir(sourceDirectory);
                    projectName = d.dirName();
                }

                if (project && runningTask && runningTask->isBuild) {
                    qDebug() << "Notifying about a good build" << project->buildDir
                             << buildDirectory << project->sourceDir << sourceDirectory
                             << runningTask->name;

                    // clang-format off
                    getManager()->handleCommand(GlobalCommands::BuildFinished, {
                        {GlobalArguments::BuildDirectory, buildDirectory },
                        {GlobalArguments::SourceDirectory, sourceDirectory },
                        {GlobalArguments::Name, runningTask->name},
                        // {"",  exitStatus == QProcess::ExitStatus::NormalExit},
                        {"code", exitStatus == 0},
                    });
                    // clang-format on
                }

                runProcess.setProperty("runningTask", {});
                runProcess.setProperty("runningProject", {});
            });
    connect(&runProcess, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        auto output = QString("[error: code=%1]").arg((int)error);
        appendAnsiHtml(outputPanel->commandOuput, output);
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
            auto path = buildConfig->sourceDir + "/" + "qtedit4.json";
            buildConfig->saveToFile(path);
            configWatcher.addPath(buildConfig->fileName);
        }
        getManager()->openFile(buildConfig->fileName);
    });

    projectModel = new ProjectBuildModel();
    searchPanelUI = new ProjectSearch(manager, projectModel);
    auto searchPanel = manager->createNewPanel(Panels::West, "search", tr("Search"), searchPanelUI);
    auto projectSearch = new QAction(tr("Search in project"));
    projectSearch->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_F));
    connect(projectSearch, &QAction::triggered, this, [searchPanel, this]() {
        searchPanel->raise();
        searchPanel->show();
        searchPanel->setFocus();
        searchPanelUI->setFocusOnSearch();
    });
    manager->addAction(projectSearch);
    kitsModel = new KitDefinitionModel();
    gui->kitComboBox->setModel(kitsModel);

    auto dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    auto kits = findKitDefinitions(QDir::toNativeSeparators(dataPath).toStdString());
    kitsModel->setKitDefinitions(kits);

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

void ProjectManagerPlugin::loadConfig(QSettings &settings) {
    IPlugin::loadConfig(settings);

    searchPanelUI->setSearchPattern(getConfig().getSearchPattern());
    searchPanelUI->setSearchInclude(getConfig().getSearchInclude());
    searchPanelUI->setSearchExclude(getConfig().getSearchExclude());
    auto dirsToLoad = getConfig().getOpenDirs();
    QTimer::singleShot(0, this, [this, dirsToLoad]() {
        for (const auto &dirName : dirsToLoad) {
            auto config = projectModel->findConfigDir(dirName);
            if (config) {
                continue;
            }

            gui->projectComboBox->blockSignals(true);
            config = ProjectBuildConfig::buildFromDirectory(dirName);
            projectModel->addConfig(config);
            if (!config->fileName.isEmpty()) {
                // qDebug("loadConfig() - adding %s to the watch dir",
                // config->fileName.toStdString().c_str());
                configWatcher.addPath(config->fileName);
            }

            auto hash = getConfigDictionary(config);
            auto workingDirectory = expand(config->buildDir, hash);
            // clang-format off
            getManager()->handleCommand(GlobalCommands::ProjectLoaded, {
                {GlobalArguments::ProjectName, config->name },
                {GlobalArguments::SourceDirectory, config->sourceDir },
                {GlobalArguments::BuildDirectory, workingDirectory },
            });
            // clang-format on

            auto selectedDirectory = getConfig().getSelectedDirectory();
            auto index = projectModel->findConfigDirIndex(selectedDirectory);
            if (index >= 0) {
                gui->projectComboBox->setCurrentIndex(index);
                newProjectSelected(index);
            }
            gui->projectComboBox->blockSignals(false);
            searchPanelUI->updateProjectList();
        }
    });
}

void ProjectManagerPlugin::saveConfig(QSettings &settings) {
    auto selectedProject = getCurrentConfig();
    auto selectProjetName = selectedProject ? selectedProject->sourceDir : "";
    getConfig().setOpenDirs(projectModel->getAllOpenDirs());
    getConfig().setSelectedDirectory(selectProjetName);
    getConfig().setSearchPattern(searchPanelUI->getSearchPattern());
    getConfig().setSearchInclude(searchPanelUI->getSearchInclude());
    getConfig().setSearchExclude(searchPanelUI->getSearchExclude());
    IPlugin::saveConfig(settings);
}

int ProjectManagerPlugin::canOpenFile(const QString &fileName) {
    auto uri = QUrl(fileName);
    if (uri.scheme() == "loaded") {
        return 5;
    }
    if (uri.scheme() == "projectmanager") {
        return 5;
    }
    return 0;
}

bool ProjectManagerPlugin::openFile(const QString &requestedUri, int x, int, int) {
    auto uri = QUrl(requestedUri);
    auto filename = uri.path();
    auto fi = QFileInfo(filename);
    auto dir = fi.dir().absolutePath();

    if (uri.scheme() == "loaded") {
        return tryOpenProject(filename, dir);
    }

    if (uri.scheme() == "projectmanager") {
        return tryScrollOutput(x);
    }

    // just edit the file
    return false;
}

std::shared_ptr<ProjectBuildConfig> ProjectManagerPlugin::getCurrentConfig() const {
    auto currentIndex = gui->projectComboBox->currentIndex();
    if (currentIndex < 0) {
        return {};
    }
    return projectModel->getConfig(currentIndex);
}

const QHash<QString, QString>
ProjectManagerPlugin::getConfigDictionary(std::shared_ptr<ProjectBuildConfig> project) const {
    auto dictionary = QHash<QString, QString>();
    if (project) {
        dictionary["source_directory"] = QDir::toNativeSeparators(project->sourceDir);
        dictionary["build_directory"] = QDir::toNativeSeparators(project->buildDir);
    }
    return dictionary;
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
        // buildConfig->fileName.toStdString().c_str());
        configWatcher.addPath(buildConfig->fileName);
    }

    auto hash = getConfigDictionary(buildConfig);
    auto buildDirectory = expand(buildConfig->buildDir, hash);
    auto sourceDirectory = expand(buildConfig->sourceDir, hash);

    // clang-format off
    getManager()->handleCommand(GlobalCommands::ProjectLoaded, {
        {GlobalArguments::ProjectName, buildConfig->name },
        {GlobalArguments::SourceDirectory, sourceDirectory },
        {GlobalArguments::BuildDirectory, buildDirectory },
    });
    // clang-format on

    projectModel->addConfig(buildConfig);
    searchPanelUI->updateProjectList();
    gui->projectComboBox->setCurrentIndex(projectModel->rowCount() - 1);
    getManager()->saveSettings();
}

void ProjectManagerPlugin::removeProject_clicked() {
    auto index = gui->projectComboBox->currentIndex();
    if (index < 0) {
        return;
    }
    auto config = projectModel->getConfig(index);
    auto path = config->fileName;
    projectModel->removeConfig(index);
    searchPanelUI->updateProjectList();
    configWatcher.removePath(path);
    getManager()->saveSettings();

    // clang-format off
    getManager()->handleCommand(GlobalCommands::ProjectRemoved, {
        {GlobalArguments::ProjectName, config->name },
        {GlobalArguments::SourceDirectory, config->sourceDir },
        {GlobalArguments::BuildDirectory, config->buildDir },
        }
    );
    // clang-format on
}

void ProjectManagerPlugin::newProjectSelected(int index) {
    // TODO - on startup this is called 2 times. I am unsure why yet.
    //        so this works around it. Its not the best solution.
    static auto lastProjectSelected = std::shared_ptr<ProjectBuildConfig>();
    auto buildConfig = std::shared_ptr<ProjectBuildConfig>();
    if (index >= 0) {
        buildConfig = projectModel->getConfig(index);
    }

    if (!buildConfig) {
        this->directoryModel->removeAllDirs();
        this->gui->filterFiles->clear();
        this->gui->filterFiles->setEnabled(false);
        this->gui->filterOutFiles->clear();
        this->gui->filterOutFiles->setEnabled(false);
    } else {
        auto project = getCurrentConfig();
        auto dictionary = getConfigDictionary(project);
        auto s1 = expand(buildConfig->displayFilter, dictionary);
        auto s2 = expand(buildConfig->hideFilter, dictionary);
        this->gui->filterFiles->setEnabled(true);
        this->gui->filterFiles->setText(s1);
        this->gui->filterOutFiles->setEnabled(true);
        this->gui->filterOutFiles->setText(s2);
        if (lastProjectSelected != buildConfig) {
            this->gui->loadingWidget->start();
            this->directoryModel->removeAllDirs();
            this->directoryModel->addDirectory(buildConfig->sourceDir);
            connect(directoryModel, &DirectoryModel::didFinishLoading, this,
                    [this]() { this->gui->loadingWidget->stop(); });
        }
    }
    lastProjectSelected = buildConfig;

    updateTasksUI(buildConfig);
    updateExecutablesUI(buildConfig);
}

void ProjectManagerPlugin::do_runExecutable(const ExecutableInfo *info) {
    if (runProcess.processId() != 0) {
        runProcess.kill();
        return;
    }

    auto project = getCurrentConfig();
    auto hash = getConfigDictionary(project);
    auto executablePath = QDir::toNativeSeparators(findExecForPlatform(info->executables));
    auto currentTask = expand(executablePath, hash);
    auto [command, interpreter] = getCommandInterpreter(currentTask);

    auto workingDirectory = info->runDirectory;
    if (workingDirectory.isEmpty()) {
        workingDirectory = project->buildDir;
    }
    workingDirectory = expand(workingDirectory, hash);
    outputPanel->commandOuput->clear();
    appendAnsiHtml(outputPanel->commandOuput, "cd " + QDir::toNativeSeparators(workingDirectory));
    appendAnsiHtml(outputPanel->commandOuput, QString("\n%1\n").arg(currentTask));
    outputDock->raise();
    outputDock->show();
    runProcess.setWorkingDirectory(workingDirectory);
    runProcess.start(interpreter, command);
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
    auto hash = getConfigDictionary(project);
    auto taskCommand = expand(task->command, hash);
    auto workingDirectory = expand(task->runDirectory, hash);
    auto buildDirectory = expand(project->buildDir, hash);
    auto sourceDirectory = expand(project->sourceDir, hash);

    if (workingDirectory.isEmpty()) {
        workingDirectory = buildDirectory;
    }
    workingDirectory = QDir::toNativeSeparators(workingDirectory);
    buildDirectory = QDir::toNativeSeparators(buildDirectory);
    sourceDirectory = QDir::toNativeSeparators(sourceDirectory);

    outputDock->raise();
    outputDock->show();
    outputPanel->commandOuput->clear();
    appendAnsiHtml(outputPanel->commandOuput, "cd " + workingDirectory + "\n");

    auto env = QProcessEnvironment::systemEnvironment();
    env.insert("FORCE_COLOR", "1");
    env.insert("CLICOLOR_FORCE", "1");

#if defined(USE_TTY_FOR_TASKS)
    auto usingPty = false;
    auto masterFd = -1;
    usingPty = setupPty(runProcess, masterFd);
    if (usingPty && masterFd >= 0) {
        runProcess.setProcessChannelMode(QProcess::MergedChannels);
        auto notifier = new QSocketNotifier(masterFd, QSocketNotifier::Read, &runProcess);
        connect(&runProcess, &QProcess::finished, notifier,
                [notifier, masterFd]() { delete notifier; });
        connect(notifier, &QSocketNotifier::activated, notifier, [this, masterFd]() {
            char buffer[4096];
            auto bytesRead = read(masterFd, buffer, sizeof(buffer) - 1);
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                auto data = QByteArray(buffer, bytesRead);
                auto lines = QString::fromUtf8(data);
                processBuildOutput(lines);
            }
        });
        env.insert("TERM", "xterm-256color");
    }
#endif

    if (!kit) {
        // run the taskCommand directly in the native shell
        auto [command, interpreter] = getCommandInterpreter(taskCommand);
        appendAnsiHtml(outputPanel->commandOuput, interpreter + " " + command.join(" "));

        runProcess.setWorkingDirectory(workingDirectory);
        runProcess.setProgram(interpreter);
        runProcess.setArguments(command);
    } else {
        // ask the active kit, to run the task
        env.insert("run_directory", workingDirectory);
        env.insert("build_directory", buildDirectory);
        env.insert("source_directory", sourceDirectory);
        env.insert("task", taskCommand);
        runProcess.setProgram(QString::fromStdString(kit->filePath));
    }

    runProcess.setWorkingDirectory(workingDirectory);
    runProcess.setProcessEnvironment(env);

    auto manager = getManager();
    auto count = manager->visibleTabs();
    for (auto i = size_t(0); i < count; i++) {
        auto client = manager->getMdiClient(i);
        if (auto editor = dynamic_cast<qmdiEditor *>(client)) {
            editor->removeMetaData();
        }
    }

    runProcess.setProperty("runningTask", QVariant::fromValue(reinterpret_cast<quintptr>(task)));
    runProcess.setProperty("runningProject", QVariant::fromValue(project));
    runProcess.setProperty("workingDirectory", QVariant::fromValue(workingDirectory));
    runProcess.setProperty("buildDirectory", QVariant::fromValue(buildDirectory));
    runProcess.setProperty("sourceDirectory", QVariant::fromValue(sourceDirectory));

    runProcess.start();
    if (!runProcess.waitForStarted()) {
        if (kit) {
            auto msg = QString("Failed to run kit %1, with task=%2")
                           .arg(QString::fromStdString(kit->filePath), taskCommand);
            appendAnsiHtml(outputPanel->commandOuput, msg);
        } else {
            appendAnsiHtml(outputPanel->commandOuput, "Process failed to start " + taskCommand);
        }
        qWarning() << "Process failed to start";
    }
}

void ProjectManagerPlugin::runButton_clicked() {
    assert(this->selectedTarget);
    do_runExecutable(this->selectedTarget);
}

void ProjectManagerPlugin::runTask_clicked() {
    auto buildConfig = getCurrentConfig();
    if (selectedTaskIndex < 0) {
        selectedTaskIndex = 0;
    }
    if (!buildConfig || selectedTaskIndex >= buildConfig->tasksInfo.size()) {
        return;
    }

    if (getConfig().getSaveBeforeTask()) {
        if (!saveAllDocuments()) {
            return;
        }
    }
    this->projectIssues->clearAllIssues();
    do_runTask(&buildConfig->tasksInfo[selectedTaskIndex]);
}

void ProjectManagerPlugin::clearProject_clicked() {
    auto project = getCurrentConfig();
    if (project == nullptr || project->buildDir.isEmpty()) {
        return;
    }

    auto hash = getConfigDictionary(project);
    auto projectBuildDir = QDir::toNativeSeparators(expand(project->buildDir, hash));
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

auto ProjectManagerPlugin::saveAllDocuments() -> bool {
    for (auto i = 0; i < mdiServer->getClientsCount(); i++) {
        auto c = mdiServer->getClient(i);
        if (!c->canCloseClient()) {
            return false;
        }
    }
    return true;
}

auto ProjectManagerPlugin::processBuildOutput(const QString &line) -> void {
    auto cursor = this->outputPanel->commandOuput->textCursor();
    auto lineNumber = cursor.blockNumber();
    auto plaintext = removeAnsiEscapeCodes(line);
    appendAnsiHtml(this->outputPanel->commandOuput, line);
    this->projectIssues->processLine(plaintext, lineNumber, this->getCurrentConfig()->sourceDir);
}

auto ProjectManagerPlugin::updateTasksUI(std::shared_ptr<ProjectBuildConfig> buildConfig) -> void {
    if (!buildConfig || buildConfig->tasksInfo.size() == 0) {
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
        selectedTaskIndex = taskIndex;
        this->gui->taskButton->setEnabled(true);
        this->gui->taskButton->setText(taskName);
        this->gui->taskButton->setToolTip(taskCommand);

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

                this->selectedTaskIndex = index;
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

auto ProjectManagerPlugin::tryOpenProject(const QString &filename, const QString &dir) -> bool {
    auto manager = getManager();
    auto client = manager->clientForFileName(filename);
    if (client) {
        emit manager->newClientAdded(client);
    }

    auto project = this->projectModel->findConfigFile(filename);
    if (project) {
        // TODO - should we select this project?
        return true;
    }

    project = this->projectModel->findConfigDir(dir);
    if (project) {
        // TODO - should we select this project?
        return true;
    }

    if (!ProjectBuildConfig::canLoadFile(filename)) {
        return true;
    }

    project = ProjectBuildConfig::buildFromFile(filename);
    if (!project) {
        // project file is not parsable. just bail out.
        return false;
    }

    auto text = tr("This is a project file, do you want to load the project, or just edit the "
                   "file?<br/><br/> <b>%1</b>")
                    .arg(filename);
    QMessageBox box;
    box.setWindowTitle(tr("Load project"));
    box.setIcon(QMessageBox::Question);
    box.setText(text);
    box.setTextFormat(Qt::RichText);
    box.addButton("&Load the project (enter)", QMessageBox::AcceptRole);
    box.addButton("&Edit the file (escape)", QMessageBox::RejectRole);
    auto result = box.exec();

    if (result == 2) {
        // User chose "Load the project"
        projectModel->addConfig(project);
        searchPanelUI->updateProjectList();
        auto hash = getConfigDictionary(project);
        auto buildDirectory = expand(project->buildDir, hash);
        auto sourceDirectory = expand(project->sourceDir, hash);

        getManager()->saveSettings();
        // clang-format off
        getManager()->handleCommand(GlobalCommands::ProjectLoaded, {
            {GlobalArguments::ProjectName, project->name },
            {GlobalArguments::SourceDirectory, sourceDirectory },
            {GlobalArguments::BuildDirectory, buildDirectory },
        });
        // clang-format on
        return true;
    }

    return false;
}

auto ProjectManagerPlugin::tryScrollOutput(int line) -> bool {

    auto browser = this->outputPanel->commandOuput;
    if (!browser) {
        return false;
    }

    const auto doc = browser->document();
    if (!doc) {
        return false;
    }

    if (line < 0 || line >= doc->blockCount()) {
        return false;
    }

    auto block = doc->findBlockByLineNumber(line);
    if (!block.isValid()) {
        return false;
    }

    auto cursor = QTextCursor(block);
    auto rect = browser->cursorRect(cursor);
    auto blockY = rect.top();
    auto viewportHeight = browser->viewport()->height();
    auto centerScrollValue = blockY - viewportHeight / 2;
    auto vScrollBar = browser->verticalScrollBar();
    centerScrollValue = qMax(0, qMin(centerScrollValue, vScrollBar->maximum()));
    vScrollBar->setValue(centerScrollValue);
    return true;
}
