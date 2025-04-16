#include "ProjectSearch.h"
#include "GenericItems.h"
#include "ProjectBuildConfig.h"
#include "ProjectManagerPlg.h"
#include "ui_ProjectSearchGUI.h"

#include <QDirIterator>
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

ProjectSearch::ProjectSearch(QWidget *parent, ProjectBuildModel *m)
    : QWidget(parent), ui(new Ui::ProjectSearchGUI) {
    ui->setupUi(this);
    ui->searchFor->setFocus();
    this->model = m;

    QStringList headerLabels;
    headerLabels << tr("Text") << tr("Line");
    ui->treeWidget->setHeaderLabels(headerLabels);
    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->searchFor->setFocus();

    ui->pathEdit->setFileMode(false);
    ui->pathEdit->setPlaceholderText(tr("Search in directory"));
    ui->pathEdit->setPath(QDir::homePath());

    auto host = dynamic_cast<PluginManager *>(parent);
    connect(ui->treeWidget, &QTreeWidget::itemClicked, this, [host](QTreeWidgetItem *item, int) {
        auto parent = item->parent();
        if (!parent) {
            return;
        }
        auto fileName = parent->text(2);
        auto line = item->text(1).toInt() - 1;
        host->openFile(fileName, line);
        host->focusCenter();

        // TODO - this would be nice. I am unsure how to do this
        // editor->displayBannerMessage("Loaded", 7);
    });

    connect(ui->sourceCombo, &QComboBox::activated, this, [this](int index) {
        if (index <= 0) {
            this->ui->pathEdit->blockSignals(true);
            auto static lastDir = QDir::homePath();
            auto lastDir2 = this->ui->pathEdit->path();
            this->ui->pathEdit->setPath(lastDir);
            lastDir = lastDir2;
            this->ui->pathEdit->blockSignals(false);
            return;
        }

        // else - use path from the selected project
        auto project = model->getConfig(index - 1);
        this->ui->pathEdit->setPath(project->sourceDir);
    });

    connect(ui->searchButton, &QPushButton::clicked, this, &ProjectSearch::searchButton_clicked);

    connect(ui->pathEdit, &PathWidget::pathChanged, this, [this](const QString &newPath) {
        auto i = this->model->findConfigDirIndex(newPath);
        if (i == -1) {
            this->ui->sourceCombo->setCurrentIndex(0);
        } else {
            this->ui->sourceCombo->setCurrentIndex(i + 1);
        }
    });
}

ProjectSearch::~ProjectSearch() { delete ui; }

void ProjectSearch::setFocusOnSearch() { ui->searchFor->setFocus(); }

const QString ProjectSearch::getSearchPattern() { return ui->searchFor->text(); }

void ProjectSearch::setSearchPattern(const QString &s) { ui->searchFor->setText(s); }

const QString ProjectSearch::getSearchInclude() { return ui->includeFiles->text(); }

void ProjectSearch::setSearchInclude(const QString &s) { ui->includeFiles->setText(s); }

const QString ProjectSearch::getSearchExclude() { return ui->excludeFiles->text(); }

void ProjectSearch::setSearchExclude(const QString &s) { ui->excludeFiles->setText(s); }

void ProjectSearch::updateProjectList() {
    ui->sourceCombo->clear();
    ui->sourceCombo->addItem(tr("Custom"));
    for (auto i = 0; i < model->rowCount(); i++) {
        auto p = model->getConfig(i);
        ui->sourceCombo->addItem(p->sourceDir);
    }
}

void ProjectSearch::searchButton_clicked() {
    static bool running = false;

    if (running) {
        running = false;
        return;
    }

    this->ui->treeWidget->clear();

    auto allowList = ui->includeFiles->text();
    auto denyList = ui->excludeFiles->text();
    auto originalText = ui->searchButton->text();
    ui->searchButton->setText("(click to &stop)");
    ui->progressIndicator->start();
    running = true;
    QThreadPool::globalInstance()->start([this, originalText, allowList, denyList]() {
        auto text = ui->searchFor->text().toStdString();
        auto startSearchPath = ui->pathEdit->path();

        QDirIterator it(startSearchPath, allowList.split(";"), QDir::Files,
                        QDirIterator::Subdirectories);
        while (it.hasNext()) {
            auto fullFileName = it.next();
            auto *foundData = new QList<FoundData>;
            if (!fullFileName.startsWith(startSearchPath)) {
                continue;
            }

            auto trimCount = startSearchPath.size();
            if (startSearchPath.endsWith('\\') || startSearchPath.endsWith('/')) {
                trimCount++;
            }
            auto shortFileName = fullFileName.mid(trimCount);

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
                    Q_ARG(QString, shortFileName),
                    Q_ARG(QList<FoundData>*, foundData)
                );
                // clang-format on
            } else {
                delete foundData;
            }

            if (!running) {
                break;
            }
        }

        // this is done, since the progress indicator needs to be stopped from the main thread
        QTimer::singleShot(0, this, [this, originalText]() {
            ui->searchButton->setText(originalText);
            ui->progressIndicator->stop();
        });
    });
}

void ProjectSearch::file_searched(QString fullFileName, QString shortFileName,
                                  QList<FoundData> *foundData) {
    QTreeWidgetItem *dirItem = new QTreeWidgetItem(ui->treeWidget);
    dirItem->setText(0, shortFileName);
    dirItem->setText(2, fullFileName);
    dirItem->setToolTip(0, fullFileName);

    for (const auto &s : *foundData) {
        auto lineItem = new QTreeWidgetItem(dirItem);
        auto trimmedText =
            QString::fromUtf8(s.line.data(), std::min(s.line.size(), size_t{256})).trimmed();

        lineItem->setText(0, trimmedText);
        lineItem->setText(1, QString::number(s.lineNumber + 1));
        lineItem->setToolTip(0, trimmedText);
        lineItem->setToolTip(1, shortFileName);
    }
    delete foundData;
    ui->treeWidget->expandItem(dirItem);
    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
}
