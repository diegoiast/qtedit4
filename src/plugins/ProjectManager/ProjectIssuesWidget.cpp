#include <QDir>
#include <QFileInfo>
#include <QHeaderView>
#include <QStyledItemDelegate>
#include <qutepart/qutepart.h>
#include <qutepart/theme.h>

#include "ProjectIssuesWidget.h"
#include "pluginmanager.h"
#include "ui_ProjectIssuesWidget.h"
#include "widgets/qmdieditor.h"

auto typeToStatus(const QString &name) -> int {
    if (name.contains("error", Qt::CaseInsensitive)) {
        return Qutepart::ERROR_BIT;
    } else if (name.compare("warning", Qt::CaseInsensitive) == 0) {
        return Qutepart::WARNING_BIT;
    } else if (name.contains("info", Qt::CaseInsensitive) == 0) {
        return Qutepart::INFO_BIT;
    }
    return 0;
}

class CenteredIconDelegate : public QStyledItemDelegate {
  public:
    explicit CenteredIconDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override {
        if (index.column() == 0) { // Only for the first column
            auto icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
            if (!icon.isNull()) {
                auto iconRect = option.rect;
                auto iconSize = option.decorationSize;
                auto x = iconRect.x() + (iconRect.width() - iconSize.width()) / 2;
                auto y = iconRect.y() + (iconRect.height() - iconSize.height()) / 2;
                icon.paint(painter, x, y, iconSize.width(), iconSize.height());
                return;
            }
        }
        QStyledItemDelegate::paint(painter, option, index);
    }
};

CompileStatusModel::CompileStatusModel(QObject *parent)
    : QAbstractTableModel(parent), showWarnings(true), showErrors(true), showOthers(true) {
    headers << tr("Type") << tr("Message") << tr("Location");
}

int CompileStatusModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : filteredStatuses.size();
}

int CompileStatusModel::columnCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : headers.size();
}

QVariant CompileStatusModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= filteredStatuses.size()) {
        return {};
    }

    auto const &status = filteredStatuses.at(index.row());

    if (index.column() == 0 && role == Qt::DecorationRole) {
        auto static iconCache = QHash<int, QIcon>();
        auto iconType = typeToStatus(status.type);
        if (!iconCache.contains(iconType)) {
            iconCache[iconType] = Qutepart::iconForStatus(iconType);
        }
        return iconCache.value(iconType);
    }

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0:
            return {};
        case 1:
            return status.message;
        case 2:
            return QString("%1 (%2:%3)")
                .arg(status.displayName)
                .arg(status.row + 1)
                .arg(status.col + 1);
        default:
            return {};
        }
    }

    if (role == Qt::ToolTipRole) {
        switch (index.column()) {
        case 1:
            return status.message;
        case 2:
            return status.fileName;
        default:
            break;
        }
    }

    return {};
}

QVariant CompileStatusModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal || section >= headers.size()) {
        return {};
    }

    return headers.at(section);
}

void CompileStatusModel::sort(int column, Qt::SortOrder order) {
    beginResetModel();
    std::sort(filteredStatuses.begin(), filteredStatuses.end(),
              [column, order](const CompileStatus &s1, const CompileStatus &s2) {
                  bool lessThan;
                  switch (column) {
                  case 0: // Type column
                      lessThan = s1.type < s2.type;
                      break;
                  case 1: // Message column
                      lessThan = s1.message < s2.message;
                      break;
                  case 2: // Location column
                      lessThan = s1.fileName < s2.fileName;
                      break;
                  default:
                      return false;
                  }
                  return order == Qt::AscendingOrder ? lessThan : !lessThan;
              });
    endResetModel();
}

void CompileStatusModel::clearAll() {
    beginResetModel();
    statuses.clear();
    filteredStatuses.clear();
    endResetModel();
}

void CompileStatusModel::addItem(const CompileStatus &status) {
    beginResetModel();
    statuses.append(status);
    applyFilter();
    endResetModel();
}

CompileStatus CompileStatusModel::getItem(const QModelIndex &index) const {
    if (!index.isValid()) {
        return {};
    }
    if (index.row() < 0) {
        return {};
    }
    if (index.row() >= filteredStatuses.size()) {
        return {};
    }
    return filteredStatuses.at(index.row());
}

QList<CompileStatus> CompileStatusModel::getItemsFor(const QString &filename) const {
    auto fileStatus = QVector<CompileStatus>();
    for (const auto &status : statuses) {
        if (status.fileName == filename) {
            fileStatus.append(status);
        }
    }
    return fileStatus;
}

void CompileStatusModel::setWarningsVisible(bool visible) {
    if (showWarnings != visible) {
        showWarnings = visible;
        beginResetModel();
        applyFilter();
        endResetModel();
    }
}

void CompileStatusModel::setErrorsVisible(bool visible) {
    if (showErrors != visible) {
        showErrors = visible;
        beginResetModel();
        applyFilter();
        endResetModel();
    }
}

void CompileStatusModel::setOthersVisible(bool visible) {
    if (showOthers != visible) {
        showOthers = visible;
        beginResetModel();
        applyFilter();
        endResetModel();
    }
}

bool CompileStatusModel::areWarningsVisible() const { return showWarnings; }

bool CompileStatusModel::areErrorsVisible() const { return showErrors; }

bool CompileStatusModel::areOthersVisible() const { return showOthers; }

void CompileStatusModel::applyFilter() {
    filteredStatuses.clear();
    for (const auto &status : statuses) {
        if (shouldShowStatus(status)) {
            filteredStatuses.append(status);
        }
    }
}

bool CompileStatusModel::shouldShowStatus(const CompileStatus &status) const {
    QString lowerType = status.type.toLower();
    if (lowerType == "warning") {
        return showWarnings;
    } else if (lowerType == "error") {
        return showErrors;
    } else {
        return showOthers;
    }
}

ProjectIssuesWidget::ProjectIssuesWidget(PluginManager *parent)
    : QWidget(parent), ui(new Ui::ProjectIssuesWidget) {
    model = new CompileStatusModel(this);
    ui->setupUi(this);
    ui->issuesList->setModel(model);
    ui->issuesList->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->issuesList->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->issuesList->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    auto delegate = new CenteredIconDelegate(ui->issuesList);
    ui->issuesList->setItemDelegateForColumn(0, delegate);

    this->manager = parent;
    connect(manager, &PluginManager::newClientAdded, this, &ProjectIssuesWidget::decorateClient);
    connect(ui->errorsButton, &QPushButton::clicked, this,
            [this]() { this->model->setErrorsVisible(this->ui->errorsButton->isChecked()); });
    connect(ui->warningsButton, &QPushButton::clicked, this,
            [this]() { this->model->setWarningsVisible(this->ui->warningsButton->isChecked()); });
    connect(ui->othersButton, &QPushButton::clicked, this,
            [this]() { this->model->setOthersVisible(this->ui->othersButton->isChecked()); });
    connect(ui->clearButton, &QPushButton::clicked, this, [this]() { this->model->clearAll(); });
    connect(ui->issuesList, &QTableView::clicked, this, [this](const QModelIndex &index) {
        auto item = this->model->getItem(index);
        if (!item.fileName.isEmpty()) {
            this->manager->openFile(QDir::toNativeSeparators(item.fileName), item.row, item.col);
        }
    });

    outputDetector.add(new ClOutputDetector);
    outputDetector.add(new GccOutputDetector);
    outputDetector.add(new CargoOutputDetector);
    outputDetector.add(new GoLangOutputDetector);
}

ProjectIssuesWidget::~ProjectIssuesWidget() { delete ui; }

auto static setEditorStatus(qmdiEditor *editor, const CompileStatus &status) {
    if (editor->isEmpty()) {
        return;
    }

    editor->setMetaDataMessage(status.row, status.message);
    switch (typeToStatus(status.type)) {
    case Qutepart::ERROR_BIT:
        editor->setLineError(status.row, true);
        break;
    case Qutepart::WARNING_BIT:
        editor->setLineWarning(status.row, true);
        break;
    case Qutepart::INFO_BIT:
        editor->setLineInfo(status.row, true);
        break;
    default:
        break;
    }
}

void ProjectIssuesWidget::processLine(const QString &rawLines, const QString &sourceDir) {
    auto lines = rawLines.split("\n");
    for (auto const &line : lines) {
        outputDetector.processLine(line, sourceDir);
        auto items = outputDetector.foundStatus();
        for (auto &item : items) {
            model->addItem(item);
            auto client = manager->clientForFileName(item.fileName);
            if (auto editor = dynamic_cast<qmdiEditor *>(client)) {
                setEditorStatus(editor, item);
                editor->update();
            }
        }
    }
}

void ProjectIssuesWidget::decorateClient(qmdiClient *client) {
    if (auto editor = dynamic_cast<qmdiEditor *>(client)) {
        auto items = model->getItemsFor(editor->mdiClientFileName());
        for (const auto &item : items) {
            setEditorStatus(editor, item);
        }
        editor->update();
    } else {
        qDebug() << "Could not decorate client for " << client->mdiClientFileName();
    }
}

void ProjectIssuesWidget::changeEvent(QEvent *e) {
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
