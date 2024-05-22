/**
 * \file qmditabwidget.cpp
 * \brief Implementation of the qmdi tab widget
 * \author Diego Iastrubni (diegoiast@gmail.com)
 * License LGPL 2 or 3
 * \see qmdiServer, QTabWidget
 */

#include <QAction>
#include <QEvent>
#include <QMainWindow>
#include <QMenu>
#include <QMouseEvent>

#include "qmdiclient.h"
#include "qmdihost.h"
#include "qmditabwidget.h"

/**
 * \class qmdiTabWidget
 * \brief An advanced tab widget, which is capable of changing menus and
 * toolbars on the fly
 *
 * This is a derived class from QTabWidget which is capable of modifying the
 * window menus and toolbars according to the widgets selected. This new tab
 * widget, will connect the lower level object (the qmdiClient, the widgets
 * inserted into the tabs) to the higher level object (the qmdiHost, usually the
 * main window). Since qmdiTabWidget inherits QTabWidget you can use it as a
 * normal QTabWidget, and all the interfaces available on the base class are
 * available for this new class.
 *
 * This class has also another improvement over QTabWidget: when the number of
 * tabs is less then 2 (one tab is open, or none) the tab bar will not be
 * displayed. (This is new in version 0.0.5)
 *
 * The relations are:
 *  - qmdiHost   : main window
 *  - qmdiClient : your new widgets
 *  - qmdiServer : this class
 *
 * When a new widget is selected on the qmdiServer (the user changes), the old
 * widget is removed from the qmdiHost, and only then the new MDI client is
 * added to the qmdiHost.
 *
 * To use this class properly, insert it into a QMainWindow which also derives
 * qmdiHost, and insert into it QWidgets which also derive qmdiClient.
 */

/**
 * \var qmdiTabWidget::activeWidget
 * \brief a pointer to the currently active widget
 *
 * An internally used pointer to the currently active widget. The widget in the
 * active tab.
 *
 * \internals
 */

/**
 * \brief default constructor
 * \param parent the parent widget and the qmdiHost
 * \param host the default MDI host to modify
 *
 * This is the default constructor for qmdiTabWidget.
 * If no host is passed, the parent widget will be queried for the qmdiHost
 * interface. This means that the easiest way to work with this
 * class is to insert it into a qmdiHost derived QMainWindow.
 *
 * This constructor also connects the tabChanged(int) slot to the
 * currentChanged(int) signal.
 *
 * \see QWidget::parentWidget()
 */
qmdiTabWidget::qmdiTabWidget(QWidget *parent, qmdiHost *host) : QTabWidget(parent) {
    if (host == nullptr) {
        mdiHost = dynamic_cast<qmdiHost *>(parent);
    } else {
        mdiHost = host;
    }

    activeWidget = nullptr;
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
    tabBar()->installEventFilter(this);
}

qmdiTabWidget::~qmdiTabWidget() {}

/**
 * \brief callback function for modifying the menu structure
 * \param i the number of the new widget
 *
 * When the user changes the active tab this this slot gets called.
 * It removes the menus and toolbars of the old MDI client and
 * installs the ones of the new client on screen.
 *
 * Since version 0.0.3 this widget also supports adding QWorkspace.
 * When the widget in focus is a QWorkspace, it's children will be
 * treated like normal qmdiClient if they provide that interface,
 * and when you select a new window in the QWorkspace that window's
 * menus and toolbars will be merged into the main application.
 *
 * \see QTabWidget::currentChanged()
 * \see workSpaceWindowActivated(QWidget*)
 */
void qmdiTabWidget::tabChanged(int i) {
    if (mdiHost == nullptr) {
        return;
    }

    QWidget *w = widget(i);

    // nothing to do, if the same tab has been selected twice
    if (w == activeWidget) {
        return;
    }

    if (activeWidget) {
        mdiHost->unmergeClient(dynamic_cast<qmdiClient *>(activeWidget));
    }

    activeWidget = w;

    if (activeWidget) {
        mdiHost->mergeClient(dynamic_cast<qmdiClient *>(activeWidget));
    }

    QMainWindow *m = dynamic_cast<QMainWindow *>(mdiHost);
    mdiHost->updateGUI(m);
}

/**
 * \brief mouse middle button click callback
 * \param i number of client pressed
 *
 * This function is connected to the mouse middle click even
 * on the tabbar. It will try to close the client.
 *
 * \see qmdiServer::tryCloseClient
 */
void qmdiTabWidget::on_middleMouse_pressed(int i, QPoint) { tryCloseClient(i); }

/**
 * \brief mouse right button click callback
 * \param i number of client pressed
 * \param p coordinate of the click event
 *
 * This function is connected to the mouse right click even
 * on the tabbar. This function will display a popup menu.
 *
 * \see qmdiServer::showClientMenu
 */
void qmdiTabWidget::on_rightMouse_pressed(int i, QPoint p) { showClientMenu(i, p); }

/**
 * \brief add a new MDI client to this tab widget
 * \param client the new client to be added
 *
 * This function is demanded by qmdiServer, and is implemented
 * as a simple call to:
 *
 * \code
 * i = QTabWidget::addTab( client, client->name );
 * QTabWidget::setCurrentIndex( i );
 * client->setFocus();
 * \endcode
 *
 * The client must derive also QWidget, since only widgets can
 * be inserted into QTabWidget. If the client does not derive
 * QWidget the function returns without doing anything.
 */
void qmdiTabWidget::addClient(qmdiClient *client) {
    QWidget *w = dynamic_cast<QWidget *>(client);

    if (w == nullptr) {
        qDebug("%s %s %d: warning trying to add a qmdiClient which does not derive "
               "QWidget",
               __FILE__, __FUNCTION__, __LINE__);
        return;
    }

    int i = addTab(w, client->mdiClientName);
    w->setFocus();
    setTabToolTip(i, client->mdiClientFileName());
    setCurrentIndex(i);
}

/**
 * \brief event filter for the tabbar
 * \param obj the object which created the event
 * \param event the event to be processed
 *
 * This function is used to catch when the user is clicking a tab.
 * On earlier version, a new class has been used. Since version 0.0.4
 * a proper event filter is used, which reduces the amount of code
 * and class count in the library.
 *
 * The function will call the functions:
 *  - on_middleMouse_pressed
 *  - on_rightMouse_pressed
 *
 * Future implementations might also re-order the tabs.
 *
 * For more information read the documentation of QObject::installEventFilter.
 *
 * \since 0.0.4
 */
bool qmdiTabWidget::eventFilter(QObject *obj, QEvent *event) {
    if (obj != tabBar()) {
        return QObject::eventFilter(obj, event);
    }

    if (event->type() != QEvent::MouseButtonPress) {
        return QObject::eventFilter(obj, event);
    }

    // compute the tab number
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    QPoint position = mouseEvent->pos();
    int clickedItem = tabBar()->tabAt(position);

    // just in case
    if (clickedItem == -1) {
        return QObject::eventFilter(obj, event);
    }

    switch (mouseEvent->button()) {
    case Qt::LeftButton:
        return QObject::eventFilter(obj, event);
        break;

    case Qt::RightButton:
        on_rightMouse_pressed(clickedItem, position);
        break;

    case Qt::MiddleButton:
        on_middleMouse_pressed(clickedItem, position);
        break;

    default:
        return QObject::eventFilter(obj, event);
    }

    return true;
}

/**
 * \brief return a specific MDI client
 * \param i the number of sub widget to return
 * \return and qmdiClinet pointer or nullptr
 *
 * This method returns the MDI client found in tab number \b i , or \b nullptr
 * if that widget does not implement the qmdiClient interface.
 */
qmdiClient *qmdiTabWidget::getClient(int i) { return dynamic_cast<qmdiClient *>(widget(i)); }

/**
 * \brief return the number of sub clients in this server
 * \return a number greater or equal to 0
 *
 * Return the number of sub-widgets in this server. Please note that
 * this function can return also non-mdi clients.
 *
 * This function returns the value of QTabWidget::count()
 */
int qmdiTabWidget::getClientsCount() { return count(); }

/**
 * \brief callback to get alarm of deleted object
 * \param client the client to delete
 *
 * As requested by qmdiServer this function implements the needed
 * interface. When an object is deleted, eighter by QTabWidget::removeTab(int),
 * or by deleting the object, this function will be called.
 *
 * This function removes the menus and toolbars of the widget (if it is the
 * active widget) and sets the active widget to nullptr. When a new tab will be
 * selected, which will happen if there is another widget on the tab widget, the
 * new client will be merged.
 *
 * \see qmdiServer::clientDeleted( QObject * )
 */
void qmdiTabWidget::deleteClient(qmdiClient *client) {
    if (client == nullptr) {
        return;
    }

    if (mdiHost == nullptr) {
        return;
    }

    if (dynamic_cast<qmdiClient *>(activeWidget) != client) {
        return;
    }

#if QT_VERSION < 0x050000 // supported on Qt4.x only
    QWorkspace *ws = qobject_cast<QWorkspace *>(activeWidget);
    if (ws) {
        foreach (QWidget *c, ws->windowList()) {
            mdiHost->unmergeClient(dynamic_cast<qmdiClient *>(c));
        }
    }
#endif
    mdiHost->unmergeClient(client);
    mdiHost->updateGUI(dynamic_cast<QMainWindow *>(mdiHost));
    activeWidget = nullptr;
}

/**
 * \brief callback for getting informed of new MDI clients
 * \param index the index of the new widget
 *
 * This function will be called when the a new tab is inserted
 * into the tab widget. This sets the MDI server property of the qmdiClient
 * to \b this, which is needed to call deleteClient().
 *
 * If this is the only widget on the tab widget it generates a call to
 * tabChanged() to update the menus and toolbars. If there are more then 1
 * widget the call will be generated by Qt for us.
 *
 * This function will also hide the tabbar, if the number of tabs is less then
 * 2 (new functionality since version 0.0.5)
 *
 * \bug Is this a bug in Qt...?
 * \see QTabWidget::tabInserted( int )
 * \see QTabWidget::tabBar()
 */
void qmdiTabWidget::tabInserted(int index) {
    QWidget *w = widget(index);
    qmdiClient *client = dynamic_cast<qmdiClient *>(w);

    if (mdiHost == nullptr) {
        mdiHost = dynamic_cast<qmdiHost *>(parent());
    }
    if (client) {
        client->mdiServer = this;
    }

#if QT_VERSION < 0x050000 // supported on Qt4.x only
    QWorkspace *ws = qobject_cast<QWorkspace *>(w);
    if (ws) {
        connect(ws, SIGNAL(windowActivated(QWidget *)), this,
                SLOT(workSpaceWindowActivated(QWidget *)));
    }
#endif
    //	if it's the only widget available, show it's number
    //	BUG is this supposed to be done by Qt?
    int c = count();
    if (c == 1) {
        tabChanged(0); // TODO: is this needed...?
        activeWidget = w;
    }
    tabBar()->setVisible(c > 1);
}

/**
 * \brief callback for getting informed of removed MDI client
 * \param index the index of the new widget
 *
 * This function will be called when a tab is removed. The MDI client
 * will un-merge itself on it's destructor, however if it was the only
 * widget available (and the tab widget is now empty), the GUI needs
 * to be updated as the tabChanged() function will not get called.
 *
 * This function will be called \b after the widget has been deleted, and thus
 * widget(index) is not the deleted widget! For this reason the qmdiClient must
 * un-merge itself - the MDI server has no way of knowing why object has been
 * deleted.
 *
 * This function will also hide the tabbar, if the number of tabs is less then
 * 2 (new functionality since version 0.0.5)
 *
 * \see QTabWidget::tabRemoved( int )
 * \see QTabWidget::tabBar()
 */
void qmdiTabWidget::tabRemoved(int index) {
    if (mdiHost == nullptr) {
        return;
    }

    int c = count();

    if (c == 0) {
        activeWidget = nullptr;

        // the deletion of menus and toolbars is made by qmdiClient itself
        mdiHost->updateGUI(dynamic_cast<QMainWindow *>(mdiHost));
    }
    tabBar()->setVisible(c > 1);

    //	if it's the only widget available, show it's number
    //	BUG is this supposed to be done by Qt?
    if (c == 1) {
        tabChanged(0);
    }

    Q_UNUSED(index);
}
