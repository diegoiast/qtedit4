/**
 * \file imageviewer_plg.h
 * \brief Definition of the image viewer system  plugin
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL
 * \see ImageViewer plugin
 */

#pragma once

#include "iplugin.h"

class QAction;

class ImageViewrPlugin : public IPlugin {
    Q_OBJECT
  public:
    ImageViewrPlugin();
    ~ImageViewrPlugin();

    QStringList myExtensions() override;
    int canOpenFile(const QString fileName) override;
    bool openFile(const QString fileName, int x = -1, int y = -1, int z = -1) override;

    void showAbout() override;
  public slots:
    void actionAbout_triggered();

  private:
    QAction *actionAbout;
};
