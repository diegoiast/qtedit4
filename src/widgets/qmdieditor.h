/**
 * \file qmdieditor
 * \brief Definition of 
 * \author Diego Iastrubni elcuco@kde.org
 * License GPL 2008
 * \see class name
 */

#ifndef QMDIEDITOR_H
#define QMDIEDITOR_H

#include <qmdiclient.h>
#include <qsvtextedit.h>

/**
A source editor with MDI interface.
This class will be a very rich text editor which will also have a set of toolbars and menus available for the qmdiHost

	@author Diego Iastrubni <elcuco@kde.org>
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
};

#endif

// kate: space-indent off; tab-indent on; tab-width 8; indent-width 7; mixedindent off; indent-mode cstyle; 
// kate: syntax: c++; auto-brackets on; auto-insert-doxygen: on; end-of-line: unix
// kate: show-tabs on;
