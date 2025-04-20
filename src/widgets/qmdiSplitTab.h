#pragma once

#include "SplitTabWidget.h"
#include "qmdiserver.h"

class DefaultButtonsProvider : public ButtonsProvider {
  public:
    virtual QWidget *requestButton(bool first, int tabIndex, SplitTabWidget *split);
};

class qmdiSplitTab : public SplitTabWidget, public qmdiServer {

  public:
    // SplitTabWidget
    virtual void onTabFocusChanged(QWidget *widget, bool focused) override;
    virtual bool eventFilter(QObject *obj, QEvent *event) override;
    virtual void onNewSplitCreated(QTabWidget *tabWidget, int count) override;

    // qmdiServer interface
    virtual void addClient(qmdiClient *client) override;
    virtual void deleteClient(qmdiClient *) override;
    virtual int getClientsCount() const override;
    virtual qmdiClient *getClient(int i) const override;
    virtual qmdiClient *getCurrentClient() const override;
    virtual int getCurrentClientIndex() const override;
    virtual void setCurrentClientIndex(int i) override;
    virtual void updateClientName(const qmdiClient *client) override;

  private:
    virtual void mdiSelected(qmdiClient *client, int index) const override;
    void on_middleMouse_pressed(int, QPoint);
    void on_rightMouse_pressed(int, QPoint);
    int computeLeading(QTabWidget *);

    QWidget *activeWidget = nullptr;
};
