#include <QFileInfo>
#include <QHeaderView>
#include <QStyledItemDelegate>

#include "ProjectIssuesWidget.h"
#include "pluginmanager.h"
#include "ui_ProjectIssuesWidget.h"

class CenteredIconDelegate : public QStyledItemDelegate {
  public:
    explicit CenteredIconDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override {
        if (index.column() == 0) { // Only for the first column
            QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
            if (!icon.isNull()) {
                QRect iconRect = option.rect;
                QSize iconSize = option.decorationSize;

                // Center the icon in the cell
                int x = iconRect.x() + (iconRect.width() - iconSize.width()) / 2;
                int y = iconRect.y() + (iconRect.height() - iconSize.height()) / 2;

                icon.paint(painter, x, y, iconSize.width(), iconSize.height());
                return;
            }
        }

        QStyledItemDelegate::paint(painter, option, index);
    }
};

CompileStatusModel::CompileStatusModel(QObject *parent)
    : QAbstractTableModel(parent), showWarnings(true), showErrors(true), showOthers(true) {
    headers << "Type" << "Message" << "Location";
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
        QString iconName;
        if (status.type.toLower().contains("error")) {
            iconName = "data-error";
        } else if (status.type.toLower() == "warning") {
            iconName = "data-warning";
        } else {
            iconName = "data-information";
        }

        if (!QIcon::hasThemeIcon(iconName)) {
            qDebug() << "Error : icon not foundm using build it icons" << iconName;

            if (status.type.toLower() == "error") {
                return qApp->style()->standardIcon(QStyle::SP_MessageBoxCritical);
            } else if (status.type.toLower() == "warning") {
                return qApp->style()->standardIcon(QStyle::SP_MessageBoxWarning);
            } else {
                return qApp->style()->standardIcon(QStyle::SP_MessageBoxInformation);
            }
        }
        return QIcon::fromTheme(iconName);
    }

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0:
            return {};
        case 1:
            return status.message;
        case 2: {
            auto fileInfo = QFileInfo(status.fileName);
            auto fileName = fileInfo.fileName();
            return QString("%1 (%2:%3)").arg(fileName).arg(status.row).arg(status.col);
        }
        default:
            return {};
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
    connect(ui->errorsButton, &QPushButton::clicked, this,
            [this]() { this->model->setErrorsVisible(this->ui->errorsButton->isChecked()); });
    connect(ui->warningsButton, &QPushButton::clicked, this,
            [this]() { this->model->setWarningsVisible(this->ui->warningsButton->isChecked()); });
    connect(ui->othersButton, &QPushButton::clicked, this,
            [this]() { this->model->setOthersVisible(this->ui->othersButton->isChecked()); });
    connect(ui->clearButton, &QPushButton::clicked, this, [this]() { this->model->clearAll(); });
    connect(ui->issuesList, &QTableView::clicked, this, [this](const QModelIndex &index) {
        auto item = this->model->getItem(index);
        this->manager->openFile(item.fileName, item.row, item.col);
        // qDebug() << "Will open file: " << item.fileName << "at" << item.col << item.row;
    });
}

ProjectIssuesWidget::~ProjectIssuesWidget() { delete ui; }

void ProjectIssuesWidget::processLine(const QString &rawLines) {
    auto static re = QRegularExpression(R"((.+):(\d+):(\d+):\s+(.+):\s+(.+))");
    auto lines = rawLines.split("\n");
    for (auto const &r : lines) {
        auto match = QRegularExpressionMatch(re.match(r));
        if (!match.hasMatch()) {
            continue;
        }
        auto file = match.captured(1);
        auto line = match.captured(2).toInt();
        auto column = match.captured(3).toInt();
        auto type = match.captured(4);
        auto message = match.captured(5);

        auto item = CompileStatus{file, line, column, type, message};
        model->addItem(item);
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
