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

#include <QPainter>
#include <QPalette>
#include <QRect>
#include <QStyleOptionTab>

#include "tabstyle.h"

TabStyle::TabStyle(QObject *parent)
{
    setParent(parent);
}

void TabStyle::drawControl(ControlElement element,
                           const QStyleOption *option,
                           QPainter *painter,
                           const QWidget *widget) const
{
    if (element == QStyle::CE_TabBarTabLabel) {

        // Cast the options for the control to QStyleOptionTab
        const QStyleOptionTab *tabOption = qstyleoption_cast<const QStyleOptionTab*>(option);

        // Determine the rect in which the text will be shown
        QRect rect = subElementRect(SE_TabBarTabText, option, widget);

        // Draw the icon if specified
        if (!tabOption->icon.isNull()) {

            // Create a pixmap (with HiDPI support if enabled)
            QPixmap tabIcon = tabOption->icon.pixmap(
                        widget ? widget->window()->windowHandle() : nullptr,
                        tabOption->iconSize);

            // Draw the pixmap in the correct location
            painter->drawPixmap(rect.left() - tabOption->iconSize.width() - 4,
                                rect.center().y() - tabOption->iconSize.height() / 2,
                                tabIcon);
        }

        // Elide the text (truncate it with an ellipsis)
        QString elidedText = tabOption->fontMetrics.elidedText(tabOption->text,
                                                               Qt::ElideMiddle,
                                                               rect.width());

        // Draw the final text
        drawItemText(painter,
                     rect,
                     Qt::AlignCenter,
                     tabOption->palette,
                     tabOption->state & State_Enabled,
                     elidedText,
                     QPalette::WindowText);

    } else {
        QProxyStyle::drawControl(element, option, painter, widget);
    }
}
