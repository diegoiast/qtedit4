#include "LspPlugin.hpp"
#include "GlobalCommands.hpp"

LspPlugin::LspPlugin() {
    name = tr("LSP support");
    author = tr("Diego Iastrubni <diegoiast@gmail.com>");
    iVersion = 0;
    sVersion = "0.0.1";
    autoEnabled = true;
    alwaysEnabled = false;
}

LspPlugin::~LspPlugin() {
    // todo
}

void LspPlugin::on_client_merged(qmdiHost *host) {
    IPlugin::on_client_merged(host);

    // todo - what more?
    client.setDocumentRoot("/home/diego/src/diego/qtedit4");
}

int LspPlugin::canHandleCommand(const QString &command, const CommandArgs &args) const {
    // using highest, to win over ctags plugin
    if (command == GlobalCommands::BuildFinished) {
        return CommandPriority::HighestPriority;
    }
    if (command == GlobalCommands::ProjectLoaded) {
        return CommandPriority::HighestPriority;
    }
    if (command == GlobalCommands::ProjectRemoved) {
        return CommandPriority::HighestPriority;
    }
    // if (command == GlobalCommands::VariableInfo) {
    // return CommandPriority::HighestPriority;
    // }
    return CommandPriority::CannotHandle;
}

CommandArgs LspPlugin::handleCommand(const QString &command, const CommandArgs &args) {
    if (command == GlobalCommands::ProjectLoaded) {
        // lspServerProcess.start(lspServer);
        // client.
        return {};
    }
    if (command == GlobalCommands::ProjectRemoved) {
        // lspServerProcess.kill();
        return {};
    }
    if (command == GlobalCommands::BuildFinished) {
        return {};
    }
    // if (command == GlobalCommands::VariableInfo) {
    // return CommandPriority::HighestPriority;
    // }
    return {};
}
