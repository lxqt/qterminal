/***************************************************************************
 *   Copyright (C) 2010 by Petr Vanek                                      *
 *   petr@scribus.info                                                     *
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

#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QApplication>
#include <QtCore>
#include <QFont>

typedef QString Session;

typedef QMap<QString,Session> Sessions;

typedef QMap<QString,QString> ShortcutMap;


class Properties
{
    public:
        static Properties *Instance(const QString& filename = QString());
        ~Properties();

        QFont defaultFont();
        void saveSettings();
        void loadSettings();
        void migrate_settings();

        QSize mainWindowSize;
        QPoint mainWindowPosition;
        QByteArray mainWindowState;
        //ShortcutMap shortcuts;
        QString shell;
        QFont font;
        QString colorScheme;
        QString guiStyle;
        bool highlightCurrentTerminal;
        bool showTerminalSizeHint;

        bool historyLimited;
        unsigned historyLimitedTo;

        QString emulation;

        Sessions sessions;

        int appTransparency;
        int termTransparency;
        QString backgroundImage;

        int scrollBarPos;
        int tabsPos;
        int keyboardCursorShape;
        bool hideTabBarWithOneTab;
        int m_motionAfterPaste;

        bool limitTabWidth;
        int limitTabWidthValue;

        bool showCloseTabButton;

        bool borderless;
        bool tabBarless;
        bool menuVisible;

        bool askOnExit;

        bool saveSizeOnExit;
        bool savePosOnExit;

        bool useCWD;

        QString term;

        bool useBookmarks;
        bool bookmarksVisible;
        QString bookmarksFile;

        int terminalsPreset;

        QKeySequence dropShortCut;
        bool dropKeepOpen;
        bool dropShowOnStart;
        int dropWidht;
        int dropHeight;

        bool changeWindowTitle;
        bool changeWindowIcon;
        bool enabledBidiSupport;

        bool confirmMultilinePaste;
        bool trimPastedTrailingNewlines;


    private:

        // Singleton handling
        static Properties *m_instance;
        QString filename;

        explicit Properties(const QString& filename);
        Properties(const Properties &) {};

        QSettings *m_settings;

};

#endif

