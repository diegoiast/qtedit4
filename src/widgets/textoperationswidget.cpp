/**
 * \file qsvtextoperationswidget.cpp
 * \brief implementation widget for searchWidget,replace,gotoline in a QTextEdit
 * \author Diego Iastrubni diegoiast@gmail.com
 * License LGPLv2 2024
 */

#include "textoperationswidget.h"
#include "ui_gotolineform.h"
#include "ui_replaceform.h"
#include "ui_searchform.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStyle>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextEdit>

#include <QDebug>

TextOperationsWidget::TextOperationsWidget(QWidget *parent) : QObject(parent) {
    setObjectName("QsvTextOperationWidget");
    gotoLineWidget = nullptr;
    searchWidget = nullptr;
    replaceWidget = nullptr;
    document = nullptr;
    searchFormUi = nullptr;
    replaceFormUi = nullptr;
    gotoLineFormUi = nullptr;
    searchFoundColor = QColor(0xDDDDFF);
    searchNotFoundColor = QColor(0xFFAAAA);

    replaceTimer.setInterval(100);
    replaceTimer.setSingleShot(true);
    connect(&replaceTimer, &QTimer::timeout, this, &TextOperationsWidget::updateReplaceInput);

    // this one is slower, to let the user think about his action
    // this is a modifying command, unlike a passive search
    searchTimer.setInterval(250);
    searchTimer.setSingleShot(true);
    connect(&searchTimer, &QTimer::timeout, this, &TextOperationsWidget::updateSearchInput);

    auto t = qobject_cast<QTextEdit *>(parent);
    if (t) {
        document = t->document();
    } else {
        auto pt = qobject_cast<QPlainTextEdit *>(parent);
        if (pt) {
            document = pt->document();
        }
    }
    parent->installEventFilter(this);
}

void TextOperationsWidget::initSearchWidget() {
    auto parentWidget = (QWidget *)parent();

    searchWidget = new QWidget(parentWidget);
    searchWidget->setPalette(parentWidget->style()->standardPalette());
    searchWidget->setObjectName("m_search");
    searchFormUi = new Ui::searchForm();
    searchFormUi->setupUi(searchWidget);
    searchFormUi->searchText->setFont(searchWidget->parentWidget()->font());
    if (searchFormUi->frame->style()->inherits("QWindowsStyle")) {
        searchFormUi->frame->setFrameStyle(QFrame::StyledPanel);
        searchWidget->setPalette(parentWidget->palette());
    }
    // otherwise it inherits the default font from the editor - fixed
    searchWidget->setFont(QApplication::font());
    searchWidget->adjustSize();
    searchWidget->hide();

    connect(searchFormUi->searchText, &QLineEdit::textChanged, this,
            &TextOperationsWidget::searchText_modified);
    connect(searchFormUi->nextButton, &QAbstractButton::clicked, this,
            &TextOperationsWidget::searchNext);
    connect(searchFormUi->previousButton, &QAbstractButton::clicked, this,
            &TextOperationsWidget::searchPrev);
    connect(searchFormUi->closeButton, &QAbstractButton::clicked, this,
            &TextOperationsWidget::showSearch);
    connect(searchFormUi->searchText, &QLineEdit::textChanged, this,
            &TextOperationsWidget::searchText_modified);
}

void TextOperationsWidget::initReplaceWidget() {
    auto parentWidget = (QWidget *)parent();
    replaceWidget = new QWidget(parentWidget);
    replaceWidget->setPalette(parentWidget->style()->standardPalette());
    replaceWidget->setObjectName("m_replace");
    replaceFormUi = new Ui::replaceForm();
    replaceFormUi->setupUi(replaceWidget);
    replaceFormUi->optionsGroupBox->hide();
    replaceFormUi->findText->setFont(replaceWidget->parentWidget()->font());
    replaceFormUi->replaceText->setFont(replaceWidget->parentWidget()->font());
    if (replaceFormUi->frame->style()->inherits("QWindowsStyle")) {
        replaceFormUi->frame->setFrameStyle(QFrame::StyledPanel);
        replaceWidget->setPalette(parentWidget->palette());
    }
    // otherwise it inherits the default font from the editor - fixed
    replaceWidget->setFont(QApplication::font());
    replaceWidget->adjustSize();
    replaceWidget->hide();

    connect(replaceFormUi->moreButton, &QAbstractButton::clicked, this,
            &TextOperationsWidget::adjustBottomWidget);
    connect(replaceFormUi->findText, &QLineEdit::textChanged, this,
            &TextOperationsWidget::replaceText_modified);
    connect(replaceFormUi->replaceButton, &QAbstractButton::clicked, this,
            &TextOperationsWidget ::replaceOldText_returnPressed);
    connect(replaceFormUi->closeButton, &QAbstractButton::clicked, this,
            &TextOperationsWidget::showReplace);
    connect(replaceFormUi->replaceText, &QLineEdit::textChanged, this,
            &TextOperationsWidget::replaceText_modified);
    connect(replaceFormUi->findText, &QLineEdit::textChanged, this,
            &TextOperationsWidget::replaceText_modified);
    connect(replaceFormUi->replaceText, &QLineEdit::returnPressed, this,
            &TextOperationsWidget::replaceOldText_returnPressed);
    connect(replaceFormUi->findText, &QLineEdit::returnPressed, this,
            &TextOperationsWidget::replaceOldText_returnPressed);
}

void TextOperationsWidget::initGotoLineWidget() {
    auto parentWidget = (QWidget *)parent();

    gotoLineWidget = new QWidget(parentWidget);
    gotoLineWidget->setPalette(parentWidget->style()->standardPalette());
    gotoLineWidget->setObjectName("gotoLine");
    gotoLineFormUi = new Ui::gotoLineForm();
    gotoLineFormUi->setupUi(gotoLineWidget);
    if (gotoLineFormUi->frame->style()->inherits("QWindowsStyle")) {
        gotoLineFormUi->frame->setFrameStyle(QFrame::StyledPanel);
        gotoLineWidget->setPalette(parentWidget->palette());
    }
    gotoLineWidget->setFont(QApplication::font());
    gotoLineWidget->adjustSize();
    gotoLineWidget->hide();

    connect(gotoLineFormUi->numberSpinBox, &QAbstractSpinBox::editingFinished, this, [this]() {
        auto document = getTextDocument();
        auto line_number = gotoLineFormUi->numberSpinBox->value();
        auto block = document->findBlockByNumber(line_number - 1);
        auto cursor = QTextCursor(block);
        setTextCursor(cursor);
    });

    connect(gotoLineFormUi->closeButton, SIGNAL(clicked()), this, SLOT(showGotoLine()));
}

void TextOperationsWidget::searchNext() {
    if (!searchFormUi) {
        return;
    }
    issue_search(searchFormUi->searchText->text(), getTextCursor(),
                 getSearchFlags() & ~QTextDocument::FindBackward, searchFormUi->searchText, true);
}

void TextOperationsWidget::searchPrevious() {
    if (!searchFormUi) {
        return;
    }
    issue_search(searchFormUi->searchText->text(), getTextCursor(),
                 getSearchFlags() | QTextDocument::FindBackward, searchFormUi->searchText, true);
}

void TextOperationsWidget::adjustBottomWidget() { showBottomWidget(nullptr); }

void TextOperationsWidget::updateSearchInput() {
    if (!searchFormUi) {
        return;
    }
    issue_search(searchFormUi->searchText->text(), searchCursor, getSearchFlags(),
                 searchFormUi->searchText, true);
}

void TextOperationsWidget::updateReplaceInput() {
    if (!replaceFormUi) {
        return;
    }
    issue_search(replaceFormUi->findText->text(), searchCursor, getReplaceFlags(),
                 replaceFormUi->findText, true);
}

bool TextOperationsWidget::eventFilter(QObject *obj, QEvent *event) {
    if (obj != parent()) {
        return false;
    }

    if (event->type() == QEvent::Resize) {
        adjustBottomWidget();
        return false;
    }

    if (event->type() != QEvent::KeyPress) {
        return false;
    }

    auto keyEvent = static_cast<QKeyEvent *>(event);
    switch (keyEvent->key()) {
    case Qt::Key_Escape:
        if (searchWidget && searchWidget->isVisible()) {
            showSearch();
            return true;
        } else if (replaceWidget && replaceWidget->isVisible()) {
            showReplace();
            return true;
        } else if (gotoLineWidget && gotoLineWidget->isVisible()) {
            showGotoLine();
            return true;
        }
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (searchWidget && searchWidget->isVisible()) {
            if (keyEvent->modifiers().testFlag(Qt::ControlModifier) ||
                keyEvent->modifiers().testFlag(Qt::AltModifier) ||
                keyEvent->modifiers().testFlag(Qt::ShiftModifier)) {
                searchPrev();
            } else {
                searchNext();
            }
            return true;
        } else if (replaceWidget && replaceWidget->isVisible()) {
            if (keyEvent->modifiers().testFlag(Qt::ControlModifier) ||
                keyEvent->modifiers().testFlag(Qt::AltModifier) ||
                keyEvent->modifiers().testFlag(Qt::ShiftModifier)) {
                replaceAll_clicked();
            } else {
                replaceOldText_returnPressed();
            }
            return true;
        } else if (gotoLineWidget && gotoLineWidget->isVisible()) {
            return true;
        }
        break;
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
        if (replaceWidget && replaceWidget->isVisible()) {
            /*
            // TODO - no cycle yet.
            if (Qt::Key_Tab == keyEvent->key())
                    m_replace->focusWidget()->nextInFocusChain()->setFocus();
            else
                    m_replace->focusWidget()->previousInFocusChain()->setFocus();
            */
            // Instead - cycle between those two input lines. IMHO good enough
            if (replaceFormUi->replaceText->hasFocus()) {
                replaceFormUi->findText->setFocus();
                replaceFormUi->findText->selectAll();
            } else {
                replaceFormUi->replaceText->setFocus();
                replaceFormUi->replaceText->selectAll();
            }
            return true;
        }
        break;
    }
    return false;
}

QFlags<QTextDocument::FindFlag> TextOperationsWidget::getSearchFlags() {
    auto f = QFlags<QTextDocument::FindFlag>();
    // one can never be too safe
    if (!searchFormUi) {
        qDebug("%s:%d - searchFormUi not available, memory problems?", __FILE__, __LINE__);
        return f;
    }
    if (searchFormUi->caseSensitiveCheckBox->isChecked()) {
        f = f | QTextDocument::FindCaseSensitively;
    }
    if (searchFormUi->wholeWorldsCheckbox->isChecked()) {
        f = f | QTextDocument::FindWholeWords;
    }
    return f;
}

QFlags<QTextDocument::FindFlag> TextOperationsWidget::getReplaceFlags() {
    QFlags<QTextDocument::FindFlag> f;
    if (!replaceFormUi) {
        qDebug("%s:%d - replaceFormUi not available, memory problems?", __FILE__, __LINE__);
        return f;
    }
    if (replaceFormUi->caseCheckBox->isChecked()) {
        f = f | QTextDocument::FindCaseSensitively;
    }
    if (replaceFormUi->wholeWordsCheckBox->isChecked()) {
        f = f | QTextDocument::FindWholeWords;
    }
    return f;
}

QTextCursor TextOperationsWidget::getTextCursor() {
    auto cursor = QTextCursor();
    auto t = qobject_cast<QTextEdit *>(parent());
    if (t) {
        cursor = t->textCursor();
    } else {
        QPlainTextEdit *pt = qobject_cast<QPlainTextEdit *>(parent());
        if (pt) {
            cursor = pt->textCursor();
        }
    }
    return cursor;
}

void TextOperationsWidget::setTextCursor(QTextCursor c) {
    auto t = qobject_cast<QTextEdit *>(parent());
    if (t) {
        t->setTextCursor(c);
    } else {
        QPlainTextEdit *pt = qobject_cast<QPlainTextEdit *>(parent());
        if (pt) {
            pt->setTextCursor(c);
        }
    }
}

QTextDocument *TextOperationsWidget::getTextDocument() {
    auto t = qobject_cast<QTextEdit *>(parent());
    if (t) {
        return t->document();
    } else {
        QPlainTextEdit *pt = qobject_cast<QPlainTextEdit *>(parent());
        if (pt) {
            return pt->document();
        }
    }
    return {};
}

void TextOperationsWidget::showSearch() {
    if (!searchWidget) {
        initSearchWidget();
    }
    if (replaceWidget && replaceWidget->isVisible()) {
        replaceWidget->hide();
    }
    if (gotoLineWidget && gotoLineWidget->isVisible()) {
        gotoLineWidget->hide();
    }

    auto parent = qobject_cast<QWidget *>(this->parent());
    if (searchWidget->isVisible()) {
        searchWidget->hide();
        if (parent) {
            parent->setFocus();
        }
        return;
    }
    searchCursor = getTextCursor();
    auto s = searchCursor.selectedText();
    if (!s.isEmpty()) {
        searchFormUi->searchText->setText(s);
    }
    searchFormUi->searchText->setFocus();
    searchFormUi->searchText->selectAll();
    showBottomWidget(searchWidget);
}

void TextOperationsWidget::replaceOldText_returnPressed() {
    if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier) ||
        QApplication::keyboardModifiers().testFlag(Qt::AltModifier) ||
        QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) {
        replaceAll_clicked();
        showReplace();
        return;
    }

    auto cursor = searchCursor;
    auto doc = getTextDocument();
    if (!doc) {
        qDebug("%s:%d - no document found, using a wrong class? wrong parent?", __FILE__, __LINE__);
        return;
    }
    cursor = doc->find(replaceFormUi->findText->text(), cursor, getReplaceFlags());
    if (cursor.isNull()) {
        return;
    }

    auto start = cursor.selectionStart();
    auto end = cursor.selectionEnd();
    cursor.beginEditBlock();
    cursor.deleteChar();
    cursor.insertText(replaceFormUi->replaceText->text());
    cursor.setPosition(start, QTextCursor::KeepAnchor);
    cursor.setPosition(end, QTextCursor::MoveAnchor);
    cursor.endEditBlock();
    setTextCursor(cursor);

    // is there any other appearance of this text?
    searchCursor = cursor;
    updateReplaceInput();
}

void TextOperationsWidget::replaceAll_clicked() {
    // WHY NOT HIDING THE WIDGET?
    // it seems that if you hide the widget, when the replace all action
    // is triggered by pressing control+enter on the replace widget
    // eventually an "enter event" is sent to the text eidtor.
    // the work around is to update the transparency of the widget, to let the user
    // see the text below the widget

    // showReplaceWidget();
    replaceWidget->hide();

    auto replaceCount = 0;
    //        replaceWidget->setWidgetTransparency( 0.2 );
    auto cursor = getTextCursor();
    cursor = getTextDocument()->find(replaceFormUi->replaceText->text(), cursor, getReplaceFlags());

    while (!cursor.isNull()) {
        setTextCursor(cursor);
        QMessageBox::StandardButton button = QMessageBox::question(
            qobject_cast<QWidget *>(parent()), tr("Replace all"), tr("Replace this text?"),
            QMessageBox::Yes | QMessageBox::Ignore | QMessageBox::Cancel);

        if (button == QMessageBox::Cancel) {
            break;
        } else if (button == QMessageBox::Yes) {
            cursor.beginEditBlock();
            cursor.deleteChar();
            cursor.insertText(replaceFormUi->replaceText->text());
            cursor.endEditBlock();
            setTextCursor(cursor);
            replaceCount++;
        }

        cursor =
            getTextDocument()->find(replaceFormUi->replaceText->text(), cursor, getReplaceFlags());
    }
    // replaceWidget->setWidgetTransparency( 0.8 );
    replaceWidget->show();
    QMessageBox::information(nullptr, tr("Replace all"),
                             tr("%1 replacement(s) made").arg(replaceCount));
}

void TextOperationsWidget::showReplace() {
    if (!replaceWidget) {
        initReplaceWidget();
    }
    if (searchWidget && searchWidget->isVisible()) {
        searchWidget->hide();
    }
    if (gotoLineWidget && gotoLineWidget->isVisible()) {
        gotoLineWidget->hide();
    }

    QWidget *parent = qobject_cast<QWidget *>(this->parent());
    if (replaceWidget->isVisible()) {
        replaceWidget->hide();
        if (parent) {
            parent->setFocus();
        }
        return;
    }

    searchCursor = getTextCursor();
    auto s = searchCursor.selectedText();
    if (!s.isEmpty()) {
        replaceFormUi->findText->setText(s);
    }
    replaceFormUi->findText->setFocus();
    replaceFormUi->findText->selectAll();
    showBottomWidget(replaceWidget);
}

void TextOperationsWidget::showGotoLine() {
    if (!gotoLineWidget) {
        initGotoLineWidget();
    }
    if (searchWidget && searchWidget->isVisible()) {
        searchWidget->hide();
    }
    if (replaceWidget && replaceWidget->isVisible()) {
        replaceWidget->hide();
    }

    auto maxLines = document->blockCount();
    QTextCursor cursor = getTextCursor();
    gotoLineFormUi->numberSpinBox->setMaximum(maxLines);
    gotoLineFormUi->numberSpinBox->setValue(cursor.blockNumber() + 1);

    auto parent = qobject_cast<QWidget *>(this->parent());
    if (gotoLineWidget->isVisible()) {
        gotoLineWidget->hide();
        if (parent) {
            parent->setFocus();
        }
        return;
    }

    showBottomWidget(gotoLineWidget);
    gotoLineFormUi->numberSpinBox->selectAll();
    gotoLineWidget->setFocus();
    gotoLineFormUi->numberSpinBox->setFocus();
}

void TextOperationsWidget::showBottomWidget(QWidget *w) {
    if (w == nullptr) {
        if (replaceWidget && replaceWidget->isVisible()) {
            w = replaceWidget;
        } else if (searchWidget && searchWidget->isVisible()) {
            w = searchWidget;
        } else if (gotoLineWidget && gotoLineWidget->isVisible()) {
            w = gotoLineWidget;
        }
    }
    if (!w) {
        return;
    }

    auto r = QRect();
    auto parent = qobject_cast<QWidget *>(this->parent());

    // I must admit this line looks ugly, but I am open to suggestions
    if (parent->inherits("QAbstractScrollArea")) {
        parent = ((QAbstractScrollArea *)(parent))->viewport();
    }

    r = parent->rect();
    w->adjustSize();
    r.adjust(10, 0, -10, 0);
    r.setHeight(w->height());
    r.moveBottom(parent->rect().height() - 10);

    r.moveLeft(parent->pos().x() + 10);
    w->setGeometry(r);
    w->show();
}

void TextOperationsWidget::searchText_modified(QString s) {
    if (searchTimer.isActive()) {
        searchTimer.stop();
    }
    searchTimer.start();
    Q_UNUSED(s);

    // this will triggered by the timer
    // updateSearchInput();
}

void TextOperationsWidget::replaceText_modified(QString s) {
    if (replaceTimer.isActive()) {
        replaceTimer.stop();
    }
    replaceTimer.start();
    Q_UNUSED(s);

    // this will be triggered by the timer
    // updateReplaceInput();
}

bool TextOperationsWidget::issue_search(const QString &text, QTextCursor newCursor,
                                        QFlags<QTextDocument::FindFlag> findOptions,
                                        QLineEdit *lineEdit, bool moveCursor) {
    auto c = document->find(text, newCursor, findOptions);
    auto found = !c.isNull();

    // lets try again, from the start
    if (!found) {
        c.movePosition(findOptions.testFlag(QTextDocument::FindBackward) ? QTextCursor::End
                                                                         : QTextCursor::Start);
        c = document->find(text, c, findOptions);
        found = !c.isNull();
    }

    auto p = lineEdit->palette();
    if (found) {
        p.setColor(QPalette::Base, searchFoundColor);
    } else {
        if (!text.isEmpty()) {
            p.setColor(QPalette::Base, searchNotFoundColor);
        } else {
            p.setColor(QPalette::Base, lineEdit->style()->standardPalette().base().color());
        }
        c = searchCursor;
    }
    lineEdit->setPalette(p);

    if (moveCursor) {
        auto start = c.selectionStart();
        auto end = c.selectionEnd();
        c.setPosition(end, QTextCursor::MoveAnchor);
        c.setPosition(start, QTextCursor::KeepAnchor);
        setTextCursor(c);
    }

    return found;
}
