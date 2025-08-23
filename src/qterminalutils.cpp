/***************************************************************************
 *   Copyright (C) 2022 by LXQt team                                       *
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

#include <QRegularExpression>

#include "qterminalutils.h"

QStringList parse_command(const QString& str)
{
    const QRegularExpression separator(QString::fromLatin1(R"('|(?<!\\)(\\{2})*(\s|")|\z)"));
    const QRegularExpression doubleQuote(QString::fromLatin1(R"((?<!\\)(\\{2})*")"));
    const QRegularExpression escapedSpace(QString::fromLatin1(R"(\\(\\{2})*\s)"));
    const QRegularExpression singleQuote(QStringLiteral("'"));

    QStringList list;
    QRegularExpressionMatch match;
    int index = 0;
    int nextIndex = 0;
    while((nextIndex = str.indexOf(separator, index, &match)) != -1)
    {
        if (nextIndex > index)
        {
            list << str.mid(index, nextIndex - index).replace(escapedSpace, QStringLiteral(" "));
        }
        if (match.capturedLength() == 0)
        { // end of string ("\z") is matched
            break;
        }
        index = nextIndex + match.capturedLength();
        auto c = str.at(index - 1); // last matched character
        if (!c.isSpace())
        { // a single quote or an unescaped double quote is matched
            nextIndex = str.indexOf(c == QLatin1Char('\'') ? singleQuote : doubleQuote, index, &match);
            if (nextIndex == -1)
            { // the quote is not closed
                break;
            }
            else
            {
                if (nextIndex > index)
                {
                    list << str.mid(index, nextIndex - index).replace(escapedSpace, QStringLiteral(" "));
                }
                index = nextIndex + match.capturedLength();
            }
        }
    }
    return list;
}

