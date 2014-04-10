#ifndef BOOKMARKSWIDGET_H
#define BOOKMARKSWIDGET_H

#include "ui_bookmarkswidget.h"

class AbstractBookmarkItem;
class BookmarksModel;


class BookmarksWidget : public QWidget, Ui::BookmarksWidget
{
    Q_OBJECT

public:
    BookmarksWidget(QWidget *parent=NULL);
    ~BookmarksWidget();

signals:
    void callCommand(const QString &cmd);

private:
    BookmarksModel *m_model;

private slots:
    void handleCommand(const QModelIndex& index);
};


class BookmarksModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    BookmarksModel(QObject *parent = 0);
    ~BookmarksModel();

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

private:
    AbstractBookmarkItem *getItem(const QModelIndex &index) const;
    AbstractBookmarkItem *m_root;
};

#endif

