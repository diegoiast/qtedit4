/**
 * \file qmdiclient.cpp
 * \brief Implementation of the qmdi client class
 * \author Diego Iastrubni (elcuco@kde.org)
 * License LGPL
 * \see qmdiClient
 */

#include <QObject>
#include "qmdiclient.h"
#include "qmdiserver.h"


qmdiClient::qmdiClient()
{
	mdiServer = NULL;
}

qmdiClient::~qmdiClient()
{
	if ((mdiServer != NULL) && (myself != NULL))
	{
		mdiServer->clientDeleted( myself );

		
//		this code is way cooler, but does not work.
//		the dynamic_cast always returns NULL
//		this is the reason for duplicating "this" with "myself"
//		qmdiClient *i = this;
//		mdiServer->clientDeleted( dynamic_cast<QObject*>(i) );
	}

	mdiServer = NULL;
	myself    = NULL;
}
