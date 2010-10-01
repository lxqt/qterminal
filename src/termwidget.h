#ifndef TERMWIDGET_H
#define TERMWIDGET_H

#include <qtermwidget.h>


class TermWidget : public QTermWidget
{
    Q_OBJECT

    public:

        TermWidget(const QString & wdir, QWidget * parent=0);
        void propertiesChanged();

    signals:
        void renameSession();
        void removeCurrentTerminal();

    private slots:
        void customContextMenuCall(const QPoint & pos);

};

#endif

