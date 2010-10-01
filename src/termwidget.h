#ifndef TERMWIDGET_H
#define TERMWIDGET_H

#include <QTermWidget.h>


class TermWidget : public QTermWidget
{
    Q_OBJECT

        QAction * actCollapse;

    public:

        TermWidget(const QString & wdir, QWidget * parent=0);
        void propertiesChanged();

    signals:
        void renameSession();
        void removeCurrentSession();
        void splitHorizontal(TermWidget * self);
        void splitVertical(TermWidget * self);
        void splitCollapse(TermWidget * self);

    public slots:
        void enableCollapse(bool enable);


    private slots:
        void customContextMenuCall(const QPoint & pos);
        void act_splitVertical();
        void act_splitHorizontal();
        void act_splitCollapse();
};

#endif

