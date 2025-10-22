#pragma once

#include "iplugin.h"

/**
 * \file LspPlugin.hpp
 * \brief Definition of the Lsp support plugin
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License MIT
 * \see ProjectManager
 */

#include "LspClientImpl.hpp"

#include <QProcess>

class LspPlugin : public IPlugin {

  public:
    LspPlugin();
    ~LspPlugin();

    virtual void on_client_merged(qmdiHost *host) override;

    virtual int canHandleCommand(const QString &command, const CommandArgs &args) const override;
    virtual CommandArgs handleCommand(const QString &command, const CommandArgs &args) override;

    LspClientImpl client;
    QProcess lspServerProcess;
    QString lspServer = "/usr/bin/clangd";
};
