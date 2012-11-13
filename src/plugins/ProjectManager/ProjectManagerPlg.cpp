#include <QMessageBox>
#include <QMainWindow>
#include <QDockWidget>
#include <QFileDialog>

#include "pluginmanager.h"
#include "ProjectManagerPlg.h"
#include "qmdiserver.h"
#include "qmdihost.h"
#include "ui_ProjectManagerGUI.h"
#include "GenericItems.h"

ProjectManagerPlugin::ProjectManagerPlugin()
{
	name = tr("Project manager");
	author = tr("Diego Iastrubni <elcuco@kde.org>");
	iVersion = 0;
	sVersion = "0.0.1";
	autoEnabled = true;
	alwaysEnabled = true;

	m_dockWidget = NULL;
	m_projectModel = NULL;
}

void ProjectManagerPlugin::showAbout()
{
    QMessageBox::information( dynamic_cast<QMainWindow*>(mdiServer), "About", "The project manager plugin" );
}

void ProjectManagerPlugin::on_client_merged(qmdiHost *host)
{
	if (m_dockWidget)
		return;

	QMainWindow *window = dynamic_cast<QMainWindow*>(host);
	m_dockWidget = new QDockWidget(window);
	m_dockWidget->setObjectName("m_dockWidget");
	m_dockWidget->setWindowTitle( tr("Project") );

#if 0
	m_treeView = new QTreeView(m_dockWidget);
	m_treeView->setAlternatingRowColors(true);
	m_dockWidget->setWidget(m_treeView);
#else
	QWidget *w = new QWidget(m_dockWidget);
	m_gui =  new Ui::ProjectManagerGUI;
	m_gui->setupUi(w);
	m_dockWidget->setWidget(w);
#endif

	m_projectModel = new FoldersModel(m_gui->filesView);
	//m_projectModel->processDir("/home/elcuco/src/qtedit4/");
//	m_projectModel->processDir("/home/elcuco/src/qt-creator/");
	m_projectModel->processDir("/home/elcuco/src/googlecode/qtedit4/trunk/");
//	m_projectModel->processDir("c:\\Users\\elcuco\\Source\\qtedit4");
	m_gui->filesView->setModel(m_projectModel);
	window->addDockWidget( Qt::LeftDockWidgetArea, m_dockWidget );

	QCompleter *completer = new GenericItemCompleter();
	completer->setModel(m_projectModel);
	completer->setParent(m_gui->filenameLineEdit);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
	completer->setCompletionRole(0);
	completer->setCompletionPrefix("/");
	m_gui->filenameLineEdit->setCompleter(completer);

	connect(m_gui->filesView,SIGNAL(clicked(QModelIndex)),this,SLOT(onItemClicked(QModelIndex)));
	connect(m_gui->addDirectoryButton,SIGNAL(clicked()),this,SLOT(onAddDirectoryClicked()));
}

void ProjectManagerPlugin::on_client_unmerged(qmdiHost *host)
{
	delete(m_dockWidget);
	m_dockWidget = NULL;
	Q_UNUSED( host );
}

void ProjectManagerPlugin::onItemClicked(const QModelIndex &index)
{
	FileItem* item = static_cast<FileItem*>(index.internalPointer());
	if (item->isDirectory)
		return;
	PluginManager *pluginManager = dynamic_cast<PluginManager*>(mdiServer->mdiHost);
	if (pluginManager)
		pluginManager->openFile(item->fullPath);
}

void ProjectManagerPlugin::onAddDirectoryClicked()
{
	QString s = QFileDialog::getExistingDirectory(m_gui->filesView,"Add directory");
	if (s.isEmpty())
		return;
	m_projectModel->processDir(s);
}
