#include "ProjectBuildConfig.h"

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

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
        this->command == other.command &&
        this->runDirectory == other.runDirectory;
    /* clang-format on */
}

auto ProjectBuildConfig::tryGuessFromCMake(const QString &directory)
    -> std::shared_ptr<ProjectBuildConfig> {
    auto cmakeFileName = directory + "/" + "CMakeLists.txt";
    auto fi = QFileInfo(cmakeFileName);
    if (!fi.isReadable()) {
        return {};
    }

    auto value = std::make_shared<ProjectBuildConfig>();
    value->sourceDir = directory;
    value->hideFilter = ".git;.vscode;.vs;build;dist";
    value->buildDir = "${source_directory}/build";

    // TODO - we should query for available binaries after configure.
    {
        auto di = QFileInfo(directory);
        auto e = ExecutableInfo();
        e.name = di.fileName();
        e.runDirectory = "${build_directory}";
        e.executables["windows"] = "${build_directory}/bin/" + e.name + ".exe";
        e.executables["linux"] = "${build_directory}/bin/" + e.name;
        value->executables.push_back(e);
    }
    {
        auto t = TaskInfo();
        t.name = "CMake configure)";
        t.command = "cmake -S ${source_directory} -B ${build_directory} -DCMAKE_BUILD_TYPE=Debug";
        t.runDirectory = "${source_directory}";
        value->tasksInfo.push_back(t);
    }
    {
        auto t = TaskInfo();
        t.name = "Build (parallel)";
        t.command = "cmake --build ${build_directory} --parallel";
        t.runDirectory = "${source_directory}";
        value->tasksInfo.push_back(t);
    }
    {
        auto t = TaskInfo();
        t.name = "Build (single thread)";
        t.command = "cmake --build ${build_directory}";
        t.runDirectory = "${source_directory}";
        value->tasksInfo.push_back(t);
    }
    return value;
}

auto ProjectBuildConfig::tryGuessFromCargo(const QString &directory)
    -> std::shared_ptr<ProjectBuildConfig> {
    auto cargoFileName = directory + "/" + "Cargo.toml";
    auto fi = QFileInfo(cargoFileName);
    if (!fi.isReadable()) {
        return {};
    }

    auto value = std::make_shared<ProjectBuildConfig>();
    value->sourceDir = directory;
    value->hideFilter = ".git;.vscode;target";
    value->buildDir = directory + "/target";

    {
        auto di = QFileInfo(directory);
        auto e = ExecutableInfo();
        e.name = "cargo run";
        e.runDirectory = "${source_directory}";
        e.executables["windows"] = "cargo run";
        e.executables["linux"] = "cargo run";
        value->executables.push_back(e);
    }
    {
        auto t = TaskInfo();
        t.name = "cargo build";
        t.runDirectory = "${source_directory}";
        t.command = "cargo build";
        value->tasksInfo.push_back(t);
    }
    {
        auto t = TaskInfo();
        t.name = "cargo build (release)";
        t.runDirectory = "${source_directory}";
        t.command = "cargo build --release";
        value->tasksInfo.push_back(t);
    }
    {
        auto t = TaskInfo();
        t.name = "cargo update";
        t.runDirectory = "${source_directory}";
        t.command = "cargo update";
        value->tasksInfo.push_back(t);
    }

    return value;
}

auto ProjectBuildConfig::tryGuessFromGo(const QString &directory)
    -> std::shared_ptr<ProjectBuildConfig> {
    auto gomodFileName = directory + "/" + "go.mod";
    auto fi = QFileInfo(gomodFileName);
    if (!fi.isReadable()) {
        return {};
    }

    auto value = std::make_shared<ProjectBuildConfig>();
    value->sourceDir = directory;
    value->hideFilter = ".git;.vscode;";
    value->buildDir = "";

    {
        auto di = QFileInfo(directory);
        auto e = ExecutableInfo();
        e.name = "go run";
        e.runDirectory = "${source_directory}";
        e.executables["windows"] = "go run ${source_directory}";
        e.executables["linux"] = "go run ${source_directory}";
        value->executables.push_back(e);
    }
    {
        auto t = TaskInfo();
        t.name = "go build";
        t.runDirectory = "${source_directory}";
        t.command = "go build";
        value->tasksInfo.push_back(t);
    }
    {
        auto t = TaskInfo();
        t.name = "go fix";
        t.runDirectory = "${source_directory}";
        t.command = "go fix";
        value->tasksInfo.push_back(t);
    }
    return value;
}

std::shared_ptr<ProjectBuildConfig>
ProjectBuildConfig::buildFromDirectory(const QString &directory) {
    auto configFileName = directory + "/" + "qtedit4.json";
    auto config = buildFromFile(configFileName);
    if (!config) {
        config = tryGuessFromCMake(directory);
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

    auto value = std::make_shared<ProjectBuildConfig>();
    auto fi = QFileInfo(jsonFileName);
    auto json = QJsonDocument::fromJson(file.readAll());
    value->sourceDir = fi.absolutePath();
    value->fileName = fi.absoluteFilePath();
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
            for (const auto &vv : v.toArray()) {
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

        value->activeExecutableName = json["activeExecutableName"].toString();
        value->activeTaskName = json["activeTaskName"].toString();
        value->displayFilter = json["displayFilter"].toString();
        value->hideFilter = json["hideFilter"].toString();
    }
    return value;
}

auto ProjectBuildConfig::saveToFile(const QString &jsonFileName) -> void {
    auto file = QFile(jsonFileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file for writing:" << file.errorString();
        return;
    }

    auto jsonObj = QJsonObject();
    jsonObj["sourceDir"] = sourceDir;
    jsonObj["build_directory"] = buildDir;

    auto execsArray = QJsonArray();
    for (const auto &exec : executables) {
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
    for (const auto &task : tasksInfo) {
        auto taskObj = QJsonObject();
        taskObj["name"] = task.name;
        taskObj["command"] = task.command;
        taskObj["runDirectory"] = task.runDirectory;

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
