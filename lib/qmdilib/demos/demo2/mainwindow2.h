#ifndef __MAIN_WIN_H__
#define __MAIN_WIN_H__

/**
 * \file mainwindow2.h
 * \brief Implementation of the main window class of the 2nd demo
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License GPL 2 or 3
 * \see MainWindow
 */

// $Id$

#include "qmdiclient.h"
#include "qmdihost.h"
#include <QMainWindow>

// do you want the actions to be put in a single toolbar
// or several toolbars...?
#define SINGLE_TOOLBAR !false

class QTabWidget;
class QToolButton;

class MainWindow2 : public QMainWindow, public qmdiHost {
    Q_OBJECT
  public:
    MainWindow2(QWidget *owner = NULL);

  public slots:
    void init_actions();
    void init_gui();
    void about();
    void fileNew();
    void fileClose();
    void helpQtTopics();

  private:
    QAction *actionQuit;
    QAction *actionFileNew;
    QAction *actionQtTopics;
    QAction *actionAbout;
    QTabWidget *tabWidget;

    QToolButton *tabCloseBtn;
    QToolButton *tabNewBtn;
};

#endif // __MAIN_WIN_H__
