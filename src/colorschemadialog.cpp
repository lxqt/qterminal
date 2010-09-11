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


#include "colorschemadialog.h"



ColorSchemaDialog::ColorSchemaDialog( QWidget * parent, Qt::WindowFlags f) : QDialog(parent,f)
{
    colorSchemes << "White On Black" << \
                    "Green On Black" << \
                    "Black On Light Yellow";
    setupUi(this);
    colorSchemaCombo->addItems(colorSchemes);
}

ColorSchemaDialog::~ColorSchemaDialog()
{
}

int ColorSchemaDialog::colorScheme()const
{
    return colorSchemaCombo->currentIndex() + 1;
}

void ColorSchemaDialog::setColorScheme(int scheme)
{
    colorSchemaCombo->setCurrentIndex(scheme - 1);
}
