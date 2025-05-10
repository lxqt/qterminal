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
#include <QFileSystemWatcher>

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
        QString getShortcut(const QString &name, const QString &defaultShortcut) const;
        QString configDir() const;
        QString profile() const;

        static void removeAccelerator(QString& str);

        QSize mainWindowSize;
        QSize fixedWindowSize;
        QSize prefDialogSize;
        QPoint mainWindowPosition;
        QByteArray mainWindowState;
        //ShortcutMap shortcuts;
        QStringList shell;
        QFont font;
        QString colorScheme;
        QString guiStyle;
        bool highlightCurrentTerminal;
        bool focusOnMoueOver;
        bool showTerminalSizeHint;

        bool historyLimited;
        unsigned historyLimitedTo;

        QString emulation;

        Sessions sessions;

        int terminalMargin;
        int termTransparency;
        QString backgroundImage;
        int backgroundMode;

        int scrollBarPos;
        int tabsPos;
        int keyboardCursorShape;
        bool keyboardCursorBlink;
        bool hideTabBarWithOneTab;
        int m_motionAfterPaste;
        bool m_disableBracketedPasteMode;

        bool fixedTabWidth;
        int fixedTabWidthValue;

        bool showCloseTabButton;
        bool closeTabOnMiddleClick;

        bool boldIntense;

        bool borderless;
        bool tabBarless;
        bool noMenubarAccel;
        bool menuVisible;

        bool askOnExit;

        bool saveSizeOnExit;
        bool savePosOnExit;
        bool saveStateOnExit;

        bool useCWD;
        bool m_openNewTabRightToActiveTab;

        bool audibleBell;

        QString term;

        QString handleHistoryCommand;

        bool useBookmarks;
        bool bookmarksVisible;
        QString bookmarksFile;

        int terminalsPreset;

        QKeySequence dropShortCut;
        bool dropKeepOpen;
        bool dropShowOnStart;
        int dropWidth;
        int dropHeight;

        bool changeWindowTitle;
        bool changeWindowIcon;
        bool enabledBidiSupport;

        bool confirmMultilinePaste;
        bool trimPastedTrailingNewlines;
        QString wordCharacters;

        bool windowMaximized;
        bool swapMouseButtons2and3;
        int mouseAutoHideDelay;

        bool useFontBoxDrawingChars;
    private:

        Properties(const Properties &) = delete;
        Properties &operator=(const Properties &) = delete;

        int versionComparison(const QString &v1, const QString &v2);

        // Singleton handling
        static Properties *m_instance;
        QString filename;

        explicit Properties(const QString& filename);

        QSettings *m_settings;

        QFileSystemWatcher *m_watcher;
};

#endif

