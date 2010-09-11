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

#include "version.h"
#include  "mainwindow.h"

#define out

const char* const short_options = "hw:";//e:";

const struct option long_options[] = {
    {"help",    0, NULL, 'h'},
    {"workdir", 1, NULL, 'w'},
    //{"execute", 1, NULL, 'e'},
    {NULL,      0, NULL,  0}
};

void print_usage_and_exit(int code)
{
    printf("This is %s\n", STR_VERSION);
    puts("Usage: qterminal [options]");
    puts("Options:");
    puts("--help                  Print this help");
    puts("--workdir <dir>         Start session with specified work directory");
    //puts("--execute <command>     Execute command instead of shell");
    puts("\nThis application based on QTermWidget by e_k@users.sourceforge.net");
    puts("Homepage: http://qterminal.sourceforge.net/");
    puts("Feature requests, bug reports etc please send to: <vovanec@gmail.com>\n");
    exit(code);
}

void parse_args(int argc, char* argv[], out QString& workdir, out QString& shell_command)
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
            //case 'e':
            //    shell_command = QString(optarg);
            //    break;
            case '?':
                print_usage_and_exit(1);
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

    MainWindow widget(workdir, shell_command);
    widget.show();

    return app.exec();
}
