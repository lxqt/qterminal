/***************************************************************************
 *   Copyright (C) 2006 by Vladimir Kuznetsov                              *
 *   vovanec@gmail.com                                                     *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef TAB_WIDGET
#define TAB_WIDGET

#include <QTabWidget>
#include <QMap>

#include "properties.h"

class TermWidgetHolder;
class QAction;
class QActionGroup;


class TabWidget : public QTabWidget
{
Q_OBJECT
public:
    TabWidget(QWidget* parent = 0);

    TermWidgetHolder * terminalHolder();

    /* re-implemented */
    void setTabBar(QTabBar *tabBar) { QTabWidget::setTabBar(tabBar); }
    QTabBar *tabBar() const { return QTabWidget::tabBar(); }

public slots:
    int addNewTab(const QString& shell_program = QString());
    void removeTab(int);
    void removeCurrentTab();
    int switchToRight();
    int switchToLeft();
    void removeFinished();
    void moveLeft();
    void moveRight();
    void renameSession();
    void setWorkDirectory(const QString&);

    void switchNextSubterminal();
    void switchPrevSubterminal();
    void splitHorizontally();
    void splitVertically();
    void splitCollapse();

    void changeTabPosition(QAction *);
    void changeScrollPosition(QAction *);
    void propertiesChanged();

    void clearActiveTerminal();

    void saveSession();
    void loadSession();

signals:
    void quit_notification();

protected:
    enum Direction{Left = 1, Right};
    void contextMenuEvent(QContextMenuEvent * event);
    void recountIndexes();
    void move(Direction);
    /*! Event filter for TabWidget's QTabBar. It's installed on tabBar()
        in the constructor.
        It's purpose is to handle doubleclicks on QTabBar for session
        renaming or new tab opening
     */
    bool eventFilter(QObject *obj, QEvent *event);

private:
    int tabNumerator;
    QString work_dir;
    /* re-order naming of the tabs then removeCurrentTab() */
    void renameTabsAfterRemove();
};

#endif
