#pragma once

#include <QAbstractItemModel>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QIcon>
#include <QStackedWidget>

class QTextBrowser;
class QLabel;
class QTreeView;

class TextPreview : public QStackedWidget {
  public:
    explicit TextPreview(QWidget *p);
    enum PreviewType {
        Markdown,
        SVG,
        XPM,
        JSON,
        XML,
        // YML
        // TOML
    };

    auto previewText(const QString &filename, const QString &str, PreviewType type) -> void;

  private:
    QTextBrowser *markdownPreview = nullptr;
    QLabel *imagePreview = nullptr;
    QTreeView *treeView = nullptr;
};
