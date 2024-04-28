/**
 * \file qmdiserver.cpp
 * \brief Implementation of the qmdi server class
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL 2 or 3
 * \see qmdiServer
 */

// $Id$

#include <QMenu>
#include <QPoint>

#include "qmdiclient.h"
#include "qmdiserver.h"

/**
 * \class qmdiServer
 * \brief A default interface for MDI servers
 *
 * This class is used only to get messages from the qmdiClient
 * that it asks to be removed from the list.
 *
 * Classes which derive this class, MUST implement the clientDeleted()
 * function. This a rather abstract class, you are probably looking for
 * qmdiTabWidget.
 *
 * Other classes which implement this interface are planned for next versions.
 */

/**
 * \var qmdiServer::mdiHost
 * \brief a pointer to the MDI host
 *
 * This is a pointer to the MDI host in which the server
 * is registered.
 *
 * \copydoc read_only_property
 * \todo make it protected maybe?
 */

/**
 * \brief default constructor, creates the object
 *
 * Constructs the mdi Server. By default does nothing.
 */
qmdiServer::qmdiServer() { mdiHost = nullptr; }

/**
 * \brief default destructor, destroys the object
 *
 * Since this class needs to be dynamic_casted by the derived classes,
 * to assign the correct qmdiServer, this class needs to have a virtual table.
 * If the class has a virtual function, it's only smart to have a virtual
 * destructor.
 *
 * \see qmdiClient
 * \see qmdiTabWidget
 */
qmdiServer::~qmdiServer() {}

/**
 * \fn qmdiServer::addClient( qmdiClient *client  )
 * \brief insert a new client to the server
 * \param client the new client to be added
 *
 * When deriving this class, you should also supply a function
 * for adding new clients to it. For example when deriving QTabWidget,
 * your new function should actually call QTabWidget::insertTab().
 *
 * Users of this class should be able to add new clients, and
 * set the properties needed using this function, but you should
 * also supply more advanced functions provided by your new widget, like
 * add the widget into another location, setting an icon for this client,
 * etc.
 *
 * Since it's defined as pure virtual, you must implement this on
 * derived classes.
 *
 * \see qmdiTabWidget
 */
// pure virtual method
// void qmdiServer::addClient( qmdiClient *client );

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
 *  - Because not all mdi servers are implemented on top of QTabWidget
 *
 * Why not using the signal QObject::destroyed( QObject * obj = 0 ) ?
 *  - Because that signal is non blocking, and you will get yourself in
 *    race conditions: this function might be called after the object itself
 *    has been deleted.
 *
 * This means that the qmdiClient needs to know the MDI server (usually a
 * qmdiTabWidget) and ask to be removed before it gets destructed.
 *
 * Why this function is not pure virtual?
 *  - Since I found that it gives you warnings, about calling a pure virtual
 *    function, lame excuse, which I would like to get rid of :)
 *  - For some reason when an mdi client wants to contact it's qmdiServer,
 *    it reaches this nullptr function insted of the overriden one (like
 *    qmdiTabWidget::deleteClient) this makes the application die, since it's
 *    calling a pure virtual function. Definetly a bug, but this a nice
 *    workaround.
 *  - On some rare implementations the MDI server implemented, would like
 *    to ignore those events. I prefer that the dummy functions be implemented
 *    by the library, and not the end clients.
 *
 * \note Even tough this is not a  pure virtual, you must implement this on
 * derived classes.
 */
void qmdiServer::deleteClient(qmdiClient *) {
    // stub function
    // If not added, the function had to be pure virtual
}

/**
 * \fn qmdiServer::getClientsCount()
 * \brief return the number of sub clients in this server
 * \return a number greater or equal to 0
 *
 * Return the number of sub-widgets in this server. Please note that
 * this function can return also non-mdi clients.
 *
 * Since it's defined as pure virtual, you must implement this on
 * derived classes.
 *
 * \see getClient
 */
// pure virtual method
// int		qmdiServer::getClientsCount() = 0;

/**
 * \fn  qmdiClient *qmdiServer::getClient( int i )
 * \brief return a pointer to an MDI client
 * \param i the number of sub widget to return
 * \return and qmdiClinet pointer or nullptr
 *
 * Return a pointer to an MDI client. If the number passed
 * denotes a sub widget which does not derive qmdiClient
 * this function will return nullptr.
 *
 * Since it's defined as pure virtual, you must implement this on
 * derived classes.
 *
 * \since 0.0.4
 * \see getClientsCount
 */
// pure virtual method
// qmdiClient	*qmdiServer::getClient( int i ) = 0;

/**
 * \brief request an MDI client to be closed
 * \param i the number of the client (tab) to be closed
 *
 * Call this slot to ask the MDI client to close itself.
 * The MDI client may show a dialog to ask for saving. It's not
 * guaranteed that the action will be handled as the MDI client
 * can abort the action.
 *
 * This function used to be part of qmdiTabWidget, but it was ported
 * down to this abstract interface at version 0.0.4.
 *
 * \since 0.0.4
 * \see qmdiClient::closeClient()
 */
void qmdiServer::tryCloseClient(int i) {
    qmdiClient *client = getClient(i);

    if (!client)
        return;

    client->closeClient();
}

/**
 * \brief request to close all other clients
 * \param i the number of the client to keep open
 *
 * Call this slot to ask all the MDI clients (but the widget found at
 * index \b i in the tab widget, passed as a parameter).
 * Each MDI client may show a dialog to ask for saving. It's not
 * guaranteed that the action will be handled as the MDI client
 * can abort the action. At the end, only the client number i will
 * not be asked to close itself.
 *
 * If some widget on the MDI server does not derive (implements)
 * the qmdiClient interface, the widget will not be closed.
 *
 * This function used to be part of qmdiTabWidget, but it was ported
 * down to this abstract interface at version 0.0.4.
 *
 * \since 0.0.4
 * \see qmdiClient::closeClient()
 * \see tryCloseClient()
 * \see tryCloseAllClients
 */
void qmdiServer::tryCloseAllButClient(int i) {
    int n = getClientsCount();
    qmdiClient *client = getClient(i);

    for (int j = 0; j < n; j++) {
        qmdiClient *c = getClient(j);

        // item is not an mdi client
        if (!c)
            continue;

        if (c == client)
            continue;

        c->closeClient();
    }
}

/**
 * \brief try to close all MDI clients
 *
 * Call this slot when you want to close all the MDI clients.
 *
 * This function used to be part of qmdiTabWidget, but it was ported
 * down to this abstract interface at version 0.0.4.
 *
 * \since 0.0.4
 */
void qmdiServer::tryCloseAllClients() {
    int c = getClientsCount();

    for (int i = 0; i < c; i++) {
        qmdiClient *client = getClient(i);
        if (!client)
            continue;

        client->closeClient();
    }
}

/**
 * \brief display the menu of a specific MDI client
 * \param i the mouse button that has been pressed
 * \param p the location of the mouse click
 *
 * This function shuold be called when a user presses the right mouse
 * button on the tab bar of the tab widget. The coordinates of the
 * click are passed on the parameter \b p , while the
 * mouse button which has been pressed is passed on the
 * parameter \b p .
 *
 * This function used to be part of qmdiTabWidget, but it was ported
 * down to this abstract interface at version 0.0.4.
 *
 * \since 0.0.4
 * \see qmdiTabBar
 */
void qmdiServer::showClientMenu(int i, QPoint p) {
    QMenu *menu = new QMenu;
    QAction *closeThis = new QAction(menu->tr("Close this window"), menu);
    QAction *closeOthers = new QAction(menu->tr("Close other windows"), menu);
    QAction *closeAll = new QAction(menu->tr("Close all windows"), menu);

    menu->setTitle(menu->tr("Local actions"));
    menu->addAction(closeThis);
    menu->addAction(closeOthers);
    menu->addAction(closeAll);

    // ugly code, but I don't know a better way of doying this
    QWidget *w = dynamic_cast<QWidget *>(this);
    if (w)
        p = w->mapToGlobal(p);
    QAction *q = menu->exec(p);

    if (q == closeThis) {
        tryCloseClient(i);
    } else if (q == closeOthers) {
        tryCloseAllButClient(i);
    } else if (q == closeAll) {
        tryCloseAllClients();
    }
}

/**
 * \class qmdiMainWindow
 * \brief A convience class that creates a main windows as the mdiServer
 *
 * This class is an dmi server inside a main window.
 *
 * \ref qmdiServer
 */

/**
 * \fn qmdiMainWindow::qmdiMainWindow( QWidget * parent = 0, Qt::WindowFlags
 * flags = 0 ): \brief a generic constructor \param parent the parent of the
 * window \param flags the window flags of the window
 *
 *
 * This implements the normal constructor of QMainWindow, to ease
 * the transition from/to a qmdiMainWindow.
 *
 * This method is added for compatibilty with QTabWidget. Trolltech calls
 * this static overloading.
 */
