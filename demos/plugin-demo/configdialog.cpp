/**
 * \file configdialog.cpp
 * \brief Implementation of the configuration dialog class
 * \author Diego Iastrubni (diegoiast@gmail.com)
 *  License LGPL 2 or 3
 * \see ConfigDialog
 */

// $Id$

#include <QDialogButtonBox>
#include <QIcon>
#include <QLabel>
#include <QListView>
#include <QtGui>

#include "configdialog.h"
#include "iplugin.h"
#include "pluginmanager.h"
#include "pluginmodel.h"

/**
 * \class ConfigDialog
 * \brief The dialog used to configure the availale plugins in the PluginManager
 *
 * This dialog lets you control the available plugins. Listing them,
 * enabling/disabling and configuring each one of them.
 *
 * The dialog displayes the list on a QListView which represents the plugins
 * available in the PluginManager, using the model-view-controller design
 * pattern (PluginModel).
 *
 * \see PluginModel
 * \see PluginManager
 */

/**
 * \brief default constructor for the ConfigDialog
 * \param owner the owner of the dialog
 *
 *
 */
ConfigDialog::ConfigDialog(QWidget *owner) : QDialog(owner) {

    pluginManager = NULL;
    pluginModel = NULL;

    pluginListUi.setupUi(this);
}

ConfigDialog::~ConfigDialog() {
    pluginManager = NULL;
    delete pluginModel;
}

void ConfigDialog::setManager(PluginManager *manager) {
    QModelIndex i;
    pluginManager = manager;
    pluginModel = new PluginModel(pluginManager);
    pluginListUi.pluginList->setModel(pluginModel);

    i = pluginModel->index(0, 0, QModelIndex());
    pluginListUi.pluginList->setCurrentIndex(i);
    updateInfo(0);
}

void ConfigDialog::on_aboutPlugin_clicked(bool) {
    int pluginNumber = pluginListUi.pluginList->currentIndex().row();
    IPlugin *p = pluginManager->plugins[pluginNumber];

    hide();
    p->showAbout();
    show();
}

void ConfigDialog::on_configurePlugin_clicked(bool) {
    int pluginNumber = pluginListUi.pluginList->currentIndex().row();
    IPlugin *p = pluginManager->plugins[pluginNumber];
    QWidget *w = p->getConfigDialog();

    if (!w)
        return;

    hide();
    p->setData();
    if (execDialog(w))
        p->getData();
    show();
}

void ConfigDialog::on_pluginList_activated(const QModelIndex &index) { updateInfo(index.row()); }

void ConfigDialog::on_pluginList_clicked(const QModelIndex &index) { updateInfo(index.row()); }

void ConfigDialog::on_pluginEnabled_toggled(bool enabled) {
    int pluginNumber = pluginListUi.pluginList->currentIndex().row();
    IPlugin *p = pluginManager->plugins[pluginNumber];

    p->setEnabled(enabled);
    if (enabled)
        pluginManager->mergeClient(p);
    else
        pluginManager->unmergeClient(p);
}

void ConfigDialog::updateInfo(int pluginNumber) {
    IPlugin *p = pluginManager->plugins[pluginNumber];
    QWidget *w = p->getConfigDialog();

    pluginListUi.pluginName->setText(p->getName());
    pluginListUi.pluginName->setCursorPosition(0);

    pluginListUi.pluginAuthor->setText(p->getAuthor());
    pluginListUi.pluginAuthor->setCursorPosition(0);

    pluginListUi.pluginVersion->setText(p->getsVersion());
    pluginListUi.pluginVersion->setCursorPosition(0);

    pluginListUi.pluginEnabled->blockSignals(true);
    pluginListUi.pluginEnabled->setChecked(p->isEnabled());
    pluginListUi.pluginEnabled->setEnabled(p->canDisable());
    pluginListUi.pluginEnabled->blockSignals(false);

    //	pluginListUi.configurePluginButton->setEnabled( w != NULL );
}

bool ConfigDialog::execDialog(QWidget *w) {
    QDialog *d = new QDialog(this);
    QVBoxLayout *l = new QVBoxLayout(d);
    QDialogButtonBox *b = new QDialogButtonBox(d);
    bool status;

    b->addButton(QDialogButtonBox::Ok);
    b->addButton(QDialogButtonBox::Cancel);
    l->addWidget(w);
    l->addWidget(b);
    d->connect(b, SIGNAL(accepted()), d, SLOT(accept()));
    d->connect(b, SIGNAL(rejected()), d, SLOT(reject()));
    d->setLayout(l);
    d->setWindowTitle("Plugin Configuration");
    d->setSizeGripEnabled(true);
    status = d->exec();

    // don't destroy the plugin's widget
    l->removeWidget(w);
    w->setParent(NULL);

    // .. but do delete the temp dialog...
    delete d;

    return status;
}
