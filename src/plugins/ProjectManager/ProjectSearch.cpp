#include "ProjectSearch.h"
#include "GenericItems.h"
#include "ui_ProjectSearchGUI.h"

#include <QPushButton>
#include <QThreadPool>
#include <fstream>
#include <functional>
#include <pluginmanager.h>
#include <qmdihost.h>
#include <string>

void searchFile(const std::string &filename, const std::string &searchString,
                std::function<void(const std::string &, size_t)> callback) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return;
    }

    auto line = std::string();
    auto lineNumner = size_t(0);

    while (std::getline(file, line)) {
        if (line.find(searchString) != std::string::npos) {
            callback(line, lineNumner);
        }
        lineNumner++;
    }
}

ProjectSearch::ProjectSearch(QWidget *parent, DirectoryModel *m)
    : QWidget(parent), ui(new Ui::ProjectSearchGUI) {
    ui->setupUi(this);
    ui->searchFor->setFocus();
    this->model = m;

    QStringList headerLabels;
    headerLabels << tr("Text") << tr("Line");
    ui->treeWidget->setHeaderLabels(headerLabels);
    //    ui->treeWidget->header()->resizeSection(1, 30);
    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->searchFor->setFocus();
    connect(ui->searchButton, &QPushButton::clicked, this, &ProjectSearch::searchButton_clicked);

    auto host = dynamic_cast<PluginManager *>(parent);
    QObject::connect(ui->treeWidget, &QTreeWidget::itemClicked, this,
                     [host](QTreeWidgetItem *item, int column) {
                         auto parent = item->parent();
                         if (!parent) {
                             return;
                         }
                         auto fileName = parent->text(2);
                         auto line = item->text(1).toInt();
                         host->openFile(fileName, line);
                         host->focusCenter();

                         // TODO - this would be nice. I am unsure how to do this
                         // editor->displayBannerMessage("Loaded", 7);
                         Q_UNUSED(column);
                     });
}

ProjectSearch::~ProjectSearch() { delete ui; }

void ProjectSearch::setFocusOnSearch() { ui->searchFor->setFocus(); }

const QString ProjectSearch::getSearchPattern() { return ui->searchFor->text(); }

void ProjectSearch::setSearchPattern(const QString s) { ui->searchFor->setText(s); }

const QString ProjectSearch::getSearchInclude() { return ui->includeFiles->text(); }

void ProjectSearch::setSearchInclude(const QString s) { ui->includeFiles->setText(s); }

const QString ProjectSearch::getSearchExclude() { return ui->excludeFiles->text(); }

void ProjectSearch::setSearchExclude(const QString s) { ui->excludeFiles->setText(s); }

void ProjectSearch::searchButton_clicked() {
    this->ui->treeWidget->clear();
    auto allowList = ui->includeFiles->text();
    auto denyList = ui->excludeFiles->text();
    QThreadPool::globalInstance()->start([this, allowList, denyList]() {
        auto text = ui->searchFor->text().toStdString();

        for (auto const &fullFileName : std::as_const(model->fileList)) {
            auto shortFilename = fullFileName;

            for (auto &d : model->directoryList) {
                if (!fullFileName.startsWith(d)) {
                    continue;
                }
                // also trim the trailing slash
                shortFilename = fullFileName.mid(d.size() + 1);
                break;
            }

            auto *foundData = new QList<FoundData>;
            if (!FilenameMatches(fullFileName, allowList, denyList)) {
                continue;
            };
            searchFile(fullFileName.toStdString(), text, [foundData](auto line, auto line_number) {
                foundData->push_back({line, line_number});
            });

            if (!foundData->empty()) {
                // clang-format off
                QMetaObject::invokeMethod(
                    this, "file_searched", Qt::QueuedConnection,
                    Q_ARG(QString, fullFileName),
                    Q_ARG(QString, shortFilename),
                    Q_ARG(QList<FoundData>*, foundData)
                );
                // clang-format on
            } else {
                delete foundData;
            }
        }
    });
}

void ProjectSearch::file_searched(QString fullFileName, QString shortFileName,
                                  QList<FoundData> *foundData) {
    QTreeWidgetItem *dirItem = new QTreeWidgetItem(ui->treeWidget);
    dirItem->setText(0, shortFileName);
    dirItem->setText(2, fullFileName);
    dirItem->setToolTip(0, fullFileName);

    for (const auto &s : *foundData) {
        QTreeWidgetItem *lineItem = new QTreeWidgetItem(dirItem);
        lineItem->setText(0, QString::fromStdString(s.line));
        lineItem->setText(1, QString::number(s.lineNumber));
        lineItem->setToolTip(0, shortFileName);
        lineItem->setToolTip(1, shortFileName);
    }
    delete foundData;
    ui->treeWidget->expandItem(dirItem);
    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    //    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
}
