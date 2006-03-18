#include <QApplication>
#include <QMessageBox>
#include <QDir>

// #include "qehighlightmanager.h"
#include "mainwindow.h"

/**
 * \file    main.cpp
 * \brief   Entry point of the application
 * \author  Diego Iastrubni (elcuco@kde.org)
 * \date    11-11-2005
 */

void test_order_map();

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setOrganizationName("Trolltech");
	app.setApplicationName("QtEdit");

	MainWindow window;
	window.show();
	return app.exec();
}


/**
 * \mainpage QtEdit4 - The 4rth version of my powerfull editor
 * 
 * \section Introduction
 * The goal is to make the best IDE ever, using Trolltech's Qt
 * library. Main goals are:
 * - Portable to all the platforms supported by Qt
 * - The IDE shuold have native look and feel on every platform
 * - Be able to use free tools, but also commercial ones
 * - Flexyble and easy to use
 * - Fast loader
 * - Integrate all the tools needed to making a project: (compiler, help, VCS etc)
 */

