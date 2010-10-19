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

#include <QtGui>
#include <QtCore>

#include "termwidget.h"
#include "tabwidget.h"
#include "config.h"
#include "properties.h"
#include "termwidgetholder.h"


#define TAB_INDEX_PROPERTY "tab_index"


TabWidget::TabWidget(QWidget* parent) : QTabWidget(parent), tabNumerator(0)
{
    setFocusPolicy(Qt::NoFocus);
    QToolButton* tb = new QToolButton(this);
    tb->setIcon(QIcon(":/icons/remove.png"));
    setCornerWidget(tb, Qt::BottomRightCorner);
    connect(tb, SIGNAL(clicked()), SLOT(removeCurrentTab()));

    tb = new QToolButton(this);
    tb->setIcon(QIcon(":/icons/add.png"));
    setCornerWidget(tb, Qt::BottomLeftCorner);
    connect(tb, SIGNAL(clicked()), SLOT(addNewTab()));
}

TermWidget * TabWidget::terminal()
{
   return reinterpret_cast<TermWidgetHolder*>(widget(0))->terminal();
}

void TabWidget::setWorkDirectory(const QString& dir)
{
    this->work_dir = dir;
}

//void TabWidget::setShellProgram(const QString& program)
//{/
//    this->shell_program = program;
//}

int TabWidget::addNewTab(const QString& shell_program)
{
    tabNumerator ++;
    QString label = QString(tr("Shell No. %1")).arg(tabNumerator);

    TermWidgetHolder * console = new TermWidgetHolder(work_dir, this);
    connect(console, SIGNAL(finished()), SLOT(removeFinished()));
    //connect(console, SIGNAL(lastTerminalClosed()), this, SLOT(removeCurrentTab()));
    connect(console, SIGNAL(lastTerminalClosed()), this, SLOT(removeFinished()));    
    connect(console, SIGNAL(renameSession()), this, SLOT(renameSession()));

    int index = addTab(console, label);
    recountIndexes();
    setCurrentIndex(index);
    console->setInitialFocus();
    return index;
}

void TabWidget::recountIndexes()
{
    for(int i = 0; i < count(); i++)
        widget(i)->setProperty(TAB_INDEX_PROPERTY, i);
}

void TabWidget::renameSession()
{
    bool ok = false;
    QString text = QInputDialog::getText(this, tr("Tab name"),
                                        tr("New tab name:"), QLineEdit::Normal,
                                        QString(), &ok);
    if(ok && !text.isEmpty())
    {
        setTabText(currentIndex(), text);
    }
}

void TabWidget::refreshWindow()
{
    QWidget* prevFocused = currentWidget();
    this->setFocus();
    prevFocused->setFocus();
}


void TabWidget::mouseDoubleClickEvent ( QMouseEvent * event )
{
    if(event->button() == Qt::LeftButton)
        addNewTab();
}

void TabWidget::contextMenuEvent ( QContextMenuEvent * event )
{
    QMenu menu(this);
    QAction * act;

    menu.addAction(QIcon(":/icons/close.png"), tr("Close session"), this, SLOT(removeCurrentTerminal()));
    menu.addAction(tr("Rename session"), this, SLOT(renameSession()), tr(RENAME_SESSION_SHORTCUT));

    menu.exec(event->globalPos());
}

void TabWidget::removeFinished()
{
    QObject* term = sender();
    QVariant prop = term->property(TAB_INDEX_PROPERTY);
    if(prop.isValid() && prop.canConvert(QVariant::Int))
    {
        int index = prop.toInt();
	    removeTab(index);
        if(count() == 0)
	    emit quit_notification();
    }
}

void TabWidget::removeTab(int index)
{
    setUpdatesEnabled(false);
    delete widget(index);
    QTabWidget::removeTab(index);
    recountIndexes();
    int current = currentIndex();
    if(current >= 0 ) widget(current)->setFocus();
    setUpdatesEnabled(true);
}

void TabWidget::removeCurrentTab()
{
    if (QMessageBox::question(this,
                    tr("Close current session"),
                    tr("Are you sure you want to close current sesstion?"),
                    QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
        if(count() > 1) removeTab(currentIndex());
        else QApplication::exit(0);
    }
}

int TabWidget::traverseRight()
{
    int next_pos = currentIndex() + 1;
    if (next_pos < count())
        setCurrentIndex(next_pos);
    else
        setCurrentIndex(0);
    return currentIndex();
}

int TabWidget::traverseLeft()
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
        recountIndexes();
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

void TabWidget::propertiesChanged()
{
    for (int i = 0; i < count(); ++i)
    {
        TermWidgetHolder *console = static_cast<TermWidgetHolder*>(widget(i));
        console->propertiesChanged();
    }
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

