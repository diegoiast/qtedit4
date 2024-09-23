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

class TextOperationsWidget : public QObject {
    Q_OBJECT
    friend class QsvTextEdit;

  public:
    TextOperationsWidget(QWidget *parent);
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
                      QFlags<QTextDocument::FindFlag> findOptions, QLineEdit *lineEdit,
                      bool moveCursor);

    QTextCursor searchCursor;
    QTextDocument *document;
    QTimer replaceTimer;
    QTimer searchTimer;

  public:
    QWidget *searchWidget;
    QWidget *replaceWidget;
    QWidget *gotoLineWidget;
    QColor searchFoundColor;
    QColor searchNotFoundColor;

    Ui::searchForm *searchFormUi;
    Ui::replaceForm *replaceFormUi;
    Ui::gotoLineForm *gotoLineFormUi;
};
