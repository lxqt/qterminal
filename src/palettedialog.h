/***************************************************************************
 *   Copyright (C) 2025 by Thomas Lübking                                  *
 *   thomas.luebking@gmail.com                                             *
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

#ifndef PALETTEDIALOG_H
#define PALETTEDIALOG_H

#include <QDialog>
#include <QMap>
#include "ui_palettedialog.h"

class PaletteDialog : public QDialog, Ui::PaletteDialog
{
    Q_OBJECT
public:
    PaletteDialog(QWidget *parent = nullptr);
    void setColorSchemeName(QString name);
    QMap<QString, QColor> colors();
    int setColors(QMap<QString, QColor>);
protected:
    bool eventFilter(QObject *o, QEvent *e);
private:
    void setColor(QLabel *l, QColor c);
};

#endif // PALETTEDIALOG_H