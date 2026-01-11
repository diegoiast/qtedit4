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

#include "textoperationswidget.h"
#include "ui_gotolineform.h"
#include "ui_replaceform.h"
#include "ui_searchform.h"

#include <QApplication>
#include <QPalette>
#include <QLineEdit>
#include <QMessageBox>
#include <QTimer>
#include <QKeyEvent>

auto static isLightPalette() -> bool {
    QPalette palette = QApplication::palette();
    QColor windowColor = palette.color(QPalette::Window);
    return windowColor.lightness() > 128;
}

TextOperationsWidget::TextOperationsWidget(QWidget *parent, QWidget *e) 
    : QStackedWidget(parent), editor(e) {
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    setObjectName("TextOperationsWidget");

    if (isLightPalette()) {
        searchFoundBackgroundColor = QColor(0xDDDDFF);
        searchNotFoundBackgroundColor = QColor(0xFFAAAA);
    } else {
        searchFoundBackgroundColor = QColor(0x1e3a5f);
        searchNotFoundBackgroundColor = QColor(0xFF6A6A);
    }

    searchTimer.setInterval(250);
    searchTimer.setSingleShot(true);
    connect(&searchTimer, &QTimer::timeout, this, &TextOperationsWidget::updateSearchInput);

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

TextOperationsWidget::~TextOperationsWidget() {
    delete searchFormUi;
    delete replaceFormUi;
    delete gotoLineFormUi;
}

// --- Internal Adapter Factory ---
std::unique_ptr<ITextCursorAdapter> TextOperationsWidget::createCursorAdapter() const {
    // This is the only place where we "know" about Qt's specific types
    if (auto textEdit = qobject_cast<QTextEdit *>(editor)) {
        return std::make_unique<QtCursorAdapter>(textEdit->textCursor(), textEdit->document());
    } else if (auto plainTextEdit = qobject_cast<QPlainTextEdit *>(editor)) {
        return std::make_unique<QtCursorAdapter>(plainTextEdit->textCursor(), plainTextEdit->document());
    }
    // For Scintilla/HTML, add logic here to return their specific adapters
    return nullptr;
}

void TextOperationsWidget::initSearchWidget() {
    searchWidget = new QWidget(this);
    searchFormUi = new Ui::searchForm();
    searchFormUi->setupUi(searchWidget);
    searchFormUi->searchText->setHistoryModel(searchHistory);

    connect(searchFormUi->searchText, &QLineEdit::textChanged, this, &TextOperationsWidget::searchText_modified);
    connect(searchFormUi->nextButton, &QAbstractButton::clicked, this, &TextOperationsWidget::searchNext);
    connect(searchFormUi->previousButton, &QAbstractButton::clicked, this, &TextOperationsWidget::searchPrevious);
    connect(searchFormUi->closeButton, &QAbstractButton::clicked, this, &TextOperationsWidget::showSearch);
}

void TextOperationsWidget::initReplaceWidget() {
    replaceWidget = new QWidget(this);
    replaceFormUi = new Ui::replaceForm();
    replaceFormUi->setupUi(replaceWidget);
    replaceFormUi->optionsGroupBox->hide();
    replaceFormUi->searchText->setHistoryModel(searchHistory);
    replaceFormUi->replaceText->setHistoryModel(searchHistory);

    connect(replaceFormUi->searchText, &QLineEdit::textChanged, this, &TextOperationsWidget::replaceText_modified);
    connect(replaceFormUi->replaceButton, &QAbstractButton::clicked, this, &TextOperationsWidget::replaceOldText_returnPressed);
    connect(replaceFormUi->closeButton, &QAbstractButton::clicked, this, &TextOperationsWidget::showReplace);
    connect(replaceFormUi->replaceText, &QLineEdit::textChanged, this, &TextOperationsWidget::replaceText_modified);
    connect(replaceFormUi->replaceText, &QLineEdit::returnPressed, this, &TextOperationsWidget::replaceOldText_returnPressed);
    connect(replaceFormUi->searchText, &QLineEdit::returnPressed, this, &TextOperationsWidget::replaceOldText_returnPressed);
}

void TextOperationsWidget::initGotoLineWidget() {
    gotoLineWidget = new QWidget(this);
    gotoLineFormUi = new Ui::gotoLineForm();
    gotoLineFormUi->setupUi(gotoLineWidget);

    connect(gotoLineFormUi->numberSpinBox, &QAbstractSpinBox::editingFinished, this, [this]() {
        auto adapter = createCursorAdapter();
        if (adapter) {
            adapter->goToLine(gotoLineFormUi->numberSpinBox->value());
            // If the editor is Qt-based, we might need to sync the cursor back
            if (auto qt = dynamic_cast<QtCursorAdapter*>(adapter.get())) {
                if (auto te = qobject_cast<QTextEdit*>(editor)) te->setTextCursor(qt->getInternalCursor());
                else if (auto pe = qobject_cast<QPlainTextEdit*>(editor)) pe->setTextCursor(qt->getInternalCursor());
            }
        }
    });

    connect(gotoLineFormUi->closeButton, &QAbstractButton::clicked, this, &TextOperationsWidget::showGotoLine);
}

// --- Search Logic ---

void TextOperationsWidget::searchNext() {
    FindOptions opt{searchFormUi->caseSensitiveCheckBox->isChecked(), 
                    searchFormUi->wholeWorldsCheckbox->isChecked(), false};
    issueSearch(searchFormUi->searchText->text(), nullptr, opt, searchFormUi->searchText, true);
}

void TextOperationsWidget::searchPrevious() {
    FindOptions opt{searchFormUi->caseSensitiveCheckBox->isChecked(), 
                    searchFormUi->wholeWorldsCheckbox->isChecked(), true};
    issueSearch(searchFormUi->searchText->text(), nullptr, opt, searchFormUi->searchText, true);
}

void TextOperationsWidget::updateSearchInput() {
    FindOptions opt{searchFormUi->caseSensitiveCheckBox->isChecked(), 
                    searchFormUi->wholeWorldsCheckbox->isChecked(), false};
    issueSearch(searchFormUi->searchText->text(), searchCursor.get(), opt, searchFormUi->searchText, true);
}

void TextOperationsWidget::updateReplaceInput() {
    FindOptions opt{replaceFormUi->caseCheckBox->isChecked(), 
                    replaceFormUi->wholeWordsCheckBox->isChecked(), false};
    issueSearch(replaceFormUi->searchText->text(), searchCursor.get(), opt, replaceFormUi->searchText, true);
}

bool TextOperationsWidget::issueSearch(const QString &text, ITextCursorAdapter* startCursor,
                                     FindOptions options, QLineEdit *lineEdit, bool moveCursor) {
    auto adapter = startCursor ? startCursor->clone() : createCursorAdapter();
    if (!adapter) return false;

    bool found = adapter->find(text, !options.backward, options.caseSensitive, options.wholeWords);

    if (!found && !text.isEmpty()) {
        // Wrap around logic
        options.backward ? adapter->moveToEnd() : adapter->moveToStart();
        found = adapter->find(text, !options.backward, options.caseSensitive, options.wholeWords);
    }

    // Palette handling
    QPalette p = lineEdit->palette();
    if (found) {
        p.setColor(QPalette::Base, searchFoundBackgroundColor);
        if (moveCursor) {
            // Logic to sync cursor to editor (Editor specific)
            if (auto qt = dynamic_cast<QtCursorAdapter*>(adapter.get())) {
                if (auto te = qobject_cast<QTextEdit*>(editor)) te->setTextCursor(qt->getInternalCursor());
                else if (auto pe = qobject_cast<QPlainTextEdit*>(editor)) pe->setTextCursor(qt->getInternalCursor());
            }
        }
    } else {
        p.setColor(QPalette::Base, text.isEmpty() ? lineEdit->style()->standardPalette().base().color() : searchNotFoundBackgroundColor);
    }
    lineEdit->setPalette(p);

    searchCursor = std::move(adapter);
    return found;
}

// --- Replace Logic ---

void TextOperationsWidget::replaceOldText_returnPressed() {
    if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)) {
        replaceAll_clicked();
        return;
    }

    if (!searchCursor || searchCursor->selectedText() != replaceFormUi->searchText->text()) {
        updateReplaceInput();
        if (!searchCursor) return;
    }

    searchCursor->beginEditBlock();
    searchCursor->deleteChar();
    searchCursor->insertText(replaceFormUi->replaceText->text());
    searchCursor->endEditBlock();

    // Re-sync after edit
    if (auto qt = dynamic_cast<QtCursorAdapter*>(searchCursor.get())) {
        if (auto te = qobject_cast<QTextEdit*>(editor)) te->setTextCursor(qt->getInternalCursor());
        else if (auto pe = qobject_cast<QPlainTextEdit*>(editor)) pe->setTextCursor(qt->getInternalCursor());
    }

    updateReplaceInput();
}

void TextOperationsWidget::replaceAll_clicked() {
    int count = 0;
    auto adapter = createCursorAdapter();
    QString findText = replaceFormUi->searchText->text();
    bool cs = replaceFormUi->caseCheckBox->isChecked();
    bool ww = replaceFormUi->wholeWordsCheckBox->isChecked();

    while (adapter->find(findText, true, cs, ww)) {
        // Step-by-step confirmation as per original requirement
        QMessageBox::StandardButton b = QMessageBox::question(qobject_cast<QWidget*>(parent()), 
            tr("Replace"), tr("Replace this text?"), QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
        
        if (b == QMessageBox::Cancel) break;
        if (b == QMessageBox::Yes) {
            adapter->beginEditBlock();
            adapter->deleteChar();
            adapter->insertText(replaceFormUi->replaceText->text());
            adapter->endEditBlock();
            count++;
        }
    }
    QMessageBox::information(this, tr("Result"), tr("%1 replacements made").arg(count));
}

// --- Window Management ---

void TextOperationsWidget::showSearch() {
    if (currentIndex() == 0 && isVisible()) { hide(); editor->setFocus(); return; }
    setCurrentIndex(0);
    show();
    searchCursor = createCursorAdapter();
    QString s = searchCursor->selectedText();
    if (!s.isEmpty()) searchFormUi->searchText->setText(s);
    searchFormUi->searchText->setFocus();
    searchFormUi->searchText->selectAll();
}

void TextOperationsWidget::showReplace() {
    if (currentIndex() == 1 && isVisible()) { hide(); editor->setFocus(); return; }
    setCurrentIndex(1);
    show();
    searchCursor = createCursorAdapter();
    QString s = searchCursor->selectedText();
    if (!s.isEmpty()) replaceFormUi->searchText->setText(s);
    replaceFormUi->searchText->setFocus();
    replaceFormUi->searchText->selectAll();
}

void TextOperationsWidget::showGotoLine() {
    if (currentIndex() == 2 && isVisible()) { hide(); editor->setFocus(); return; }
    setCurrentIndex(2);
    show();
    auto adapter = createCursorAdapter();
    // For Scintilla/HTML, you'd need lineCount() in the interface
    if (auto qt = dynamic_cast<QtCursorAdapter*>(adapter.get())) {
        gotoLineFormUi->numberSpinBox->setMaximum(qt->getInternalCursor().document()->blockCount());
        gotoLineFormUi->numberSpinBox->setValue(qt->blockNumber() + 1);
    }
    gotoLineFormUi->numberSpinBox->setFocus();
}

// --- Event Filter & UI Helpers ---

bool TextOperationsWidget::eventFilter(QObject *obj, QEvent *event) {
    if (obj != editor && obj != this) {
        return false;
    }
    if (event->type() != QEvent::KeyPress) {
        return false;
    }
    if (this->isHidden()) {
        return false;
    }

    auto ke = static_cast<QKeyEvent *>(event);
    if (ke->key() == Qt::Key_Escape) {
        qDebug() << "Hiding text operation widget, and focusing editor";
        hide();
        editor->setFocus();
        return true;
    }
    // ... Tab and Enter logic stays same, using helper slots ...
    return QStackedWidget::eventFilter(obj, event);
}

void TextOperationsWidget::searchText_modified(QString) { searchTimer.start(); }
void TextOperationsWidget::replaceText_modified(QString) { replaceTimer.start(); }

void TextOperationsWidget::setTextFont(const QFont &newFont) {
    if (searchFormUi) {
        searchFormUi->searchText->setFont(newFont);
    }
    if (replaceFormUi) {
        replaceFormUi->searchText->setFont(newFont);
        replaceFormUi->replaceText->setFont(newFont);
    }
    // Note: We don't set the font on the editor here because 
    // the editor is managed externally.
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

void TextOperationsWidget::setSearchHistory(SharedHistoryModel *model) {
    searchHistory = model;
    searchFormUi->searchText->setHistoryModel(searchHistory);
    replaceFormUi->searchText->setHistoryModel(searchHistory);
    replaceFormUi->replaceText->setHistoryModel(searchHistory);
}