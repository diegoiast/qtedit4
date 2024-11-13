#include "textpreview.h"
#include "xmltreemodel.h"

#include <QAbstractItemModel>
#include <QHeaderView>
#include <QJsonModel.hpp>
#include <QLabel>
#include <QPainter>
#include <QSvgRenderer>
#include <QTextBrowser>
#include <QScrollBar>>
#include <QTreeView>
#include <QVector>

TextPreview::TextPreview(QWidget *p) : QStackedWidget(p) {
    markdownPreview = new QTextBrowser(this);
    imagePreview = new QLabel(this);
    treeView = new QTreeView(this);

    imagePreview->setAlignment(Qt::AlignCenter);
    addWidget(markdownPreview);
    addWidget(imagePreview);
    addWidget(treeView);
}

auto TextPreview::previewText(const QString &filename, const QString &str, PreviewType type) -> void {
    switch (type) {
    case Markdown: {
        auto scrollBar = markdownPreview->verticalScrollBar();
        auto scrollPosition = scrollBar->value();

        setCurrentIndex(0);
        markdownPreview->setSource(filename, QTextDocument::MarkdownResource);
        markdownPreview->setMarkdown(str);
        scrollBar->setValue(scrollPosition);
        break;
    }
    case SVG: {
        setCurrentIndex(1);
        QSvgRenderer renderer(str.toUtf8());
        QPixmap pixmap(renderer.defaultSize());
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        renderer.render(&painter);
        imagePreview->setPixmap(pixmap);
        break;
    }
    case XPM: {
        setCurrentIndex(1);
        QPixmap pixmap;
        pixmap.loadFromData(str.toUtf8());
        imagePreview->setPixmap(pixmap);
        break;
    }
    case JSON: {
        setCurrentIndex(2);
        auto model = new QJsonModel(this);
        model->loadJson(str.toUtf8());
        treeView->setModel(model);
        treeView->expandAll();
        treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        break;
    }
    case XML:
        setCurrentIndex(2);
        auto model = new XmlTreeModel(str, this);
        treeView->setModel(model);
        treeView->expandAll();
        treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        break;
    }
}
