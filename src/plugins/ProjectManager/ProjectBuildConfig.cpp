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

std::shared_ptr<ProjectBuildConfig>
ProjectBuildConfig::buildFromDirectory(const QString &directory) {
    auto configFileName = directory + "/" + "qtedit4.json";
    return buildFromFile(configFileName);
}

std::shared_ptr<ProjectBuildConfig> ProjectBuildConfig::buildFromFile(const QString &jsonFileName) {
    auto value = std::make_shared<ProjectBuildConfig>();
    auto fi = QFileInfo(jsonFileName);
    value->sourceDir = fi.absolutePath();
    value->fileName = fi.absoluteFilePath();

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
    auto parsePlatformConfig = [](QJsonValue v) -> PlatformConfig {
        auto config = PlatformConfig();
        config.pathAppend = v["path_append"].toString();
        config.setup = v["setup"].toString();
        config.pathPrepend = v["path_prepend"].toString();
        return config;
    };

    if (!json.isNull()) {
        value->buildDir = json["build_directory"].toString();
        value->executables = parseExecutables(json["executables"]);
        value->tasksInfo = parseTasksInfo(json["tasks"]);

        value->activeExecutableName = json["activeExecutableName"].toString();
        value->activeTaskName = json["activeTaskName"].toString();
        value->displayFilter = json["displayFilter"].toString();
        value->hideFilter = json["hideFilter"].toString();

        if (json["config"].isObject()) {
            QJsonObject jsonObj = json["config"].toObject();
            for (const auto &vv : jsonObj.keys()) {
                value->platformConfig[vv] = parsePlatformConfig(jsonObj[vv]);
            }
        }
    }
    return value;
}

auto ProjectBuildConfig::saveToFile(const QString &jsonFileName) -> void {
    // todo
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
