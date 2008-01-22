/**
 * \file filesystembrowser.h
 * \brief Definition of the system browser widget
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL
 * \see FileSystemBrowser
 */

#ifndef FILESYSTEMBROWSER_H
#define FILESYSTEMBROWSER_H

#include <QString>
#include <QWidget>
#include <QStack>

#include "ui_filesystembrowser.h"

class QDirModel;
class QCompleter;

class FileSystemBrowser : public QWidget, public Ui::FileSystemBrowser
{
	Q_OBJECT
public:
	FileSystemBrowser( QWidget * parent = 0, Qt::WFlags f = 0 );

private slots:
	void on_upButton_clicked();
	void on_backButton_clicked();
	void on_homeButton_clicked();
	void on_reloadButton_clicked();
	void on_forwardButton_clicked();
	void on_treeView_clicked(QModelIndex index);

	void setRootPath(const QString& path = QString(), bool remember = true);
	void setRootIndex(const QModelIndex& index, bool remember = true);
	void init();

private:
	void	updateActions();
	
	QDirModel*	m_dirModel;
	QString		m_currentPath;
	QStack<QString>	m_history;
	QStack<QString>	m_future;
	QCompleter*	m_completer;
};

#endif

// kate: space-indent off; tab-indent on; tab-width 8; indent-width 8; mixedindent off; indent-mode cstyle;
// kate: syntax: c++; auto-brackets on; auto-insert-doxygen: on; end-of-line: unix
// kate: show-tabs on;
