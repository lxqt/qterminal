#ifndef TERMWIDGET_H
#define TERMWIDGET_H

#include <QTermWidget.h>


class TermWidget : public QTermWidget
{
    Q_OBJECT

    public:

        TermWidget(const QString & wdir, QWidget * parent=0);

    signals:
        void renameSession();
        void removeCurrentTerminal();

    private slots:
        void customContextMenuCall(const QPoint & pos);

};

#endif

