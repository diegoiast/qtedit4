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
            qDebug("Found %lu: %s", lineNumner, line.c_str());
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
    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    ui->treeWidget->header()->resizeSection(1, 30);
    //    ui->treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

ProjectSearch::~ProjectSearch() { delete ui; }

void ProjectSearch::on_searchButton_clicked() {
    this->ui->treeWidget->clear();
    QtConcurrent::run([=]() {
        qDebug("Started");
        auto text = ui->searchFor->text().toStdString();

        for (auto const &s : model->fileList) {
            auto filename = s.toStdString();

            QTreeWidgetItem *fileItem = new QTreeWidgetItem(ui->treeWidget);
            fileItem->setText(0, s);
            searchFile(filename, text, [&filename, &text, fileItem](auto line, auto line_number) {
                QTreeWidgetItem *lineItem = new QTreeWidgetItem(fileItem);
                lineItem->setText(1, QString::number(line_number));
                lineItem->setText(0, QString::fromStdString(line));
            });
            if (fileItem->childCount() == 0) {
                delete fileItem;
            } else {
                QMetaObject::invokeMethod(this, "on_line_found", Qt::QueuedConnection,
                                          Q_ARG(QTreeWidgetItem *, fileItem));
            }
        }
        qDebug("done");
    });
}

void ProjectSearch::on_line_found(QTreeWidgetItem *item) {
    this->ui->treeWidget->addTopLevelItem(item);
}
