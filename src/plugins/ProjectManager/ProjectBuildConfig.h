#pragma once

#include <QHash>
#include <QString>

struct ExecutableInfo {
    QString name;
    QString runDirectory;
    QHash<QString, QString> executables;
};

struct TaskInfo {
    QString name;
    QString command;
    QString runDirectory;
};

struct ProjectBuildConfig {
    QString sourceDir;
    QString buildDir;
    QList<ExecutableInfo> executables;
    QList<TaskInfo> tasksInfo;
    TaskInfo *buildTask;

    QString activeExecutableName;
    QString activeTaskName;
    QString displayFilter;
    QString hideFilter;

    static auto buildFromFile(const QString &jsonFileName) -> std::shared_ptr<ProjectBuildConfig>;
    static auto buildFromDirectory(const QString &directory) -> std::shared_ptr<ProjectBuildConfig>;

    auto saveToFile(const QString &jsonFileName) -> void;
    auto findIndexOfTask(const QString &taskName) -> int;
    auto findIndexOfExecutable(const QString &executableName) -> int;
};
