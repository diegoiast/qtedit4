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

struct ProjectBuildConfig {
    ProjectBuildConfig() = default;
    QString sourceDir;
    QString buildDir;
    QList<ExecutableInfo> executables;
    QList<TaskInfo> tasksInfo;

    QString activeExecutableName;
    QString activeTaskName;
    QString displayFilter;
    QString hideFilter;

    // meta data
    QString fileName;

    static auto tryGuessFromCMake(const QString &directory) -> std::shared_ptr<ProjectBuildConfig>;
    static auto tryGuessFromCargo(const QString &directory) -> std::shared_ptr<ProjectBuildConfig>;
    static auto tryGuessFromGo(const QString &directory) -> std::shared_ptr<ProjectBuildConfig>;
    static auto buildFromDirectory(const QString &directory) -> std::shared_ptr<ProjectBuildConfig>;
    static auto buildFromFile(const QString &jsonFileName) -> std::shared_ptr<ProjectBuildConfig>;
    static auto canLoadFile(const QString &filename) -> bool;

    auto saveToFile(const QString &jsonFileName) -> void;
    auto findIndexOfTask(const QString &taskName) -> int;
    auto findIndexOfExecutable(const QString &executableName) -> int;
    bool operator==(const ProjectBuildConfig &other) const;
};
