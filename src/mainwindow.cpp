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
#include "properties.h"
#include "propertiesdialog.h"


MainWindow::MainWindow(const QString& work_dir,
                       const QString& command,
                       QWidget * parent,
                       Qt::WindowFlags f) :
QMainWindow(parent,f)
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
    consoleTabulator->setTabPosition((QTabWidget::TabPosition)Properties::Instance()->tabsPos);
    //consoleTabulator->setShellProgram(command);
    consoleTabulator->addNewTab(command);

    setWindowTitle(QString("QTerminal ") + STR_VERSION);
    setWindowIcon(QIcon(":/icons/qterminal.png"));

    setup_ActionsMenu_Actions();
    setup_WindowMenu_Actions();


    if(Properties::Instance()->borderless) {
        toggleBorder->setChecked(true);
        toggleBorderless();
    }

    if(Properties::Instance()->tabBarless) {
        toggleTabbar->setChecked(true);
        toggleTabBar();
    }

}

MainWindow::~MainWindow()
{
}

void MainWindow::setup_ActionsMenu_Actions()
{
    QSettings settings(QDir::homePath()+"/.qterminal", QSettings::IniFormat);
    settings.beginGroup("Shortcuts");

    QAction* act = new QAction(QIcon(":/icons/list-add.png"), tr("Add New Session"), this);
    act->setShortcut(QKeySequence::AddTab);//Properties::Instance()->shortcuts[ADD_TAB]);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(addNewTab()));
    menu_Actions->addAction(act);

    act = new QAction(QIcon(":/icons/list-remove.png"), tr("Close Active Session"), this);
    act->setShortcut(QKeySequence::Close);//Properties::Instance()->shortcuts[CLOSE_TAB]);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(removeCurrentTab()));
    menu_Actions->addAction(act);

    menu_Actions->addSeparator();

    act = new QAction(tr("Split Terminal Horizontally"), this);
//    act->setShortcut(Properties::Instance()->shortcuts[SUB_NEXT]);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(splitHorizontally()));
    menu_Actions->addAction(act);

    act = new QAction(tr("Split Terminal Vertically"), this);
//    act->setShortcut(Properties::Instance()->shortcuts[SUB_NEXT]);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(splitVertically()));
    menu_Actions->addAction(act);

    act = new QAction(tr("Collapse Sub-Terminal"), this);
//    act->setShortcut(Properties::Instance()->shortcuts[SUB_NEXT]);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(splitCollapse()));
    menu_Actions->addAction(act);

    act = new QAction(tr("Switch to the Next Sub-terminal"), this);
    act->setShortcut(Properties::Instance()->shortcuts[SUB_NEXT]);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(switchNextSubterminal()));
    menu_Actions->addAction(act);

    act = new QAction(tr("Switch to the Previous Sub-terminal"), this);
    act->setShortcut(Properties::Instance()->shortcuts[SUB_PREV]);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(switchPrevSubterminal()));
    menu_Actions->addAction(act);

    menu_Actions->addSeparator();

    act = new QAction(tr("Switch To Right"), this);
    act->setShortcut(Properties::Instance()->shortcuts[TAB_RIGHT]);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(switchToRight()));
    menu_Actions->addAction(act);

    act = new QAction(tr("Switch To Left"), this);
    act->setShortcut(Properties::Instance()->shortcuts[TAB_LEFT]);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(switchToLeft()));
    menu_Actions->addAction(act);

    act = new QAction(tr("Move Tab To Left"), this);
    act->setShortcut(Properties::Instance()->shortcuts[MOVE_LEFT]);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(moveLeft()));
    menu_Actions->addAction(act);

    act = new QAction(tr("Move Tab To Right"), this);
    act->setShortcut(Properties::Instance()->shortcuts[MOVE_RIGHT]);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(moveRight()));
    menu_Actions->addAction(act);

    menu_Actions->addSeparator();


#if 0
    act = new QAction(this);
    act->setSeparator(true);
    addAction(act);

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

    //menu_Actions->addSeparator();

    settings.endGroup();

    //menu_Actions->insertActions(actQuit, actions());

    // apply props
    propertiesChanged();
}

void MainWindow::setup_WindowMenu_Actions()
{
    toggleBorder = new QAction(tr("Toggle Borderless"), this);
    //toggleBorder->setObjectName("toggle_Borderless");
    toggleBorder->setCheckable(true);
    connect(toggleBorder, SIGNAL(triggered()), this, SLOT(toggleBorderless()));
    menu_Window->addAction(toggleBorder);

    toggleTabbar = new QAction(tr("Toggle TabBar"), this);
    //toggleTabbar->setObjectName("toggle_TabBar");
    toggleTabbar->setCheckable(true);
    connect(toggleTabbar, SIGNAL(triggered()), this, SLOT(toggleTabBar()));
    menu_Window->addAction(toggleTabbar);

    menu_Window->addSeparator();

    /* tabs position */
    tabPosition = new QActionGroup(this);
    QAction *tabBottom = new QAction(tr("Bottom"), this);
    QAction *tabTop = new QAction(tr("Top"), this);
    QAction *tabRight = new QAction(tr("Right"), this);
    QAction *tabLeft = new QAction(tr("Left"), this);
    tabPosition->addAction(tabTop);
    tabPosition->addAction(tabBottom);
    tabPosition->addAction(tabLeft);
    tabPosition->addAction(tabRight);

    for(int i = 0; i < tabPosition->actions().size(); ++i)
        tabPosition->actions().at(i)->setCheckable(true);

    tabPosition->actions().at(Properties::Instance()->tabsPos)->setChecked(true);

    connect(tabPosition, SIGNAL(triggered(QAction *)),
             consoleTabulator, SLOT(changeTabPosition(QAction *)) );

    tabPosMenu = new QMenu(tr("Tabs Layout"), menu_Window);
    tabPosMenu->setObjectName("tabPosMenu");

    for(int i=0; i < tabPosition->actions().size(); ++i) {
        tabPosMenu->addAction(tabPosition->actions().at(i));
    }

    connect(menu_Window, SIGNAL(hovered(QAction *)),
            this, SLOT(updateActionGroup(QAction *)));
    menu_Window->addMenu(tabPosMenu);
    /* */

    /* Scrollbar */
    scrollBarPosition = new QActionGroup(this);
    QAction *scrollNone = new QAction(tr("None"), this);
    QAction *scrollRight = new QAction(tr("Right"), this);
    QAction *scrollLeft = new QAction(tr("Left"), this);

    /* order of insertion is dep. on QTermWidget::ScrollBarPosition enum */
    scrollBarPosition->addAction(scrollNone);
    scrollBarPosition->addAction(scrollLeft);
    scrollBarPosition->addAction(scrollRight);

    for(int i = 0; i < scrollBarPosition->actions().size(); ++i)
        scrollBarPosition->actions().at(i)->setCheckable(true);

    scrollBarPosition->actions().at(Properties::Instance()->scrollBarPos)->setChecked(true);

    connect(scrollBarPosition, SIGNAL(triggered(QAction *)),
             consoleTabulator, SLOT(changeScrollPosition(QAction *)) );

    scrollPosMenu = new QMenu(tr("Scrollbar Layout"), menu_Window);
    scrollPosMenu->setObjectName("scrollPosMenu");

    for(int i=0; i < scrollBarPosition->actions().size(); ++i) {
        scrollPosMenu->addAction(scrollBarPosition->actions().at(i));
    }

    menu_Window->addMenu(scrollPosMenu);
    /* */
}

void MainWindow::on_consoleTabulator_currentChanged(int)
{
}

void MainWindow::toggleTabBar()
{
    if(toggleTabbar->isChecked())
        consoleTabulator->tabBar()->hide();
    else
        consoleTabulator->tabBar()->show();

    Properties::Instance()->tabBarless = toggleTabbar->isChecked();
}

void MainWindow::toggleBorderless()
{
    setWindowFlags(windowFlags() ^ Qt::FramelessWindowHint);
    show();
    setWindowState(Qt::WindowActive); /* don't loose focus on the window */
    Properties::Instance()->borderless = toggleBorder->isChecked();
}

void MainWindow::closeEvent(QCloseEvent *ev)
{
    if (!Properties::Instance()->askOnExit)
    {
        ev->accept();
        return;
    }

    QDialog * dia = new QDialog(this);
    dia->setObjectName("exitDialog");
    dia->setWindowTitle(tr("Exit QTerminal"));

    QCheckBox * dontAskCheck = new QCheckBox(tr("Do not ask again"), dia);
    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::No, Qt::Horizontal, dia);

    connect(buttonBox, SIGNAL(accepted()), dia, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), dia, SLOT(reject()));
    
    QVBoxLayout * lay = new QVBoxLayout();
    lay->addWidget(new QLabel(tr("Are you sure you want to exit?")));
    lay->addWidget(dontAskCheck);
    lay->addWidget(buttonBox);
    dia->setLayout(lay);

    if (dia->exec() == QDialog::Accepted) {
        Properties::Instance()->mainWindowGeometry = saveGeometry();
        Properties::Instance()->mainWindowState = saveState();
        Properties::Instance()->askOnExit = !dontAskCheck->isChecked();
        Properties::Instance()->saveSettings();
        ev->accept();
    } else {
        ev->ignore();
    }

    dia->deleteLater();
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
    QMessageBox::about(this, QString("QTerminal ") + STR_VERSION, tr("A lightweight multiplatform terminal emulator"));
}

void MainWindow::actProperties_triggered()
{
    QStringList emulations = QTermWidget::availableKeyBindings();
    QStringList colorSchemes = QTermWidget::availableColorSchemes();
    PropertiesDialog *p = new PropertiesDialog(emulations, colorSchemes, this);
    connect(p, SIGNAL(propertiesChanged()), this, SLOT(propertiesChanged()));
    p->exec();
}

void MainWindow::propertiesChanged()
{
    QApplication::setStyle(Properties::Instance()->guiStyle);
    setWindowOpacity(Properties::Instance()->appOpacity/100.0);
    consoleTabulator->setTabPosition((QTabWidget::TabPosition)Properties::Instance()->tabsPos);
    consoleTabulator->propertiesChanged();
}

void MainWindow::updateActionGroup(QAction *a)
{
    if (a->parent()->objectName() == tabPosMenu->objectName()) {
        tabPosition->actions().at(Properties::Instance()->tabsPos)->setChecked(true);
    }
}
