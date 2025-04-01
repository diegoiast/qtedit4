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
    return CommandPriority::CannotHandle;
}

CommandArgs CTagsPlugin::handleCommand(const QString &command, const CommandArgs &args) {
    if (command != GlobalCommands::BuildSucceeded) {
        return {};
    }

    // TODO - run in another thread
    // create tags file for this project
    auto sourceDir = args["sourceDir"].toString().toStdString();
    auto buildDirectory = args["buildDirectory"].toString().toStdString();
    auto projectName = args["projectName"].toString().toStdString();
    auto ctagsFile = buildDirectory + "/" + projectName + ".tags";

    qDebug() << "Will create tags file" << ctagsFile << "for dir=" << sourceDir;
    ctags->scanDirs(ctagsFile, sourceDir);

    return {};
}
