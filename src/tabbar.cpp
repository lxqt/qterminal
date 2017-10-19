/***************************************************************************
 *   Copyright (C) 2017 by Nathan Osman                                    *
 *   nathan@quickmediasolutions.com                                        *
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

#include "tabbar.h"
#include "tabstyle.h"

TabBar::TabBar(QWidget *parent)
    : QTabBar(parent),
      mLimitWidth(false),
      mLimitWidthValue(0)
{
    setStyle(new TabStyle(this));
}

void TabBar::setLimitWidth(bool limitWidth)
{
    mLimitWidth = limitWidth;
}

void TabBar::setLimitWidthValue(int value)
{
    mLimitWidthValue = value;
}

void TabBar::updateWidth()
{
    // This seems to be the only way to trigger an update
    setIconSize(iconSize());
}

QSize TabBar::tabSizeHint(int index) const
{
    QSize size = QTabBar::tabSizeHint(index);

    // If the width is limited, use that for the width hint
    if (mLimitWidth) {
        size.setWidth(mLimitWidthValue);
    }

    return size;
}
