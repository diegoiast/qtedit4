/**
 * \file xmltreemodel.h
 * \brief Definition of a model to an XML to treeview
 * \author Diego Iastrubni diegoiast@gmail.com
 */

// SPDX-License-Identifier: MIT

#pragma once

#include <QAbstractItemModel>

class QXmlStreamReader;

class Node {
  public:
    explicit Node(const QString &name = QString(), const QString &value = QString(),
                  Node *parent = nullptr);
    ~Node();
    void appendChild(Node *child);
    Node *child(int row);
    int childCount() const;
    int row() const;
    Node *parent();
    QString name() const;
    QString value() const;

  private:
    QString m_name;
    QString m_value;
    QVector<Node *> m_children;
    Node *m_parent;
};

class XmlTreeModel : public QAbstractItemModel {
    Q_OBJECT

  public:
    explicit XmlTreeModel(const QString &xmlData, QObject *parent = nullptr);
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex & = QModelIndex()) const override { return 2; }

  private:
    void buildTree(QXmlStreamReader &reader, Node *parent);

    std::unique_ptr<Node> m_rootNode;
};
