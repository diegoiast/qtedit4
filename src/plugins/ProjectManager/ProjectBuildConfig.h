#pragma once

#include <QHash>
#include <QString>

struct ExecutableInfo {
    QString name;
    QString runDirectory;
    QHash<QString, QString> executables;

    bool operator==(const ExecutableInfo &other) const;
};

struct TaskInfo {
    QString name;
    QString command;
    QString runDirectory;

    bool operator==(const TaskInfo &other) const;
};

struct PlatformConfig {
    QString setup;
    QString pathPrepend;
    QString pathAppend;
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
    QHash<QString, PlatformConfig> platformConfig;

    // meta data
    QString fileName;

    static auto buildFromFile(const QString &jsonFileName) -> std::shared_ptr<ProjectBuildConfig>;
    static auto buildFromDirectory(const QString &directory) -> std::shared_ptr<ProjectBuildConfig>;

    auto saveToFile(const QString &jsonFileName) -> void;
    auto findIndexOfTask(const QString &taskName) -> int;
    auto findIndexOfExecutable(const QString &executableName) -> int;
    bool operator==(const ProjectBuildConfig &other) const;
};
