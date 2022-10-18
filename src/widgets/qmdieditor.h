/**
 * \file qmdieditor
 * \brief Definition of 
 * \author Diego Iastrubni diegoiast@gmail.com
 * License GPL 2008
 * \see class name
 */

#pragma once

#include <qmdiclient.h>
#include <qsvte/qsvtextedit.h>

class QsvTextOperationsWidget;
/**
A source editor with MDI interface.
This class will be a very rich text editor which will also have a set of toolbars and menus available for the qmdiHost

	@author Diego Iastrubni <diegoiast@gmail.com>
*/
class qmdiEditor : public QsvTextEdit, public qmdiClient
{
	Q_OBJECT
	
public:
	qmdiEditor(QString fName, QWidget* p);
	~qmdiEditor();

	bool canCloseClient();
	QString mdiClientFileName();
	
private:
    QsvTextOperationsWidget *operationsWidget;
	QString getShortFileName();
	
	QMenu *bookmarksMenu;
	QMenu *textOperationsMenu;

	QAction *actionSave;
	QAction *actionUndo;
	QAction *actionRedo;
	QAction *actionCopy;
	QAction *actionCut;
	QAction *actionPaste;
	QAction *actiohAskHelp;
	QAction *actionFind;
	QAction *actionFindNext;
	QAction *actionFindPrev;
	QAction *actionReplace;
	QAction *actionGotoLine;
};
