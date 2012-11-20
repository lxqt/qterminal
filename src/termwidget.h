#ifndef TERMWIDGET_H
#define TERMWIDGET_H

#include <qtermwidget.h>


class TermWidgetImpl : public QTermWidget
{
    Q_OBJECT

        QMap< QString, QAction * > actionMap;

    public:

        TermWidgetImpl(const QString & wdir, const QString & shell=QString(), QWidget * parent=0);
        void propertiesChanged();

    signals:
        void renameSession();
        void removeCurrentSession();
        void splitHorizontal();
        void splitVertical();
        void splitCollapse();

    public slots:
        void enableCollapse(bool enable);
        void updateShortcuts();

    private slots:
        void customContextMenuCall(const QPoint & pos);
        void act_splitVertical();
        void act_splitHorizontal();
        void act_splitCollapse();
        void zoomIn();
        void zoomOut();
        void zoomReset();
};


class TermWidget : public QWidget
{
    Q_OBJECT

    TermWidgetImpl * m_term;
    QVBoxLayout * m_layout;
    QColor m_border;

    public:
        TermWidget(const QString & wdir, const QString & shell=QString(), QWidget * parent=0);

        void propertiesChanged(); 
        QStringList availableKeyBindings() { return m_term->availableKeyBindings(); }

        TermWidgetImpl * impl() { return m_term; }

    signals:
        void finished();
        void renameSession();
        void removeCurrentSession();
        void splitHorizontal(TermWidget * self);
        void splitVertical(TermWidget * self);
        void splitCollapse(TermWidget * self);
        void termGetFocus(TermWidget * self);

    public slots:
        void enableCollapse(bool enable);

    protected:
        void paintEvent (QPaintEvent * event);

    private slots:
        void term_splitHorizontal();
        void term_splitVertical();
        void term_splitCollapse();
        void term_termGetFocus();
        void term_termLostFocus();
};

#endif

