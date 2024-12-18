#include <QHeaderView>
#include <QFileInfo>

#include "ProjectIssuesWidget.h"
#include "ui_ProjectIssuesWidget.h"


CompileStatusModel::CompileStatusModel(QObject *parent)
    : QAbstractTableModel(parent), m_showWarnings(true), m_showErrors(true), m_showOthers(true)
{
    m_headers << "Type" << "Message" << "Location";
}

int CompileStatusModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_filteredStatuses.size();
}

int CompileStatusModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_headers.size();
}

QVariant CompileStatusModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_filteredStatuses.size())
        return {};

    const CompileStatus &status = m_filteredStatuses.at(index.row());
    
    if (index.column() == 0 && role == Qt::DecorationRole) {
        QString iconName;
        if (status.type.toLower() == "error")
            iconName = "data-error";
        else if (status.type.toLower() == "warning")
            iconName = "data-warning";
        else
            iconName = "data-information";
        
        if (!QIcon::hasThemeIcon(iconName)) {
            qDebug() << "Error : icon not found" << iconName;

            if (status.type.toLower() == "error")
                return qApp->style()->standardIcon(QStyle::SP_MessageBoxCritical);
            else if (status.type.toLower() == "warning")
                return qApp->style()->standardIcon(QStyle::SP_MessageBoxWarning);
            else
                return qApp->style()->standardIcon(QStyle::SP_MessageBoxInformation);
        }
        return QIcon::fromTheme(iconName);
    }

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0: return {};
        case 1: return status.message;
        case 2: {
            QFileInfo fileInfo(status.fileName);
            QString fileName = fileInfo.fileName();
            return QString("%1 (%2:%3)").arg(fileName).arg(status.row).arg(status.col);
        }
        default: return {};
        }
    }

    return {};
}


QVariant CompileStatusModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal || section >= m_headers.size())
        return {};

    return m_headers.at(section);
}

void CompileStatusModel::sort(int column, Qt::SortOrder order)
{
     beginResetModel();
    std::sort(m_filteredStatuses.begin(), m_filteredStatuses.end(),
        [column, order](const CompileStatus &s1, const CompileStatus &s2) {
            bool lessThan;
            switch(column) {
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

void CompileStatusModel::clearAll()
{
    beginResetModel();
    m_statuses.clear();
    m_filteredStatuses.clear();
    endResetModel();
}

void CompileStatusModel::addItem(const CompileStatus &status)
{
    beginResetModel();
    m_statuses.append(status);
    applyFilter();
    endResetModel();
}

void CompileStatusModel::setWarningsVisible(bool visible)
{
    if (m_showWarnings != visible) {
        m_showWarnings = visible;
        beginResetModel();
        applyFilter();
        endResetModel();
    }
}

void CompileStatusModel::setErrorsVisible(bool visible)
{
    if (m_showErrors != visible) {
        m_showErrors = visible;
        beginResetModel();
        applyFilter();
        endResetModel();
    }
}

void CompileStatusModel::setOthersVisible(bool visible)
{
    if (m_showOthers != visible) {
        m_showOthers = visible;
        beginResetModel();
        applyFilter();
        endResetModel();
    }
}

bool CompileStatusModel::areWarningsVisible() const
{
    return m_showWarnings;
}

bool CompileStatusModel::areErrorsVisible() const
{
    return m_showErrors;
}

bool CompileStatusModel::areOthersVisible() const
{
    return m_showOthers;
}

void CompileStatusModel::applyFilter()
{
    m_filteredStatuses.clear();
    for (const auto &status : m_statuses) {
        if (shouldShowStatus(status)) {
            m_filteredStatuses.append(status);
        }
    }
}

bool CompileStatusModel::shouldShowStatus(const CompileStatus &status) const
{
    QString lowerType = status.type.toLower();
    if (lowerType == "warning")
        return m_showWarnings;
    else if (lowerType == "error")
        return m_showErrors;
    else
        return m_showOthers;
}


ProjectIssuesWidget::ProjectIssuesWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::ProjectIssuesWidget) {
    model = new CompileStatusModel(this);
    ui->setupUi(this);
    
    ui->issuesList->setModel(model);    
    ui->issuesList->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->issuesList->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->issuesList->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    // ui->issuesList->setColumnWidth(0, 30);  // Adjust as needed
    // ui->issuesList->setIconSize(QSize(16, 16));
    
    connect(ui->errorsButton, &QPushButton::clicked, this, [this](){
        this->model->setErrorsVisible(this->ui->errorsButton->isChecked());
    });
    connect(ui->warningsButton, &QPushButton::clicked, this, [this](){
        this->model->setWarningsVisible(this->ui->warningsButton->isChecked());
    });
    connect(ui->othersButton, &QPushButton::clicked, this, [this](){
        this->model->setOthersVisible(this->ui->othersButton->isChecked());
    });
}

ProjectIssuesWidget::~ProjectIssuesWidget() { delete ui; }

// /home/diego/src/diego/fdbox/src/dos/echo.c:45:48: error: passing argument 2 of ‘command_merge_args’ from incompatible pointer type [-Wincompatible-pointer-types]

void ProjectIssuesWidget::processLine(const QString &rawLines)
{
    auto static re = QRegularExpression (R"((.+):(\d+):(\d+):\s+(.+):\s+(.+))");
    auto lines = rawLines.split("\n");
    auto count = 0;
    for (auto const &r: lines) {
        count ++;
        auto match = QRegularExpressionMatch(re.match(r));
        if (!match.hasMatch()) {
            continue;
        }
        auto file = match.captured(1);
        auto line = match.captured(2).toInt();
        auto column = match.captured(3).toInt();
        auto type = match.captured(4);
        auto message = match.captured(5);
        
        auto item = CompileStatus{file, line, column, type, message };
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
