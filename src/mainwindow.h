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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_qterminal.h"

#include <QMainWindow>

class MainWindow : public QMainWindow , private Ui::mainWindow
{
Q_OBJECT
public:
    MainWindow(const QString& work_dir, const QString& command, QWidget * parent = 0, Qt::WindowFlags f = 0);
    ~MainWindow();
protected slots:
    void on_consoleTabulator_currentChanged(int);
    void quit();
private:
    void addActions();
    void closeEvent(QCloseEvent*);
};
#endif //MAINWINDOW_H
