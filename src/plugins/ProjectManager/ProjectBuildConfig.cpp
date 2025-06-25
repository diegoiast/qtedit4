#include "ProjectBuildConfig.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QString>

using StringHash = QHash<QString, QString>;
using StringPair = QPair<QString, QString>;

// Returns map: executable target name -> full path to executable binary
auto getExecutableFromTargetFile(const QString &filePath) -> std::optional<StringPair> {
    auto file = QFile(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "getExecutableFromTargetFile: Failed to open target file:" << filePath;
        return std::nullopt;
    }

    auto doc = QJsonDocument::fromJson(file.readAll());
    auto obj = doc.object();
    if (obj["type"].toString() != "EXECUTABLE") {
        return std::nullopt;
    }

    auto artifacts = obj["artifacts"].toArray();
    if (artifacts.isEmpty()) {
        return std::nullopt;
    }
    auto name = obj["name"].toString();
    auto path = artifacts[0].toObject()["path"].toString();
    return StringPair{name, path};
}

auto getExecutablesFromCMakeFileAPI(const QString &buildDir) -> StringHash {
    auto executables = StringHash();
    auto replyDir = QDir(buildDir + "/.cmake/api/v1/reply");
    if (!replyDir.exists()) {
        qWarning() << "getExecutablesFromCMakeFileAPI: Reply directory does not exist:"
                   << replyDir.path();
        return executables;
    }

    auto indexFiles = replyDir.entryList({"index-*.json"}, QDir::Files);
    if (indexFiles.isEmpty()) {
        qWarning() << "getExecutablesFromCMakeFileAPI: No index file found";
        return executables;
    }

    auto indexFilePath = replyDir.filePath(indexFiles.first());
    auto indexFile = QFile(indexFilePath);
    if (!indexFile.open(QIODevice::ReadOnly)) {
        qWarning() << "getExecutablesFromCMakeFileAPI: Failed to open index file:" << indexFilePath;
        return executables;
    }

    auto indexDoc = QJsonDocument::fromJson(indexFile.readAll());
    auto indexObj = indexDoc.object();
    auto objects = indexObj["objects"].toArray();

    auto codemodelFile = QString();
    for (const auto &entry : objects) {
        auto obj = entry.toObject();
        if (obj["kind"].toString() == "codemodel") {
            codemodelFile = obj["jsonFile"].toString();
            break;
        }
    }

    if (codemodelFile.isEmpty()) {
        qWarning() << "getExecutablesFromCMakeFileAPI: No codemodel found in index file";
        return executables;
    }

    auto codemodelPath = replyDir.filePath(codemodelFile);
    auto codemodel = QFile(codemodelPath);
    if (!codemodel.open(QIODevice::ReadOnly)) {
        qWarning() << "getExecutablesFromCMakeFileAPI: Failed to open codemodel file:"
                   << codemodelPath;
        return executables;
    }

    auto codeDoc = QJsonDocument::fromJson(codemodel.readAll());
    auto codeObj = codeDoc.object();
    auto configurations = codeObj["configurations"].toArray();
    if (configurations.isEmpty()) {
        qWarning() << "getExecutablesFromCMakeFileAPI: No configurations in codemodel";
        return executables;
    }

    auto targets = configurations[0].toObject()["targets"].toArray();
    for (auto const &target : targets) {
        auto targetFile = target.toObject()["jsonFile"].toString();
        if (targetFile.isEmpty()) {
            continue;
        }

        auto targetFilePath = replyDir.filePath(targetFile);
        auto result = getExecutableFromTargetFile(targetFilePath);
        if (result.has_value()) {
            executables.insert(result->first, result->second);
        }
    }

    return executables;
}

auto static cargoListBinUnits(const QString &directoryPath, bool /*recursive*/ = true)
    -> StringHash {
    auto fileMap = StringHash{};
    auto process = QProcess{};
    process.setProgram("cargo");
    process.setArguments({"metadata", "--format-version=1", "--no-deps"});
    process.setWorkingDirectory(directoryPath);
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start();
    if (!process.waitForFinished(10000)) {
        qWarning() << "cargoListBinUnits: cargo metadata timed out or failed";
        return fileMap;
    }

    auto output = process.readAllStandardOutput();
    if (output.isEmpty()) {
        qWarning() << "cargoListBinUnits: cargo metadata output is empty";
        return fileMap;
    }

    auto jsonDoc = QJsonDocument::fromJson(output);
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        qWarning() << "cargoListBinUnits: Failed to parse cargo metadata JSON";
        return fileMap;
    }

    auto rootObj = jsonDoc.object();
    auto targetDirStr = rootObj.value("target_directory").toString();
    if (targetDirStr.isEmpty()) {
        qWarning() << "cargoListBinUnits: target_directory missing";
        return fileMap;
    }

    auto targetDir = QDir(targetDirStr);
    auto packages = rootObj.value("packages").toArray();
    for (const auto &packageVal : packages) {
        if (!packageVal.isObject()) {
            continue;
        }

        auto packageObj = packageVal.toObject();
        auto targets = packageObj.value("targets").toArray();

        for (const auto &targetVal : targets) {
            if (!targetVal.isObject()) {
                continue;
            }

            auto targetObj = targetVal.toObject();
            auto kindArray = targetObj.value("kind").toArray();
            bool isBin = std::any_of(kindArray.constBegin(), kindArray.constEnd(),
                                     [](const auto &val) { return val.toString() == "bin"; });
            if (!isBin) {
                continue;
            }

            auto targetName = targetObj.value("name").toString();
#ifdef Q_OS_WIN
            auto exeName = targetName + ".exe";
#else
            auto exeName = targetName;
#endif
            auto exePath = targetDir.filePath(QDir("debug").filePath(exeName));
            fileMap.insert(targetName, exePath);
        }
    }
    return fileMap;
}

auto static arePlatformCommandsIdentical(const QHash<QString, QStringList> &commands) -> bool {
    if (commands.isEmpty()) {
        return false;
    }
    auto firstCommand = commands.begin().value();
    for (auto it = std::next(commands.begin()); it != commands.end(); ++it) {
        if (it.value() != firstCommand) {
            return false;
        }
    }
    return true;
}

auto static parsePlatformCommands(const QJsonObject &commandsObj, TaskInfo &taskInfo) -> void {
    for (auto it = commandsObj.begin(); it != commandsObj.end(); ++it) {
        auto value = it.value();
        if (value.isString()) {
            taskInfo.commands.insert(it.key(), QStringList{value.toString()});
        } else if (value.isArray()) {
            QStringList commands;
            for (const auto &cmd : value.toArray()) {
                commands.append(cmd.toString());
            }
            taskInfo.commands.insert(it.key(), commands);
        }
    }
}

auto static savePlatformCommands(const QHash<QString, QStringList> &commands,
                                 QJsonObject &commandsObj) -> auto {
    for (auto it = commands.constBegin(); it != commands.constEnd(); ++it) {
        auto commands = it.value();
        if (commands.size() == 1) {
            commandsObj[it.key()] = commands.first();
        } else {
            auto commandsArray = QJsonArray();
            for (const auto &cmd : commands) {
                commandsArray.append(cmd);
            }
            commandsObj[it.key()] = commandsArray;
        }
    }
}

bool ExecutableInfo::operator==(const ExecutableInfo &other) const {
    /* clang-format off */
    return
        this->name == other.name &&
        this->runDirectory == other.runDirectory &&
        this->executables == other.executables;
    /* clang-format on */
}

bool TaskInfo::operator==(const TaskInfo &other) const {
    /* clang-format off */
    return
        this->name == other.name &&
        this->commands == other.commands &&
        this->isBuild == other.isBuild &&
        this->runDirectory == other.runDirectory;
    /* clang-format on */
}

auto ProjectBuildConfig::tryGuessFromCMake(const QString &directory)
    -> std::shared_ptr<ProjectBuildConfig> {
    auto cmakeFileName = directory + QDir::separator() + "CMakeLists.txt";
    auto di = QFileInfo(directory);
    auto fi = QFileInfo(cmakeFileName);
    if (!fi.isReadable()) {
        return {};
    }

    auto value = std::make_shared<ProjectBuildConfig>();
    value->projectType = ProjectType::cmake;
    value->autoGenerated = true;
    value->name = di.baseName();
    value->sourceDir = directory;
    value->hideFilter = ".git;.vscode;.vs;cbuild;dist";
    value->buildDir = "${source_directory}/cbuild";

    // Debug configuration
    {
        auto t = TaskInfo();
        t.name = "CMake (configure/Debug)";
        t.commands.insert(PLATFORM_LINUX,
                          {"mkdir -p ${build_directory}/.cmake/api/v1/query/",
                           "touch ${build_directory}/.cmake/api/v1/query/codemodel-v2",
                           "cmake -S ${source_directory} -B ${build_directory} "
                           "-DCMAKE_BUILD_TYPE=Debug"});
        t.commands.insert(PLATFORM_WINDOWS,
                          {"mkdir -p ${build_directory}/.cmake/api/v1/query/",
                           "type nul > ${build_directory}/.cmake/api/v1/query/codemodel-v2",
                           "cmake -S ${source_directory} -B ${build_directory} "
                           "-DCMAKE_BUILD_TYPE=Debug"});
        t.runDirectory = "${source_directory}";
        t.isBuild = true;
        value->tasksInfo.push_back(t);
    }

    // Release configuration
    {
        auto t = TaskInfo();
        t.name = "CMake (configure/Release)";
        t.commands.insert(PLATFORM_LINUX,
                          {"mkdir -p ${build_directory}/.cmake/api/v1/query/",
                           "touch ${build_directory}/.cmake/api/v1/query/codemodel-v2",
                           "cmake -S ${source_directory} -B ${build_directory} "
                           "-DCMAKE_BUILD_TYPE=Release"});
        t.commands.insert(PLATFORM_WINDOWS,
                          {"mkdir -p ${build_directory}/.cmake/api/v1/query/",
                           "type nul > ${build_directory}/.cmake/api/v1/query/codemodel-v2",
                           "cmake -S ${source_directory} -B ${build_directory} "
                           "-DCMAKE_BUILD_TYPE=Release"});
        t.runDirectory = "${source_directory}";
        t.isBuild = true;
        value->tasksInfo.push_back(t);
    }

    // Build commands
    auto cmakeBuildParallel = "cmake --build ${build_directory} --parallel";
    auto cmakeBuildSingle = "cmake --build ${build_directory}";

    {
        auto t = TaskInfo();
        t.name = "CMake Build (parallel)";
        t.commands.insert(PLATFORM_LINUX, {cmakeBuildParallel});
        t.commands.insert(PLATFORM_WINDOWS, {cmakeBuildParallel});
        t.runDirectory = "${source_directory}";
        t.isBuild = true;
        value->tasksInfo.push_back(t);
    }
    {
        auto t = TaskInfo();
        t.name = "CMake Build (single thread)";
        t.commands.insert(PLATFORM_LINUX, {cmakeBuildSingle});
        t.commands.insert(PLATFORM_WINDOWS, {cmakeBuildSingle});
        t.runDirectory = "${source_directory}";
        t.isBuild = true;
        value->tasksInfo.push_back(t);
    }

    value->updateBinariesCMake();
    return value;
}

auto ProjectBuildConfig::tryGuessFromCargo(const QString &directory)
    -> std::shared_ptr<ProjectBuildConfig> {
    auto cargoFileName = directory + "/" + "Cargo.toml";
    auto di = QFileInfo(directory);
    auto fi = QFileInfo(cargoFileName);
    if (!fi.isReadable()) {
        return {};
    }

    auto value = std::make_shared<ProjectBuildConfig>();
    value->projectType = ProjectType::cargo;
    value->autoGenerated = true;
    value->name = di.baseName();
    value->sourceDir = directory;
    value->hideFilter = ".git;.vscode;target";
    value->buildDir = "${source_directory}/target";

    auto cargoBuild = "cargo build";
    auto cargoBuildRelease = "cargo build --release";
    auto cargoUpdate = "cargo update";

    {
        auto t = TaskInfo();
        t.name = "cargo build";
        t.runDirectory = "${source_directory}";
        t.commands.insert(PLATFORM_LINUX, {cargoBuild});
        t.commands.insert(PLATFORM_WINDOWS, {cargoBuild});
        t.isBuild = true;
        value->tasksInfo.push_back(t);
    }
    {
        auto t = TaskInfo();
        t.name = "cargo build (release)";
        t.runDirectory = "${source_directory}";
        t.commands.insert(PLATFORM_LINUX, {cargoBuildRelease});
        t.commands.insert(PLATFORM_WINDOWS, {cargoBuildRelease});
        t.isBuild = true;
        value->tasksInfo.push_back(t);
    }
    {
        auto t = TaskInfo();
        t.name = "cargo update";
        t.runDirectory = "${source_directory}";
        t.commands.insert(PLATFORM_LINUX, {cargoUpdate});
        t.commands.insert(PLATFORM_WINDOWS, {cargoUpdate});
        value->tasksInfo.push_back(t);
    }
    value->updateBinariesCargo();

    return value;
}

auto ProjectBuildConfig::tryGuessFromGo(const QString &directory)
    -> std::shared_ptr<ProjectBuildConfig> {
    auto gomodFileName = directory + "/" + "go.mod";
    auto di = QFileInfo(directory);
    auto fi = QFileInfo(gomodFileName);
    if (!fi.isReadable()) {
        return {};
    }

    auto value = std::make_shared<ProjectBuildConfig>();
    value->projectType = ProjectType::golang;
    value->autoGenerated = true;
    value->name = di.baseName();
    value->sourceDir = directory;
    value->hideFilter = ".git;.vscode;";
    value->buildDir = "";

    auto goBuild = "go build";
    auto goFix = "go fix";

    {
        auto t = TaskInfo();
        t.name = "go build";
        t.runDirectory = "${source_directory}";
        t.commands.insert(PLATFORM_LINUX, {goBuild});
        t.commands.insert(PLATFORM_WINDOWS, {goBuild});
        value->tasksInfo.push_back(t);
    }
    {
        auto t = TaskInfo();
        t.name = "go fix";
        t.runDirectory = "${source_directory}";
        t.commands.insert(PLATFORM_LINUX, {goFix});
        t.commands.insert(PLATFORM_WINDOWS, {goFix});
        value->tasksInfo.push_back(t);
    }
    value->updateBinariesGo();
    return value;
}

std::shared_ptr<ProjectBuildConfig>
ProjectBuildConfig::tryGuessFromMeson(const QString &directory) {
    auto mesonBuildFile = directory + "/" + "meson.build";
    auto di = QFileInfo(directory);
    auto fi = QFileInfo(mesonBuildFile);
    if (!fi.isReadable()) {
        return {};
    }

    auto value = std::make_shared<ProjectBuildConfig>();
    value->projectType = ProjectType::meson;
    value->name = di.baseName();
    value->sourceDir = directory;
    value->hideFilter = ".git;.vscode;";
    value->buildDir = "${source_directory}/mbuild"; // meson build?

    auto mesonSetup = "meson setup ${build_directory}";
    auto mesonBuild = "meson compile -C ${build_directory}";
    auto mesonTest = "meson test -C ${build_directory}";

    {
        auto t = TaskInfo();
        t.name = "meson setup";
        t.runDirectory = "${source_directory}";
        t.commands.insert(PLATFORM_LINUX, {mesonSetup});
        t.commands.insert(PLATFORM_WINDOWS, {mesonSetup});
        t.isBuild = true;
        value->tasksInfo.push_back(t);
    }
    {
        auto t = TaskInfo();
        t.name = "meson build";
        t.runDirectory = "${source_directory}";
        t.commands.insert(PLATFORM_LINUX, {mesonBuild});
        t.commands.insert(PLATFORM_WINDOWS, {mesonBuild});
        t.isBuild = true;
        value->tasksInfo.push_back(t);
    }
    {
        auto t = TaskInfo();
        t.name = "meson tests";
        t.runDirectory = "${source_directory}";
        t.commands.insert(PLATFORM_LINUX, {mesonTest});
        t.commands.insert(PLATFORM_WINDOWS, {mesonTest});
        value->tasksInfo.push_back(t);
    }
    value->updateBinariesMeson();
    return value;
}

std::shared_ptr<ProjectBuildConfig>
ProjectBuildConfig::buildFromDirectory(const QString &directory) {
    auto configFileName = directory + QDir::separator() + "qtedit4.json";
    auto config = buildFromFile(configFileName);
    if (!config) {
        config = tryGuessFromCMake(directory);
    }
    if (!config) {
        config = tryGuessFromMeson(directory);
    }
    if (!config) {
        config = tryGuessFromCargo(directory);
    }
    if (!config) {
        config = tryGuessFromGo(directory);
    }
    if (!config) {
        config = std::make_shared<ProjectBuildConfig>();
        config->sourceDir = directory;
    }
    return config;
}

std::shared_ptr<ProjectBuildConfig> ProjectBuildConfig::buildFromFile(const QString &jsonFileName) {
    auto file = QFile();
    file.setFileName(jsonFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    auto value = std::shared_ptr<ProjectBuildConfig>();
    auto fi = QFileInfo(jsonFileName);
    auto json = QJsonDocument::fromJson(file.readAll());
    file.close();

    auto toHash = [](QJsonValueRef v) -> QHash<QString, QString> {
        QHash<QString, QString> hash;
        if (v.isObject()) {
            auto jsonObj = v.toObject();
            for (const auto &vv : jsonObj.keys()) {
                hash[vv] = jsonObj[vv].toString();
            }
        }
        return hash;
    };
    auto parseExecutables = [&toHash](QJsonValue v) -> QList<ExecutableInfo> {
        QList<ExecutableInfo> info;
        if (v.isArray()) {
            for (const auto &vv : v.toArray()) {
                ExecutableInfo execInfo;
                execInfo.name = vv.toObject().value("name").toString();
                execInfo.executables = toHash(vv.toObject()["executables"]);
                execInfo.runDirectory = vv.toObject()["runDirectory"].toString();
                info.push_back(execInfo);
            };
        }
        return info;
    };
    auto parseTasksInfo = [](QJsonValue v) -> QList<TaskInfo> {
        QList<TaskInfo> info;
        if (v.isArray()) {
            for (auto const &vv : v.toArray()) {
                auto obj = vv.toObject();
                TaskInfo taskInfo;
                taskInfo.name = obj["name"].toString();
                auto commandsObj = obj["commands"].toObject();
                if (commandsObj.size() == 1) {
                    auto value = commandsObj.begin().value();
                    if (value.isString()) {
                        // Single command for all platforms
                        auto cmd = QStringList(value.toString());
                        taskInfo.commands.insert(PLATFORM_LINUX, cmd);
                        taskInfo.commands.insert(PLATFORM_WINDOWS, cmd);
                    } else if (value.isArray()) {
                        // Multiple commands for all platforms
                        auto commands = QStringList();
                        for (auto const &cmd : value.toArray()) {
                            commands.append(cmd.toString());
                        }
                        taskInfo.commands.insert(PLATFORM_LINUX, commands);
                        taskInfo.commands.insert(PLATFORM_WINDOWS, commands);
                    } else {
                        // Different commands per platform
                        parsePlatformCommands(commandsObj, taskInfo);
                    }
                } else {
                    // Different commands per platform
                    parsePlatformCommands(commandsObj, taskInfo);
                }
                taskInfo.runDirectory = obj["runDirectory"].toString();
                taskInfo.isBuild = obj["isBuild"].toBool(false);
                info.push_back(taskInfo);
            };
        }
        return info;
    };
    if (!json.isNull()) {
        value = std::make_shared<ProjectBuildConfig>();
        value->autoGenerated = false;
        value->sourceDir = fi.absolutePath();
        value->fileName = fi.absoluteFilePath();
        value->name = json["name"].toString();
        value->buildDir = json["build_directory"].toString();
        value->executables = parseExecutables(json["executables"]);
        value->tasksInfo = parseTasksInfo(json["tasks"]);

        value->activeExecutableName = json["activeExecutableName"].toString();
        value->activeTaskName = json["activeTaskName"].toString();
        value->displayFilter = json["displayFilter"].toString();
        value->hideFilter = json["hideFilter"].toString();

        if (value->name.isEmpty()) {
            value->name = fi.dir().dirName();
        }
    } else {
        qDebug() << "ProjectBuildConfig::buildFromFile: Loading file failed, malformed JSON?"
                 << jsonFileName;
    }
    return value;
}

bool ProjectBuildConfig::canLoadFile(const QString &filename) {
    auto fi = QFileInfo(filename);
    if (fi.fileName().compare("CMakeLists.txt", Qt::CaseInsensitive) == 0) {
        return true;
    }
    if (fi.fileName().compare("Cargo.toml", Qt::CaseInsensitive) == 0) {
        return true;
    }
    if (fi.fileName().compare("go.mod", Qt::CaseInsensitive) == 0) {
        return true;
    }
    if (fi.fileName().compare("qtedit4.json", Qt::CaseInsensitive) == 0) {
        return true;
    }
    return false;
}

auto ProjectBuildConfig::updateBinaries() -> void {
    switch (projectType) {
    case ProjectType::cmake:
        updateBinariesCMake();
        break;
    case ProjectType::cargo:
        updateBinariesCargo();
        break;
    case ProjectType::golang:
        updateBinariesGo();
        break;
    case ProjectType::meson:
        updateBinariesMeson();
        break;
    case ProjectType::unkown:
        break;
    }
}

auto ProjectBuildConfig::updateBinariesCMake() -> void {
    this->executables.clear();
    auto buildDir = expand(this->buildDir);
    auto binaries = getExecutablesFromCMakeFileAPI(buildDir);
    for (const auto &[key, value] : binaries.asKeyValueRange()) {
        auto e = ExecutableInfo();
        e.name = value;
        e.runDirectory = "${build_directory}";
        e.executables[PLATFORM_LINUX] = "${build_directory}/" + value;
        e.executables[PLATFORM_WINDOWS] = "${build_directory}/" + value;
        this->executables.push_back(e);
    }
}

auto ProjectBuildConfig::updateBinariesCargo() -> void {
    auto e = ExecutableInfo();
    this->executables.clear();

#if 0
    e.name = "cargo run";
    e.runDirectory = "${source_directory}";
    e.executables[PLATFORM_LINUX] = "cargo run";
    e.executables[PLATFORM_WINDOWS] = "cargo run";
    this->executables.push_back(e);
#endif
    auto binaries = cargoListBinUnits(this->sourceDir);
    for (const auto &[key, value] : binaries.asKeyValueRange()) {
        e.name = key;
        e.runDirectory = "${source_directory}";
        e.executables[PLATFORM_LINUX] = value;
        e.executables[PLATFORM_WINDOWS] = value;
        this->executables.push_back(e);
    }
}

auto ProjectBuildConfig::updateBinariesGo() -> void {
    auto e = ExecutableInfo();
    e.name = "go run";
    e.runDirectory = "${source_directory}";
    e.executables[PLATFORM_LINUX] = "go run ${source_directory}";
    e.executables[PLATFORM_WINDOWS] = "go run ${source_directory}";
    this->executables.clear();
    this->executables.push_back(e);
}

auto ProjectBuildConfig::updateBinariesMeson() -> void {
    auto findMesonExecutables = [](const QString &directory,
                                   const QString &buildDir) -> QHash<QString, QString> {
        auto fullBuildPath = buildDir;
        auto process = QProcess();
        process.start("meson", QStringList() << "introspect" << fullBuildPath << "--targets");
        process.waitForFinished();

        auto output = process.readAllStandardOutput();
        if (output.isEmpty()) {
            qCritical() << "No output from meson introspect.";
            return {};
        }

        auto parseError = QJsonParseError();
        auto doc = QJsonDocument::fromJson(output, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qCritical() << "Failed to parse JSON:" << parseError.errorString();
            return {};
        }

        auto targets = doc.array();
        auto result = QHash<QString, QString>();
        for (auto value : targets) {
            QJsonObject obj = value.toObject();
            if (obj["type"].toString() == "executable") {
                auto name = obj["name"].toString();
                auto filenames = obj["filename"].toArray();
                if (!filenames.isEmpty()) {
                    auto fullPath = filenames.first().toString();
                    auto relativeToDir = QDir(directory).relativeFilePath(fullPath);
                    result.insert(name, relativeToDir);
                }
            }
        }
        return result;
    };

    auto executables = findMesonExecutables(this->sourceDir, this->buildDir);
    this->executables.clear();
    for (auto it = executables.constBegin(); it != executables.constEnd(); ++it) {
        auto name = it.key();
        auto path = it.value();
        auto e = ExecutableInfo();
        e.name = name;
        e.runDirectory = "${source_directory}";
        e.executables[PLATFORM_LINUX] = path;
        e.executables[PLATFORM_WINDOWS] = path + ".exe";
        this->executables.push_back(e);
    }
}

auto ProjectBuildConfig::saveToFile(const QString &jsonFileName) -> void {
    auto file = QFile(jsonFileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file for writing:" << file.errorString();
        return;
    }

    auto jsonObj = QJsonObject();
    jsonObj["build_directory"] = buildDir;

    auto execsArray = QJsonArray();
    for (const auto &exec : std::as_const(executables)) {
        auto execObj = QJsonObject();
        execObj["name"] = exec.name;
        execObj["runDirectory"] = exec.runDirectory;

        auto execsDetailsObj = QJsonObject();
        for (auto it = exec.executables.constBegin(); it != exec.executables.constEnd(); ++it) {
            execsDetailsObj[it.key()] = it.value();
        }
        execObj["executables"] = execsDetailsObj;

        execsArray.append(execObj);
    }
    jsonObj["executables"] = execsArray;

    auto tasksArray = QJsonArray();
    for (const auto &task : std::as_const(tasksInfo)) {
        auto taskObj = QJsonObject();
        taskObj["name"] = task.name;
        if (!task.commands.isEmpty()) {
            if (arePlatformCommandsIdentical(task.commands)) {
                // All platforms have the same command
                auto firstCommand = task.commands.begin().value();
                if (firstCommand.size() == 1) {
                    taskObj["commands"] = firstCommand.first();
                } else {
                    auto commandsArray = QJsonArray();
                    for (const auto &cmd : firstCommand) {
                        commandsArray.append(cmd);
                    }
                    taskObj["commands"] = commandsArray;
                }
            } else {
                // Different commands per platform
                auto commandsObj = QJsonObject();
                savePlatformCommands(task.commands, commandsObj);
                taskObj["commands"] = commandsObj;
            }
        } else {
            // No commands at all
            taskObj["commands"] = QJsonValue();
        }
        taskObj["runDirectory"] = task.runDirectory;
        taskObj["isBuild"] = task.isBuild;

        tasksArray.append(taskObj);
    }
    jsonObj["tasks"] = tasksArray;

    jsonObj["activeExecutableName"] = activeExecutableName;
    jsonObj["activeTaskName"] = activeTaskName;
    jsonObj["displayFilter"] = displayFilter;
    jsonObj["hideFilter"] = hideFilter;

    auto jsonDoc = QJsonDocument(jsonObj);
    file.write(jsonDoc.toJson());
    file.close();

    this->fileName = jsonFileName;
}

auto ProjectBuildConfig::findIndexOfTask(const QString &taskName) -> int {
    for (auto n = 0; n < tasksInfo.length(); n++) {
        auto &t = tasksInfo[n];
        if (t.name == taskName) {
            return n;
        }
    }
    return -1;
}

auto ProjectBuildConfig::findIndexOfExecutable(const QString &executableName) -> int {
    for (auto n = 0; n < executables.length(); n++) {
        auto &e = executables[n];
        if (e.name == executableName) {
            return n;
        }
    }
    return -1;
}

auto ProjectBuildConfig::getConfigDictionary() const -> const QHash<QString, QString> {
    auto dictionary = QHash<QString, QString>();
    dictionary["source_directory"] = QDir::toNativeSeparators(sourceDir);
    dictionary["build_directory"] = QDir::toNativeSeparators(buildDir);
    return dictionary;
}

auto ProjectBuildConfig::expand(const QString &input) -> QString {
    static auto regex = QRegularExpression(R"(\$\{([a-zA-Z0-9_]+)\})");
    auto output = input;
    auto depth = 0;
    auto maxDepth = 10;
    auto hashTable = getConfigDictionary();

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

bool ProjectBuildConfig::operator==(const ProjectBuildConfig &other) const {
    /* clang-format off */
    return sourceDir == other.sourceDir &&
           buildDir == other.buildDir &&
           executables == other.executables &&
           tasksInfo == other.tasksInfo &&
           activeExecutableName == other.activeExecutableName &&
           activeTaskName == other.activeTaskName &&
           displayFilter == other.displayFilter &&
           hideFilter == other.hideFilter &&
           fileName == other.fileName;
    /* clang-format on */
}
