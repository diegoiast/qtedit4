#pragma once

#include "kitdefinitions.h"
#include <QAbstractListModel>
#include <QString>
#include <vector>

class KitDefinitionModel : public QAbstractListModel {
    Q_OBJECT

  public:
    KitDefinitionModel(QObject *parent = nullptr);
    void setKitDefinitions(const std::vector<KitDefinition> &kits);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  private:
    std::vector<KitDefinition> kitDefinitions; // Store a copy of the vector
};
