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

#include <QTabBar>
#include <QInputDialog>
#include <QMouseEvent>
#include <QMenu>

#include "mainwindow.h"
#include "termwidgetholder.h"
#include "tabwidget.h"
#include "config.h"
#include "properties.h"
#include "qterminalapp.h"


#define TAB_INDEX_PROPERTY "tab_index"
#define TAB_CUSTOM_NAME_PROPERTY "custom_name"


TabWidget::TabWidget(QWidget* parent) : QTabWidget(parent), tabNumerator(0)
{
    setFocusPolicy(Qt::NoFocus);

    /* On Mac OS X this will look similar to
     * the tabs in Safari or Leopard's Terminal.app .
     * I love this!
     */
    setDocumentMode(true);

    tabBar()->setUsesScrollButtons(true);

    setTabsClosable(true);
    setMovable(true);
    setUsesScrollButtons(true);

    tabBar()->installEventFilter(this);

    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(removeTab(int)));
    connect(tabBar(), SIGNAL(tabMoved(int,int)), this, SLOT(updateTabIndices()));
    connect(this, SIGNAL(tabRenameRequested(int)), this, SLOT(renameSession(int)));
}

TermWidgetHolder * TabWidget::terminalHolder()
{
    return reinterpret_cast<TermWidgetHolder*>(widget(currentIndex()));
}


int TabWidget::addNewTab(TerminalConfig config)
{
    tabNumerator++;
    QString label = QString(tr("Shell No. %1")).arg(tabNumerator);

    TermWidgetHolder *ch = terminalHolder();
    if (ch)
        config.provideCurrentDirectory(ch->currentTerminal()->impl()->workingDirectory());

    TermWidgetHolder *console = new TermWidgetHolder(config, this);
    console->setWindowTitle(label);
    connect(console, SIGNAL(finished()), SLOT(removeFinished()));
    connect(console, SIGNAL(lastTerminalClosed()), this, SLOT(removeFinished()));
    connect(console, &TermWidgetHolder::termTitleChanged, this, &TabWidget::onTermTitleChanged);
    connect(this, &QTabWidget::currentChanged, this, &TabWidget::currentTitleChanged);

    int index = addTab(console, label);
    console->setProperty(TAB_CUSTOM_NAME_PROPERTY, false);
    updateTabIndices();
    setCurrentIndex(index);
    console->setInitialFocus();

    showHideTabBar();

    return index;
}

void TabWidget::switchLeftSubterminal()
{
    terminalHolder()->directionalNavigation(NavigationDirection::Left);
}

void TabWidget::switchRightSubterminal()
{
    terminalHolder()->directionalNavigation(NavigationDirection::Right);
}

void TabWidget::switchTopSubterminal() {
    terminalHolder()->directionalNavigation(NavigationDirection::Top);
}

void TabWidget::switchBottomSubterminal() {
    terminalHolder()->directionalNavigation(NavigationDirection::Bottom);
}

void TabWidget::splitHorizontally()
{
    terminalHolder()->splitHorizontal(terminalHolder()->currentTerminal());
}

void TabWidget::splitVertically()
{
    terminalHolder()->splitVertical(terminalHolder()->currentTerminal());
}

void TabWidget::splitCollapse()
{
    terminalHolder()->splitCollapse(terminalHolder()->currentTerminal());
}

void TabWidget::copySelection()
{
    terminalHolder()->currentTerminal()->impl()->copyClipboard();
}

void TabWidget::pasteClipboard()
{
    terminalHolder()->currentTerminal()->impl()->pasteClipboard();
}

void TabWidget::pasteSelection()
{
    terminalHolder()->currentTerminal()->impl()->pasteSelection();
}

void TabWidget::zoomIn()
{
    terminalHolder()->currentTerminal()->impl()->zoomIn();
}

void TabWidget::zoomOut()
{
    terminalHolder()->currentTerminal()->impl()->zoomOut();
}

void TabWidget::zoomReset()
{
    terminalHolder()->currentTerminal()->impl()->zoomReset();
}

void TabWidget::updateTabIndices()
{
    for(int i = 0; i < count(); i++)
        widget(i)->setProperty(TAB_INDEX_PROPERTY, i);
}

void TabWidget::onTermTitleChanged(QString title, QString icon)
{
    TermWidgetHolder * console = qobject_cast<TermWidgetHolder*>(sender());
    const bool custom_name = console->property(TAB_CUSTOM_NAME_PROPERTY).toBool();
    if (!custom_name)
    {
        const int index = console->property(TAB_INDEX_PROPERTY).toInt();

        setTabIcon(index, QIcon::fromTheme(icon));
        setTabText(index, title);
        if (currentIndex() == index)
            emit currentTitleChanged(index);
    }
}

void TabWidget::renameSession(int index)
{
    bool ok = false;
    QString text = QInputDialog::getText(this, tr("Tab name"),
                                        tr("New tab name:"), QLineEdit::Normal,
                                        QString(), &ok);
    if(ok && !text.isEmpty())
    {
        setTabIcon(index, QIcon{});
        setTabText(index, text);
        widget(index)->setProperty(TAB_CUSTOM_NAME_PROPERTY, true);
        if (currentIndex() == index)
            emit currentTitleChanged(index);
    }
}

void TabWidget::renameCurrentSession()
{
    renameSession(currentIndex());
}

void TabWidget::renameTabsAfterRemove()
{
// it breaks custom names - it replaces original/custom title with shell no #
#if 0
    for(int i = 0; i < count(); i++) {
        setTabText(i, QString(tr("Shell No. %1")).arg(i+1));
    }
#endif
}

void TabWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    QMap< QString, QAction * > actions = findParent<MainWindow>(this)->leaseActions();

    QAction *close = menu.addAction(QIcon::fromTheme("document-close"), tr("Close session"));
    QAction *rename = menu.addAction(actions[RENAME_SESSION]->text());
    rename->setShortcut(actions[RENAME_SESSION]->shortcut());
    rename->blockSignals(true);

    int tabIndex = tabBar()->tabAt(event->pos());
    QAction *action = menu.exec(event->globalPos());
    if (action == close) {
        emit tabCloseRequested(tabIndex);
    } else if (action == rename) {
        emit tabRenameRequested(tabIndex);
    }
}

bool TabWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick)
    {
        QMouseEvent *e = reinterpret_cast<QMouseEvent*>(event);
        // if user doubleclicks on tab button - rename it. If user
        // clicks on free space - open new tab
        int index = tabBar()->tabAt(e->pos());
        if (index == -1)
        {
            TerminalConfig defaultConfig;
            addNewTab(defaultConfig);
        }
        else
            renameSession(index);
        return true;
    }
    return QTabWidget::eventFilter(obj, event);
}

void TabWidget::removeFinished()
{
    QObject* term = sender();
    QVariant prop = term->property(TAB_INDEX_PROPERTY);
    if(prop.isValid() && prop.canConvert(QVariant::Int))
    {
        int index = prop.toInt();
        removeTab(index);
//        if (count() == 0)
//            emit closeTabNotification();
    }
}

void TabWidget::removeTab(int index)
{
    if (count() > 1) {
        setUpdatesEnabled(false);

        QWidget * w = widget(index);
        QTabWidget::removeTab(index);
        w->deleteLater();

        updateTabIndices();
        int current = currentIndex();
        if (current >= 0 )
        {
            qobject_cast<TermWidgetHolder*>(widget(current))->setInitialFocus();
        }
    // do not decrease it as renaming is disabled in renameTabsAfterRemove
    //    tabNumerator--;
        setUpdatesEnabled(true);
    } else {
        emit closeTabNotification();
    }

    renameTabsAfterRemove();
    showHideTabBar();
}

void TabWidget::removeCurrentTab()
{
    // question disabled due user requests. Yes I agree it was anoying.
//    if (QMessageBox::question(this,
//                    tr("Close current session"),
//                    tr("Are you sure you want to close current sesstion?"),
//                    QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
//    {
    if (count() > 1) {
        removeTab(currentIndex());
    } else {
        emit closeTabNotification();
    }
}

int TabWidget::switchToRight()
{
    int next_pos = currentIndex() + 1;
    if (next_pos < count())
        setCurrentIndex(next_pos);
    else
        setCurrentIndex(0);
    return currentIndex();
}

int TabWidget::switchToLeft()
{
    int previous_pos = currentIndex() - 1;
    if (previous_pos < 0)
        setCurrentIndex(count() - 1);
    else
        setCurrentIndex(previous_pos);
    return currentIndex();
}


void TabWidget::move(Direction dir)
{
    if(count() > 1)
    {
        int index = currentIndex();
        QWidget* child  = widget(index);
        QString label   = tabText(index);
        QString toolTip = tabToolTip(index);
        QIcon   icon    = tabIcon(index);

        int newIndex = 0;
        if(dir == Left)
            if(index == 0)
                newIndex = count() -1;
            else
                newIndex = index - 1;
        else
            if(index == count() - 1)
                newIndex = 0;
            else
                newIndex = index + 1;

        setUpdatesEnabled(false);
        QTabWidget::removeTab(index);
        newIndex = insertTab(newIndex, child, label);
        setTabToolTip(newIndex, toolTip);
        setTabIcon(newIndex, icon);
        setUpdatesEnabled(true);
        setCurrentIndex(newIndex);
        child->setFocus();
        updateTabIndices();
    }
}

void TabWidget::moveLeft()
{
    move(Left);
}

void TabWidget::moveRight()
{
    move(Right);
}

void TabWidget::changeScrollPosition(QAction *triggered)
{
    QActionGroup *scrollPosition = static_cast<QActionGroup *>(sender());
    if(!scrollPosition)
        qFatal("scrollPosition is NULL");

    Properties::Instance()->scrollBarPos =
            scrollPosition->actions().indexOf(triggered);

    Properties::Instance()->saveSettings();
    propertiesChanged();

}

void TabWidget::changeTabPosition(QAction *triggered)
{
    QActionGroup *tabPosition = static_cast<QActionGroup *>(sender());
    if(!tabPosition)
        qFatal("tabPosition is NULL");

    Properties *prop = Properties::Instance();
    /* order is dictated from mainwindow.cpp */
    QTabWidget::TabPosition position = (QTabWidget::TabPosition)tabPosition->actions().indexOf(triggered);
    setTabPosition(position);
    prop->tabsPos = position;
    prop->saveSettings();
}

void TabWidget::changeKeyboardCursorShape(QAction *triggered)
{
    QActionGroup *keyboardCursorShape = static_cast<QActionGroup *>(sender());
    if(!keyboardCursorShape)
        qFatal("keyboardCursorShape is NULL");

    Properties::Instance()->keyboardCursorShape =
            keyboardCursorShape->actions().indexOf(triggered);

    Properties::Instance()->saveSettings();
    propertiesChanged();
}

void TabWidget::propertiesChanged()
{
    for (int i = 0; i < count(); ++i)
    {
        TermWidgetHolder *console = static_cast<TermWidgetHolder*>(widget(i));
        console->propertiesChanged();
    }
    showHideTabBar();
}

void TabWidget::clearActiveTerminal()
{
    reinterpret_cast<TermWidgetHolder*>(widget(currentIndex()))->clearActiveTerminal();
}

void TabWidget::saveSession()
{
    int ix = currentIndex();
    reinterpret_cast<TermWidgetHolder*>(widget(ix))->saveSession(tabText(ix));
}

void TabWidget::loadSession()
{
    reinterpret_cast<TermWidgetHolder*>(widget(currentIndex()))->loadSession();
}

void TabWidget::preset2Horizontal()
{
    TerminalConfig defaultConfig;
    int ix = TabWidget::addNewTab(defaultConfig);
    TermWidgetHolder* term = reinterpret_cast<TermWidgetHolder*>(widget(ix));
    term->splitHorizontal(term->currentTerminal());
    // switch to the 1st terminal
    term->directionalNavigation(NavigationDirection::Left);
}

void TabWidget::preset2Vertical()
{
    TerminalConfig defaultConfig;
    int ix = TabWidget::addNewTab(defaultConfig);
    TermWidgetHolder* term = reinterpret_cast<TermWidgetHolder*>(widget(ix));
    term->splitVertical(term->currentTerminal());
    // switch to the 1st terminal
    term->directionalNavigation(NavigationDirection::Left);
}

void TabWidget::preset4Terminals()
{
    TerminalConfig defaultConfig;
    int ix = TabWidget::addNewTab(defaultConfig);
    TermWidgetHolder* term = reinterpret_cast<TermWidgetHolder*>(widget(ix));
    term->splitVertical(term->currentTerminal());
    term->splitHorizontal(term->currentTerminal());
    term->directionalNavigation(NavigationDirection::Left);

    term->splitHorizontal(term->currentTerminal());
    // switch to the 1st terminal
    term->directionalNavigation(NavigationDirection::Top);
}

void TabWidget::showHideTabBar()
{
    if (Properties::Instance()->tabBarless)
        tabBar()->setVisible(false);
    else
        tabBar()->setVisible(!Properties::Instance()->hideTabBarWithOneTab || count() > 1);
}
