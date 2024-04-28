#ifndef MAINWINDOW2_H
#define MAINWINDOW2_H

#include <QMainWindow>
#include <QStringList>
#include "ui_mainwindow2.h"

class QsvColorDefFactory;
class QsvLangDef;
class QsvSyntaxHighlighter;

class MainWindow2: public QMainWindow, private Ui::MainWindow2
{
	Q_OBJECT
public:	
	MainWindow2( QMainWindow *parent=nullptr ); 
	
public slots:
	void fillComboBoxes();
	
	void on_action_New_triggered();
	void on_action_Open_triggered();
	void on_action_About_triggered();
	void on_actionAbout_Qt_triggered();
	void on_actionE_xit_triggered();
    void on_comboBox_colors_currentIndexChanged(int index );
    void on_comboBox_syntax_currentIndexChanged(int index );
	void update_syntax_color();
	
private:
	QsvColorDefFactory	*defColors;
	QsvLangDef		*defLang;
	QsvSyntaxHighlighter	*highlight;
	QStringList		colorFiles, syntaxFiles;
	QString			dataPath;
	bool disable_combo_updates;
};

#endif // MAINWINDOW2_H
