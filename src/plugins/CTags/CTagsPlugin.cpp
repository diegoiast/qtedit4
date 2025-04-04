#include "CTagsPlugin.hpp"
#include "CTagsLoader.hpp"
#include "GlobalCommands.hpp"

#include <QDebug>
#include <QDir>

CTagsPlugin::CTagsPlugin() {
    name = tr("CTags support");
    author = tr("Diego Iastrubni <diegoiast@gmail.com>");
    iVersion = 0;
    sVersion = "0.0.1";
    autoEnabled = true;
    alwaysEnabled = false;
}

CTagsPlugin::~CTagsPlugin() { projects.clear(); }

CTagsPlugin::Config &CTagsPlugin::getConfig() {
    static Config configObject{&this->config};
    return configObject;
}

int CTagsPlugin::canHandleCommand(const QString &command, const CommandArgs &) const {
    if (command == GlobalCommands::BuildSucceeded) {
        return CommandPriority::HighPriority;
    }
    if (command == GlobalCommands::ProjectLoaded) {
        return CommandPriority::HighPriority;
    }
    if (command == GlobalCommands::VariableInfo) {
        return CommandPriority::HighPriority;
    }
    return CommandPriority::CannotHandle;
}

CommandArgs CTagsPlugin::handleCommand(const QString &command, const CommandArgs &args) {
    if (command == GlobalCommands::BuildSucceeded) {
        auto sourceDir = args["sourceDir"].toString();
        auto buildDirectory = args["buildDirectory"].toString();
        auto projectName = args["projectName"].toString();
        newProjectBuilt(projectName, sourceDir, buildDirectory);
    }

    if (command == GlobalCommands::ProjectLoaded) {
        auto projectName = args[GlobalArguments::ProjectName].toString();
        auto sourceDir = args[GlobalArguments::SourceDirectory].toString();
        auto buildDirectory = args[GlobalArguments::BuildDirectory].toString();
        newProjectAdded(projectName, sourceDir, buildDirectory);
    }

    if (command == GlobalCommands::VariableInfo) {
        auto filename = args[GlobalArguments::FileName].toString();
        auto symbol = args[GlobalArguments::RequestedSymbol].toString();
        return symbolInfoRequested(filename, symbol);
    }
    return {};
}

void CTagsPlugin::setCTagsBinary(const QString &newBinary) {
    this->ctagsBinary = newBinary;
    for (const auto &k : projects.keys()) {
        projects[k]->setCTagsBinary(newBinary.toStdString());
    }
}

void CTagsPlugin::newProjectAdded(const QString &projectName, const QString &sourceDir,
                                  const QString &buildDirectory) {
    CTagsLoader *ctags = nullptr;
    if (projectName.isEmpty() || buildDirectory.isEmpty()) {
        return;
    }

    auto nativeSourceDir = QDir::toNativeSeparators(sourceDir);
    if (projects.contains(nativeSourceDir)) {
        ctags = projects.value(nativeSourceDir);
    } else {
        ctags = new CTagsLoader(ctagsBinary.toStdString());
        projects[nativeSourceDir] = ctags;
    }

    auto ctagsFile = buildDirectory + QDir::separator() + projectName + ".tags";
    ctags->loadFile(ctagsFile.toStdString());
}

void CTagsPlugin::newProjectBuilt(const QString &projectName, const QString &sourceDir,
                                  const QString &buildDirectory) {
    // project should be loaded first, something is borked
    auto nativeSourceDir = QDir::toNativeSeparators(sourceDir);
    if (!projects.contains(nativeSourceDir)) {
        qDebug() << "Project build, but not added first! ctags will not suppor it"
                 << nativeSourceDir;
        return;
    }
    auto ctags = projects.value(nativeSourceDir);
    auto ctagsFile = buildDirectory + QDir::separator() + projectName + ".tags";
    ctags->scanDirs(ctagsFile.toStdString(),
                    QDir::toNativeSeparators(nativeSourceDir).toStdString());
}

CommandArgs CTagsPlugin::symbolInfoRequested(const QString &fileName, const QString &symbol) {
    CTagsLoader *project = nullptr;
    for (const auto &k : projects.keys()) {
        if (fileName.startsWith(k)) {
            project = projects[k];
            break;
        }
    }

    if (!project) {
        return {};
    }

    auto tag = project->findTag(symbol.toStdString());
    if (tag) {
        CommandArgs res = {
            {GlobalArguments::FileName, QString::fromStdString(tag->file)},
            {GlobalArguments::LineNumber, tag->lineNumber},
            {GlobalArguments::ColumnNumber, tag->columnNumber},
            {"raw", QString ::fromStdString(tag->address)},
        };
        return res;
    }
    return {};
}
