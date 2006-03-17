#ifndef __QMDI_CLIENT_H__
#define __QMDI_CLIENT_H__

/**
 * \file qmdiclient.h
 * \brief Definition of the qmdi client class
 * \author Diego Iastrubni (elcuco@kde.org)
 * License LGPL
 * \see qmdiClient
 */

#include "actiongrouplist.h"

class QString;
class qmdiServer;
class QObject;

class qmdiClient
{
public:
	qmdiClient();
	virtual ~qmdiClient();
	
	QString name;
	QString fileName;
	qmdiActionGroupList menus;
	qmdiActionGroupList toolbars;

	qmdiServer *mdiServer;
	QObject *myself;
};

#endif // __QMDI_CLIENT_H__
