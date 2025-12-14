/**
 * \file HistoryLineEdit.cpp
 * \brief Definition of a line editing with history
 * \author Diego Iastrubni diegoiast@gmail.com
 */

// SPDX-License-Identifier: MIT

#include "HistoryLineEdit.h"

#include <QStringListModel>

void SharedHistoryModel::addToHistory(const QString &entry) {
    if (!entry.isEmpty()) {
        history.removeAll(entry);
        history.append(entry);
        while (history.size() > maxHistorySize) {
            history.removeFirst();
        }
        emit historyUpdated(history);
    }
}

void SharedHistoryModel::setHistory(QStringList &strings) {
    history = strings;
    emit historyUpdated(history);
}

void SharedHistoryModel::setMaxHistorySize(int newSize) {
    auto old = maxHistorySize;
    maxHistorySize = newSize;
    while (history.size() > maxHistorySize) {
        history.removeFirst();
    }

    if (old != history.size()) {
        emit historyUpdated(history);
    }
}

HistoryLineEdit::HistoryLineEdit(QWidget *parent) : HistoryLineEdit(nullptr, parent) {}

HistoryLineEdit::HistoryLineEdit(SharedHistoryModel *sharedModel, QWidget *parent)
    : QLineEdit(parent) {
    connect(this, &QLineEdit::returnPressed, this, &HistoryLineEdit::addToHistory);
    setHistoryModel(sharedModel);
}

void HistoryLineEdit::setHistoryModel(SharedHistoryModel *m) {
    historyModel = m;
    if (historyModel) {
        completer = new QCompleter(this);
        completer->setModel(new QStringListModel(historyModel->getHistory(), completer));
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        // Use QueuedConnection to allow QCompleter to emit 'activated' before the model is updated
        connect(historyModel, &SharedHistoryModel::historyUpdated, this,
                &HistoryLineEdit::updateCompleter, Qt::QueuedConnection);
    } else {
        if (completer) {
            completer->deleteLater();
        }
        completer = nullptr;
    }
    setCompleter(completer);
}

void HistoryLineEdit::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Up) {
        navigateHistory(-1);
    } else if (event->key() == Qt::Key_Down) {
        navigateHistory(1);
    } else {
        QLineEdit::keyPressEvent(event);
    }
}

void HistoryLineEdit::addToHistory() {
    if (!historyModel) {
        return;
    }
    QString text = this->text().trimmed();
    historyModel->addToHistory(text);
}

void HistoryLineEdit::updateCompleter(const QStringList &history) {
    QStringListModel *model = qobject_cast<QStringListModel *>(completer->model());
    if (model) {
        model->setStringList(history);
    }
    historyIndex = history.size();
}

void HistoryLineEdit::navigateHistory(int direction) {
    QStringList history = historyModel->getHistory();
    if (history.isEmpty()) {
        return;
    }

    historyIndex += direction;
    if (historyIndex < 0) {
        historyIndex = 0;
    }
    if (historyIndex >= history.size()) {
        historyIndex = history.size() - 1;
    }

    this->setText(history[historyIndex]);
}
