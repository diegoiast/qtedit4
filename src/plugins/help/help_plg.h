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
