/**
 * \file help_plg.h
 * \brief Definition of the help system  plugin
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL
 * \see HelpPlugin
 */

#include "iplugin.h"

class QAction;

class HelpPlugin: public IPlugin
{
	Q_OBJECT
public:
	HelpPlugin();
	~HelpPlugin();

	void	showAbout();
// 	void	on_client_merged( qmdiHost* host );
// 	void	on_client_unmerged( qmdiHost* host );
public slots:
	void	on_actionAbout_triggered();
private:
	QAction* actionAbout;
};

// kate: space-indent off; tab-indent on; tab-width 8; indent-width 7; mixedindent off; indent-mode cstyle; 
// kate: syntax: c++; auto-brackets on; auto-insert-doxygen: on; end-of-line: unix
// kate: show-tabs on;
