#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include "iplugin.h"
#include <QList>
#include <QAbstractItemModel>
#include <QCompleter>

class QDockWidget;
class QTreeView;
class QCompleter;
class FoldersModel;

namespace Ui{
	class ProjectManagerGUI;
}

class ProjectManagerPlugin : public IPlugin
{
	Q_OBJECT
public:
	ProjectManagerPlugin();

	virtual void	showAbout();
	virtual void	on_client_merged( qmdiHost* host );
	virtual void	on_client_unmerged( qmdiHost* host );
public slots:
	// our code
	void onItemClicked(const QModelIndex &index);
	void onAddDirectoryClicked();

private:
	QCompleter *m_completer;
	QDockWidget *m_dockWidget;
	FoldersModel *m_projectModel;
	Ui::ProjectManagerGUI *m_gui;
};

#endif // PROJECTMANAGER_H
