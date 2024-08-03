/**
 * \file kitdefinitionmodel.h
 * \brief Implementation of kit definition modes
 * \author Diego Iastrubni (diegoiast@gmail.com)
 *  License MIT
 */

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
    const KitDefinition &getKit(size_t index) const { return this->kitDefinitions.at(index); }

  private:
    std::vector<KitDefinition> kitDefinitions;
};
