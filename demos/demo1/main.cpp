/**
 * \file main.cpp
 * \brief Entry point of first demo
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License GPL 2 or 3
 */

#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}
