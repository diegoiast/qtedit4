#include "ProjectBuildConfig.h"

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

std::shared_ptr<ProjectBuildConfig>
ProjectBuildConfig::buildFromDirectory(const QString &directory) {
    auto configFileName = directory + "/" + "qtedit4.json";
    return buildFromFile(configFileName);
}

std::shared_ptr<ProjectBuildConfig> ProjectBuildConfig::buildFromFile(const QString &jsonFileName) {
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

        value->activeExecutableName = json["activeExecutableName"].toString();
        value->activeTaskName = json["activeTaskName"].toString();
        value->displayFilter = json["displayFilter"].toString();
        value->hideFilter = json["hideFilter"].toString();
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
