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

#include <qtermwidget.h>
#include "tabwidget.h"
#include "config.h"
#include "colorschemadialog.h"

#define TAB_INDEX_PROPERTY "tab_index"


TabWidget::TabWidget(QWidget* parent) : QTabWidget(parent), tabNumerator(0)
{
    loadSettings();
    setFocusPolicy(Qt::NoFocus);
    tabBar()->installEventFilter(this);
    QToolButton* tb = new QToolButton(this);
    tb->setIcon(QIcon(":/icons/remove.png"));
    setCornerWidget(tb, Qt::BottomRightCorner);
    connect(tb, SIGNAL(clicked()), SLOT(removeCurrentTerminal()));

    tb = new QToolButton(this);
    tb->setIcon(QIcon(":/icons/add.png"));
    setCornerWidget(tb, Qt::BottomLeftCorner);
    connect(tb, SIGNAL(clicked()), SLOT(addTerminal()));
}

void TabWidget::setWorkDirectory(const QString& dir)
{
    this->work_dir = dir;
}

//void TabWidget::setShellProgram(const QString& program)
//{/
//    this->shell_program = program;
//}

int TabWidget::addTerminal(const QString& shell_program)
{
    tabNumerator ++;
    QString label = QString(tr("Shell No. %1")).arg(tabNumerator);
    QTermWidget* console = createNewTerminal(shell_program);
    int index = addTab(console, label);
    recountIndexes();
    setCurrentIndex(index);
    console->setFocus();
    return index;
}

void TabWidget::recountIndexes()
{
    for(int i = 0; i < count(); i++)
        widget(i)->setProperty(TAB_INDEX_PROPERTY, i);
}

QTermWidget* TabWidget::createNewTerminal(const QString& shell_program)
{
    QTermWidget *console = new QTermWidget(0);

    console->setFlowControlEnabled(FLOW_CONTROL_ENABLED);
    console->setFlowControlWarningEnabled(FLOW_CONTROL_WARNING_ENABLED);

    console->installEventFilter(this);
    console->setColorScheme(currentColorScheme);
    console->setTerminalFont(currentFont);
    console->setHistorySize(5000);
    console->setScrollBarPosition(QTermWidget::ScrollBarRight);
    
    if(!this->work_dir.isNull())
        console->setWorkingDirectory(this->work_dir);
	
    if(!shell_program.isNull())
        console->setShellProgram(shell_program);

    this->createActions(console);
    connect(console, SIGNAL(finished()), SLOT(removeFinished()));

    console->startShellProgram();
    
    return console;
}

void TabWidget::createActions(QWidget* console)
{
    QAction* act = new QAction(QIcon(":/icons/editcopy.png"), tr("&Copy selection"), this);
    act->setShortcuts(QList<QKeySequence>() << shortcuts[COPY_SELECTION]);
    connect(act, SIGNAL(triggered()), console, SLOT(copyClipboard()));
    console->addAction(act);

    act = new QAction(QIcon(":/icons/editpaste.png"), tr("&Paste Selection"), this);
    act->setShortcuts(QList<QKeySequence>() << shortcuts[PASTE_SELECTION]);
    connect(act, SIGNAL(triggered()), console, SLOT(pasteClipboard()));
    console->addAction(act);

    act = new QAction(this);
    act->setSeparator(true);
    console->addAction(act);

    act = new QAction(tr("&Rename session..."), this);
    act->setShortcut(shortcuts[RENAME_SESSION]);
    connect(act, SIGNAL(triggered()), this, SLOT(renameSession()));
    console->addAction(act);

    // Setting windows style actions
    styleAct = new QActionGroup(this);
    foreach (QString s, QStyleFactory::keys())
    {
        act = new QAction(s, this);
        act->setData(s);
        styleAct->addAction(act);
    }
	connect(styleAct, SIGNAL(triggered(QAction*)), this, SLOT(changeStyle(QAction*)));

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

bool TabWidget::eventFilter(QObject* watched, QEvent* event)
{
    if(event->type() == QEvent::MouseButtonDblClick)
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(event);
        if(me->button() == Qt::LeftButton)
            renameSession();
        return true;
    }
    else if(event->type() == QEvent::ContextMenu)
    {
        QContextMenuEvent* ce = static_cast<QContextMenuEvent*>(event);
        QTermWidget* console = qobject_cast<QTermWidget*>(watched);
        if(console)
        {
            QMenu menu(console);
            menu.addActions(console->actions());
            menu.addSeparator();
            menu.addAction(QIcon(":/icons/close.png"), tr("Close session"), this, SLOT(removeCurrentTerminal()));
            menu.exec(ce->globalPos());
            return true;
        }
        QTabBar* tabBar =  qobject_cast<QTabBar*>(watched);
        if(tabBar)
        {
            QMenu menu(tabBar);
            menu.addAction(tr("Rename session"), this, SLOT(renameSession()), tr(RENAME_SESSION_SHORTCUT));
            menu.addSeparator();
            menu.addAction(QIcon(":/icons/close.png"), tr("Close session"), this, SLOT(removeCurrentTerminal()));
            menu.exec(ce->globalPos());
            return true;
        }
    }
    return false;
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
        addTerminal();
}

void TabWidget::contextMenuEvent ( QContextMenuEvent * event )
{
    QMenu menu(this);
    QAction * act;

// TODO/FIXME: a bug in the Qt library: http://bugreports.qt.nokia.com/browse/QTBUG-7769
#ifndef Q_WS_MAC || QT_VERSION >= 0x040603
    act = new QAction(QIcon(":/icons/font.png"), tr("Change Font"), this);
    connect(act, SIGNAL(triggered()), this, SLOT(changeFont()));
    menu.addAction(act);
#endif

    act = new QAction(this);
    act->setSeparator(true);
    menu.addAction(act);

    act = new QAction(QIcon(":/icons/theme.png"), tr("Change Color Scheme"), this);
    connect(act, SIGNAL(triggered()), this, SLOT(changeColorSchema()));
    menu.addAction(act);

    QMenu styleMenu("Change Style", this);
    foreach(act, styleAct->actions())
        styleMenu.addAction(act);
    menu.addMenu(&styleMenu);

    menu.exec(event->globalPos());
}

void TabWidget::removeFinished()
{
    QObject* term = sender();
    QVariant prop = term->property(TAB_INDEX_PROPERTY);
    if(prop.isValid() && prop.canConvert(QVariant::Int))
    {
        int index = prop.toInt();
	removeTerminal(index);
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

void TabWidget::removeTerminal(int index)
{    
    removeTab(index);
}

void TabWidget::removeCurrentTerminal()
{
    if(QMessageBox::question(this,
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

void TabWidget::changeFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, currentFont, this);
    if(ok)
    {
        for(int i = 0; i < count(); ++i)
        {
            QTermWidget *console = static_cast<QTermWidget*>(widget(i));
            console->setTerminalFont(font);
            currentFont = font;
            console->update();
        }
    }
}

void TabWidget::changeColorSchema()
{
    ColorSchemaDialog dlg(this);
    dlg.setColorScheme(currentColorScheme);

    if (dlg.exec() == QDialog::Accepted)
    {
        int scheme = dlg.colorScheme();
        for(int i = 0; i < count(); ++i)
        {
            QTermWidget *console = static_cast<QTermWidget*>(widget(i));
            console->setColorScheme(scheme);
            currentColorScheme = scheme;
            console->update();
        }
    }

}

void TabWidget::loadSettings()
{
    QSettings settings(QDir::homePath()+"/.qterminal", QSettings::IniFormat);
    currentColorScheme = settings.value("color_scheme", QVariant(COLOR_SCHEME_GREEN_ON_BLACK)).toInt();

    QFont default_font = QApplication::font();
    default_font.setFamily("Monospace");
    default_font.setPointSize(10);
    default_font.setStyleHint(QFont::TypeWriter);

    currentFont = qvariant_cast<QFont>(settings.value("font", default_font));
    
    settings.beginGroup("Shortcuts");
    shortcuts[COPY_SELECTION] = settings.value(COPY_SELECTION, COPY_SELECTION_SHORTCUT).toString();
    shortcuts[PASTE_SELECTION] = settings.value(PASTE_SELECTION, PASTE_SELECTION_SHORTCUT).toString();
    shortcuts[RENAME_SESSION] = settings.value(RENAME_SESSION, RENAME_SESSION_SHORTCUT).toString();
    settings.endGroup();
				
}

void TabWidget::saveSettings()
{
    QSettings settings(QDir::homePath()+"/.qterminal", QSettings::IniFormat);
    settings.setValue("color_scheme", currentColorScheme);
    settings.setValue("font", currentFont);
    
    settings.beginGroup("Shortcuts");
    QMapIterator<QString, QString> it(shortcuts);
    while (it.hasNext()) {
	it.next();
	settings.setValue(it.key(), it.value());
    }				    
}


void TabWidget::changeStyle(QAction* act)
{
    QString style = act->data().toString();
    QApplication::setStyle(style);
}
