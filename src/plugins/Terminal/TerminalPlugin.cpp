/**
 * \file TerminalPlugin.cpp
 * \brief Implementation of the Terminal plugin
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License MIT
 */

#include <QSettings>
#include <QDockWidget>
#include <QKeySequence>

#include <plugins/Terminal/TerminalPlugin.hpp>
#include "KodoTerm/KodoTerm.hpp"

static QString systemCurrentShell() {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

#if defined(Q_OS_WIN)
    // Windows shell (usually cmd.exe or powershell)
    if (env.contains("ComSpec")) {
        return env.value("ComSpec");
    }
    return QString();
#else
    // Unix / Linux / macOS shell (bash, zsh, fish, etc.)
    if (env.contains("SHELL")) {
        return env.value("SHELL");
    }
    return QString();
#endif
}

TerminalPlugin::TerminalPlugin() {
    name = tr("Terminal support");
    author = tr("Diego Iastrubni <diegoiast@gmail.com>");
    iVersion = 0;
    sVersion = "0.0.1";
    autoEnabled = true;
    alwaysEnabled = false;
    
    toggleTerminal = new QAction(tr("Toggle terminal"), this);
    toggleTerminal->setShortcut(QKeySequence("Ctrl+T"));
    connect(toggleTerminal, &QAction::triggered, this, [this]() {
        if (terminalDock->isVisible()) {
            terminalDock->hide();
        } else {
            terminalDock->show();
            terminalDock->raise();
            terminalDock->widget()->setFocus();
            console->setFocus();
        }
    });
    
    menus[tr("&File")]->addAction(toggleTerminal);
}

TerminalPlugin::~TerminalPlugin() {
    // TODO
}

// IPlugin interface
void TerminalPlugin::on_client_merged(qmdiHost *host) {
    auto manager = dynamic_cast<PluginManager *>(host);
    console = new KodoTerm(manager);
    console->setProgram(systemCurrentShell());
    console->start();
    terminalDock = manager->createNewPanel(Panels::South, "terminalPanel", tr("Terminal"), console);
}
    
void TerminalPlugin::on_client_unmerged(qmdiHost *) {
    delete terminalDock;
}
    
void TerminalPlugin::loadConfig(QSettings &settings) {
    // TODO
}
