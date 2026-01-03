/**
 * \file TerminalPlugin.hpp
 * \brief Definition of the Terminal plugin
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License MIT
 */

#pragma once

#include "iplugin.h"

class QDockWidget;
class QAction;
class KodoTerm;

class TerminalPlugin : public IPlugin {
    Q_OBJECT
  public:
    TerminalPlugin();
    ~TerminalPlugin();
    
    // IPlugin interface
    virtual void on_client_merged(qmdiHost *host) override;
    virtual void on_client_unmerged(qmdiHost *host) override;
    virtual void loadConfig(QSettings &settings) override;
    
  private:
    QDockWidget *terminalDock = nullptr;
    QAction *toggleTerminal;
    KodoTerm *console = nullptr;
};
