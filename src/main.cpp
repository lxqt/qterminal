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

#include <QApplication>
#include <QtGlobal>

#include <QLockFile>
#include <QLocalServer>
#include <QLocalSocket>

#include <cassert>
#include <cstdio>
#include <getopt.h>
#include <cstdlib>
#include <unistd.h>
#ifdef HAVE_QDBUS
    #include <QtDBus/QtDBus>
    #include "processadaptor.h"
#endif

#include "mainwindow.h"
#include "qterminalapp.h"
#include "qterminalutils.h"
#include "terminalconfig.h"
#include "instance-locker.h"

#define out

const char* const short_options = "vhw:e:dp:";

const struct option long_options[] = {
    {"version", 0, nullptr, 'v'},
    {"help",    0, nullptr, 'h'},
    {"workdir", 1, nullptr, 'w'},
    {"execute", 1, nullptr, 'e'},
    {"drop",    0, nullptr, 'd'},
    {"profile", 1, nullptr, 'p'},
    {nullptr,   0, nullptr,  0}
};

QTerminalApp * QTerminalApp::m_instance = nullptr;

[[ noreturn ]] void print_usage_and_exit(int code)
{
    printf("QTerminal %s\n", QTERMINAL_VERSION);
    puts("Usage: qterminal [OPTION]...\n");
    puts("  -d,  --drop               Start in \"dropdown mode\" (like Yakuake or Tilda)");
    puts("  -e,  --execute <command>  Execute command instead of shell");
    puts("  -h,  --help               Print this help");
    puts("  -p,  --profile <name>     Load profile from ~/.config/<name>.conf");
    puts("  -v,  --version            Prints application version and exits");
    puts("  -w,  --workdir <dir>      Start session with specified work directory");
    puts("\nHomepage: <https://github.com/lxqt/qterminal>");
    puts("Report bugs to <https://github.com/lxqt/qterminal/issues>");
    exit(code);
}

[[ noreturn ]] void print_version_and_exit(int code=0)
{
    printf("%s\n", QTERMINAL_VERSION);
    exit(code);
}

void parse_args(int argc, char* argv[], QString& workdir, QStringList & shell_command, out bool& dropMode)
{
    int next_option = 0;
    dropMode = false;
    do{
        next_option = getopt_long(argc, argv, short_options, long_options, nullptr);
        switch(next_option)
        {
            case 'h':
                print_usage_and_exit(0);
                break;
            case 'w':
                workdir = QString::fromLocal8Bit(optarg);
                break;
            case 'e':
                shell_command << parse_command(QString::fromLocal8Bit(optarg));
                // #15 "Raw" -e params
                // Passing "raw" params (like konsole -e mcedit /tmp/tmp.txt") is more preferable - then I can call QString("qterminal -e ") + cmd_line in other programs
                while (optind < argc)
                {
                    //printf("arg: %d - %s\n", optind, argv[optind]);
                    shell_command << QString::fromLocal8Bit(argv[optind++]);
                }
                break;
            case 'd':
                dropMode = true;
                break;
            case 'p':
                Properties::Instance(QString::fromLocal8Bit(optarg));
                break;
            case '?':
                print_usage_and_exit(1);
                break;
            case 'v':
                print_version_and_exit();
                break;
        }
    }
    while(next_option != -1);
}

int main(int argc, char *argv[])
{
    if (!qEnvironmentVariableIsEmpty("XPC_SERVICE_NAME")) {
        // On macOS, if qterminal.app is spawned by launchd (e.g., from Finder
        // or use `open qterminal.app`, $PWD is set to /. Workaround that by
        // go to $HOME first.
        if (chdir(QDir::homePath().toLatin1().data())) {
            qDebug() << "Failed to chdir to $HOME" << QDir::homePath() << strerror(errno);
        }

        // also initializes $LANG
        QString systemLocaleName(QLocale().name());
        systemLocaleName.append(QLatin1String(".UTF-8"));
        qputenv("LANG", systemLocaleName.toLatin1());
    }

    QApplication::setApplicationName(QStringLiteral("qterminal"));
    QApplication::setApplicationVersion(QStringLiteral(QTERMINAL_VERSION));
    QApplication::setOrganizationDomain(QStringLiteral("qterminal.org"));
    QApplication::setDesktopFileName(QLatin1String("qterminal.desktop"));
    // Warning: do not change settings format. It can screw bookmarks later.
    QSettings::setDefaultFormat(QSettings::IniFormat);

    QTerminalApp *app = QTerminalApp::Instance(argc, argv);
    #ifdef HAVE_QDBUS
        app->registerOnDbus();
    #endif

    QString workdir;
    QStringList shell_command;
    bool dropMode = false;
    parse_args(argc, argv, workdir, shell_command, dropMode);

    Properties::Instance()->migrate_settings();
    Properties::Instance()->loadSettings();

    if (workdir.isEmpty())
        workdir = QDir::currentPath();
    app->setWorkingDirectory(workdir);

    const QSettings settings;
    const QFileInfo customStyle = QFileInfo(
        QFileInfo(settings.fileName()).canonicalPath() +
        QStringLiteral("/style.qss")
    );
    if (customStyle.isFile() && customStyle.isReadable())
    {
        QFile style(customStyle.canonicalFilePath());
        style.open(QFile::ReadOnly);
        QString styleString = QLatin1String(style.readAll());
        app->setStyleSheet(styleString);
    }

    // icons
    /* setup our custom icon theme if there is no system theme (OS X, Windows) */
    if (QIcon::themeName().isEmpty())
        QIcon::setThemeName(QStringLiteral("QTerminal"));

    // translations

    // install the translations built-into Qt itself
    QTranslator qtTranslator;
    qtTranslator.load(QStringLiteral("qt_") + QLocale::system().name(), QLibraryInfo::path(QLibraryInfo::TranslationsPath));
    app->installTranslator(&qtTranslator);

    QTranslator translator;
    QString fname = QString::fromLatin1("qterminal_%1.qm").arg(QLocale::system().name().left(5));
#ifdef TRANSLATIONS_DIR
    //qDebug() << "TRANSLATIONS_DIR: Loading translation file" << fname << "from dir" << TRANSLATIONS_DIR;
    /*qDebug() << "load success:" <<*/ translator.load(fname, QString::fromUtf8(TRANSLATIONS_DIR), QStringLiteral("_"));
#endif
#ifdef APPLE_BUNDLE
    QDir translations_dir = QDir(QApplication::applicationDirPath());
    translations_dir.cdUp();
    if (translations_dir.cd(QStringLiteral("Resources/translations"))) {
        //qDebug() << "APPLE_BUNDLE: Loading translator file" << fname << "from dir" << translations_dir.path();
        /*qDebug() << "load success:" <<*/ translator.load(fname, translations_dir.path(), QStringLiteral("_"));
    } /*else {
        qWarning() << "Unable to find \"Resources/translations\" dir in" << translations_dir.path();
    }*/
#endif
    app->installTranslator(&translator);

    TerminalConfig initConfig = TerminalConfig(workdir, shell_command);
    MainWindow *window = app->newWindow(dropMode, initConfig);

    /** Instance locker */
    if ( dropMode ) {
        InstanceLocker *locker = new InstanceLocker( QString::fromUtf8( "%1-%2" ).arg( qApp->organizationName() ).arg( qApp->applicationName() ), nullptr );

        /** Another isntance is running */
        if ( locker->isRunning() ) {
            /** Ask the other instance to toggle the instance. */
            locker->sendMessage( QString::fromUtf8( "toggle" ) );

            delete locker;

            return 0;
        }

        QObject::connect(
            locker->mServer, &QLocalServer::newConnection, [ = ] () {
                QString msg = locker->handleConnection();
                if (msg == QString::fromUtf8( "toggle" ))
                {
                    if ( window->isVisible() ) {
                        window->hide();
                    }

                    else {
                        window->show();
                    }
                }
            }
        );
    }

    int ret = app->exec();
    delete Properties::Instance();
    app->cleanup();

    return ret;
}

MainWindow *QTerminalApp::newWindow(bool dropMode, TerminalConfig &cfg)
{
    MainWindow *window = nullptr;
    if (dropMode)
    {
        window = new MainWindow(cfg, dropMode);
        if (Properties::Instance()->dropShowOnStart)
            window->show();
    }
    else
    {
        window = new MainWindow(cfg, dropMode);
        if (Properties::Instance()->windowMaximized)
            window->setWindowState(Qt::WindowMaximized);
        window->show();
    }
    return window;
}

QTerminalApp *QTerminalApp::Instance()
{
    assert(m_instance != nullptr);
    return m_instance;
}

QTerminalApp *QTerminalApp::Instance(int &argc, char **argv)
{
    assert(m_instance == nullptr);
    m_instance = new QTerminalApp(argc, argv);
    return m_instance;
}

QTerminalApp::QTerminalApp(int &argc, char **argv)
    :QApplication(argc, argv)
{
}

QString &QTerminalApp::getWorkingDirectory()
{
    return m_workDir;
}

void QTerminalApp::setWorkingDirectory(const QString &wd)
{
    m_workDir = wd;
}

void QTerminalApp::cleanup() {
    delete m_instance;
    m_instance = nullptr;
}


void QTerminalApp::addWindow(MainWindow *window)
{
    m_windowList.append(window);
}

void QTerminalApp::removeWindow(MainWindow *window)
{
    m_windowList.removeOne(window);
}

QList<MainWindow *> QTerminalApp::getWindowList()
{
    return m_windowList;
}

#ifdef HAVE_QDBUS
void QTerminalApp::registerOnDbus()
{
    if (!QDBusConnection::sessionBus().isConnected())
    {
        fprintf(stderr, "Cannot connect to the D-Bus session bus.\n"
                "To start it, run:\n"
                "\teval `dbus-launch --auto-syntax`\n");
        return;
    }
    QString serviceName = QStringLiteral("org.lxqt.QTerminal-%1").arg(getpid());
    if (!QDBusConnection::sessionBus().registerService(serviceName))
    {
        fprintf(stderr, "%s\n", qPrintable(QDBusConnection::sessionBus().lastError().message()));
        return;
    }
    new ProcessAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/"), this);
}

QList<QDBusObjectPath> QTerminalApp::getWindows()
{
    QList<QDBusObjectPath> windows;
    for (MainWindow *wnd : qAsConst(m_windowList))
    {
        windows.push_back(wnd->getDbusPath());
    }
    return windows;
}

QDBusObjectPath QTerminalApp::newWindow(const QHash<QString,QVariant> &termArgs)
{
    TerminalConfig cfg = TerminalConfig::fromDbus(termArgs);
    MainWindow *wnd = newWindow(false, cfg);
    assert(wnd != nullptr);
    return wnd->getDbusPath();
}

QDBusObjectPath QTerminalApp::getActiveWindow()
{
    QWidget *aw = activeWindow();
    if (aw == nullptr)
        return QDBusObjectPath("/");
    return qobject_cast<MainWindow*>(aw)->getDbusPath();
}

bool QTerminalApp::isDropMode() {
  if (m_windowList.count() == 0) {
    return false;
  }
  MainWindow *wnd = m_windowList.at(0);
  return wnd->dropMode();
}

bool QTerminalApp::toggleDropdown() {
  if (m_windowList.count() == 0) {
    return false;
  }
  MainWindow *wnd = m_windowList.at(0);
  if (!wnd->dropMode()) {
    return false;
  }
  wnd->showHide();
  return true;
}


#endif
