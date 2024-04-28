#pragma once

/**
 * \file iplugin.h
 * \brief Declaration of the IPlugin interface
 * \author Diego Iastrubni (diegoiast@gmail.com)
 *  License LGPL 2 or 3
 * \see IPlugin
 */

#include "qmdiclient.h"
#include <QObject>

class QString;
class QWidget;
class QSettings;
class QAction;
class QActionGroup;
class QIcon;

QAction *new_action(QIcon icon, QString name, QObject *obj, QString shortcut, QString status_tip,
                    const char *slot);

class IPlugin : public QObject, public qmdiClient {
    friend class PluginManager;

  public:
    IPlugin();
    virtual ~IPlugin();

    virtual void showAbout();
    virtual QWidget *getConfigDialog();
    virtual void getData();
    virtual void setData();

    virtual void loadConfig(QSettings &settings);
    virtual void saveConfig(QSettings &settings);

    virtual QActionGroup *newFileActions();
    virtual QStringList myExtensions();
    virtual int canOpenFile(const QString fileName);
    virtual bool openFile(const QString fileName, int x = -1, int y = -1, int z = -1);

    bool isEnabled();
    void setEnabled(bool enable);
    bool canDisable();
    QIcon getIcon();
    QString getName();
    QString getAuthor();
    QString getsVersion();
    int getiVersion();

  protected:
    QString name;
    QString author;
    QString sVersion;
    int iVersion;

    bool alwaysEnabled;
    bool autoEnabled;
    bool enabled;
};
