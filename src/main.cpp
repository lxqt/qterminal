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

#include  <QApplication>
#include <QtGlobal>

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>

#include  "mainwindow.h"

#define out

const char* const short_options = "vhw:e:";

const struct option long_options[] = {
    {"version", 0, NULL, 'v'},
    {"help",    0, NULL, 'h'},
    {"workdir", 1, NULL, 'w'},
    {"execute", 1, NULL, 'e'},
    {NULL,      0, NULL,  0}
};

void print_usage_and_exit(int code)
{
    printf("This is QTerminal %s\n", STR_VERSION);
    puts("Usage: qterminal [options]");
    puts("Options:");
    puts("-h|--help               Print this help");
    puts("-v|--version            Prints application version and exits");
    puts("-w|--workdir <dir>      Start session with specified work directory");
    puts("-e|--execute <command>  Execute command instead of shell");
    puts("\nHomepage: http://qterminal.sourceforge.net/");
    puts("Feature requests, bug reports etc please send to: <petr@scribus.info>\n");
    exit(code);
}

void print_version_and_exit(int code=0)
{
    printf("%s\n", STR_VERSION);
    exit(code);
}

void parse_args(int argc, char* argv[], QString& workdir, QString & shell_command)
{
    int next_option;
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
    setenv("TERM", "xterm", 1);
    QApplication app(argc, argv);
    QString workdir, shell_command;
    parse_args(argc, argv, workdir, shell_command);

    if (workdir.isEmpty())
        workdir = QDir::homePath();

    MainWindow widget(workdir, shell_command);
    widget.show();

    return app.exec();
}
