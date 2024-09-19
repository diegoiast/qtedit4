/**
 * \file qmdieditor
 * \brief Definition of
 * \author Diego Iastrubni diegoiast@gmail.com
 * License GPL 2008
 * \see class name
 */

#pragma once

#include "endlinestyle.h"
#include <qmdiclient.h>
#include <qtoolbutton.h>
#include <qutepart/qutepart.h>

class QsvTextOperationsWidget;
class QFileSystemWatcher;

class QComboBox;
namespace Ui {
class BannerMessage;
};

/**
A source editor with MDI interface.
This class will be a very rich text editor which will also have a set of toolbars and menus
available for the qmdiHost.

@author Diego Iastrubni <diegoiast@gmail.com>
*/
class qmdiEditor : public QWidget, public qmdiClient {
    Q_OBJECT

  public:
    qmdiEditor(QWidget *p);
    ~qmdiEditor();

    virtual bool canCloseClient() override;
    virtual QString mdiClientFileName() override;
    virtual std::optional<std::tuple<int, int, int>> get_coordinates() const override;

    void setupActions();
    QString getFileName() const { return fileName; }
    bool getModificationsLookupEnabled();
    void setModificationsLookupEnabled(bool);

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
    void toggleHeaderImpl();

    void chooseHighliter(const QString &newText);
    void chooseIndenter(QAction *action);

  private slots:
    void updateFileDetails();
    void updateIndenterMenu();
    void updateHighlighterMenu();

    void fileMessage_clicked(const QString &s);
    void hideTimer_timeout();

  public:
    EndLineStyle endLineStyle = EndLineStyle::KeepOriginalEndline;
    bool trimSpacesOnSave = false;

    // No longer inheriting Qutepart, instead use "static inheritance"
    void goTo(int x, int y) { textEditor->goTo(x, y); }
    void setLineWrapMode(QPlainTextEdit::LineWrapMode mode) { textEditor->setLineWrapMode(mode); }
    void setDrawAnyWhitespace(bool b) { textEditor->setDrawAnyWhitespace(b); }
    void setDrawIndentations(bool b) { textEditor->setDrawIndentations(b); }
    void setBracketHighlightingEnabled(bool b) { textEditor->setBracketHighlightingEnabled(b); }
    void setLineNumbersVisible(bool b) { textEditor->setLineNumbersVisible(b); }
    void setSmartHomeEnd(bool b) { textEditor->setSmartHomeEnd(b); }
    void setDrawSolidEdge(bool b) { textEditor->setDrawSolidEdge(b); }
    void setLineLengthEdge(int l) { textEditor->setLineLengthEdge(l); }

  private:
    Qutepart::Qutepart *textEditor;
    QsvTextOperationsWidget *operationsWidget;
    QString getShortFileName();

    QFileSystemWatcher *fileSystemWatcher;
    QWidget *topWidget = nullptr;
    QWidget *bottomWidget = nullptr;
    QWidget *banner;
    Ui::BannerMessage *ui_banner;
    int m_timerHideout;
    bool fileModifications = true;

    QString fileName;
    QMenu *bookmarksMenu;
    QMenu *textOperationsMenu;

    QToolButton *buttonChangeIndenter = nullptr;
    QComboBox *comboChangeHighlighter = nullptr;
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
    QAction *actionCopyFileName = nullptr;
    QAction *actionCopyFilePath = nullptr;

    QAction *actionToggleHeader = nullptr;
};
