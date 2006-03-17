/**
 * \file qmdiserver.cpp
 * \brief Implementation of the qmdi server class
 * \author Diego Iastrubni (elcuco@kde.org)
 * License LGPL
 * \see qmdiServer
 */
 
#include "qmdiserver.h"

/**
 * \class qmdiServer
 * \brief a default niterface for mdi servers
 *  
 * This class is used only to get messages from the qmdiClient
 * that it asks to be removed from the list.
 * 
 * Classes which derive this class, MUST implement the clientDeleted
 * function.
 */

/**
 * Empty destructor. Destroyes the object.
 * 
 * Since this class needs to be dynamic_casted by the derived classes,
 * to assign the correct qmdiSever, this class needs to have a vitrual table.
 * If the class has a virtual function, it's only smart to have a virtual destructor.
 * 
 * \see qmdiClient
 * \see qmdiTabWidget
 */
qmdiServer::~qmdiServer()
{
}

/**
 * \brief callback to get alarm of deleted object
 *
 * This function gets called on the destructor of qmdiClient,
 * to announce that the object is about to be deleted. This
 * function should be used to remove the menus and toolbars
 * and other cleanup actions needed.
 * 
 * Why not using QTabWidget::tabRemoved ( int index ) ?
 *  - Because that function is been called after the qmdiClient 
 *    has been deleted. 
 *    
 * Why not using the signal QObject::destroyed( QObject * obj = 0 ) ?
 *  - Because that signal is non blocking, and you will get yourself in
 *    race conditions: this function might be called after the object itself
 *    has been delete.
 *    
 * This means that the qmdiClient needs to know the mdi server (usually a qmdiTabWidget)
 * and ask to be removed before it gets destructed.
 * 
 * Why this function is not pure virtual?
 *  - Since I found that it gives you warnings, about calling a pure virtual
 *    function, lame excuse, which I would like to get rid of :)
 */
void qmdiServer::clientDeleted( QObject *o )
{
	// stub function
	// If not added, the function had to be pure virtual

	o = NULL;
}
