#pragma once

#include "qmdihost.h"
#include <QMainWindow>

class QWidget;

class qmdiMainWindow : public QMainWindow, public qmdiHost {
  public:
    qmdiMainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0) : QMainWindow(parent, flags) {
        // stub function
    }
};
