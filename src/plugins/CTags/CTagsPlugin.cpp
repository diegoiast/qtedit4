#include "CTagsPlugin.hpp"
#include "CTagsLoader.hpp"
#include "GlobalCommands.hpp"

#include <QDebug>

CTagsPlugin::CTagsPlugin() {
    name = tr("CTags support");
    author = tr("Diego Iastrubni <diegoiast@gmail.com>");
    iVersion = 0;
    sVersion = "0.0.1";
    autoEnabled = true;
    alwaysEnabled = false;

    ctags = new CTagsLoader;
}

CTagsPlugin::~CTagsPlugin() { delete ctags; }

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
        // TODO - run in another thread
        // create tags file for this project
        auto sourceDir = args["sourceDir"].toString().toStdString();
        auto buildDirectory = args["buildDirectory"].toString().toStdString();
        auto projectName = args["projectName"].toString().toStdString();
        auto ctagsFile = buildDirectory + "/" + projectName + ".tags";

        // TODO - the tag should be realtive to the source dir
        qDebug() << "Will create tags file" << ctagsFile << "for dir=" << sourceDir;
        ctags->scanDirs(ctagsFile, sourceDir);
    }

    if (command == GlobalCommands::ProjectLoaded) {
        auto projectName = args[GlobalArguments::ProjectName].toString().toStdString();
        auto sourceDir = args[GlobalArguments::SourceDirectory].toString().toStdString();
        auto buildDirectory = args[GlobalArguments::BuildDirectory].toString().toStdString();
        auto ctagsFile = buildDirectory + "/" + projectName + ".tags";
        qDebug() << "Will loadtags file" << ctagsFile << "for dir=" << sourceDir;
        ctags->loadFile(ctagsFile);
    }

    if (command == GlobalCommands::VariableInfo) {
        // auto sourceDir = args["sourceDir"].toString().toStdString();
        // auto buildDirectory = args["buildDirectory"].toString();
        // auto projectName = args["projectName"].toString().toStdString();
        // auto ctagsFile = buildDirectory + "/" + projectName + ".tags";
        auto filename = args[GlobalArguments::FileName].toString();
        auto symbol = args[GlobalArguments::RequestedSymbol].toString();

        // TODO - the tag should be realtive to the source dir
        auto tag = ctags->findTag(symbol.toStdString());

        if (tag) {
            CommandArgs res = {
                {GlobalArguments::FileName, QString::fromStdString(tag->tagFile)},
                {GlobalArguments::LineNumber, tag->lineNumber},
                {GlobalArguments::ColumnNumber, tag->columnNumber},
                {"raw", QString ::fromStdString(tag->tagAddress)},
            };
            return res;
        }
    }
    return {};
}
