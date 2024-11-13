#include "textpreview.h"
#include "xmltreemodel.h"

#include <QAbstractItemModel>
#include <QHeaderView>
#include <QJsonModel.hpp>
#include <QPainter>
#include <QScrollBar>
#include <QSvgRenderer>
#include <QTextBrowser>
#include <QTreeView>
#include <QVector>
#include <pal/image-viewer.h>

TextPreview::TextPreview(QWidget *p) : QStackedWidget(p) {
    markdownPreview = new QTextBrowser(this);
    imagePreview = new pal::ImageViewer(this);
    treeView = new QTreeView(this);
    imagePreview->zoomOriginal();

    addWidget(markdownPreview);
    addWidget(imagePreview);
    addWidget(treeView);
}

auto TextPreview::previewText(const QString &filename, const QString &str, PreviewType type)
    -> void {
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
        imagePreview->setImage(pixmap.toImage());
        break;
    }
    case XPM: {
        setCurrentIndex(1);
        QPixmap pixmap;
        pixmap.loadFromData(str.toUtf8());
        imagePreview->setImage(pixmap.toImage());
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
