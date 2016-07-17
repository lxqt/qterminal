/***************************************************************************
 *   Copyright (C) 2010 by Petr Vanek                                      *
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
        TermWidgetHolder(const QString & wdir, const QString & shell=QString(), QWidget * parent=0);
        ~TermWidgetHolder();

        void propertiesChanged();
        void setInitialFocus();

        void loadSession();
        void saveSession(const QString & name);
        void zoomIn(uint step);
        void zoomOut(uint step);

        TermWidget* currentTerminal();

    public slots:
        void splitHorizontal(TermWidget * term);
        void splitVertical(TermWidget * term);
        void splitCollapse(TermWidget * term);
        void setWDir(const QString & wdir);
        void switchNextSubterminal();
        void switchPrevSubterminal();
        void clearActiveTerminal();

    signals:
        void finished();
        void lastTerminalClosed();
        void renameSession();
        void termTitleChanged(TermWidgetHolder * term, QString userTitle, QString iconText);

    private:
        QString m_wdir;
        QString m_shell;
        TermWidget * m_currentTerm;

        void split(TermWidget * term, Qt::Orientation orientation);
        TermWidget * newTerm(const QString & wdir=QString(), const QString & shell=QString());

    private slots:
        void setCurrentTerminal(TermWidget* term);
        void setTerminalTitle(TermWidget* term);
        void handle_finished();
};

#endif

