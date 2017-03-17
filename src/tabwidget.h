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
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef TAB_WIDGET
#define TAB_WIDGET

#include <QTabWidget>
#include <QMap>
#include <QAction>

#ifdef HAVE_QDBUS
    #include <QtDBus/QtDBus>
    #include "dbusaddressable.h"
#endif

#include "terminalconfig.h"
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

    void showHideTabBar();

public slots:
    int addNewTab(TerminalConfig cfg);
    void removeTab(int);
    void removeCurrentTab();
    int switchToRight();
    int switchToLeft();
    void removeFinished();
    void moveLeft();
    void moveRight();
    void renameSession(int);
    void renameCurrentSession();

    void switchNextSubterminal();
    void switchPrevSubterminal();
    void splitHorizontally();
    void splitVertically();
    void splitCollapse();

    void copySelection();
    void pasteClipboard();
    void pasteSelection();
    void zoomIn();
    void zoomOut();
    void zoomReset();

    void changeTabPosition(QAction *);
    void changeScrollPosition(QAction *);
    void changeKeyboardCursorShape(QAction *);
    void propertiesChanged();

    void clearActiveTerminal();

    void saveSession();
    void loadSession();

    void preset2Horizontal();
    void preset2Vertical();
    void preset4Terminals();

signals:
    void closeTabNotification();
    void tabRenameRequested(int);
    void currentTitleChanged(int);

protected:
    enum Direction{Left = 1, Right};
    void contextMenuEvent(QContextMenuEvent * event);
    void move(Direction);
    /*! Event filter for TabWidget's QTabBar. It's installed on tabBar()
        in the constructor.
        It's purpose is to handle doubleclicks on QTabBar for session
        renaming or new tab opening
     */
    bool eventFilter(QObject *obj, QEvent *event);
protected slots:
    void updateTabIndices();
    void onTermTitleChanged(QString title, QString icon);

private:
    int tabNumerator;
    /* re-order naming of the tabs then removeCurrentTab() */
    void renameTabsAfterRemove();
};

#endif
