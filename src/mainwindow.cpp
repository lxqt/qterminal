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

#include "mainwindow.h"
#include "tabwidget.h"
#include "config.h"
#include "version.h"


MainWindow::MainWindow(const QString& work_dir, const QString& command, QWidget * parent, Qt::WindowFlags f) : QMainWindow(parent,f)
{
    setupUi(this);
    loadSettings();

    connect(consoleTabulator, SIGNAL(quit_notification()), SLOT(quit()));
    consoleTabulator->setWorkDirectory(work_dir);
    //consoleTabulator->setShellProgram(command);
    consoleTabulator->addTerminal(command);
    setWindowTitle(STR_VERSION);
    setWindowIcon(QIcon(":/icons/main.png"));
    this->addActions();
}


void MainWindow::addActions()
{
    QSettings settings(QDir::homePath()+"/.qterminal", QSettings::IniFormat);
    settings.beginGroup("Shortcuts");

    QAction* act = new QAction(this);

    act->setShortcut(shortcuts[ADD_TAB]);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(addTerminal()));
    addAction(act);

    act = new QAction(this);
    act->setShortcut(shortcuts[TAB_RIGHT]);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(traverseRight()));
    addAction(act);

    act = new QAction(this);
    act->setShortcut(shortcuts[TAB_LEFT]);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(traverseLeft()));
    addAction(act);

    act = new QAction(this);
    act->setShortcut(shortcuts[MOVE_LEFT]);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(moveLeft()));
    addAction(act);

    act = new QAction(this);
    act->setShortcut(shortcuts[MOVE_RIGHT]);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(moveRight()));
    addAction(act);

    settings.endGroup();
}

MainWindow::~MainWindow()
{
}

void MainWindow::on_consoleTabulator_currentChanged(int)
{
}

void MainWindow::closeEvent(QCloseEvent* ev)
{
    if(QMessageBox::question(this,
                                tr("Close qterminal"),
                                tr("Are you sure you want to exit?"),
                                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
        saveSettings();
        ev->accept();
    }
    else
    {
        ev->ignore();
    }
}

void MainWindow::quit()
{
    saveSettings();
    QApplication::exit(0);
}

void MainWindow::loadSettings()
{
    QSettings settings(QDir::homePath() + "/.qterminal", QSettings::IniFormat);
    int width = settings.value("width", QVariant(DEFAULT_WIDTH)).toInt();
    int height = settings.value("height", QVariant(DEFAULT_HEIGHT)).toInt();
    int x = settings.value("x", QVariant(0)).toInt();
    int y = settings.value("y", QVariant(0)).toInt();
    this->setGeometry(QRect(x, y, width, height));
    
    consoleTabulator->loadSettings();

    settings.beginGroup("Shortcuts");
    shortcuts[ADD_TAB] = settings.value(ADD_TAB, ADD_TAB_SHORTCUT).toString();
    shortcuts[TAB_RIGHT] = settings.value(TAB_RIGHT, TAB_RIGHT_SHORTCUT).toString();
    shortcuts[TAB_LEFT] = settings.value(TAB_LEFT, TAB_LEFT_SHORTCUT).toString();
    shortcuts[MOVE_LEFT] = settings.value(MOVE_LEFT, MOVE_LEFT_SHORTCUT).toString();
    shortcuts[MOVE_RIGHT] = settings.value(MOVE_RIGHT, MOVE_RIGHT_SHORTCUT).toString();
    settings.endGroup();
}

void MainWindow::saveSettings()
{
    QSettings settings(QDir::homePath()+"/.qterminal", QSettings::IniFormat);
    settings.setValue("width", this->width());
    settings.setValue("height", this->height());
    settings.setValue("x", this->pos().x());
    settings.setValue("y", this->pos().y());

    consoleTabulator->saveSettings();

    settings.beginGroup("Shortcuts");
    QMapIterator<QString, QString> it(shortcuts);
    while (it.hasNext()) {
        it.next();
        settings.setValue(it.key(), it.value());
    }
}
