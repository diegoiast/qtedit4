#include <algorithm>
#include <cstring>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

#include <QDirIterator>
#include <QPushButton>
#include <QThreadPool>

#include <pluginmanager.h>
#include <qmdihost.h>

#include "AnsiToHTML.hpp"
#include "GenericItems.h"
#include "ProjectBuildConfig.h"
#include "ProjectManagerPlg.h"
#include "ProjectSearch.h"
#include "ui_ProjectSearchGUI.h"

void searchTextFile(std::ifstream &file, const std::string &searchString,
                    std::function<void(const std::string &, size_t)> callback) {
    auto line = std::string();
    auto lineNumber = size_t(0);

    while (std::getline(file, line)) {
        if (line.find(searchString) != std::string::npos) {
            callback(line, lineNumber);
        }
        lineNumber++;
    }
}

auto searchBinaryFile(std::ifstream &file, const std::string &searchString,
                      std::function<void(const std::string &, size_t)> callback) -> void {
    if (!file.is_open() || searchString.empty()) {
        return;
    }

    constexpr size_t bufferSize = 4096;
    const size_t searchLen = searchString.size();
    const size_t overlap = searchLen > 1 ? searchLen - 1 : 0;

    auto buffer = std::vector<char>(bufferSize + overlap);
    auto fileOffset = size_t{0};
    auto prevTailSize = size_t{0};

    while (file) {
        file.read(buffer.data() + prevTailSize, bufferSize);
        auto bytesRead = file.gcount();

        if (bytesRead == 0) {
            break;
        }

        auto totalBytes = prevTailSize + bytesRead;
        for (auto i = 0; i + searchLen <= totalBytes; ++i) {
            if (std::memcmp(buffer.data() + i, searchString.data(), searchLen) == 0) {
                auto afterMatch = std::min<size_t>(100, totalBytes - (i + searchLen));
                auto result = std::string(buffer.data() + i + searchLen, afterMatch);

                if (afterMatch < 100) {
                    auto extra = std::vector<char>(100 - afterMatch);
                    auto oldPos = file.tellg();
                    file.seekg(fileOffset + i + searchLen + afterMatch, std::ios::beg);
                    file.read(extra.data(), extra.size());

                    auto extraRead = file.gcount();
                    result.append(extra.data(), extraRead);
                    file.seekg(oldPos);
                }
                callback(result, fileOffset + i);
            }
        }

        if (overlap > 0) {
            std::memmove(buffer.data(), buffer.data() + totalBytes - overlap, overlap);
        }
        fileOffset += bytesRead;
        prevTailSize = overlap;
    }
}

auto searchFile(const std::string &filename, bool searchInBinaries, const std::string &searchString,
                std::function<void(const std::string &, size_t)> callback) -> void {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return;
    }

    auto firstLines = std::string();
    auto line = std::string();
    for (auto i = 0; i < 5; i++) {
        std::getline(file, line);
        firstLines += line;
    }

    file.seekg(0);
    if (isPlainText(QString::fromStdString(firstLines))) {
        searchTextFile(file, searchString, callback);
    } else {
        if (searchInBinaries) {
            searchBinaryFile(file, searchString, callback);
        } else {
            qDebug() << "Not searching inside file " << filename;
        }
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
        auto w = dynamic_cast<QWidget *>(host->currentClient());
        if (w) {
            w->setFocus();
        }

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
            auto searchInBinaries = ui->searchInBinaryFiles->isChecked();

            if (!FilenameMatches(fullFileName, allowList, denyList)) {
                continue;
            };
            searchFile(fullFileName.toStdString(), searchInBinaries, text,
                       [foundData](auto line, auto line_number) {
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
    auto dirItem = new QTreeWidgetItem(ui->treeWidget);
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
