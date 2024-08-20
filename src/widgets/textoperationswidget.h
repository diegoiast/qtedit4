/**
 * \file qsvtextoperationswidget.cpp
 * \brief definition of widget for search,replace,gotoline in a QTextEdit
 * \author Diego Iastrubni diegoiast@gmail.com
 * License LGPLv2 2024
 */

#pragma once

#include <QColor>
#include <QObject>
#include <QString>
#include <QTextCursor>
#include <QTextDocument>
#include <QTimer>

class QWidget;
class QLineEdit;

namespace Ui {
class searchForm;
class replaceForm;
class gotoLineForm;
} // namespace Ui

class QsvTextEdit;

class QsvTextOperationsWidget : public QObject {
    Q_OBJECT
    friend class QsvTextEdit;

  public:
    QsvTextOperationsWidget(QWidget *parent);
    void initSearchWidget();
    void initReplaceWidget();
    void initGotoLineWidget();

    QFlags<QTextDocument::FindFlag> getSearchFlags();
    QFlags<QTextDocument::FindFlag> getReplaceFlags();

    virtual QTextCursor getTextCursor();
    virtual void setTextCursor(QTextCursor c);
    virtual QTextDocument *getTextDocument();

  public slots:
    void showSearch();
    void showReplace();
    void showGotoLine();
    void showBottomWidget(QWidget *w = nullptr);
    void searchText_modified(QString s);
    void replaceText_modified(QString s);
    void replaceOldText_returnPressed();
    void replaceAll_clicked();
    void searchNext();
    void searchPrevious();
    void searchPrev() { searchPrevious(); }
    void adjustBottomWidget();

    void updateSearchInput();
    void updateReplaceInput();

  protected:
    bool eventFilter(QObject *obj, QEvent *event);
    bool issue_search(const QString &text, QTextCursor newCursor,
                      QFlags<QTextDocument::FindFlag> findOptions, QLineEdit *l, bool moveCursor);

    QTextCursor m_searchCursor;
    QTextDocument *m_document;
    QTimer m_replaceTimer;
    QTimer m_searchTimer;

  public:
    QWidget *m_search;
    QWidget *m_replace;
    QWidget *m_gotoLine;
    QColor searchFoundColor;
    QColor searchNotFoundColor;

    Ui::searchForm *searchFormUi;
    Ui::replaceForm *replaceFormUi;
    Ui::gotoLineForm *gotoLineFormUi;
};
