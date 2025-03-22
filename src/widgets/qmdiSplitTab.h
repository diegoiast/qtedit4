#pragma once

#include "SplitTabWidget.h"
#include "qmdiserver.h"

class qmdiSplitTab : public SplitTabWidget, public qmdiServer {

  public:
    // SplitTabWidget
    virtual void onTabFocusChanged(QWidget *widget, bool focused) override;

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

    QWidget *activeWidget = nullptr;
};
