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

class QFileSystemWatcher;
class QPushButton;
class QComboBox;

class TextPreview;
class TextOperationsWidget;
class SharedHistoryModel;

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
    inline bool getModificationsLookupEnabled() const { return fileModifications; }
    void setModificationsLookupEnabled(bool);
    inline void setEditorFont(QFont newFont) { textEditor->setFont(newFont); }
    inline const Qutepart::Theme *getEditorTheme() { return textEditor->getTheme(); }
    inline void setEditorTheme(const Qutepart::Theme *theme) { textEditor->setTheme(theme); }
    void setEditorHighlighter(QString id);
    inline void setEditorMarkWord(bool b) { textEditor->setMarkCurrentWord(b); }

    void setPreviewEnabled(bool enabled);
    void setPreviewVisible(bool enabled);
    bool isPreviewRequested();
    bool isPreviewVisible() const;

    void setHistoryModel(SharedHistoryModel *model);

    inline const QString &getSyntaxID() const { return this->syntaxLangID; }
    inline const QString &getIndentatorID() const { return this->indentationID; }

    bool isMarkDownDocument() const;
    bool isXPMDocument() const;
    bool isSVGDocument() const;
    bool isXMLDocument() const;
    bool isJSONDocument() const;
    inline bool hasPreview() const {
        return isMarkDownDocument() || isXPMDocument() || isSVGDocument() || isXMLDocument() ||
               isJSONDocument();
    }

  public slots:
    void on_fileChanged(const QString &filename);
    void displayBannerMessage(QString message, int time);
    void hideBannerMessage();

    void newDocument();
    bool doSave();
    bool doSaveAs();
    bool loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    void transformBlockToUpper();
    void transformBlockToLower();
    void transformBlockCase();
    void toggleHeaderImpl();

    void chooseHighliter(const QString &newText);
    void chooseIndenter(const QAction *action);

  private slots:
    void updateFileDetails();
    void updateIndenterMenu();
    void updateHighlighterMenu();
    void updatePreview();
    void loadContent();

    void fileMessage_clicked(const QString &s);
    void hideTimer_timeout();

  public:
    QString originalLineEnding = "\n";
    EndLineStyle endLineStyle = EndLineStyle::KeepOriginalEndline;
    bool trimSpacesOnSave = false;
    bool autoPreview = true;

    // No longer inheriting Qutepart, instead use "static inheritance"
    void goTo(int x, int y);
    inline void setLineWrapMode(QPlainTextEdit::LineWrapMode mode) {
        textEditor->setLineWrapMode(mode);
    }
    inline void setDrawAnyWhitespace(bool b) { textEditor->setDrawAnyWhitespace(b); }
    inline void setDrawIndentations(bool b) {
        textEditor->setDrawIndentations(b);
        textEditor->setDrawIncorrectIndentation(b);
    }
    inline void setBracketHighlightingEnabled(bool b) {
        textEditor->setBracketHighlightingEnabled(b);
    }
    inline void setLineNumbersVisible(bool b) { textEditor->setLineNumbersVisible(b); }
    inline void setSmartHomeEnd(bool b) { textEditor->setSmartHomeEnd(b); }
    inline void setDrawSolidEdge(bool b) { textEditor->setDrawSolidEdge(b); }
    inline void setLineLengthEdge(int l) { textEditor->setLineLengthEdge(l); }
    inline void removeMetaData() { textEditor->removeMetaData(); }
    inline void setMetaDataMessage(int lineNumber, const QString &message) {
        textEditor->setLineMessage(lineNumber, message);
    }
    inline void setLineBookmark(int lineNumber, bool value) {
        textEditor->setLineBookmark(lineNumber, value);
    }
    inline void setLineWarning(int lineNumber, bool value) {
        textEditor->setLineWarning(lineNumber, value);
    }
    inline void setLineError(int lineNumber, bool value) {
        textEditor->setLineError(lineNumber, value);
    }
    inline void setLineInfo(int lineNumber, bool value) {
        textEditor->setLineInfo(lineNumber, value);
    }
    inline void setLineBreakpoint(int lineNumber, bool value) {
        textEditor->setLineBreakpoint(lineNumber, value);
    }
    inline void setLineExecuting(int lineNumber, bool value) {
        textEditor->setLineExecuting(lineNumber, value);
    }
    inline bool isEmpty() const {
        if (!textEditor->document()) {
            return true;
        }
        return textEditor->document()->isEmpty();
    }

  protected:
    void focusInEvent(QFocusEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void handleTabSelected();
    void handleTabDeselected();

  private:
    QString getShortFileName();

    Qutepart::ThemeManager *themeManager = nullptr;
    Qutepart::Qutepart *textEditor = nullptr;
    TextOperationsWidget *operationsWidget = nullptr;
    QString syntaxLangID;
    QString indentationID;

    QFileSystemWatcher *fileSystemWatcher;
    QWidget *banner;
    Ui::BannerMessage *ui_banner;
    int m_timerHideout;
    bool fileModifications = true;
    QTimer *loadingTimer = nullptr;
    bool documentHasBeenLoaded = true;
    QPoint requestedPosition;

    QString fileName;
    QMenu *bookmarksMenu;
    QMenu *textOperationsMenu;

    QComboBox *comboChangeHighlighter = nullptr;
    QToolButton *buttonChangeIndenter = nullptr;
    QPushButton *previewButton = nullptr;
    TextPreview *textPreview;

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
    QAction *actionCopyFileName = nullptr;
    QAction *actionCopyFilePath = nullptr;
    QAction *actionToggleHeader = nullptr;
};
