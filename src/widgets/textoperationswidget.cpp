/**
 * \file qsvtextoperationswidget.cpp
 * \brief implementation widget for searchWidget, replace, gotoline in a QTextEdit
 * \author Diego Iastrubni diegoiast@gmail.com
 */

// SPDX-License-Identifier: MIT

#include "textoperationswidget.h"
#include "ui_gotolineform.h"
#include "ui_replaceform.h"
#include "ui_searchform.h"

#include <QHBoxLayout>
#include <QLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QStyle>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextEdit>

#include <QDebug>

auto static isLightPalette() -> bool {
    QPalette palette = QApplication::palette();
    QColor windowColor = palette.color(QPalette::Window);
    return windowColor.lightness() > 128;
}

TextOperationsWidget::TextOperationsWidget(QWidget *parent, QWidget *e) : QStackedWidget(parent) {
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    setObjectName("TextOperationsWidget");

    if (isLightPalette()) {
        searchFoundBackgroundColor = QColor(0xDDDDFF);
        searchNotFoundBackgroundColor = QColor(0xFFAAAA);
    } else {
        searchFoundBackgroundColor = QColor(0x1e3a5f);
        searchNotFoundBackgroundColor = QColor(0xFF6A6A);
    }

    editor = e;

    searchTimer.setInterval(250);
    searchTimer.setSingleShot(true);
    connect(&searchTimer, &QTimer::timeout, this, &TextOperationsWidget::updateSearchInput);

    // this one is slower, to let the user think about his action
    // this is a modifying command, unlike a passive search
    replaceTimer.setInterval(400);
    replaceTimer.setSingleShot(true);
    connect(&replaceTimer, &QTimer::timeout, this, &TextOperationsWidget::updateReplaceInput);

    editor->installEventFilter(this);
    this->installEventFilter(this);
    this->searchHistory = new SharedHistoryModel(this);

    initSearchWidget();
    initReplaceWidget();
    initGotoLineWidget();

    addWidget(searchWidget);
    addWidget(replaceWidget);
    addWidget(gotoLineWidget);
}

void TextOperationsWidget::initSearchWidget() {
    searchWidget = new QWidget(this);
    searchWidget->setObjectName("searchWidget");
    searchFormUi = new Ui::searchForm();
    searchFormUi->setupUi(searchWidget);
    searchFormUi->searchText->setHistoryModel(searchHistory);

    connect(searchFormUi->searchText, &QLineEdit::textChanged, this,
            &TextOperationsWidget::searchText_modified);
    connect(searchFormUi->nextButton, &QAbstractButton::clicked, this,
            &TextOperationsWidget::searchNext);
    connect(searchFormUi->previousButton, &QAbstractButton::clicked, this,
            &TextOperationsWidget::searchPrevious);
    connect(searchFormUi->closeButton, &QAbstractButton::clicked, this,
            &TextOperationsWidget::showSearch);
    connect(searchFormUi->searchText, &QLineEdit::textChanged, this,
            &TextOperationsWidget::searchText_modified);
}

void TextOperationsWidget::initReplaceWidget() {
    replaceWidget = new QWidget(this);
    replaceWidget->setObjectName("replaceWidget");
    replaceFormUi = new Ui::replaceForm();
    replaceFormUi->setupUi(replaceWidget);
    replaceFormUi->optionsGroupBox->hide();
    replaceFormUi->searchText->setHistoryModel(searchHistory);
    replaceFormUi->replaceText->setHistoryModel(searchHistory);

    connect(replaceFormUi->searchText, &QLineEdit::textChanged, this,
            &TextOperationsWidget::replaceText_modified);
    connect(replaceFormUi->replaceButton, &QAbstractButton::clicked, this,
            &TextOperationsWidget ::replaceOldText_returnPressed);
    connect(replaceFormUi->closeButton, &QAbstractButton::clicked, this,
            &TextOperationsWidget::showReplace);
    connect(replaceFormUi->replaceText, &QLineEdit::textChanged, this,
            &TextOperationsWidget::replaceText_modified);
    connect(replaceFormUi->searchText, &QLineEdit::textChanged, this,
            &TextOperationsWidget::replaceText_modified);
    connect(replaceFormUi->replaceText, &QLineEdit::returnPressed, this,
            &TextOperationsWidget::replaceOldText_returnPressed);
    connect(replaceFormUi->searchText, &QLineEdit::returnPressed, this,
            &TextOperationsWidget::replaceOldText_returnPressed);
}

void TextOperationsWidget::initGotoLineWidget() {
    gotoLineWidget = new QWidget(this);
    gotoLineWidget->setObjectName("gotoLineWidget");
    gotoLineFormUi = new Ui::gotoLineForm();
    gotoLineFormUi->setupUi(gotoLineWidget);
    gotoLineWidget->adjustSize();
    gotoLineWidget->hide();

    connect(gotoLineFormUi->numberSpinBox, &QAbstractSpinBox::editingFinished, this, [this]() {
        auto document = getTextDocument();
        auto line_number = gotoLineFormUi->numberSpinBox->value();
        auto block = document->findBlockByNumber(line_number - 1);
        auto cursor = QTextCursor(block);
        setTextCursor(cursor);
    });

    connect(gotoLineFormUi->closeButton, &QAbstractButton::clicked, this,
            &TextOperationsWidget::showGotoLine);
}

void TextOperationsWidget::setSearchHistory(SharedHistoryModel *model) {
    searchHistory = model;
    searchFormUi->searchText->setHistoryModel(searchHistory);
    replaceFormUi->searchText->setHistoryModel(searchHistory);
    replaceFormUi->replaceText->setHistoryModel(searchHistory);
}

void TextOperationsWidget::searchNext() {
    issueSearch(searchFormUi->searchText->text(), getTextCursor(),
                getSearchFlags() & ~QTextDocument::FindBackward, searchFormUi->searchText, true);
}

void TextOperationsWidget::searchPrevious() {
    issueSearch(searchFormUi->searchText->text(), getTextCursor(),
                getSearchFlags() | QTextDocument::FindBackward, searchFormUi->searchText, true);
}

void TextOperationsWidget::updateSearchInput() {
    issueSearch(searchFormUi->searchText->text(), searchCursor, getSearchFlags(),
                searchFormUi->searchText, true);
}

void TextOperationsWidget::updateReplaceInput() {
    issueSearch(replaceFormUi->searchText->text(), searchCursor, getReplaceFlags(),
                replaceFormUi->searchText, true);
}

bool TextOperationsWidget::eventFilter(QObject *obj, QEvent *event) {
    if (obj != editor && obj != this) {
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
        if (searchWidget && searchFormUi->searchText->hasFocus()) {
            if (keyEvent->modifiers().testFlag(Qt::ControlModifier) ||
                keyEvent->modifiers().testFlag(Qt::AltModifier) ||
                keyEvent->modifiers().testFlag(Qt::ShiftModifier)) {
                searchPrevious();
            } else {
                searchNext();
            }
            return true;
        } else if (replaceWidget && replaceFormUi->searchText->hasFocus()) {
            if (keyEvent->modifiers().testFlag(Qt::ControlModifier) ||
                keyEvent->modifiers().testFlag(Qt::AltModifier) ||
                keyEvent->modifiers().testFlag(Qt::ShiftModifier)) {
                replaceAll_clicked();
            } else {
                replaceOldText_returnPressed();
            }
            return true;
        } else if (gotoLineWidget && gotoLineFormUi->numberSpinBox->hasFocus()) {
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
                replaceFormUi->searchText->setFocus();
                replaceFormUi->searchText->selectAll();
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
    if (auto textEdit = qobject_cast<QTextEdit *>(editor)) {
        cursor = textEdit->textCursor();
    } else if (auto plainTextEdit = qobject_cast<QPlainTextEdit *>(editor)) {
        cursor = plainTextEdit->textCursor();
    }

    return cursor;
}

void TextOperationsWidget::setTextCursor(QTextCursor cursor) {
    if (auto textEdit = qobject_cast<QTextEdit *>(editor)) {
        textEdit->setTextCursor(cursor);
    } else if (auto plainTextEdit = qobject_cast<QPlainTextEdit *>(editor)) {
        plainTextEdit->setTextCursor(cursor);
    }
}

QTextDocument *TextOperationsWidget::getTextDocument() {
    if (auto textEdit = qobject_cast<QTextEdit *>(editor)) {
        return textEdit->document();
    } else if (auto plainTextEdit = qobject_cast<QPlainTextEdit *>(editor)) {
        return plainTextEdit->document();
    }
    return {};
}

void TextOperationsWidget::setTextFont(const QFont &newFont) {
    searchFormUi->searchText->setFont(newFont);
    replaceFormUi->searchText->setFont(newFont);
    replaceFormUi->replaceText->setFont(newFont);
}

QSize TextOperationsWidget::sizeHint() const {
    if (currentWidget()) {
        return currentWidget()->sizeHint();
    }
    return QStackedWidget::sizeHint();
}

QSize TextOperationsWidget::minimumSizeHint() const {
    if (currentWidget()) {
        return currentWidget()->minimumSizeHint();
    }
    return QStackedWidget::minimumSizeHint();
}

void TextOperationsWidget::showSearch() {
    if (currentIndex() == 0) {
        if (isVisible()) {
            hide();
            editor->setFocus();
            return;
        }
    }
    searchWidget->adjustSize();
    adjustSize();
    setCurrentIndex(0);
    show();
    setFocus();
    searchCursor = getTextCursor();
    auto s = searchCursor.selectedText();
    if (!s.isEmpty()) {
        searchFormUi->searchText->setText(s);
    }
    searchFormUi->searchText->setFocus();
    searchFormUi->searchText->selectAll();
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
    cursor = doc->find(replaceFormUi->searchText->text(), cursor, getReplaceFlags());
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
    auto replaceCount = 0;
    auto cursor = getTextCursor();
    auto text = replaceFormUi->searchText->text();
    cursor = getTextDocument()->find(text, cursor, getReplaceFlags());

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
        cursor = getTextDocument()->find(text, cursor, getReplaceFlags());
    }
    QMessageBox::information(nullptr, tr("Replace all"),
                             tr("%1 replacement(s) made").arg(replaceCount));
}

void TextOperationsWidget::showReplace() {
    if (currentIndex() == 1) {
        if (isVisible()) {
            hide();
            editor->setFocus();
            return;
        }
    }
    replaceWidget->adjustSize();
    adjustSize();
    setCurrentIndex(1);
    show();
    setFocus();
    searchCursor = getTextCursor();
    auto s = searchCursor.selectedText();
    if (!s.isEmpty()) {
        replaceFormUi->searchText->setText(s);
    }
    replaceFormUi->searchText->setFocus();
    replaceFormUi->searchText->selectAll();
}

void TextOperationsWidget::showGotoLine() {
    if (currentIndex() == 2) {
        if (isVisible()) {
            hide();
            editor->setFocus();
            return;
        }
    }

    gotoLineWidget->adjustSize();
    adjustSize();
    setCurrentIndex(2);
    show();
    setFocus();
    auto maxLines = getTextDocument()->blockCount();
    auto cursor = getTextCursor();
    gotoLineFormUi->numberSpinBox->setMaximum(maxLines);
    gotoLineFormUi->numberSpinBox->setValue(cursor.blockNumber() + 1);
    gotoLineFormUi->numberSpinBox->selectAll();
    gotoLineWidget->setFocus();
    gotoLineFormUi->numberSpinBox->setFocus();
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

bool TextOperationsWidget::issueSearch(const QString &text, QTextCursor newCursor,
                                       QFlags<QTextDocument::FindFlag> findOptions,
                                       QLineEdit *lineEdit, bool moveCursor) {
    auto document = getTextDocument();
    if (!document) {
        return false;
    }
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
        p.setColor(QPalette::Base, searchFoundBackgroundColor);
    } else {
        if (!text.isEmpty()) {
            p.setColor(QPalette::Base, searchNotFoundBackgroundColor);
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
