/**
 * \file systembrowser_plg.h
 * \brief Definition of the system browser plugin
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL
 * \see FSBrowserPlugin
 */

#include "iplugin.h"

class QDockWidget;
class FileSystemBrowser;

class FSBrowserPlugin: public IPlugin
{
	Q_OBJECT
public:
	FSBrowserPlugin();
	~FSBrowserPlugin();

	void	showAbout();
	void	on_client_merged( qmdiHost* host );
	void	on_client_unmerged( qmdiHost* host );
private:
	QDockWidget	*m_dockWidget;
	FileSystemBrowser *m_fsBrowser;
};

// kate: space-indent off; tab-indent on; tab-width 8; indent-width 7; mixedindent off; indent-mode cstyle; 
// kate: syntax: c++; auto-brackets on; auto-insert-doxygen: on; end-of-line: unix
// kate: show-tabs on;
