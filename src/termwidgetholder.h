#ifndef TERMWIDGETHOLDER_H
#define TERMWIDGETHOLDER_H

#include <QWidget>
#include "termwidget.h"
class QSplitter;



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
        void setInitialFocus();

        void loadSession();
        void saveSession(const QString & name);

    public slots:
        void splitHorizontal(TermWidget * term);
        void splitVertical(TermWidget * term);
        void splitCollapse(TermWidget * term);
        void setWDir(const QString & wdir);
        void switchNextSubterminal();
        void switchPrevSubterminal();

    signals:
        void finished();
        void lastTerminalClosed();
        void renameSession();

        void enableCollapse(bool);

    private:
        QString m_wdir;

        void split(TermWidget * term, Qt::Orientation orientation);
        TermWidget * newTerm();
    
    private slots:
        void handle_finished();
};

#endif

