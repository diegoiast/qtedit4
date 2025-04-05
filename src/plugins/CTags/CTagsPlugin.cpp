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

    ctagsBinary = "c:\\Users\\diego\\Downloads\\ctags\\ctags.exe";
}

CTagsPlugin::~CTagsPlugin() { projects.clear(); }

CTagsPlugin::Config &CTagsPlugin::getConfig() {
    static Config configObject{&this->config};
    return configObject;
}

int CTagsPlugin::canHandleCommand(const QString &command, const CommandArgs &) const {
    if (command == GlobalCommands::BuildFinished) {
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
    if (command == GlobalCommands::BuildFinished) {
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
        auto exactMatch = args[GlobalArguments::RequestedSymbol].toBool();
        return symbolInfoRequested(filename, symbol, exactMatch);
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

CommandArgs CTagsPlugin::symbolInfoRequested(const QString &fileName, const QString &symbol,
                                             bool exactMatch) {
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

    QVariantList tagList;
    auto tags = project->findTags(symbol.toStdString(), exactMatch);
    for (auto &tagRef : tags) {
        const CTag &tag = tagRef.get();

        // Convert fields map to QVariantMap
        QVariantMap fields;
        for (const auto &[key, value] : tag.fields) {
            fields[QString::fromStdString(tagFieldKeyToString(key))] =
                QString::fromStdString(value);
        }

        tagList.append(QVariant::fromValue(
            CommandArgs{{GlobalArguments::FileName, QString::fromStdString(tag.file)},
                        {"fields", fields},
                        {"raw", QString::fromStdString(tag.address)}}));
    }

    CommandArgs res;
    res["tags"] = tagList;

    return res;
}
