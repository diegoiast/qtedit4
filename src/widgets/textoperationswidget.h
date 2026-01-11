/**
 * \file qsvtextoperationswidget.cpp
 * \brief definition of widget for search, replace, gotoline in a QTextEdit
 * \author Diego Iastrubni diegoiast@gmail.com
 */

// SPDX-License-Identifier: MIT

#pragma once

#include <QColor>
#include <QObject>
#include <QStackedWidget>
#include <QString>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextBlock>
#include <QTimer>
#include <QPointer>
#include <memory>

class QWidget;
class QLineEdit;

namespace Ui {
class searchForm;
class replaceForm;
class gotoLineForm;
} // namespace Ui


class ITextCursorAdapter {
public:
    virtual ~ITextCursorAdapter() = default;

    virtual bool find(const QString &text, bool forward, bool caseSensitive, bool wholeWords) = 0;
    virtual void moveToPosition(int position, bool keepAnchor = false) = 0;
    virtual void moveToStart() = 0;
    virtual void moveToEnd() = 0;
    virtual void goToLine(int line) = 0;

    virtual QString selectedText() const = 0;
    virtual bool isNull() const = 0;
    virtual int blockNumber() const = 0;
    virtual int selectionStart() const = 0; // Required for override
    virtual int selectionEnd() const = 0;   // Required for override

    virtual void beginEditBlock() = 0;
    virtual void insertText(const QString &text) = 0;
    virtual void deleteChar() = 0;
    virtual void endEditBlock() = 0;

    virtual std::unique_ptr<ITextCursorAdapter> clone() const = 0;
};

class QtCursorAdapter : public ITextCursorAdapter {
public:
    QtCursorAdapter(QTextCursor cursor, QTextDocument* document)
        : m_cursor(cursor), m_document(document) {}

    bool find(const QString &text, bool forward, bool caseSensitive, bool wholeWords) override {
        if (m_document.isNull()) return false;
        QTextDocument::FindFlags flags;
        if (!forward) flags |= QTextDocument::FindBackward;
        if (caseSensitive) flags |= QTextDocument::FindCaseSensitively;
        if (wholeWords) flags |= QTextDocument::FindWholeWords;

        QTextCursor found = m_document->find(text, m_cursor, flags);
        if (!found.isNull()) {
            m_cursor = found;
            return true;
        }
        return false;
    }

    void moveToPosition(int position, bool keepAnchor = false) override {
        m_cursor.setPosition(position, keepAnchor ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
    }

    void moveToStart() override { m_cursor.movePosition(QTextCursor::Start); }
    void moveToEnd() override { m_cursor.movePosition(QTextCursor::End); }
    void goToLine(int line) override {
        if (m_document) m_cursor = QTextCursor(m_document->findBlockByNumber(line - 1));
    }

    QString selectedText() const override { return m_cursor.selectedText(); }
    bool isNull() const override { return m_cursor.isNull(); }
    int blockNumber() const override { return m_cursor.blockNumber(); }
    int selectionStart() const override { return m_cursor.selectionStart(); }
    int selectionEnd() const override { return m_cursor.selectionEnd(); }

    void beginEditBlock() override { m_cursor.beginEditBlock(); }
    void insertText(const QString &text) override { m_cursor.insertText(text); }
    void deleteChar() override { m_cursor.deleteChar(); }
    void endEditBlock() override { m_cursor.endEditBlock(); }

    std::unique_ptr<ITextCursorAdapter> clone() const override {
        return std::make_unique<QtCursorAdapter>(m_cursor, m_document.data());
    }

    QTextCursor getInternalCursor() const { return m_cursor; }

private:
    QTextCursor m_cursor;
    QPointer<QTextDocument> m_document;
};

class SharedHistoryModel;

class TextOperationsWidget : public QStackedWidget {
    Q_OBJECT

public:
    // The constructor still takes a QWidget* for the editor to handle event filtering,
    // but internal logic will use the adapter.
    TextOperationsWidget(QWidget *parent, QWidget *editor);
    ~TextOperationsWidget();

    void initSearchWidget();
    void initReplaceWidget();
    void initGotoLineWidget();
    void setSearchHistory(SharedHistoryModel *model);
    void setTextFont(const QFont &newFont);

    // Replaced specific QTextDocument flags with generic booleans or a custom enum
    // to keep the UI logic decoupled from Qt's text engine.
    struct FindOptions {
        bool caseSensitive;
        bool wholeWords;
        bool backward;
    };

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void showSearch();
    void showReplace();
    void showGotoLine();

    void searchText_modified(QString s);
    void replaceText_modified(QString s);
    void replaceOldText_returnPressed();
    void replaceAll_clicked();
    void searchNext();
    void searchPrevious();

    void updateSearchInput();
    void updateReplaceInput();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    
    // Core search logic now uses the interface
    bool issueSearch(const QString &text, 
                     ITextCursorAdapter* cursor,
                     FindOptions options, 
                     QLineEdit *lineEdit,
                     bool moveCursor);

    // Helper to create the correct adapter based on the editor's type
    std::unique_ptr<ITextCursorAdapter> createCursorAdapter() const;

private:
    QWidget *editor;
    
    // Instead of QTextCursor, we store a unique_ptr to our interface
    std::unique_ptr<ITextCursorAdapter> searchCursor;
    
    QTimer replaceTimer;
    QTimer searchTimer;
    QColor searchFoundBackgroundColor;
    QColor searchNotFoundBackgroundColor;

    SharedHistoryModel *searchHistory = nullptr;
    
    // UI Containers
    QWidget *searchWidget = nullptr;
    QWidget *replaceWidget = nullptr;
    QWidget *gotoLineWidget = nullptr;

    // UI Forms
    Ui::searchForm *searchFormUi = nullptr;
    Ui::replaceForm *replaceFormUi = nullptr;
    Ui::gotoLineForm *gotoLineFormUi = nullptr;
};
