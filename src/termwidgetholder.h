#ifndef TERMWIDGETHOLDER_H
#define TERMWIDGETHOLDER_H

#include <QWidget>

class QSplitter;
class TermWidget;


/*! \brief TermWidget group/session manager.

This widget (one per TabWidget tab) is a "proxy" widget beetween TabWidget and
unspecified count of TermWidgets. Basically it should look like a single TermWidget
for TabWidget - with its signals and slots.

Splitting and collapsing of TermWidgets is done here.
*/
class TermWidgetHolder : public QWidget
{
    Q_OBJECT

    public:
        TermWidgetHolder(const QString & wdir, QWidget * parent);
        ~TermWidgetHolder();

        void propertiesChanged();
        TermWidget * terminal();

    public slots:
        void splitHorizontal(TermWidget * term);
        void splitVertical(TermWidget * term);
        void splitCollapse(TermWidget * term);
        void setWDir(const QString & wdir);

    signals:
        void finished();
        void lastTerminalClosed();
        void renameSession();

        void enableCollapse(bool);

    private:
        QString m_wdir;

        void split(TermWidget * term, Qt::Orientation orientation);
        TermWidget * newTerm();
};

#endif

