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

#include <cassert>
#include <cstdio>
#include <getopt.h>
#include <cstdlib>
#include <unistd.h>
#include <utility>

#ifdef HAVE_QDBUS
    #include <QtDBus/QtDBus>
    #include "processadaptor.h"
#endif


#include "mainwindow.h"
#include "qterminalapp.h"
#include "qterminalutils.h"
#include "terminalconfig.h"
#include "termwidget.h"

#define out

const char* const short_options = "vhw:e:dp:i:s:";

static const char* serviceName = "org.lxqt.QTerminal";
static const char* ifaceName = "org.lxqt.QTerminal.Process";

const struct option long_options[] = {
    {"version", 0, nullptr, 'v'},
    {"help",    0, nullptr, 'h'},
    {"workdir", 1, nullptr, 'w'},
    {"execute", 1, nullptr, 'e'},
    {"drop",    0, nullptr, 'd'},
    {"profile", 1, nullptr, 'p'},
    {"dbus_id", 1, nullptr, 'i'},
    {"size",    1, nullptr, 's'},
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
    puts("  -i,  --dbus_id <name>     Register with predetermined dbus interface, org.lxqt.QTerminal-<id>");
    puts("  -p,  --profile <name>     Load profile from ~/.config/<name>.conf");
    puts("  -s,  --size    <CxL>      Set initial size in columns and lines");
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

void parse_args(int argc, char* argv[], QString& workdir, QStringList & shell_command, out bool& dropMode, QString &dbus_id, QSize &size)
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
            case 'i':
                dbus_id = QString::fromLocal8Bit(optarg);
                break;
            case 's':
            {
                QStringList values = QString::fromLocal8Bit(optarg).split(QStringLiteral("x"));
                if (values.size() == 2)
                {
                    int cols = values.at(0).toInt();
                    int lines = values.at(1).toInt();
                    if (cols > 0 && lines > 0)
                        size = QSize(cols, lines);
                }
                break;
            }
            case '?':
                print_usage_and_exit(1);
                break;
            case 'v':
                print_version_and_exit();
                break;
        }
    }
    while(next_option != -1);

    // FIXME: The app might not exit in the dropdown mode after the shell command is terminated
    // and the window is closed. For now, the dropdown mode is disabled with command execution.
    if (!shell_command.isEmpty())
    {
        dropMode = false;
    }
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
    QApplication::setDesktopFileName(QLatin1String("qterminal"));
    // Warning: do not change settings format. It can screw bookmarks later.
    QSettings::setDefaultFormat(QSettings::IniFormat);

    QTerminalApp *app = QTerminalApp::Instance(argc, argv);

    QString workdir;
    QStringList shell_command;
    bool dropMode = false;
    QString dbus_id;
    QSize size;
    parse_args(argc, argv, workdir, shell_command, dropMode, dbus_id, size);

    #ifdef HAVE_QDBUS
        app->registerOnDbus(dropMode, dbus_id);
    #endif

    if (!app->isPrimaryInstance())
    {
        app->requestDropDown();
        return 0;
    }

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
    if (qtTranslator.load(QStringLiteral("qt_") + QLocale::system().name(), QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
    {
        app->installTranslator(&qtTranslator);
    }

    QTranslator translator;
    bool installTr = false;
    QString fname = QString::fromLatin1("qterminal_%1.qm").arg(QLocale::system().name().left(5));
#ifdef TRANSLATIONS_DIR
    //qDebug() << "TRANSLATIONS_DIR: Loading translation file" << fname << "from dir" << TRANSLATIONS_DIR;
    installTr = translator.load(fname, QString::fromUtf8(TRANSLATIONS_DIR), QStringLiteral("_"));
#endif
#ifdef APPLE_BUNDLE
    QDir translations_dir = QDir(QApplication::applicationDirPath());
    translations_dir.cdUp();
    if (translations_dir.cd(QStringLiteral("Resources/translations"))) {
        installTr = translator.load(fname, translations_dir.path(), QStringLiteral("_"));
    } /*else {
        qWarning() << "Unable to find \"Resources/translations\" dir in" << translations_dir.path();
    }*/
#endif
    if (installTr)
    {
        app->installTranslator(&translator);
    }

#ifndef HAVE_LAYERSHELLQT
    if (QGuiApplication::platformName() == QStringLiteral("wayland"))
    {
        fprintf(stderr, "Running on Wayland, although Wayland support was disabled at compile-time. Expect errors.\n");
    }
#endif

    TerminalConfig initConfig = TerminalConfig(workdir, shell_command);
    if (MainWindow *wnd = app->newWindow(dropMode, initConfig, dbus_id))
        wnd->setInitialSize(size);

    int ret = app->exec();
    delete Properties::Instance();
    app->cleanup();

    return ret;
}

MainWindow *QTerminalApp::newWindow(bool dropMode, TerminalConfig &cfg, const QString &dbus_id)
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
        window = new MainWindow(cfg, dropMode, dbus_id);
        if (Properties::Instance()->saveSizeOnExit
            && Properties::Instance()->windowMaximized)
        {
            window->setWindowState(Qt::WindowMaximized);
        }
        // this gives us time to resize the window on user control
        QMetaObject::invokeMethod(window, "show", Qt::QueuedConnection);
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
void QTerminalApp::registerOnDbus(bool dropDown, QString dbus_id)
{
    if (!QDBusConnection::sessionBus().isConnected())
    {
        fprintf(stderr, "Cannot connect to the D-Bus session bus.\n"
                "To start it, run:\n"
                "\teval `dbus-launch --auto-syntax`\n");
        return;
    }

    if (dropDown)
    {
        if (!QDBusConnection::sessionBus().registerService(QLatin1String(serviceName)))
        {
            m_isPrimaryInstance = false;
            return;
        }
        m_dbusService = QLatin1String(serviceName);
        new ProcessAdaptor(this);
        QDBusConnection::sessionBus().registerObject(QStringLiteral("/"), this);
    }
    else
    {
        const QString suffix = dbus_id.isEmpty() ? QStringLiteral("-%1").arg(getpid()) : QStringLiteral("-%1").arg(dbus_id);
        if (!QDBusConnection::sessionBus().registerService(QLatin1String(serviceName) + suffix))
        {
            fprintf(stderr, "%s\n", qPrintable(QDBusConnection::sessionBus().lastError().message()));
            return;
        }
        m_dbusService = QLatin1String(serviceName) + suffix;
        new ProcessAdaptor(this);
        QDBusConnection::sessionBus().registerObject(QStringLiteral("/"), this);
    }
}

QList<QDBusObjectPath> QTerminalApp::getWindows()
{
    QList<QDBusObjectPath> windows;
    for (MainWindow *wnd : std::as_const(m_windowList))
    {
        windows.push_back(wnd->getDbusPath());
    }
    return windows;
}

static QDBusObjectPath spawnNewProcess(const QString &dbus_id, const QString &shell_command, const QString& workdir, int columns, int lines)
{
    QStringList args;
    args <<  QStringLiteral("-i") << dbus_id;
    if (columns > 0 && lines > 0)
        args <<  QStringLiteral("-s") << QStringLiteral("%1x%2").arg(columns).arg(lines);
    args <<  QStringLiteral("-w") << workdir;
    QString profile = Properties::Instance()->profile();
    if (!profile.isEmpty())
        args << QStringLiteral("-p") << profile;
    args <<  QStringLiteral("-e") << shell_command;
    QProcess::startDetached(QCoreApplication::applicationFilePath(), args);
    return QDBusObjectPath();
}

QDBusObjectPath QTerminalApp::newWindow(const QString &dbus_id, const QString &shell_command, const QString& workdir, int columns, int lines)
{
    // dropDown can have only one window
    for (MainWindow *wnd : m_windowList)
        if (wnd->dropMode())
            return spawnNewProcess(dbus_id, shell_command, workdir.isEmpty() ? m_workDir : workdir, columns, lines);

    TerminalConfig cfg = TerminalConfig(workdir.isEmpty() ? m_workDir : workdir, parse_command(shell_command));
    MainWindow *wnd = newWindow(false, cfg, dbus_id);
    assert(wnd != nullptr);
    if (columns > 0 || lines > 0)
        wnd->setInitialSize(QSize(columns, lines));
    return wnd->getDbusPath();
}

QDBusObjectPath QTerminalApp::newWindow(const QHash<QString,QVariant> &termArgs)
{
    TerminalConfig cfg = TerminalConfig::fromDbus(termArgs);

    for (MainWindow *wnd : m_windowList)
        if (wnd->dropMode())
            return spawnNewProcess(QString(), cfg.getShell().join(QStringLiteral(" ")), cfg.getWorkingDirectory(), 0, 0);

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

void QTerminalApp::requestDropDown()
{
    QDBusInterface iface(QLatin1String(serviceName),
                         QStringLiteral("/"),
                         QLatin1String(ifaceName), QDBusConnection::sessionBus(), this);
    iface.call(QStringLiteral("toggleDropdown"));
}

bool QTerminalApp::isPrimaryInstance() {
  return m_isPrimaryInstance;
}


#endif

