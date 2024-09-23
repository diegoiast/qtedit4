/**
 * \file help_plg.h
 * \brief Definition of the help system  plugin
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL
 * \see HelpPlugin
 */

#pragma once

#include "iplugin.h"

class QAction;

class HelpPlugin : public IPlugin {
    Q_OBJECT
  public:
    HelpPlugin();
    ~HelpPlugin();

    void showAbout();
  public slots:
    void actionAbout_triggered();
    void checkForUpdates_triggered();
};
