#ifndef MAINWINDOW_4_H
#define MAINWINDOW_4_H

#include <QObject>
#include <QSyntaxHighlighter>
#include <QApplication>
#include <QAction>
#include <QToolBar>
#include <QTimer>
#include <QMainWindow>
#include <QFileDialog>

class MainWindow4 : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow4( const QString &file );
public slots:
	void loadFile( QString filename ="" );
private:
};

#endif // MAINWINDOW_4_H
