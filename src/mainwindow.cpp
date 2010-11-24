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
#include "termwidgetholder.h"
#include "config.h"
#include "version.h"
#include "properties.h"
#include "propertiesdialog.h"


MainWindow::MainWindow(const QString& work_dir, const QString& command, QWidget * parent, Qt::WindowFlags f) : QMainWindow(parent,f)
{
    setupUi(this);
    connect(actAbout, SIGNAL(triggered()), SLOT(actAbout_triggered()));
    connect(actAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(actQuit, SIGNAL(triggered()), SLOT(close()));
    connect(actProperties, SIGNAL(triggered()), SLOT(actProperties_triggered()));

    restoreGeometry(Properties::Instance()->mainWindowGeometry);
    restoreState(Properties::Instance()->mainWindowState);

    connect(consoleTabulator, SIGNAL(quit_notification()), SLOT(quit()));
    consoleTabulator->setWorkDirectory(work_dir);
    //consoleTabulator->setShellProgram(command);
    consoleTabulator->addNewTab(command);
    setWindowTitle(STR_VERSION);
    setWindowIcon(QIcon(":/icons/main.png"));
    this->addActions();
}


void MainWindow::addActions()
{
    QSettings settings(QDir::homePath()+"/.qterminal", QSettings::IniFormat);
    settings.beginGroup("Shortcuts");

    QAction* act = new QAction(tr("Add New Session"), this);
    act->setShortcut(Properties::Instance()->shortcuts[ADD_TAB]);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(addNewTab()));
    addAction(act);

    act = new QAction(tr("Switch to the Next Sub-terminal"), this);
    act->setShortcut(Properties::Instance()->shortcuts[SUB_NEXT]);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(switchNextSubterminal()));
    addAction(act);

    act = new QAction(tr("Switch to the Previous Sub-terminal"), this);
    act->setShortcut(Properties::Instance()->shortcuts[SUB_PREV]);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(switchPrevSubterminal()));
    addAction(act);

    act = new QAction(tr("Switch To Right"), this);
    act->setShortcut(Properties::Instance()->shortcuts[TAB_RIGHT]);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(switchToRight()));
    addAction(act);

    act = new QAction(tr("Switch To Left"), this);
    act->setShortcut(Properties::Instance()->shortcuts[TAB_LEFT]);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(switchToLeft()));
    addAction(act);

    act = new QAction(tr("Move Tab To Left"), this);
    act->setShortcut(Properties::Instance()->shortcuts[MOVE_LEFT]);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(moveLeft()));
    addAction(act);

    act = new QAction(tr("Move Tab To Right"), this);
    act->setShortcut(Properties::Instance()->shortcuts[MOVE_RIGHT]);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(moveRight()));
    addAction(act);

    act = new QAction(this);
    act->setSeparator(true);
    addAction(act);

#if 0
    // TODO/FIXME: unimplemented for now
    act = new QAction(tr("Save Session"), this);
    // do not use sequences for this task - it collides with eg. mc shorcuts
    // and mainly - it's not used too often
    //act->setShortcut(QKeySequence::Save);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(saveSession()));
    addAction(act);

    act = new QAction(tr("Load Session"), this);
    // do not use sequences for this task - it collides with eg. mc shorcuts
    // and mainly - it's not used too often
    //act->setShortcut(QKeySequence::Open);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(loadSession()));
    addAction(act);
#endif
    act = new QAction(this);
    act->setSeparator(true);
    addAction(act);

    settings.endGroup();

    menu_File->insertActions(actQuit, actions());

    // apply props
    propertiesChanged();
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
        Properties::Instance()->mainWindowGeometry = saveGeometry();
        Properties::Instance()->mainWindowState = saveState();
        Properties::Instance()->saveSettings();
        ev->accept();
    }
    else
    {
        ev->ignore();
    }
}

void MainWindow::quit()
{
    Properties::Instance()->mainWindowGeometry = saveGeometry();
    Properties::Instance()->mainWindowState = saveState();
    Properties::Instance()->saveSettings();
    QApplication::exit(0);
}

void MainWindow::actAbout_triggered()
{
    QMessageBox::about(this, STR_VERSION, tr("A lightweight multiplatform terminal emulator"));
}

void MainWindow::actProperties_triggered()
{
    QStringList emulations = QTermWidget::availableKeyBindings();
    PropertiesDialog * p = new PropertiesDialog(emulations, this);
    connect(p, SIGNAL(propertiesChanged()), this, SLOT(propertiesChanged()));
    p->exec();
}

void MainWindow::propertiesChanged()
{
    QApplication::setStyle(Properties::Instance()->guiStyle);
    setWindowOpacity(Properties::Instance()->appOpacity/100.0);
    consoleTabulator->propertiesChanged();
}

