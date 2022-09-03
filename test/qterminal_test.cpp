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

#include "qterminal_test.h"

#include "qterminalutils.h"

#include <QtTest>

// handy shortcut copied from liblxqt
#ifndef QL1S
#define QL1S(x) QLatin1String(x)
#endif

void QTerminalTest::testParseCommand()
{
    /* Common usage */
    // qterminal -e 'fpad -s "PATH/ha ha"'
    // qterminal -e "fpad -s \"PATH/ha ha\"" # \" is decoded as " by the shell
    QCOMPARE(parse_command(QL1S(R"(fpad -s "PATH/ha ha")")),
             QStringList() << QL1S("fpad") << QL1S("-s") << QL1S("PATH/ha ha"));
    // qterminal -e "fpad -s 'PATH/ha ha'"
    QCOMPARE(parse_command(QL1S(R"(fpad -s 'PATH/ha ha')")),
             QStringList() << QL1S("fpad") << QL1S("-s") << QL1S("PATH/ha ha"));
    // qterminal -e 'fpad -s PATH/ha\ ha'
    // qterminal -e "fpad -s PATH/ha\ ha"
    QCOMPARE(parse_command(QL1S(R"(fpad -s PATH/ha\ ha)")),
             QStringList() << QL1S("fpad") << QL1S("-s") << QL1S("PATH/ha ha"));

    /* Uncommon usage */
    // qterminal -e 'fpad -s \"PATH/ha ha\"'
    QCOMPARE(parse_command(QL1S(R"(fpad -s \"PATH/ha ha\")")),
             QStringList() << QL1S("fpad") << QL1S("-s") << QL1S("\\\"PATH/ha") << QL1S("ha\\\""));
    // qterminal -e 'fpad -s "PATH/ha\ ha"'
    QCOMPARE(parse_command(QL1S(R"(fpad -s "PATH/ha\ ha")")),
             QStringList() << QL1S("fpad") << QL1S("-s") << QL1S("PATH/ha ha"));
}

QTEST_MAIN(QTerminalTest)
