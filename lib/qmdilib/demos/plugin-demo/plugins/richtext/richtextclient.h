#pragma once

/**
 * \file richtext_browser.h
 * \brief Definition of the QexRichTextBrowser class
 * \author Diego Iastrubni (diegoiast@gmail.com)
 *  License LGPL
 * \see QexRichTextBrowser
 */

#include "qmdiclient.h"
#include "richtextwidget.h"

class RichTextClient : public RichTextWidget, public qmdiClient {
    Q_OBJECT
  public:
    RichTextClient(QString fileName, QWidget *parent);
    bool canCloseClient();
    QString mdiClientFileName();

  private:
    QTextEdit *edit;
};
