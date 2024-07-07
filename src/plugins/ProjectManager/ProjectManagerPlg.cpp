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
        return "crashed";
    }
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
    auto seachID = manager->createNewPanel(Panels::West, "Search", searchPanelUI);

    auto projectSearch = new QAction(tr("Search in project"));
    projectSearch->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_F));
    connect(projectSearch, &QAction::triggered, [seachID, manager, searchPanelUI]() {
        manager->showPanel(Panels::West, seachID);
        searchPanelUI->setFocusOnSearch();
    });
    manager->addAction(projectSearch);

    projectModel = new ProjectBuildModel();
    gui->comboBox->setModel(projectModel);
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
    QString s = QFileDialog::getExistingDirectory(gui->filesView, tr("Add directory"));
    if (s.isEmpty()) {
        return;
    }

    auto config = ProjectBuildConfig::buildFromDirectory(s);
    projectModel->addConfig(config);
    directoryModel->addDirectory(s);
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
        executableName.clear();
        executablePath.clear();
        gui->runButton->setText("...");
        gui->runButton->setToolTip("...");
        gui->runButton->setEnabled(false);
    } else {
        executableName = config->executables[0].name;
        executablePath = findExecForPlatform(config->executables[0].executables);
        gui->runButton->setEnabled(true);
        gui->runButton->setText(executableName);
        gui->runButton->setToolTip(executablePath);

        if (config->executables.size() == 1) {
            gui->runButton->setMenu(nullptr);
        } else {
            auto menu = new QMenu(gui->runButton);
            QList<QAction *> actions;
            for (auto a : config->executables) {
                QAction *action = new QAction(a.name, this);
                menu->addAction(action);
                actions.append(action);
            }
            gui->runButton->setMenu(menu);
            connect(menu, &QMenu::triggered, [this, actions, config](QAction *action) {
                int index = actions.indexOf(action);
                executableName = config->executables[index].name;
                executablePath = findExecForPlatform(config->executables[index].executables);
                gui->runButton->setEnabled(true);
                gui->runButton->setText(executableName);
                gui->runButton->setToolTip(executablePath);
            });
        }
    }

    if (!config || config->tasksInfo.size() == 0) {
        taskName.clear();
        taskCommand.clear();
        gui->taskButton->setText("...");
        gui->taskButton->setToolTip("...");
        gui->taskButton->setEnabled(false);
    } else {
        taskName = config->tasksInfo[0].name;
        taskCommand = config->tasksInfo[0].command;
        gui->taskButton->setEnabled(true);
        gui->taskButton->setText(taskName);
        gui->taskButton->setToolTip(taskCommand);
        if (config->tasksInfo.size() == 1) {
            gui->taskButton->setMenu(nullptr);
        } else {
            auto menu = new QMenu(gui->runButton);
            QList<QAction *> actions;
            for (auto a : config->tasksInfo) {
                QAction *action = new QAction(a.name, this);
                menu->addAction(action);
                actions.append(action);
            }
            gui->taskButton->setMenu(menu);
            connect(menu, &QMenu::triggered, [this, actions, config](QAction *action) {
                int index = actions.indexOf(action);
                taskName = config->tasksInfo[index].name;
                taskCommand = config->tasksInfo[index].command;
                gui->taskButton->setEnabled(true);
                gui->taskButton->setText(taskName);
                gui->taskButton->setToolTip(taskCommand);
            });
        }
    }
}

static auto expand(const QString &input, const QHash<QString, QString> &hashTable) -> QString {
    QString output = input;
    QRegularExpression regex(R"(\$\{([a-zA-Z0-9_]+)\})");
    QRegularExpressionMatchIterator it = regex.globalMatch(input);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString key = match.captured(1);
        QString replacement = hashTable.value(key, "");
        output.replace(match.captured(0), replacement);
    }
    return output;
}

void ProjectManagerPlugin::on_removeProject_clicked(bool checked) {
    // TODO
}

void ProjectManagerPlugin::on_runButton_clicked() {
    auto project = getCurrentConfig();
    auto hash = QHash<QString, QString>();
    hash["source_directory"] = project->sourceDir;
    hash["build_directory"] = project->buildDir;

    auto command = QStringList();
    auto currentTask = expand(executablePath, hash);
    outputPanel->commandOuput->clear();
    outputPanel->commandOuput->appendPlainText(currentTask);

    command.append("-c");
    command.append(currentTask);
    runProcess.start("/bin/bash", command);
    if (!runProcess.waitForStarted()) {
        qWarning() << "Process failed to start";
    }
}

void ProjectManagerPlugin::on_runTask_clicked() {
    auto project = getCurrentConfig();
    auto hash = QHash<QString, QString>();
    hash["source_directory"] = project->sourceDir;
    hash["build_directory"] = project->buildDir;

    auto command = QStringList();
    auto currentTask = expand(taskCommand, hash);
    outputPanel->commandOuput->clear();
    outputPanel->commandOuput->appendPlainText(currentTask);

    command.append("-c");
    command.append(currentTask);
    runProcess.start("/bin/bash", command);
    if (!runProcess.waitForStarted()) {
        qWarning() << "Process failed to start";
    }
}

void ProjectManagerPlugin::on_clearProject_clicked() {
    // TODO
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
    QFile file;
    file.setFileName(jsonFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return value;
    }
    QJsonDocument json = QJsonDocument::fromJson(file.readAll());
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
                info.push_back(taskInfo);
            };
        }
        return info;
    };

    auto fi = QFileInfo(jsonFileName);
    value->sourceDir = fi.absolutePath();
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
