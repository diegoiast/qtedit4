#include "ProjectSearch.h"
#include "GenericItems.h"
#include "QtConcurrent/QtConcurrent"
#include "ui_ProjectSearchGUI.h"

#include <fstream>
#include <functional>
#include <string>

void searchFile(const std::string &filename, const std::string &searchString,
                std::function<void(const std::string &, size_t)> callback) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        //        std::cerr << "Error: Unable to open file: " << filename << std::endl;
        return;
    }

    auto line = std::string();
    auto lineNumner = size_t(0);

    while (std::getline(file, line)) {
        lineNumner++;
        if (line.find(searchString) != std::string::npos) {
            //            qDebug("Found %lu: %s", lineNumner, line.c_str());
            callback(line, lineNumner);
        }
    }
}

ProjectSearch::ProjectSearch(QWidget *parent, DirectoryModel *m)
    : QWidget(parent), ui(new Ui::ProjectSearchGUI) {
    ui->setupUi(this);
    this->model = m;

    QStringList headerLabels;
    headerLabels << tr("Text") << tr("Line");
    ui->treeWidget->setHeaderLabels(headerLabels);
    //    ui->treeWidget->header()->resizeSection(1, 30);
    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->searchFor->setFocus();
}

ProjectSearch::~ProjectSearch() { delete ui; }

void ProjectSearch::on_searchButton_clicked() {
    this->ui->treeWidget->clear();

    auto allowList = ui->includeFiles->text();
    auto denyList = ui->excludeFiles->text();
    QtConcurrent::run([=]() {
        auto text = ui->searchFor->text().toStdString();

        for (auto const &fullFileName : model->fileList) {
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
            searchFile(shortFilename.toStdString(), text, [foundData](auto line, auto line_number) {
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
                                  QList<FoundData> *data) {
    QTreeWidgetItem *dirItem = new QTreeWidgetItem(ui->treeWidget);
    dirItem->setText(0, shortFileName);
    dirItem->setText(2, fullFileName);
    dirItem->setToolTip(0, fullFileName);

    for (auto s : *data) {
        QTreeWidgetItem *lineItem = new QTreeWidgetItem(dirItem);
        lineItem->setText(0, QString::fromStdString(s.line));
        lineItem->setText(1, QString::number(s.lineNumber));
        lineItem->setToolTip(0, shortFileName);
        lineItem->setToolTip(1, shortFileName);
    }
    delete data;
    ui->treeWidget->expandItem(dirItem);
    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    //    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
}
