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

#include <QDockWidget>
#include <QDesktopWidget>
#include <QToolButton>
#include <QMessageBox>
#include <functional>

#ifdef HAVE_QDBUS
    #include <QtDBus/QtDBus>
    #include "windowadaptor.h"
#endif

#include "terminalconfig.h"
#include "mainwindow.h"
#include "tabwidget.h"
#include "termwidgetholder.h"
#include "config.h"
#include "properties.h"
#include "propertiesdialog.h"
#include "bookmarkswidget.h"
#include "qterminalapp.h"
#include "dbusaddressable.h"

typedef std::function<bool(MainWindow&)> checkfn;
Q_DECLARE_METATYPE(checkfn)

// TODO/FXIME: probably remove. QSS makes it unusable on mac...
#define QSS_DROP    "MainWindow {border: 1px solid rgba(0, 0, 0, 50%);}\n"

MainWindow::MainWindow(TerminalConfig &cfg,
                       bool dropMode,
                       QWidget * parent,
                       Qt::WindowFlags f)
    : QMainWindow(parent,f),
      DBusAddressable("/windows"),
      tabPosition(NULL),
      scrollBarPosition(NULL),
      keyboardCursorShape(NULL),
      tabPosMenu(NULL),
      scrollPosMenu(NULL),
      keyboardCursorShapeMenu(NULL),
      settingOwner(NULL),
      presetsMenu(NULL),
      m_config(cfg),
      m_dropLockButton(0),
      m_dropMode(dropMode)
{
#ifdef HAVE_QDBUS
    registerAdapter<WindowAdaptor, MainWindow>(this);
#endif
    QTerminalApp::Instance()->addWindow(this);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);

    setupUi(this);
    Properties::Instance()->migrate_settings();
    Properties::Instance()->loadSettings();

    m_bookmarksDock = new QDockWidget(tr("Bookmarks"), this);
    m_bookmarksDock->setObjectName("BookmarksDockWidget");
    m_bookmarksDock->setAutoFillBackground(true);
    BookmarksWidget *bookmarksWidget = new BookmarksWidget(m_bookmarksDock);
    bookmarksWidget->setAutoFillBackground(true);
    m_bookmarksDock->setWidget(bookmarksWidget);
    addDockWidget(Qt::LeftDockWidgetArea, m_bookmarksDock);
    connect(bookmarksWidget, SIGNAL(callCommand(QString)),
            this, SLOT(bookmarksWidget_callCommand(QString)));

    connect(m_bookmarksDock, SIGNAL(visibilityChanged(bool)),
            this, SLOT(bookmarksDock_visibilityChanged(bool)));

    connect(actAbout, SIGNAL(triggered()), SLOT(actAbout_triggered()));
    connect(actAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(&m_dropShortcut, SIGNAL(activated()), SLOT(showHide()));

    setContentsMargins(0, 0, 0, 0);
    if (m_dropMode) {
        this->enableDropMode();
        setStyleSheet(QSS_DROP);
    }
    else {
	if (Properties::Instance()->saveSizeOnExit) {
	    resize(Properties::Instance()->mainWindowSize);
	}
	if (Properties::Instance()->savePosOnExit) {
	    move(Properties::Instance()->mainWindowPosition);
	}
        restoreState(Properties::Instance()->mainWindowState);
    }

    consoleTabulator->setAutoFillBackground(true);
    connect(consoleTabulator, SIGNAL(closeTabNotification()), SLOT(close()));
    consoleTabulator->setTabPosition((QTabWidget::TabPosition)Properties::Instance()->tabsPos);
    //consoleTabulator->setShellProgram(command);

    // apply props
    propertiesChanged();

    setupCustomDirs();

    connect(consoleTabulator, &TabWidget::currentTitleChanged, this, &MainWindow::onCurrentTitleChanged);
    connect(menu_Actions, SIGNAL(aboutToShow()), this, SLOT(updateDisabledActions()));

    /* The tab should be added after all changes are made to
       the main window; otherwise, the initial prompt might
       get jumbled because of changes in internal geometry. */
    consoleTabulator->addNewTab(m_config);
}

void MainWindow::rebuildActions()
{
    // Delete all setting-related QObjects
    delete settingOwner;
    settingOwner = new QWidget(this);
    settingOwner->setGeometry(0,0,0,0);

    // Then create them again
    setup_FileMenu_Actions();
    setup_ActionsMenu_Actions();
    setup_ViewMenu_Actions();
}

MainWindow::~MainWindow()
{
    QTerminalApp::Instance()->removeWindow(this);
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

void MainWindow::setup_Action(const char *name, QAction *action, const char *defaultShortcut, const QObject *receiver,
                              const char *slot, QMenu *menu, const QVariant &data)
{
    QSettings settings;
    settings.beginGroup("Shortcuts");

    QList<QKeySequence> shortcuts;

    actions[name] = action;
    foreach (const QString &sequenceString, settings.value(name, defaultShortcut).toString().split('|'))
        shortcuts.append(QKeySequence::fromString(sequenceString));
    actions[name]->setShortcuts(shortcuts);

    if (receiver)
    {
        connect(actions[name], SIGNAL(triggered(bool)), receiver, slot);
        addAction(actions[name]);
    }

    if (menu)
        menu->addAction(actions[name]);

    if (!data.isNull())
        actions[name]->setData(data);
}

void MainWindow::setup_ActionsMenu_Actions()
{
    QVariant data;

    const checkfn checkTabs = &MainWindow::hasMultipleTabs;
    const checkfn checkSubterminals = &MainWindow::hasMultipleSubterminals;

    menu_Actions->clear();

    setup_Action(CLEAR_TERMINAL, new QAction(QIcon::fromTheme("edit-clear"), tr("&Clear Active Terminal"), settingOwner),
                 CLEAR_TERMINAL_SHORTCUT, consoleTabulator, SLOT(clearActiveTerminal()), menu_Actions);

    menu_Actions->addSeparator();

    data.setValue(checkTabs);

    setup_Action(TAB_NEXT, new QAction(QIcon::fromTheme("go-next"), tr("&Next Tab"), settingOwner),
                 TAB_NEXT_SHORTCUT, consoleTabulator, SLOT(switchToRight()), menu_Actions, data);

    setup_Action(TAB_PREV, new QAction(QIcon::fromTheme("go-previous"), tr("&Previous Tab"), settingOwner),
                 TAB_PREV_SHORTCUT, consoleTabulator, SLOT(switchToLeft()), menu_Actions, data);

    setup_Action(MOVE_LEFT, new QAction(tr("Move Tab &Left"), settingOwner),
                 MOVE_LEFT_SHORTCUT, consoleTabulator, SLOT(moveLeft()), menu_Actions, data);

    setup_Action(MOVE_RIGHT, new QAction(tr("Move Tab &Right"), settingOwner),
                 MOVE_RIGHT_SHORTCUT, consoleTabulator, SLOT(moveRight()), menu_Actions, data);

    menu_Actions->addSeparator();

    setup_Action(SPLIT_HORIZONTAL, new QAction(tr("Split Terminal &Horizontally"), settingOwner),
                 NULL, consoleTabulator, SLOT(splitHorizontally()), menu_Actions);

    setup_Action(SPLIT_VERTICAL, new QAction(tr("Split Terminal &Vertically"), settingOwner),
                 NULL, consoleTabulator, SLOT(splitVertically()), menu_Actions);

    data.setValue(checkSubterminals);

    setup_Action(SUB_COLLAPSE, new QAction(tr("&Collapse Subterminal"), settingOwner),
                 NULL, consoleTabulator, SLOT(splitCollapse()), menu_Actions, data);

    setup_Action(SUB_TOP, new QAction(QIcon::fromTheme("go-up"), tr("&Top Subterminal"), settingOwner),
                 SUB_TOP_SHORTCUT, consoleTabulator, SLOT(switchTopSubterminal()), menu_Actions, data);

    setup_Action(SUB_BOTTOM, new QAction(QIcon::fromTheme("go-down"), tr("&Bottom Subterminal"), settingOwner),
                 SUB_BOTTOM_SHORTCUT, consoleTabulator, SLOT(switchBottomSubterminal()), menu_Actions, data);

    setup_Action(SUB_LEFT, new QAction(QIcon::fromTheme("go-previous"), tr("L&eft Subterminal"), settingOwner),
                 SUB_LEFT_SHORTCUT, consoleTabulator, SLOT(switchLeftSubterminal()), menu_Actions, data);

    setup_Action(SUB_RIGHT, new QAction(QIcon::fromTheme("go-next"), tr("R&ight Subterminal"), settingOwner),
                 SUB_RIGHT_SHORTCUT, consoleTabulator, SLOT(switchRightSubterminal()), menu_Actions, data);


    menu_Actions->addSeparator();

    // Copy and Paste are only added to the table for the sake of bindings at the moment; there is no Edit menu, only a context menu.
    setup_Action(COPY_SELECTION, new QAction(QIcon::fromTheme("edit-copy"), tr("Copy &Selection"), settingOwner),
                 COPY_SELECTION_SHORTCUT, consoleTabulator, SLOT(copySelection()), menu_Edit);

    setup_Action(PASTE_CLIPBOARD, new QAction(QIcon::fromTheme("edit-paste"), tr("Paste Clip&board"), settingOwner),
                 PASTE_CLIPBOARD_SHORTCUT, consoleTabulator, SLOT(pasteClipboard()), menu_Edit);

    setup_Action(PASTE_SELECTION, new QAction(QIcon::fromTheme("edit-paste"), tr("Paste S&election"), settingOwner),
                 PASTE_SELECTION_SHORTCUT, consoleTabulator, SLOT(pasteSelection()), menu_Edit);

    setup_Action(ZOOM_IN, new QAction(QIcon::fromTheme("zoom-in"), tr("Zoom &in"), settingOwner),
                 ZOOM_IN_SHORTCUT, consoleTabulator, SLOT(zoomIn()), menu_Edit);

    setup_Action(ZOOM_OUT, new QAction(QIcon::fromTheme("zoom-out"), tr("Zoom &out"), settingOwner),
                 ZOOM_OUT_SHORTCUT, consoleTabulator, SLOT(zoomOut()), menu_Edit);

    setup_Action(ZOOM_RESET, new QAction(QIcon::fromTheme("zoom-original"), tr("Zoom rese&t"), settingOwner),
                 ZOOM_RESET_SHORTCUT, consoleTabulator, SLOT(zoomReset()), menu_Edit);

    menu_Actions->addSeparator();

    setup_Action(FIND, new QAction(QIcon::fromTheme("edit-find"), tr("&Find..."), settingOwner),
                 FIND_SHORTCUT, this, SLOT(find()), menu_Actions);

#if 0
    act = new QAction(this);
    act->setSeparator(true);

    // TODO/FIXME: unimplemented for now
    act = new QAction(tr("&Save Session"), this);
    // do not use sequences for this task - it collides with eg. mc shorcuts
    // and mainly - it's not used too often
    //act->setShortcut(QKeySequence::Save);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(saveSession()));

    act = new QAction(tr("&Load Session"), this);
    // do not use sequences for this task - it collides with eg. mc shorcuts
    // and mainly - it's not used too often
    //act->setShortcut(QKeySequence::Open);
    connect(act, SIGNAL(triggered()), consoleTabulator, SLOT(loadSession()));
#endif

    setup_Action(TOGGLE_MENU, new QAction(tr("&Toggle Menu"), settingOwner),
                 TOGGLE_MENU_SHORTCUT, this, SLOT(toggleMenu()));
    // this is correct - add action to main window - not to menu to keep toggle working

    // Add global rename current session shortcut
    setup_Action(RENAME_SESSION, new QAction(tr("Rename session"), settingOwner),
                 RENAME_SESSION_SHORTCUT, consoleTabulator, SLOT(renameCurrentSession()));
    // this is correct - add action to main window - not to menu

}
void MainWindow::setup_FileMenu_Actions()
{
    menu_File->clear();
    setup_Action(ADD_TAB, new QAction(QIcon::fromTheme("list-add"), tr("&New Tab"), settingOwner),
                 ADD_TAB_SHORTCUT, this, SLOT(addNewTab()), menu_File);

    if (presetsMenu == NULL) {
        presetsMenu = new QMenu(tr("New Tab From &Preset"), this);
        presetsMenu->addAction(QIcon(), tr("1 &Terminal"),
                               this, SLOT(addNewTab()));
        presetsMenu->addAction(QIcon(), tr("2 &Horizontal Terminals"),
                               consoleTabulator, SLOT(preset2Horizontal()));
        presetsMenu->addAction(QIcon(), tr("2 &Vertical Terminals"),
                               consoleTabulator, SLOT(preset2Vertical()));
        presetsMenu->addAction(QIcon(), tr("4 Terminal&s"),
                               consoleTabulator, SLOT(preset4Terminals()));
    }

    menu_File->addMenu(presetsMenu);

    setup_Action(CLOSE_TAB, new QAction(QIcon::fromTheme("list-remove"), tr("&Close Tab"), settingOwner),
                 CLOSE_TAB_SHORTCUT, consoleTabulator, SLOT(removeCurrentTab()), menu_File);

    setup_Action(NEW_WINDOW, new QAction(QIcon::fromTheme("window-new"), tr("&New Window"), settingOwner),
                 NEW_WINDOW_SHORTCUT, this, SLOT(newTerminalWindow()), menu_File);

    menu_File->addSeparator();

    setup_Action(PREFERENCES, new QAction(tr("&Preferences..."), settingOwner), "", this, SLOT(actProperties_triggered()), menu_File);

    menu_File->addSeparator();

    setup_Action(QUIT, new QAction(QIcon::fromTheme("application-exit"), tr("&Quit"), settingOwner), "", this, SLOT(close()), menu_File);
}

void MainWindow::setup_ViewMenu_Actions()
{
    menu_Window->clear();
    QAction *hideBordersAction = new QAction(tr("&Hide Window Borders"), settingOwner);
    hideBordersAction->setCheckable(true);
    hideBordersAction->setVisible(!m_dropMode);
    setup_Action(HIDE_WINDOW_BORDERS, hideBordersAction,
                 NULL, this, SLOT(toggleBorderless()), menu_Window);
    //Properties::Instance()->actions[HIDE_WINDOW_BORDERS]->setObjectName("toggle_Borderless");
// TODO/FIXME: it's broken somehow. When I call toggleBorderless() here the non-responsive window appear
//    actions[HIDE_WINDOW_BORDERS]->setChecked(Properties::Instance()->borderless);
//    if (Properties::Instance()->borderless)
//        toggleBorderless();

    QAction *showTabBarAction = new QAction(tr("&Show Tab Bar"), settingOwner);
    //toggleTabbar->setObjectName("toggle_TabBar");
    showTabBarAction->setCheckable(true);
    showTabBarAction->setChecked(!Properties::Instance()->tabBarless);
    setup_Action(SHOW_TAB_BAR, showTabBarAction,
                 NULL, this, SLOT(toggleTabBar()), menu_Window);
    toggleTabBar();

    QAction *toggleFullscreen = new QAction(tr("Fullscreen"), settingOwner);
    toggleFullscreen->setCheckable(true);
    toggleFullscreen->setChecked(false);
    setup_Action(FULLSCREEN, toggleFullscreen,
                 FULLSCREEN_SHORTCUT, this, SLOT(showFullscreen(bool)), menu_Window);

    setup_Action(TOGGLE_BOOKMARKS, new QAction(tr("Toggle Bookmarks"), settingOwner),
                 TOGGLE_BOOKMARKS_SHORTCUT, NULL, NULL, menu_Window);

    menu_Window->addSeparator();

    /* tabs position */
    if (tabPosition == NULL) {
        tabPosition = new QActionGroup(this);
        QAction *tabBottom = new QAction(tr("&Bottom"), this);
        QAction *tabTop = new QAction(tr("&Top"), this);
        QAction *tabRight = new QAction(tr("&Right"), this);
        QAction *tabLeft = new QAction(tr("&Left"), this);
        tabPosition->addAction(tabTop);
        tabPosition->addAction(tabBottom);
        tabPosition->addAction(tabLeft);
        tabPosition->addAction(tabRight);

        for(int i = 0; i < tabPosition->actions().size(); ++i)
            tabPosition->actions().at(i)->setCheckable(true);
    }


    if( tabPosition->actions().count() > Properties::Instance()->tabsPos )
        tabPosition->actions().at(Properties::Instance()->tabsPos)->setChecked(true);

    connect(tabPosition, SIGNAL(triggered(QAction *)),
             consoleTabulator, SLOT(changeTabPosition(QAction *)) );

    if (tabPosMenu == NULL) {
        tabPosMenu = new QMenu(tr("&Tabs Layout"), menu_Window);
        tabPosMenu->setObjectName("tabPosMenu");

        for(int i=0; i < tabPosition->actions().size(); ++i) {
            tabPosMenu->addAction(tabPosition->actions().at(i));
        }

        connect(menu_Window, SIGNAL(hovered(QAction *)),
                this, SLOT(updateActionGroup(QAction *)));
    }
    menu_Window->addMenu(tabPosMenu);
    /* */

    /* Scrollbar */
    if (scrollBarPosition == NULL) {
        scrollBarPosition = new QActionGroup(this);
        QAction *scrollNone = new QAction(tr("&None"), this);
        QAction *scrollRight = new QAction(tr("&Right"), this);
        QAction *scrollLeft = new QAction(tr("&Left"), this);
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

    }
    if (scrollPosMenu == NULL) {
        scrollPosMenu = new QMenu(tr("S&crollbar Layout"), menu_Window);
        scrollPosMenu->setObjectName("scrollPosMenu");

        for(int i=0; i < scrollBarPosition->actions().size(); ++i) {
            scrollPosMenu->addAction(scrollBarPosition->actions().at(i));
        }
    }

    menu_Window->addMenu(scrollPosMenu);

    /* Keyboard cursor shape */
    if (keyboardCursorShape == NULL) {
        keyboardCursorShape = new QActionGroup(this);
        QAction *block = new QAction(tr("&BlockCursor"), this);
        QAction *underline = new QAction(tr("&UnderlineCursor"), this);
        QAction *ibeam = new QAction(tr("&IBeamCursor"), this);

        /* order of insertion is dep. on QTermWidget::KeyboardCursorShape enum */
        keyboardCursorShape->addAction(block);
        keyboardCursorShape->addAction(underline);
        keyboardCursorShape->addAction(ibeam);
        for(int i = 0; i < keyboardCursorShape->actions().size(); ++i)
            keyboardCursorShape->actions().at(i)->setCheckable(true);

        if( Properties::Instance()->keyboardCursorShape < keyboardCursorShape->actions().size() )
            keyboardCursorShape->actions().at(Properties::Instance()->keyboardCursorShape)->setChecked(true);

        connect(keyboardCursorShape, SIGNAL(triggered(QAction *)),
                 consoleTabulator, SLOT(changeKeyboardCursorShape(QAction *)) );
    }

    if (keyboardCursorShapeMenu == NULL) {
        keyboardCursorShapeMenu = new QMenu(tr("&Keyboard Cursor Shape"), menu_Window);
        keyboardCursorShapeMenu->setObjectName("keyboardCursorShapeMenu");

        for(int i=0; i < keyboardCursorShape->actions().size(); ++i) {
            keyboardCursorShapeMenu->addAction(keyboardCursorShape->actions().at(i));
        }
    }

    menu_Window->addMenu(keyboardCursorShapeMenu);
}

void MainWindow::setupCustomDirs()
{
    const QSettings settings;
    const QString dir = QFileInfo(settings.fileName()).canonicalPath() + "/color-schemes/";
    TermWidgetImpl::addCustomColorSchemeDir(dir);
}

void MainWindow::on_consoleTabulator_currentChanged(int)
{
}

void MainWindow::toggleTabBar()
{
    Properties::Instance()->tabBarless
            = !actions[SHOW_TAB_BAR]->isChecked();
    consoleTabulator->showHideTabBar();
}

void MainWindow::toggleBorderless()
{
    setWindowFlags(windowFlags() ^ Qt::FramelessWindowHint);
    show();
    setWindowState(Qt::WindowActive); /* don't loose focus on the window */
    Properties::Instance()->borderless
            = actions[HIDE_WINDOW_BORDERS]->isChecked(); realign();
}

void MainWindow::toggleMenu()
{
    m_menuBar->setVisible(!m_menuBar->isVisible());
    Properties::Instance()->menuVisible = m_menuBar->isVisible();
}

void MainWindow::showFullscreen(bool fullscreen)
{
    if(fullscreen)
        setWindowState(windowState() | Qt::WindowFullScreen);
    else
        setWindowState(windowState() & ~Qt::WindowFullScreen);
}

void MainWindow::toggleBookmarks()
{
    m_bookmarksDock->toggleViewAction()->trigger();
}


void MainWindow::closeEvent(QCloseEvent *ev)
{
    if (!Properties::Instance()->askOnExit
            || !consoleTabulator->count())
    {
        // #80 - do not save state and geometry in drop mode
        if (!m_dropMode) {
            if (Properties::Instance()->savePosOnExit) {
            	Properties::Instance()->mainWindowPosition = pos();
            }
            if (Properties::Instance()->saveSizeOnExit) {
            	Properties::Instance()->mainWindowSize = size();
            }
            Properties::Instance()->mainWindowState = saveState();
        }
        Properties::Instance()->saveSettings();
        for (int i = consoleTabulator->count(); i > 0; --i) {
            consoleTabulator->removeTab(i - 1);
        }
        ev->accept();
        return;
    }

    // ask user for cancel only when there is at least one terminal active in this window
    QDialog * dia = new QDialog(this);
    dia->setObjectName("exitDialog");
    dia->setWindowTitle(tr("Exit QTerminal"));

    QCheckBox * dontAskCheck = new QCheckBox(tr("Do not ask again"), dia);
    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::No, Qt::Horizontal, dia);
    buttonBox->button(QDialogButtonBox::Yes)->setDefault(true);

    connect(buttonBox, SIGNAL(accepted()), dia, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), dia, SLOT(reject()));

    QVBoxLayout * lay = new QVBoxLayout();
    lay->addWidget(new QLabel(tr("Are you sure you want to exit?")));
    lay->addWidget(dontAskCheck);
    lay->addWidget(buttonBox);
    dia->setLayout(lay);

    if (dia->exec() == QDialog::Accepted) {
        Properties::Instance()->mainWindowPosition = pos();
        Properties::Instance()->mainWindowSize = size();
        Properties::Instance()->mainWindowState = saveState();
        Properties::Instance()->askOnExit = !dontAskCheck->isChecked();
        Properties::Instance()->saveSettings();
        for (int i = consoleTabulator->count(); i > 0; --i) {
            consoleTabulator->removeTab(i - 1);
        }
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
    rebuildActions();

    QApplication::setStyle(Properties::Instance()->guiStyle);
    setWindowOpacity(1.0 - Properties::Instance()->appTransparency/100.0);
    consoleTabulator->setTabPosition((QTabWidget::TabPosition)Properties::Instance()->tabsPos);
    consoleTabulator->propertiesChanged();
    setDropShortcut(Properties::Instance()->dropShortCut);

    m_menuBar->setVisible(Properties::Instance()->menuVisible);

    m_bookmarksDock->setVisible(Properties::Instance()->useBookmarks
                                && Properties::Instance()->bookmarksVisible);
    actions[TOGGLE_BOOKMARKS]->setVisible(Properties::Instance()->useBookmarks);

    if (Properties::Instance()->useBookmarks)
    {
        qobject_cast<BookmarksWidget*>(m_bookmarksDock->widget())->setup();
    }

    onCurrentTitleChanged(consoleTabulator->currentIndex());

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
        if (geometry != this->geometry()) {
          setGeometry(geometry);
        }
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
        m_dropLockButton->setIcon(QIcon::fromTheme("object-locked"));
    else
        m_dropLockButton->setIcon(QIcon::fromTheme("object-unlocked"));

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
    TerminalConfig cfg;
    TermWidgetHolder *ch = consoleTabulator->terminalHolder();
    if (ch)
        cfg.provideCurrentDirectory(ch->currentTerminal()->impl()->workingDirectory());

    MainWindow *w = new MainWindow(cfg, false);
    w->show();
}

void MainWindow::bookmarksWidget_callCommand(const QString& cmd)
{
    consoleTabulator->terminalHolder()->currentTerminal()->impl()->sendText(cmd);
    consoleTabulator->terminalHolder()->currentTerminal()->setFocus();
}

void MainWindow::bookmarksDock_visibilityChanged(bool visible)
{
    Properties::Instance()->bookmarksVisible = visible;
}

void MainWindow::addNewTab()
{
    TerminalConfig cfg;
    if (Properties::Instance()->terminalsPreset == 3)
        consoleTabulator->preset4Terminals();
    else if (Properties::Instance()->terminalsPreset == 2)
        consoleTabulator->preset2Vertical();
    else if (Properties::Instance()->terminalsPreset == 1)
        consoleTabulator->preset2Horizontal();
    else
        consoleTabulator->addNewTab(cfg);
    updateDisabledActions();
}

void MainWindow::onCurrentTitleChanged(int index)
{
    QString title;
    QIcon icon;
    if (-1 != index)
    {
        title = consoleTabulator->tabText(index);
        icon = consoleTabulator->tabIcon(index);
    }
    setWindowTitle(title.isEmpty() || !Properties::Instance()->changeWindowTitle ? QStringLiteral("QTerminal") : title);
    setWindowIcon(icon.isNull() || !Properties::Instance()->changeWindowIcon ? QIcon::fromTheme("utilities-terminal") : icon);
}

bool MainWindow::hasMultipleTabs()
{
    return consoleTabulator->findChildren<TermWidgetHolder*>().count() > 1;
}

bool MainWindow::hasMultipleSubterminals()
{
    return consoleTabulator->terminalHolder()->findChildren<TermWidget*>().count() > 1;
}

void MainWindow::updateDisabledActions()
{
    const QList<QAction*> actions = menu_Actions->actions();
    for (QAction *action : actions) {
        if (!action->data().isNull()) {
            const checkfn check = action->data().value<checkfn>();
            action->setEnabled(check(*this));
        }
    }
}


QMap< QString, QAction * >& MainWindow::leaseActions() {
        return actions;
}
#ifdef HAVE_QDBUS

QDBusObjectPath MainWindow::getActiveTab()
{
    return qobject_cast<TermWidgetHolder*>(consoleTabulator->currentWidget())->getDbusPath();
}

QList<QDBusObjectPath> MainWindow::getTabs()
{
    QList<QDBusObjectPath> tabs;
    for (int i = 0; i<consoleTabulator->count(); ++i)
    {
        tabs.push_back(qobject_cast<TermWidgetHolder*>(consoleTabulator->widget(i))->getDbusPath());
    }
    return tabs;

}

QDBusObjectPath MainWindow::newTab(const QHash<QString,QVariant> &termArgs)
{
    TerminalConfig cfg = TerminalConfig::fromDbus(termArgs);
    int idx = consoleTabulator->addNewTab(cfg);
    return qobject_cast<TermWidgetHolder*>(consoleTabulator->widget(idx))->getDbusPath();
}

void MainWindow::closeWindow()
{
    close();
}

#endif
