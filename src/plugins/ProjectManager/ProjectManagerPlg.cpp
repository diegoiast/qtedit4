#include <QDockWidget>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QSettings>
#include <QSortFilterProxyModel>

#include "pluginmanager.h"
#include "qmdiserver.h"
#include "qmdihost.h"
#include "ProjectManagerPlg.h"
#include "ui_ProjectManagerGUI.h"
#include "GenericItems.h"

ProjectManagerPlugin::ProjectManagerPlugin()
{
	name = tr("Project manager");
	author = tr("Diego Iastrubni <diegoiast@gmail.com>");
	iVersion = 0;
	sVersion = "0.0.1";
	autoEnabled = true;
	alwaysEnabled = true;

	m_dockWidget = NULL;
    directoryModel = NULL;
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

	QWidget *w = new QWidget(m_dockWidget);
    gui =  new Ui::ProjectManagerGUI;
    gui->setupUi(w);
    connect(gui->addDirectory, &QAbstractButton::clicked, this, &ProjectManagerPlugin::on_addDirectory_clicked);
    connect(gui->removeDirectory, &QAbstractButton::clicked, this, &ProjectManagerPlugin::on_removeDirectory_clicked);
    connect(gui->filesView, &QAbstractItemView::clicked, this, &ProjectManagerPlugin::onItemClicked);
	m_dockWidget->setWidget(w);

    directoryModel = new DirectoryModel(this);
    directoryModel->addDirectory(QDir::currentPath());
    filesFilterModel = new FilterOutProxyModel(this);
    filesFilterModel->setSourceModel(directoryModel);
    filesFilterModel->sort(0);
    gui->filesView->setModel(filesFilterModel);
    connect(gui->filterFiles, &QLineEdit::textChanged, [this](const QString &newText){
        filesFilterModel->setFilterWildcard(newText);
    });
    connect(gui->filterOutFiles, &QLineEdit::textChanged, [this](const QString &newText) {
        filesFilterModel->setFilterOutWildcard(newText);
    });
}

void ProjectManagerPlugin::on_client_unmerged(qmdiHost *host)
{
    delete (m_dockWidget);
    m_dockWidget = NULL;
    Q_UNUSED(host);
}

void ProjectManagerPlugin::loadConfig(QSettings &settings)
{
    settings.beginGroup("ProjectManager");
    gui->filterOutFiles->setText(settings.value("FilterOut", "").toString());
    gui->filterFiles->setText(settings.value("FilterIn", "").toString());
    filesFilterModel->invalidate();
    settings.endGroup();
}

void ProjectManagerPlugin::saveConfig(QSettings &settings)
{
    settings.beginGroup("ProjectManager");
    settings.setValue("FilterOut", gui->filterOutFiles->text());
    settings.setValue("FilterIn", gui->filterFiles->text());
    settings.endGroup();
}

void ProjectManagerPlugin::onItemClicked(const QModelIndex &index)
{
    auto i = filesFilterModel->mapToSource(index);
    auto s = directoryModel->getItem(i.row());
    PluginManager *pluginManager = dynamic_cast<PluginManager*>(mdiServer->mdiHost);
    if (pluginManager)
        pluginManager->openFile(s);
}

void ProjectManagerPlugin::on_addDirectory_clicked(bool checked)
{
    QString s = QFileDialog::getExistingDirectory(gui->filesView,"Add directory");
	if (s.isEmpty())
		return;
    directoryModel->addDirectory(s);
}

void ProjectManagerPlugin::on_removeDirectory_clicked(bool checked)
{

}
