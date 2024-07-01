/**
 * \file qmdieditor
 * \brief Definition of
 * \author Diego Iastrubni diegoiast@gmail.com
 * License GPL 2008
 * \see class name
 */

#pragma once

#include <qmdiclient.h>
#include <qutepart/qutepart.h>

class QsvTextOperationsWidget;
/**
A source editor with MDI interface.
This class will be a very rich text editor which will also have a set of toolbars and menus
available for the qmdiHost

        @author Diego Iastrubni <diegoiast@gmail.com>
*/
class qmdiEditor : public Qutepart::Qutepart, public qmdiClient {
    Q_OBJECT

  public:
    qmdiEditor(QString fName, QWidget *p);
    ~qmdiEditor();

    virtual bool canCloseClient() override;
    virtual QString mdiClientFileName() override;

    void setupActions();
    QString getFileName() const { return m_fileName; }

  public slots:
    void newDocument();
    int loadFile(const QString &fileName);
    int saveFile(const QString &fileName);
    // int saveFile();
    // int saveFileAs();
    void smartHome();
    void smartEnd();
    void transformBlockToUpper();
    void transformBlockToLower();
    void transformBlockCase();
    void gotoMatchingBracket();
    void gotoLine(int linenumber, int rownumber);
  signals:
    void widgetResized();

  protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

  private:
    QsvTextOperationsWidget *operationsWidget;
    QString getShortFileName();

    QString m_fileName;

    QMenu *bookmarksMenu;
    QMenu *textOperationsMenu;

    QAction *actionSave = nullptr;
    QAction *actionUndo = nullptr;
    QAction *actionRedo = nullptr;
    QAction *actionCopy = nullptr;
    QAction *actionCut = nullptr;
    QAction *actionPaste = nullptr;
    QAction *actiohAskHelp = nullptr;
    QAction *actionFind = nullptr;
    QAction *actionFindNext = nullptr;
    QAction *actionFindPrev = nullptr;
    QAction *actionReplace = nullptr;
    QAction *actionGotoLine = nullptr;

    QAction *actionCapitalize = nullptr;
    QAction *actionLowerCase = nullptr;
    QAction *actionChangeCase = nullptr;
    QAction *actionFindMatchingBracket = nullptr;
};
