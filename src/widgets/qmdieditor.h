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

#if defined(WIN32)
#define DEFAULT_EDITOR_FONT "Courier new"
#define DEFAULT_EDITOR_FONT_SIZE 10
#else
#define DEFAULT_EDITOR_FONT "Monospace"
#define DEFAULT_EDITOR_FONT_SIZE 10
#endif

class TextOperationsWidget;
class QFileSystemWatcher;

class QComboBox;
namespace Ui {
class BannerMessage;
}

namespace Qutepart {
class ThemeManager;
}

/**
A source editor with MDI interface.
This class will be a very rich text editor which will also have a set of toolbars and menus
available for the qmdiHost.

@author Diego Iastrubni <diegoiast@gmail.com>
*/
class qmdiEditor : public QWidget, public qmdiClient {
    Q_OBJECT

  public:
    qmdiEditor(QWidget *p, Qutepart::ThemeManager *theme);
    ~qmdiEditor();

    virtual bool canCloseClient() override;
    virtual QString mdiClientFileName() override;
    virtual std::optional<std::tuple<int, int, int>> get_coordinates() const override;

    void setupActions();
    QString getFileName() const { return fileName; }
    bool getModificationsLookupEnabled();
    void setModificationsLookupEnabled(bool);
    inline void setEditorFont(QFont newFont) { textEditor->setFont(newFont); }
    inline const Qutepart::Theme *getEditorTheme() { return textEditor->getTheme(); }
    inline void setEditorTheme(const Qutepart::Theme *theme) { textEditor->setTheme(theme); }
    inline void setEditorHighlighter(QString id, const Qutepart::Theme *theme) {
        /*
            TODO: themes and synyax are merged. When we de-couple them this will get restoredr
            if (this->syntaxLangID == id) {
                return;
            }
        */
        this->syntaxLangID = id;
        textEditor->setHighlighter(id, theme);
    }
    inline void setEditorMarkWord(bool b) { textEditor->setMarkCurrentWord(b); }

    inline const QString &getSyntaxID() const { return this->syntaxLangID; }
    inline const QString &getIndentatorID() const { return this->indentationID; }

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
    void chooseIndenter(const QAction *action);

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

  protected:
    void focusInEvent(QFocusEvent *event) override;

  private:
    QString getShortFileName();

    Qutepart::ThemeManager *themeManager = nullptr;
    Qutepart::Qutepart *textEditor = nullptr;
    TextOperationsWidget *operationsWidget = nullptr;
    QString syntaxLangID;
    QString indentationID;

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

    QComboBox *comboChangeHighlighter = nullptr;
    QToolButton *buttonChangeIndenter = nullptr;
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
