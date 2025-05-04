/**
 * \file SplitViewPlugin.cpp
 * \brief Definition of
 * \author Diego Iastrubni diegoiast@gmail.com
 * License GPL 2008
 * \see class name
 */

#pragma once

#include "iplugin.h"

class TextEditorPlugin;

class SplitViewPlugin : public IPlugin {
  public:
    SplitViewPlugin(TextEditorPlugin *editorPlugin);
    virtual void on_client_merged(qmdiHost *host) override;
    virtual void loadConfig(QSettings &settings) override;
    virtual void saveConfig(QSettings &settings) override;

  private:
    TextEditorPlugin *editorPlugin;
};
