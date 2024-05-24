#include <QDockWidget>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QSettings>
#include <QSortFilterProxyModel>

#include "GenericItems.h"
#include "ProjectManagerPlg.h"
#include "ProjectSearch.h"
#include "pluginmanager.h"
#include "qmdihost.h"
#include "qmdiserver.h"
#include "ui_ProjectManagerGUI.h"

ProjectManagerPlugin::ProjectManagerPlugin() {
    name = tr("Project manager");
    author = tr("Diego Iastrubni <diegoiast@gmail.com>");
    iVersion = 0;
    sVersion = "0.0.1";
    autoEnabled = true;
    alwaysEnabled = true;

    directoryModel = NULL;
}

void ProjectManagerPlugin::showAbout() {
    QMessageBox::information(dynamic_cast<QMainWindow *>(mdiServer), "About",
                             "The project manager plugin");
}

void ProjectManagerPlugin::on_client_merged(qmdiHost *host) {
    auto manager = dynamic_cast<PluginManager *>(host);
    auto *w = new QWidget;
    gui = new Ui::ProjectManagerGUI;
    gui->setupUi(w);
    connect(gui->addDirectory, &QAbstractButton::clicked, this,
            &ProjectManagerPlugin::on_addDirectory_clicked);
    connect(gui->removeDirectory, &QAbstractButton::clicked, this,
            &ProjectManagerPlugin::on_removeDirectory_clicked);
    connect(gui->filesView, &QAbstractItemView::clicked, this,
            &ProjectManagerPlugin::onItemClicked);
    manager->createNewPanel(Panels::West, QString("Project"), w);

    directoryModel = new DirectoryModel(this);
    directoryModel->addDirectory(QDir::currentPath());
    filesFilterModel = new FilterOutProxyModel(this);
    filesFilterModel->setSourceModel(directoryModel);
    filesFilterModel->sort(0);
    gui->filesView->setModel(filesFilterModel);
    connect(gui->filterFiles, &QLineEdit::textChanged,
            [this](const QString &newText) { filesFilterModel->setFilterWildcards(newText); });
    connect(gui->filterOutFiles, &QLineEdit::textChanged,
            [this](const QString &newText) { filesFilterModel->setFilterOutWildcard(newText); });

    auto *w2 = new ProjectSearch(manager, directoryModel);
    auto seachID = manager->createNewPanel(Panels::West, "Search", w2);

    auto projectSearch = new QAction(tr("Search in project"));
    projectSearch->setShortcut(QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_F));
    connect(projectSearch, &QAction::triggered,
            [seachID, manager]() { manager->showPanel(Panels::West, seachID); });
    manager->addAction(projectSearch);
}

void ProjectManagerPlugin::on_client_unmerged(qmdiHost *host) { Q_UNUSED(host); }

void ProjectManagerPlugin::loadConfig(QSettings &settings) {
    settings.beginGroup("ProjectManager");
    gui->filterOutFiles->setText(settings.value("FilterOut", "").toString());
    gui->filterFiles->setText(settings.value("FilterIn", "").toString());
    filesFilterModel->invalidate();
    settings.endGroup();
}

void ProjectManagerPlugin::saveConfig(QSettings &settings) {
    settings.beginGroup("ProjectManager");
    settings.setValue("FilterOut", gui->filterOutFiles->text());
    settings.setValue("FilterIn", gui->filterFiles->text());
    settings.endGroup();
}

void ProjectManagerPlugin::onItemClicked(const QModelIndex &index) {
    auto i = filesFilterModel->mapToSource(index);
    auto s = directoryModel->getItem(i.row());
    PluginManager *pluginManager = dynamic_cast<PluginManager *>(mdiServer->mdiHost);
    if (pluginManager) {
        pluginManager->openFile(s);
    }
}

void ProjectManagerPlugin::on_addDirectory_clicked(bool checked) {
    QString s = QFileDialog::getExistingDirectory(gui->filesView, "Add directory");
    if (s.isEmpty()) {
        return;
    }
    directoryModel->addDirectory(s);
}

void ProjectManagerPlugin::on_removeDirectory_clicked(bool checked) {}
