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

// TODO/FXIME: probably remove. QSS makes it unusable on mac...
#define QSS_DROP    "MainWindow {border: 1px solid rgba(0, 0, 0, 50%);}\n"

MainWindow::MainWindow(const QString& work_dir,
                       const QString& command,
                       bool dropMode,
                       QWidget * parent,
                       Qt::WindowFlags f)
    : QMainWindow(parent,f),
      m_initShell(command),
      m_initWorkDir(work_dir),
      m_dropLockButton(0),
      m_dropMode(dropMode)
{
    setupUi(this);
   
    migrate_settings();

    Properties::Instance()->loadSettings();

    connect(actAbout, SIGNAL(triggered()), SLOT(actAbout_triggered()));
    connect(actAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(&m_dropShortcut, SIGNAL(activated()), SLOT(showHide()));

    setContentsMargins(0, 0, 0, 0);
    if (m_dropMode) {
        this->enableDropMode();
        setStyleSheet(QSS_DROP);
    }
    else {
        restoreGeometry(Properties::Instance()->mainWindowGeometry);
        restoreState(Properties::Instance()->mainWindowState);
    }

    connect(consoleTabulator, SIGNAL(closeTabNotification()), SLOT(close()));
    consoleTabulator->setWorkDirectory(work_dir);
    consoleTabulator->setTabPosition((QTabWidget::TabPosition)Properties::Instance()->tabsPos);
    //consoleTabulator->setShellProgram(command);
    consoleTabulator->addNewTab(command);

    setWindowTitle("QTerminal");
    setWindowIcon(QIcon(":/icons/qterminal.png"));
    
    setup_FileMenu_Actions();
    setup_ActionsMenu_Actions();
    setup_ViewMenu_Actions();

    // Add global rename Session shortcut
    renameSession = new QAction(tr("Rename Session"), this);
    renameSession->setShortcut(QKeySequence(tr(RENAME_SESSION_SHORTCUT)));
    connect(renameSession, SIGNAL(triggered()), consoleTabulator, SLOT(renameSession()));
    addAction(renameSession);
}

MainWindow::~MainWindow()
{
}

void MainWindow::migrate_settings()
{
    // Deal with rearrangements of settings. 
    // If this method becomes unbearably huge we should look at the config-update
    // system used by kde and razor.
    QSettings settings;
    QString last_version = settings.value("version", "0.0.0").toString();
    // Handle configchanges in 0.4.0 (renaming 'Paste Selection' -> 'Paste Clipboard') 
    if (last_version < "0.4.0") 
    {
        qDebug() << "Migrating settings from" << last_version << "to 0.4.0";
        settings.beginGroup("Shortcuts");
        QString tmp = settings.value("Paste Selection", PASTE_CLIPBOARD_SHORTCUT).toString();
        settings.setValue(PASTE_CLIPBOARD, tmp);
        settings.remove("Paste Selection");
        settings.endGroup();

        settings.setValue("version", "0.4.0");
    }
}


void MainWindow::enableDropMode()
{
    setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);

    m_dropLockButton = new QToolButton(this);
    consoleTabulator->setCornerWidget(m_dropLockButton, Qt::BottomRightCorner);
    m_dropLockButton->setCheckable(true);
    m_dropLockButton->connect(m_dropLockButton, SIGNAL(clicked(bool)), this, SLOT(setKeepOpen(bool)));
    setKeepOpen(Properties::Instance()->dropKeepOpen);
    m_dropLockButton->setAutoRaise(true);


    setDropShortcut(Properties::Instance()->dropShortCut);
    realign();
}

void MainWindow::setDropShortcut(QKeySequence dropShortCut)
{
    if (!m_dropMode)
        return;

    if (m_dropShortcut.shortcut() != dropShortCut)
    {
        m_dropShortcut.setShortcut(dropShortCut);
        qWarning() << tr("Press \"%1\" to see the terminal.").arg(dropShortCut.toString());
    }
}

void MainWindow::setup_ActionsMenu_Actions()
{
    QSettings settings;
    settings.beginGroup("Shortcuts");

    QKeySequence seq;

    Properties::Instance()->actions[CLEAR_TERMINAL] = new QAction(tr("Clear Current Tab"), this);
    seq = QKeySequence::fromString(settings.value(CLEAR_TERMINAL, CLEAR_TERMINAL_SHORTCUT).toString());
    Properties::Instance()->actions[CLEAR_TERMINAL]->setShortcut(seq);
    connect(Properties::Instance()->actions[CLEAR_TERMINAL], SIGNAL(triggered()), consoleTabulator, SLOT(clearActiveTerminal()));
    menu_Actions->addAction(Properties::Instance()->actions[CLEAR_TERMINAL]);

    menu_Actions->addSeparator();

    Properties::Instance()->actions[TAB_NEXT] = new QAction(tr("Next Tab"), this);
    seq = QKeySequence::fromString( settings.value(TAB_NEXT, TAB_NEXT_SHORTCUT).toString() );
    Properties::Instance()->actions[TAB_NEXT]->setShortcut(seq);
    connect(Properties::Instance()->actions[TAB_NEXT], SIGNAL(triggered()), consoleTabulator, SLOT(switchToRight()));
    menu_Actions->addAction(Properties::Instance()->actions[TAB_NEXT]);

    Properties::Instance()->actions[TAB_PREV] = new QAction(tr("Previous Tab"), this);
    seq = QKeySequence::fromString( settings.value(TAB_PREV, TAB_PREV_SHORTCUT).toString() );
    Properties::Instance()->actions[TAB_PREV]->setShortcut(seq);
    connect(Properties::Instance()->actions[TAB_PREV], SIGNAL(triggered()), consoleTabulator, SLOT(switchToLeft()));
    menu_Actions->addAction(Properties::Instance()->actions[TAB_PREV]);

    Properties::Instance()->actions[MOVE_LEFT] = new QAction(tr("Move Tab Left"), this);
    seq = QKeySequence::fromString( settings.value(MOVE_LEFT, MOVE_LEFT_SHORTCUT).toString() );
    Properties::Instance()->actions[MOVE_LEFT]->setShortcut(seq);
    connect(Properties::Instance()->actions[MOVE_LEFT], SIGNAL(triggered()), consoleTabulator, SLOT(moveLeft()));
    menu_Actions->addAction(Properties::Instance()->actions[MOVE_LEFT]);

    Properties::Instance()->actions[MOVE_RIGHT] = new QAction(tr("Move Tab Right"), this);
    seq = QKeySequence::fromString( settings.value(MOVE_RIGHT, MOVE_RIGHT_SHORTCUT).toString() );
    Properties::Instance()->actions[MOVE_RIGHT]->setShortcut(seq);
    connect(Properties::Instance()->actions[MOVE_RIGHT], SIGNAL(triggered()), consoleTabulator, SLOT(moveRight()));
    menu_Actions->addAction(Properties::Instance()->actions[MOVE_RIGHT]);

    menu_Actions->addSeparator();


    Properties::Instance()->actions[SPLIT_HORIZONTAL] = new QAction(tr("Split Terminal Horizontally"), this);
    seq = QKeySequence::fromString( settings.value(SPLIT_HORIZONTAL).toString() );
    Properties::Instance()->actions[SPLIT_HORIZONTAL]->setShortcut(seq);
    connect(Properties::Instance()->actions[SPLIT_HORIZONTAL], SIGNAL(triggered()), consoleTabulator, SLOT(splitHorizontally()));
    menu_Actions->addAction(Properties::Instance()->actions[SPLIT_HORIZONTAL]);

    Properties::Instance()->actions[SPLIT_VERTICAL] = new QAction(tr("Split Terminal Vertically"), this);
    seq = QKeySequence::fromString( settings.value(SPLIT_VERTICAL).toString() );
    Properties::Instance()->actions[SPLIT_VERTICAL]->setShortcut(seq);
    connect(Properties::Instance()->actions[SPLIT_VERTICAL], SIGNAL(triggered()), consoleTabulator, SLOT(splitVertically()));
    menu_Actions->addAction(Properties::Instance()->actions[SPLIT_VERTICAL]);

    Properties::Instance()->actions[SUB_COLLAPSE] = new QAction(tr("Collapse Subterminal"), this);
    seq = QKeySequence::fromString( settings.value(SUB_COLLAPSE).toString() );
    Properties::Instance()->actions[SUB_COLLAPSE]->setShortcut(seq);
    connect(Properties::Instance()->actions[SUB_COLLAPSE], SIGNAL(triggered()), consoleTabulator, SLOT(splitCollapse()));
    menu_Actions->addAction(Properties::Instance()->actions[SUB_COLLAPSE]);

    Properties::Instance()->actions[SUB_NEXT] = new QAction(tr("Next Subterminal"), this);
    seq = QKeySequence::fromString( settings.value(SUB_NEXT, SUB_NEXT_SHORTCUT).toString() );
    Properties::Instance()->actions[SUB_NEXT]->setShortcut(seq);
    connect(Properties::Instance()->actions[SUB_NEXT], SIGNAL(triggered()), consoleTabulator, SLOT(switchNextSubterminal()));
    menu_Actions->addAction(Properties::Instance()->actions[SUB_NEXT]);

    Properties::Instance()->actions[SUB_PREV] = new QAction(tr("Previous Subterminal"), this);
    seq = QKeySequence::fromString( settings.value(SUB_PREV, SUB_PREV_SHORTCUT).toString() );
    Properties::Instance()->actions[SUB_PREV]->setShortcut(seq);
    connect(Properties::Instance()->actions[SUB_PREV], SIGNAL(triggered()), consoleTabulator, SLOT(switchPrevSubterminal()));
    menu_Actions->addAction(Properties::Instance()->actions[SUB_PREV]);

    menu_Actions->addSeparator();

    // Copy and Paste are only added to the table for the sake of bindings at the moment; there is no Edit menu, only a context menu.
    Properties::Instance()->actions[COPY_SELECTION] = new QAction(tr("Copy Selection"), this);
    seq = QKeySequence::fromString( settings.value(COPY_SELECTION, COPY_SELECTION_SHORTCUT).toString() );
    Properties::Instance()->actions[COPY_SELECTION]->setShortcut(seq);

    Properties::Instance()->actions[PASTE_CLIPBOARD] = new QAction(tr("Paste Clipboard"), this);
    seq = QKeySequence::fromString( settings.value(PASTE_CLIPBOARD, PASTE_CLIPBOARD_SHORTCUT).toString() );
    Properties::Instance()->actions[PASTE_CLIPBOARD]->setShortcut(seq);

    Properties::Instance()->actions[PASTE_SELECTION] = new QAction(tr("Paste Selection"), this);
    seq = QKeySequence::fromString( settings.value(PASTE_SELECTION, PASTE_SELECTION_SHORTCUT).toString() );
    Properties::Instance()->actions[PASTE_SELECTION]->setShortcut(seq);

    Properties::Instance()->actions[ZOOM_IN] = new QAction(tr("Zoom in"), this);
    seq = QKeySequence::fromString( settings.value(ZOOM_IN, ZOOM_IN_SHORTCUT).toString() );
    Properties::Instance()->actions[ZOOM_IN]->setShortcut(seq);

    Properties::Instance()->actions[ZOOM_OUT] = new QAction(tr("Zoom out"), this);
    seq = QKeySequence::fromString( settings.value(ZOOM_OUT, ZOOM_OUT_SHORTCUT).toString() );
    Properties::Instance()->actions[ZOOM_OUT]->setShortcut(seq);
    
    Properties::Instance()->actions[ZOOM_RESET] = new QAction(tr("Zoom reset"), this);
    seq = QKeySequence::fromString( settings.value(ZOOM_RESET, ZOOM_RESET_SHORTCUT).toString() );
    Properties::Instance()->actions[ZOOM_RESET]->setShortcut(seq);

    menu_Actions->addSeparator();

    Properties::Instance()->actions[FIND] = new QAction(tr("Find..."), this);
    seq = QKeySequence::fromString( settings.value(FIND, FIND_SHORTCUT).toString() );
    Properties::Instance()->actions[FIND]->setShortcut(seq);
    connect(Properties::Instance()->actions[FIND], SIGNAL(triggered()), this, SLOT(find()));
    menu_Actions->addAction(Properties::Instance()->actions[FIND]);

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

    Properties::Instance()->actions[TOGGLE_MENU] = new QAction(tr("Toggle Menu"), this);
    seq = QKeySequence::fromString( settings.value(TOGGLE_MENU, TOGGLE_MENU_SHORTCUT).toString() );
    Properties::Instance()->actions[TOGGLE_MENU]->setShortcut(seq);
    connect(Properties::Instance()->actions[TOGGLE_MENU], SIGNAL(triggered()), this, SLOT(toggleMenu()));
    // tis is correct - add action to main window - not to menu to keep toggle working
    addAction(Properties::Instance()->actions[TOGGLE_MENU]);

    settings.endGroup();

    // apply props
    propertiesChanged();
}
void MainWindow::setup_FileMenu_Actions()
{
    QSettings settings;
    settings.beginGroup("Shortcuts");

    QKeySequence seq;

    Properties::Instance()->actions[ADD_TAB] = new QAction(QIcon(":/icons/list-add.png"), tr("New Tab"), this);
    seq = QKeySequence::fromString( settings.value(ADD_TAB, ADD_TAB_SHORTCUT).toString() );
    Properties::Instance()->actions[ADD_TAB]->setShortcut(seq);
    connect(Properties::Instance()->actions[ADD_TAB], SIGNAL(triggered()), consoleTabulator, SLOT(addNewTab()));
    menu_File->addAction(Properties::Instance()->actions[ADD_TAB]);

    Properties::Instance()->actions[CLOSE_TAB] = new QAction(QIcon(":/icons/list-remove.png"), tr("Close Tab"), this);
    seq = QKeySequence::fromString( settings.value(CLOSE_TAB, CLOSE_TAB_SHORTCUT).toString() );
    Properties::Instance()->actions[CLOSE_TAB]->setShortcut(seq);
    connect(Properties::Instance()->actions[CLOSE_TAB], SIGNAL(triggered()), consoleTabulator, SLOT(removeCurrentTab()));
    menu_File->addAction(Properties::Instance()->actions[CLOSE_TAB]);

    Properties::Instance()->actions[NEW_WINDOW] = new QAction(tr("New Window"), this);
    seq = QKeySequence::fromString( settings.value(NEW_WINDOW, NEW_WINDOW_SHORTCUT).toString() );
    Properties::Instance()->actions[NEW_WINDOW]->setShortcut(seq);
    connect(Properties::Instance()->actions[NEW_WINDOW], SIGNAL(triggered()), this, SLOT(newTerminalWindow()));
    menu_File->addAction(Properties::Instance()->actions[NEW_WINDOW]);
    
    menu_File->addSeparator();
 
    Properties::Instance()->actions[PREFERENCES] = actProperties;
    connect(actProperties, SIGNAL(triggered()), SLOT(actProperties_triggered()));
    menu_File->addAction(Properties::Instance()->actions[PREFERENCES]);

    menu_File->addSeparator();

    Properties::Instance()->actions[QUIT] = actQuit;
    connect(actQuit, SIGNAL(triggered()), SLOT(close()));
    menu_File->addAction(Properties::Instance()->actions[QUIT]);

    settings.endGroup();
} 

void MainWindow::setup_ViewMenu_Actions()
{
    toggleBorder = new QAction(tr("Hide Window Borders"), this);
    //toggleBorder->setObjectName("toggle_Borderless");
    toggleBorder->setCheckable(true);
// TODO/FIXME: it's broken somehow. When I call toggleBorderless() here the non-responsive window appear
//    toggleBorder->setChecked(Properties::Instance()->borderless);
//    if (Properties::Instance()->borderless)
//        toggleBorderless();
    connect(toggleBorder, SIGNAL(triggered()), this, SLOT(toggleBorderless()));
    menu_Window->addAction(toggleBorder);
    toggleBorder->setVisible(!m_dropMode);

    toggleTabbar = new QAction(tr("Show Tab Bar"), this);
    //toggleTabbar->setObjectName("toggle_TabBar");
    toggleTabbar->setCheckable(true);
    toggleTabbar->setChecked(!Properties::Instance()->tabBarless);
    toggleTabBar();
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

    if( tabPosition->actions().count() > Properties::Instance()->tabsPos )
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

    if( Properties::Instance()->scrollBarPos < scrollBarPosition->actions().size() )
        scrollBarPosition->actions().at(Properties::Instance()->scrollBarPos)->setChecked(true);

    connect(scrollBarPosition, SIGNAL(triggered(QAction *)),
             consoleTabulator, SLOT(changeScrollPosition(QAction *)) );

    scrollPosMenu = new QMenu(tr("Scrollbar Layout"), menu_Window);
    scrollPosMenu->setObjectName("scrollPosMenu");

    for(int i=0; i < scrollBarPosition->actions().size(); ++i) {
        scrollPosMenu->addAction(scrollBarPosition->actions().at(i));
    }

    menu_Window->addMenu(scrollPosMenu);
}

void MainWindow::on_consoleTabulator_currentChanged(int)
{
}

void MainWindow::toggleTabBar()
{
    consoleTabulator->tabBar()->setVisible(toggleTabbar->isChecked());
    Properties::Instance()->tabBarless = !toggleTabbar->isChecked();
}

void MainWindow::toggleBorderless()
{
    setWindowFlags(windowFlags() ^ Qt::FramelessWindowHint);
    show();
    setWindowState(Qt::WindowActive); /* don't loose focus on the window */
    Properties::Instance()->borderless = toggleBorder->isChecked();
    realign();
}

void MainWindow::toggleMenu()
{
    m_menuBar->setVisible(!m_menuBar->isVisible());
    Properties::Instance()->menuVisible = m_menuBar->isVisible();
}

void MainWindow::closeEvent(QCloseEvent *ev)
{
    if (!Properties::Instance()->askOnExit
            || !consoleTabulator->count())
    {
        Properties::Instance()->mainWindowGeometry = saveGeometry();
        Properties::Instance()->mainWindowState = saveState();
        Properties::Instance()->saveSettings();
        ev->accept();
        return;
    }

    // ask user for cancel only when there is at least one terminal active in this window
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

void MainWindow::actAbout_triggered()
{
    QMessageBox::about(this, QString("QTerminal ") + STR_VERSION, tr("A lightweight multiplatform terminal emulator"));
}

void MainWindow::actProperties_triggered()
{
    PropertiesDialog *p = new PropertiesDialog(this);
    connect(p, SIGNAL(propertiesChanged()), this, SLOT(propertiesChanged()));
    p->exec();
}

void MainWindow::propertiesChanged()
{
    QApplication::setStyle(Properties::Instance()->guiStyle);
    setWindowOpacity(Properties::Instance()->appOpacity/100.0);
    consoleTabulator->setTabPosition((QTabWidget::TabPosition)Properties::Instance()->tabsPos);
    consoleTabulator->propertiesChanged();
    setDropShortcut(Properties::Instance()->dropShortCut);

    m_menuBar->setVisible(Properties::Instance()->menuVisible);

    Properties::Instance()->saveSettings();
    realign();
}

void MainWindow::realign()
{
    if (m_dropMode)
    {
        QRect desktop = QApplication::desktop()->availableGeometry(this);
        QRect geometry = QRect(0, 0,
                               desktop.width()  * Properties::Instance()->dropWidht  / 100,
                               desktop.height() * Properties::Instance()->dropHeight / 100
                              );
        geometry.moveCenter(desktop.center());
        // do not use 0 here - we need to calculate with potential panel on top
        geometry.setTop(desktop.top());

        setGeometry(geometry);
    }
}

void MainWindow::updateActionGroup(QAction *a)
{
    if (a->parent()->objectName() == tabPosMenu->objectName()) {
        tabPosition->actions().at(Properties::Instance()->tabsPos)->setChecked(true);
    }
}

void MainWindow::showHide()
{
    if (isVisible())
        hide();
    else
    {
       realign();
       show();
       activateWindow();
    }
}

void MainWindow::setKeepOpen(bool value)
{
    Properties::Instance()->dropKeepOpen = value;
    if (!m_dropLockButton)
        return;

    if (value)
        m_dropLockButton->setIcon(QIcon(":/icons/locked.png"));
    else
        m_dropLockButton->setIcon(QIcon(":/icons/notlocked.png"));

    m_dropLockButton->setChecked(value);
}

void MainWindow::find()
{
    // A bit ugly perhaps with 4 levels of indirection... 
    consoleTabulator->terminalHolder()->currentTerminal()->impl()->toggleShowSearchBar();
}


bool MainWindow::event(QEvent *event)
{
    if (event->type() == QEvent::WindowDeactivate)
    {
        if (m_dropMode &&
            !Properties::Instance()->dropKeepOpen &&
            qApp->activeWindow() == 0
           )
           hide();
    }
    return QMainWindow::event(event);
}

void MainWindow::newTerminalWindow()
{
    MainWindow *w = new MainWindow(m_initWorkDir, m_initShell, false);
    w->show();
}

