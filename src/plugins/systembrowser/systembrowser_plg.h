/**
 * \file systembrowser_plg.h
 * \brief Definition of the system browser plugin
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL
 * \see FSBrowserPlugin
 */

#include "iplugin.h"

class QDockWidget;
class QModelIndex;
class FileSystemBrowser;

class FSBrowserPlugin : public IPlugin {
    Q_OBJECT
  public:
    FSBrowserPlugin();
    ~FSBrowserPlugin();

    void showAbout();
    void on_client_merged(qmdiHost *host);
    void on_client_unmerged(qmdiHost *host);

  public slots:
    void on_fileClick(const QModelIndex &index);

  private:
    FileSystemBrowser *m_fsBrowser;
};

