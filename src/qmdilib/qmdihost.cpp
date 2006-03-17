/**
 * \file qmdihost.cpp
 * \brief Implemetation of the qmdi host class
 * \author Diego Iastrubni (elcuco@kde.org)
 * License LGPL
 * \see qmdiHost
 */

#include "qmdihost.h"

qmdiHost::qmdiHost()
{
	toolBarList = NULL;
}

qmdiHost::~qmdiHost()
{
	delete toolBarList;
}
