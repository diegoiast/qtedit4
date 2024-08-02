#include "kitdefinitionmodel.h"

KitDefinitionModel::KitDefinitionModel(QObject *parent)
    : QAbstractListModel(parent)
{}

void KitDefinitionModel::setKitDefinitions(const std::vector<KitDefinition> &kits)
{
    kitDefinitions = kits;
    // Notify the view that the model data has changed
    beginResetModel();
    endResetModel();
}

int KitDefinitionModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return kitDefinitions.size();
}

QVariant KitDefinitionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= kitDefinitions.size()) {
        return QVariant();
    }

    const auto &kit = kitDefinitions[index.row()];
    if (role == Qt::DisplayRole) {
        return QString::fromStdString(kit.name); // Display the name in the combo box
    }
    return QVariant();
}
