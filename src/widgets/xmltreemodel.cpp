/**
 * \file xmltreemodel.cpp
 * \brief Definition of a model to an XML to treeview
 * \author Diego Iastrubni diegoiast@gmail.com
 */

// SPDX-License-Identifier: MIT

#include "xmltreemodel.h"

#include <QXmlStreamReader>

Node::Node(const QString &name, const QString &value, Node *parent)
    : m_name(name), m_value(value), m_parent(parent) {}

Node::~Node() { qDeleteAll(m_children); }

void Node::appendChild(Node *child) { m_children.append(child); }

Node *Node::child(int row) { return m_children.value(row); }

int Node::childCount() const { return m_children.count(); }

int Node::row() const {
    if (m_parent) {
        return m_parent->m_children.indexOf(const_cast<Node *>(this));
    }
    return 0;
}

Node *Node::parent() { return m_parent; }

QString Node::name() const { return m_name; }

QString Node::value() const { return m_value; }

void XmlTreeModel::buildTree(QXmlStreamReader &reader, Node *parent) {
    while (!reader.atEnd() && !reader.hasError()) {
        QXmlStreamReader::TokenType token = reader.readNext();

        if (token == QXmlStreamReader::StartElement) {
            Node *child = new Node(reader.name().toString(), QString(), parent);
            parent->appendChild(child);

            // Handle attributes
            for (const auto &attr : reader.attributes()) {
                Node *attrNode = new Node(attr.name().toString(), attr.value().toString(), child);
                child->appendChild(attrNode);
            }

            buildTree(reader, child);
        } else if (token == QXmlStreamReader::Characters && !reader.isWhitespace()) {
            parent->appendChild(new Node("#text", reader.text().toString(), parent));
        } else if (token == QXmlStreamReader::EndElement) {
            return;
        }
    }

    if (reader.hasError()) {
        qWarning() << "XML error:" << reader.errorString();
    }
}

XmlTreeModel::XmlTreeModel(const QString &xmlData, QObject *parent)
    : QAbstractItemModel(parent), m_rootNode(std::make_unique<Node>()) {
    QXmlStreamReader reader(xmlData);
    buildTree(reader, m_rootNode.get());
}

QVariant XmlTreeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole) {
        return QVariant();
    }

    Node *item = static_cast<Node *>(index.internalPointer());
    if (index.column() == 0) {
        return item->name();
    } else if (index.column() == 1) {
        return item->value();
    }
    return QVariant();
}

Qt::ItemFlags XmlTreeModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return QAbstractItemModel::flags(index);
}

QVariant XmlTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return tr("Name");
        } else if (section == 1) {
            return tr("Value");
        }
    }
    return QVariant();
}

QModelIndex XmlTreeModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    Node *parentItem;

    if (!parent.isValid()) {
        parentItem = m_rootNode.get();
    } else {
        parentItem = static_cast<Node *>(parent.internalPointer());
    }

    Node *childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    } else {
        return QModelIndex();
    }
}

QModelIndex XmlTreeModel::parent(const QModelIndex &index) const {
    if (!index.isValid()) {
        return QModelIndex();
    }

    Node *childItem = static_cast<Node *>(index.internalPointer());
    Node *parentItem = childItem->parent();

    if (parentItem == m_rootNode.get()) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int XmlTreeModel::rowCount(const QModelIndex &parent) const {
    Node *parentItem;
    if (parent.column() > 0) {
        return 0;
    }

    if (!parent.isValid()) {
        parentItem = m_rootNode.get();
    } else {
        parentItem = static_cast<Node *>(parent.internalPointer());
    }

    return parentItem->childCount();
}
