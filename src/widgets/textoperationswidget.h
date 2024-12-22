/**
 * \file qsvtextoperationswidget.cpp
 * \brief definition of widget for search,replace,gotoline in a QTextEdit
 * \author Diego Iastrubni diegoiast@gmail.com
 * License LGPLv2 2024
 */

#pragma once

#include <QColor>
#include <QObject>
#include <QStackedWidget>
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

class SharedHistoryModel;

class TextOperationsWidget : public QStackedWidget {
    Q_OBJECT
    friend class QsvTextEdit;

  public:
    TextOperationsWidget(QWidget *parent, QWidget *editor);
    void initSearchWidget();
    void initReplaceWidget();
    void initGotoLineWidget();
    void setSearchHistory(SharedHistoryModel *model);

    QFlags<QTextDocument::FindFlag> getSearchFlags();
    QFlags<QTextDocument::FindFlag> getReplaceFlags();

    QTextCursor getTextCursor();
    void setTextCursor(QTextCursor c);
    QTextDocument *getTextDocument();

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
    bool eventFilter(QObject *obj, QEvent *event);
    bool issueSearch(const QString &text, QTextCursor newCursor,
                     QFlags<QTextDocument::FindFlag> findOptions, QLineEdit *lineEdit,
                     bool moveCursor);

    QWidget *editor;
    QTextCursor searchCursor;
    QTimer replaceTimer;
    QTimer searchTimer;

  public:
    SharedHistoryModel *searchHistory;
    QWidget *searchWidget;
    QWidget *replaceWidget;
    QWidget *gotoLineWidget;
    QColor searchFoundColor;
    QColor searchNotFoundColor;

    Ui::searchForm *searchFormUi;
    Ui::replaceForm *replaceFormUi;
    Ui::gotoLineForm *gotoLineFormUi;
};
