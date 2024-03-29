/***************************************************************************
 *   Copyright (C) 2014 by Petr Vanek                                      *
 *   petr@scribus.info                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include <QDebug>
#include <QShortcut>

#include "bookmarkswidget.h"
#include "properties.h"
#include "config.h"


class AbstractBookmarkItem
{
public:
    enum ItemType {
        Root = 0,
        Group = 1,
        Command = 2
    };

    AbstractBookmarkItem(ItemType type, AbstractBookmarkItem* parent = nullptr)
        : m_type(type),
          m_parent(parent)
    {
    }
    virtual ~AbstractBookmarkItem()
    {
        qDeleteAll(m_children);
    }

    ItemType type() { return m_type; }
    QString value() { return m_value; }
    QString display() { return m_display; }

    void addChild(AbstractBookmarkItem* item) { m_children << item; }
    int childCount() { return m_children.count(); }
    QList<AbstractBookmarkItem*> children() { return m_children; }
    AbstractBookmarkItem *child(int number) { return m_children.value(number); }
    AbstractBookmarkItem *parent() { return m_parent; }

    int childNumber() const
    {
        if (m_parent)
            return m_parent->children().indexOf(const_cast<AbstractBookmarkItem*>(this));

        return 0;
    }

protected:
    ItemType m_type;
    AbstractBookmarkItem *m_parent;
    QList<AbstractBookmarkItem*> m_children;
    QString m_value;
    QString m_display;
};

class BookmarkRootItem : public AbstractBookmarkItem
{
public:
    BookmarkRootItem()
        : AbstractBookmarkItem(AbstractBookmarkItem::Root)
    {
        m_value = m_display = QStringLiteral("root");
    }
};

class BookmarkCommandItem : public AbstractBookmarkItem
{
public:
    BookmarkCommandItem(const QString &name, const QString &command, AbstractBookmarkItem *parent)
        : AbstractBookmarkItem(AbstractBookmarkItem::Command, parent)
    {
        m_value = command;
        m_display = name;
    }
};


class BookmarkGroupItem : public AbstractBookmarkItem
{
public:
    BookmarkGroupItem(const QString &name, AbstractBookmarkItem *parent)
        : AbstractBookmarkItem(AbstractBookmarkItem::Group, parent)
    {
        m_display = name;
    }
};

class BookmarkFileGroupItem : public BookmarkGroupItem
{
    // hierarchy handling
    // m_pos to group map. Example: group1.group2=item
    QHash<QString,AbstractBookmarkItem*> m_map;
    // current position level. Example "group1", "group2"
    QStringList m_pos;

public:
    BookmarkFileGroupItem(AbstractBookmarkItem *parent, const QString &fname)
        : BookmarkGroupItem(QObject::tr("Synchronized Bookmarks"), parent)
    {
        QFile f(fname);
        if (!f.open(QIODevice::ReadOnly))
        {
            //qDebug() << "Cannot open file" << fname;
            // TODO/FIXME: message box
            return;
        }

        QXmlStreamReader xml;
        xml.setDevice(&f);

        while (true)
        {
            xml.readNext();

            switch (xml.tokenType())
            {
            case QXmlStreamReader::StartElement:
            {
                AbstractBookmarkItem *parent = m_map.contains(xmlPos()) ? m_map[xmlPos()] : this;
                QString tag = xml.name().toString();
                if (tag == QLatin1String("group"))
                {
                    QString name = xml.attributes().value(QLatin1String("name")).toString();
                    m_pos.append(name);

                    BookmarkGroupItem *i = new BookmarkGroupItem(name, parent);
                    parent->addChild(i);

                    m_map[xmlPos()] = i;
                }
                else if (tag == QLatin1String("command"))
                {
                    QString name = xml.attributes().value(QLatin1String("name")).toString();
                    QString cmd = xml.attributes().value(QLatin1String("value")).toString();

                    BookmarkCommandItem *i = new BookmarkCommandItem(name, cmd, parent);
                    parent->addChild(i);
                }
                break;
            }
            case QXmlStreamReader::EndElement:
            {
                QString tag = xml.name().toString();
                if (tag == QLatin1String("group"))
                {
                    m_pos.removeLast();
                }
                break;
            }
            case QXmlStreamReader::Invalid:
                qDebug() << "XML error: " << xml.errorString().constData()
                         << xml.lineNumber() << xml.columnNumber();
                m_map.clear();
                return;
            case QXmlStreamReader::EndDocument:
                m_map.clear();
                return;
            default:
                break;
            } // switch
        } // while
    } // constructor

    QString xmlPos()
    {
        return m_pos.join(QLatin1Char('.'));
    }
};


BookmarksModel::BookmarksModel(QObject *parent)
    : QAbstractItemModel(parent),
      m_root(nullptr)
{
    setup();
}

void BookmarksModel::setup()
{
    delete m_root;
    m_root = new BookmarkRootItem();
    m_root->addChild(new BookmarkFileGroupItem(m_root, Properties::Instance()->bookmarksFile));
    beginResetModel();
    endResetModel();
}

BookmarksModel::~BookmarksModel()
{
    delete m_root;
    m_root = nullptr;
}

int BookmarksModel::columnCount(const QModelIndex & /* parent */) const
{
    return 2;
}

QVariant BookmarksModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return index.column() == 0 ? getItem(index)->display() : getItem(index)->value();
    case Qt::FontRole:
    {
        QFont f;
        if (static_cast<AbstractBookmarkItem*>(index.internalPointer())->type() == AbstractBookmarkItem::Group)
        {
            f.setBold(true);
        }
        return f;
    }
    default:
        return QVariant();
    }
}

AbstractBookmarkItem *BookmarksModel::getItem(const QModelIndex &index) const
{
    if (index.isValid())
    {
        AbstractBookmarkItem *item = static_cast<AbstractBookmarkItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return m_root;
 }

QVariant BookmarksModel::headerData(int /*section*/, Qt::Orientation /*orientation*/,
                                    int /*role*/) const
{
    return QVariant();
}

QModelIndex BookmarksModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    AbstractBookmarkItem *parentItem = getItem(parent);

    AbstractBookmarkItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}


QModelIndex BookmarksModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    AbstractBookmarkItem *childItem = getItem(index);
    AbstractBookmarkItem *parentItem = childItem->parent();

    if (parentItem == m_root)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

int BookmarksModel::rowCount(const QModelIndex &parent) const
{
    AbstractBookmarkItem *parentItem = getItem(parent);
    return parentItem->childCount();
}

QModelIndexList BookmarksModel::allChildRows(const QModelIndex& parent) const
{
    QModelIndexList list;
    for (int row = 0; row < rowCount(parent); ++row)
    {
        auto child = index(row, 0, parent); // we only need the row
        list << child;
        if (hasChildren(child))
        {
            list << allChildRows(child);
        }
    }
    return list;
}

#if 0
bool BookmarksModel::setData(const QModelIndex &index, const QVariant &value,
                             int role)
{
    if (role != Qt::EditRole)
        return false;

    AbstractBookmarkItem *item = getItem(index);
    bool result = item->setData(index.column(), value);

    if (result)
        emit dataChanged(index, index);

    return result;
}
#endif


BookmarksWidget::BookmarksWidget(QWidget *parent)
    : QWidget(parent)
    , m_model(new BookmarksModel(this))
{
    setupUi(this);

    treeView->setModel(m_model);
    treeView->header()->hide();
    setFocusProxy(filterEdit);

    connect(treeView, &QTreeView::activated,
            this, &BookmarksWidget::handleCommand);
    connect(filterEdit, &QLineEdit::textChanged,
            this, &BookmarksWidget::filter);

    QShortcut *clearFilter = new QShortcut(QKeySequence (Qt::Key_Escape), this);
    connect(clearFilter, &QShortcut::activated, this, [this] {
        filterEdit->clear();
    });
}

BookmarksWidget::~BookmarksWidget() = default;

void BookmarksWidget::setup()
{
    m_model->setup();

    treeView->setRootIndex(m_model->index(0, 0)); // do not show BookmarkFileGroupItem's top branch
    treeView->expandAll();
    treeView->resizeColumnToContents(0);
    treeView->resizeColumnToContents(1);
}

void BookmarksWidget::handleCommand(const QModelIndex& index)
{
    AbstractBookmarkItem *item = static_cast<AbstractBookmarkItem*>(index.internalPointer());
    if (!item || item->type() != AbstractBookmarkItem::Command)
        return;

    emit callCommand(item->value() + QLatin1Char('\n')); // TODO/FIXME: decide how to handle EOL
}

void BookmarksWidget::filter(const QString& str)
{
    treeView->clearSelection();
    const QModelIndexList list = m_model->allChildRows(QModelIndex());
    for (const auto& index : list)
    {
        AbstractBookmarkItem *item = static_cast<AbstractBookmarkItem*>(index.internalPointer());
        if (item && item->type() == AbstractBookmarkItem::Command)
        {
            if (item->value().contains(str, Qt::CaseInsensitive)
                || item->display().contains(str, Qt::CaseInsensitive))
            {
                treeView->setRowHidden(index.row(), index.parent(), false);
            }
            else
            {
                treeView->setRowHidden(index.row(), index.parent(), true);
            }
        }
    }
}
