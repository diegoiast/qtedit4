#pragma once

/**
 * \file helpbrowse.h
 * \brief Definition of the extended help browser class
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL 2 or 3
 * \see MainWindow
 */

#include "qmdiclient.h"
#include <QTextBrowser>

class QAction;
class QWidget;
class QComboBox;

class QexHelpBrowser : public QTextBrowser, public qmdiClient {
    Q_OBJECT
  public:
    QexHelpBrowser(QUrl home, bool singleToolbar = false, QWidget *parent = 0);
    void initInterface(bool singleToolbar = false);
    QString mdiClientFileName();

  public slots:
    void goHome();
    void on_documentCombo_currentIndexChanged(int index);

  private:
    QAction *actionBack;
    QAction *actionNext;
    QAction *actionHome;
    QAction *actionZoomIn;
    QAction *actionZoomOut;

    QAction *actionCopy;
    QAction *actionFind;

    QComboBox *documentCombo;
    QUrl homePage;
};
