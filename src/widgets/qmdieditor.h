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
class QFileSystemWatcher;

namespace Ui {
class BannerMessage;
};

/**
A source editor with MDI interface.
This class will be a very rich text editor which will also have a set of toolbars and menus
available for the qmdiHost.

@author Diego Iastrubni <diegoiast@gmail.com>
*/
class qmdiEditor : public Qutepart::Qutepart, public qmdiClient {
    Q_OBJECT

  public:
    qmdiEditor(QWidget *p);
    ~qmdiEditor();

    virtual bool canCloseClient() override;
    virtual QString mdiClientFileName() override;
    virtual std::optional<std::tuple<int, int, int>> get_coordinates() const override;

    void setupActions();
    QString getFileName() const { return fileName; }

  public slots:
    void on_fileChanged(const QString &filename);
    void adjustBottomAndTopWidget();
    void showUpperWidget(QWidget *w);
    void showBottomWidget(QWidget *w);
    void displayBannerMessage(QString message, int time);
    void hideBannerMessage();

    void newDocument();
    bool doSave();
    bool doSaveAs();
    bool loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    void smartHome();
    void smartEnd();
    void transformBlockToUpper();
    void transformBlockToLower();
    void transformBlockCase();
    void gotoMatchingBracket();

  private slots:
    void fileMessage_clicked(const QString &s);
    void hideTimer_timeout();

  signals:
    void widgetResized();

  protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

  private:
    QsvTextOperationsWidget *operationsWidget;
    QString getShortFileName();

    QFileSystemWatcher *fileSystemWatcher;
    QWidget *topWidget = nullptr;
    QWidget *bottomWidget = nullptr;
    QWidget *banner;
    Ui::BannerMessage *ui_banner;
    int m_timerHideout;

    QString fileName;
    QMenu *bookmarksMenu;
    QMenu *textOperationsMenu;

    QAction *actionSave = nullptr;
    QAction *actionSaveAs = nullptr;
    QAction *actionUndo = nullptr;
    QAction *actionRedo = nullptr;
    QAction *actionCopy = nullptr;
    QAction *actionCut = nullptr;
    QAction *actionPaste = nullptr;
    QAction *actionFind = nullptr;
    QAction *actionFindNext = nullptr;
    QAction *actionFindPrev = nullptr;
    QAction *actionReplace = nullptr;
    QAction *actionGotoLine = nullptr;

    QAction *actionCapitalize = nullptr;
    QAction *actionLowerCase = nullptr;
    QAction *actionChangeCase = nullptr;
    QAction *actionFindMatchingBracket = nullptr;

    QAction *actiohAskHelp = nullptr;
};
