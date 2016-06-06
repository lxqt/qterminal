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

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>

#include  "mainwindow.h"

#define out

const char* const short_options = "vhw:e:dp:";

const struct option long_options[] = {
    {"version", 0, NULL, 'v'},
    {"help",    0, NULL, 'h'},
    {"workdir", 1, NULL, 'w'},
    {"execute", 1, NULL, 'e'},
    {"drop",    0, NULL, 'd'},
    {"profile", 1, NULL, 'p'},
    {NULL,      0, NULL,  0}
};

void print_usage_and_exit(int code)
{
    printf("QTerminal %s\n", STR_VERSION);
    puts("Usage: qterminal [OPTION]...\n");
    puts("  -d,  --drop               Start in \"dropdown mode\" (like Yakuake or Tilda)");
    puts("  -e,  --execute <command>  Execute command instead of shell");
    puts("  -h,  --help               Print this help");
    puts("  -p,  --profile            Load qterminal with specific options");
    puts("  -v,  --version            Prints application version and exits");
    puts("  -w,  --workdir <dir>      Start session with specified work directory");
    puts("\nHomepage: <https://github.com/lxde/qterminal>");
    puts("Report bugs to <https://github.com/lxde/qterminal/issues>");
    exit(code);
}

void print_version_and_exit(int code=0)
{
    printf("%s\n", STR_VERSION);
    exit(code);
}

void parse_args(int argc, char* argv[], QString& workdir, QString & shell_command, out bool& dropMode)
{
    int next_option;
    dropMode = false;
    do{
        next_option = getopt_long(argc, argv, short_options, long_options, NULL);
        switch(next_option)
        {
            case 'h':
                print_usage_and_exit(0);
            case 'w':
                workdir = QString(optarg);
                break;
            case 'e':
                shell_command = QString(optarg);
                // #15 "Raw" -e params
                // Passing "raw" params (like konsole -e mcedit /tmp/tmp.txt") is more preferable - then I can call QString("qterminal -e ") + cmd_line in other programs
                while (optind < argc)
                {
                    //printf("arg: %d - %s\n", optind, argv[optind]);
                    shell_command += ' ' + QString(argv[optind++]);
                }
                break;
            case 'd':
                dropMode = true;
                break;
            case 'p':
                Properties::Instance(QString(optarg));
                break;
            case '?':
                print_usage_and_exit(1);
            case 'v':
                print_version_and_exit();
        }
    }
    while(next_option != -1);
}

int main(int argc, char *argv[])
{
    setenv("TERM", "xterm", 1); // TODO/FIXME: why?

    QApplication::setApplicationName("qterminal");
    QApplication::setApplicationVersion(STR_VERSION);
    QApplication::setOrganizationDomain("qterminal.org");
    // Warning: do not change settings format. It can screw bookmarks later.
    QSettings::setDefaultFormat(QSettings::IniFormat);

    QApplication app(argc, argv);
    QString workdir, shell_command;
    bool dropMode;
    parse_args(argc, argv, workdir, shell_command, dropMode);

    if (workdir.isEmpty())
        workdir = QDir::currentPath();

    // icons
    /* setup our custom icon theme if there is no system theme (OS X, Windows) */
    if (QIcon::themeName().isEmpty())
        QIcon::setThemeName("QTerminal");

    // translations
    QString fname = QString("qterminal_%1.qm").arg(QLocale::system().name().left(5));
    QTranslator translator;
#ifdef TRANSLATIONS_DIR
    qDebug() << "TRANSLATIONS_DIR: Loading translation file" << fname << "from dir" << TRANSLATIONS_DIR;
    qDebug() << "load success:" << translator.load(fname, TRANSLATIONS_DIR, "_");
#endif
#ifdef APPLE_BUNDLE
    qDebug() << "APPLE_BUNDLE: Loading translator file" << fname << "from dir" << QApplication::applicationDirPath()+"../translations";
    qDebug() << "load success:" << translator.load(fname, QApplication::applicationDirPath()+"../translations", "_");
#endif
    app.installTranslator(&translator);

    MainWindow *window;
    if (dropMode)
    {
        QWidget *hiddenPreviewParent = new QWidget(0, Qt::Tool);
        window = new MainWindow(workdir, shell_command, dropMode, hiddenPreviewParent);
        if (Properties::Instance()->dropShowOnStart)
            window->show();
    }
    else
    {
        window = new MainWindow(workdir, shell_command, dropMode);
        window->show();
    }

    int ret = app.exec();
    delete Properties::Instance();
    delete window;

    return ret;
}
