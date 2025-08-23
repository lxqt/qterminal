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
#include <QPainter>
#include <QStyleOption>
#include <QProxyStyle>

class TabTextStyle : public QProxyStyle
{ // Give a normal font to inactive tabs.
public:
    using QProxyStyle::QProxyStyle;
    void drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const override
    {
        bool painterSaved = false;
        if (element == QStyle::CE_TabBarTabLabel)
        {
            if (const auto opt = qstyleoption_cast<const QStyleOptionTab*>(option))
            {
                if (const auto tb = qobject_cast<const QTabBar*>(widget))
                {
                    int indx = tb->currentIndex();
                    if (indx > -1 && !tb->tabRect(indx).contains(opt->rect.center()))
                    {
                        QFont f = painter->font();
                        f.setBold(false);
                        painter->save();
                        painterSaved = true;
                        painter->setFont(f);
                    }
                }
            }
        }
        QProxyStyle::drawControl(element, option, painter, widget);
        if (painterSaved)
        {
            painter->restore();
        }
    }
};

TabBar::TabBar(QWidget *parent)
    : QTabBar(parent),
      mFixedWidth(false),
      mFixedWidthValue(0)
{
    // To make the text of the active tab bold, first give a bold font to the tabbar
    // for QStyle::sizeFromContents(QStyle::CT_TabBarTab, ...) to make room for the
    // bold text, and then use TabTextStyle as the style.
    // WARNING: Using a stylesheet could result in elided texts with Qt6,
    // although it worked with Qt5.
    QFont f = font();
    f.setBold(true);
    setFont(f);
    TabTextStyle *s = new TabTextStyle;
    s->setParent(this);
    setStyle(s);
}

void TabBar::setFixedWidth(bool fixedWidth)
{
    mFixedWidth = fixedWidth;
}

void TabBar::setFixedWidthValue(int value)
{
    mFixedWidthValue = value;
}

void TabBar::updateWidth()
{
    // This seems to be the only way to trigger an update
    setIconSize(iconSize());
    setElideMode(Qt::ElideMiddle);
}

QSize TabBar::tabSizeHint(int index) const
{
    QSize size = QTabBar::tabSizeHint(index);

    // If the width is fixed, use that for the width hint
    if (mFixedWidth) {
        if (shape() == QTabBar::RoundedEast || shape() == QTabBar::TriangularEast
            || shape() == QTabBar::RoundedWest || shape() == QTabBar::TriangularWest) {
            size.setHeight(mFixedWidthValue);
        }
        else {
            size.setWidth(mFixedWidthValue);
        }
    }

    return size;
}
