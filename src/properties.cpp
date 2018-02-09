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

#include <qtermwidget.h>
#include <assert.h>

#include "properties.h"
#include "config.h"
#include "mainwindow.h"
#include "qterminalapp.h"

Properties * Properties::m_instance = 0;


Properties * Properties::Instance(const QString& filename)
{
    if (!m_instance)
        m_instance = new Properties(filename);
    return m_instance;
}

Properties::Properties(const QString& filename)
    : filename(filename)
{
    if (filename.isEmpty())
        m_settings = new QSettings();
    else
        m_settings = new QSettings(filename);
    qDebug("Properties constructor called");
}

Properties::~Properties()
{
    qDebug("Properties destructor called");
    delete m_settings;
    m_instance = 0;
}

QFont Properties::defaultFont()
{
    QFont default_font = QApplication::font();
    default_font.setFamily(DEFAULT_FONT);
    default_font.setPointSize(12);
    default_font.setStyleHint(QFont::TypeWriter);
    return default_font;
}

void Properties::loadSettings()
{
    guiStyle = m_settings->value("guiStyle", QString()).toString();
    if (!guiStyle.isNull())
        QApplication::setStyle(guiStyle);

    colorScheme = m_settings->value("colorScheme", "Linux").toString();

    highlightCurrentTerminal = m_settings->value("highlightCurrentTerminal", true).toBool();
    showTerminalSizeHint = m_settings->value("showTerminalSizeHint", true).toBool();

    font = QFont(qvariant_cast<QString>(m_settings->value("fontFamily", defaultFont().family())),
                 qvariant_cast<int>(m_settings->value("fontSize", defaultFont().pointSize())));
    //Legacy font setting
    font = qvariant_cast<QFont>(m_settings->value("font", font));

    mainWindowSize = m_settings->value("MainWindow/size").toSize();
    mainWindowPosition = m_settings->value("MainWindow/pos").toPoint();
    mainWindowState = m_settings->value("MainWindow/state").toByteArray();

    historyLimited = m_settings->value("HistoryLimited", true).toBool();
    historyLimitedTo = m_settings->value("HistoryLimitedTo", 1000).toUInt();

    emulation = m_settings->value("emulation", "default").toString();

    // sessions
    int size = m_settings->beginReadArray("Sessions");
    for (int i = 0; i < size; ++i)
    {
        m_settings->setArrayIndex(i);
        QString name(m_settings->value("name").toString());
        if (name.isEmpty())
            continue;
        sessions[name] = m_settings->value("state").toByteArray();
    }
    m_settings->endArray();

    appTransparency = m_settings->value("MainWindow/ApplicationTransparency", 0).toInt();
    termTransparency = m_settings->value("TerminalTransparency", 0).toInt();
    backgroundImage = m_settings->value("TerminalBackgroundImage", QString()).toString();

    /* default to Right. see qtermwidget.h */
    scrollBarPos = m_settings->value("ScrollbarPosition", 2).toInt();
    /* default to North. I'd prefer South but North is standard (they say) */
    tabsPos = m_settings->value("TabsPosition", 0).toInt();
    /* default to BlockCursor */
    keyboardCursorShape = m_settings->value("KeyboardCursorShape", 0).toInt();
    hideTabBarWithOneTab = m_settings->value("HideTabBarWithOneTab", false).toBool();
    m_motionAfterPaste = m_settings->value("MotionAfterPaste", 0).toInt();

    /* tab width limit */
    limitTabWidth = m_settings->value("LimitTabWidth", true).toBool();
    limitTabWidthValue = m_settings->value("LimitTabWidthValue", 500).toInt();
    showCloseTabButton = m_settings->value("ShowCloseTabButton", true).toBool();

    /* toggles */
    borderless = m_settings->value("Borderless", false).toBool();
    tabBarless = m_settings->value("TabBarless", false).toBool();
    menuVisible = m_settings->value("MenuVisible", true).toBool();
    askOnExit = m_settings->value("AskOnExit", true).toBool();
    saveSizeOnExit = m_settings->value("SaveSizeOnExit", true).toBool();
    savePosOnExit = m_settings->value("SavePosOnExit", true).toBool();
    useCWD = m_settings->value("UseCWD", false).toBool();
    term = m_settings->value("Term", "xterm-256color").toString();

    // bookmarks
    useBookmarks = m_settings->value("UseBookmarks", false).toBool();
    bookmarksVisible = m_settings->value("BookmarksVisible", true).toBool();
    const QString s = QFileInfo(m_settings->fileName()).canonicalPath() + QString::fromLatin1("/qterminal_bookmarks.xml");
    bookmarksFile = m_settings->value("BookmarksFile", s).toString();

    terminalsPreset = m_settings->value("TerminalsPreset", 0).toInt();

    m_settings->beginGroup("DropMode");
    dropShortCut = QKeySequence(m_settings->value("ShortCut", "F12").toString());
    dropKeepOpen = m_settings->value("KeepOpen", false).toBool();
    dropShowOnStart = m_settings->value("ShowOnStart", true).toBool();
    dropWidht = m_settings->value("Width", 70).toInt();
    dropHeight = m_settings->value("Height", 45).toInt();
    m_settings->endGroup();

    changeWindowTitle = m_settings->value("ChangeWindowTitle", true).toBool();
    changeWindowIcon = m_settings->value("ChangeWindowIcon", true).toBool();
    enabledBidiSupport = m_settings->value("enabledBidiSupport", true).toBool();

    confirmMultilinePaste = m_settings->value("ConfirmMultilinePaste", false).toBool();
    trimPastedTrailingNewlines = m_settings->value("TrimPastedTrailingNewlines", false).toBool();
}

void Properties::saveSettings()
{
    m_settings->setValue("guiStyle", guiStyle);
    m_settings->setValue("colorScheme", colorScheme);
    m_settings->setValue("highlightCurrentTerminal", highlightCurrentTerminal);
    m_settings->setValue("showTerminalSizeHint", showTerminalSizeHint);
    m_settings->setValue("fontFamily", font.family());
    m_settings->setValue("fontSize", font.pointSize());
    //Clobber legacy setting
    m_settings->remove("font");

    m_settings->beginGroup("Shortcuts");
    MainWindow *mainWindow = QTerminalApp::Instance()->getWindowList()[0];
    assert(mainWindow != NULL);

    QMapIterator< QString, QAction * > it(mainWindow->leaseActions());
    while( it.hasNext() )
    {
        it.next();
        QStringList sequenceStrings;
        const auto shortcuts = it.value()->shortcuts();
        for (const QKeySequence &shortcut : shortcuts)
            sequenceStrings.append(shortcut.toString());
        m_settings->setValue(it.key(), sequenceStrings.join('|'));
    }
    m_settings->endGroup();

    m_settings->setValue("MainWindow/size", mainWindowSize);
    m_settings->setValue("MainWindow/pos", mainWindowPosition);
    m_settings->setValue("MainWindow/state", mainWindowState);

    m_settings->setValue("HistoryLimited", historyLimited);
    m_settings->setValue("HistoryLimitedTo", historyLimitedTo);

    m_settings->setValue("emulation", emulation);

    // sessions
    m_settings->beginWriteArray("Sessions");
    int i = 0;
    Sessions::iterator sit = sessions.begin();
    while (sit != sessions.end())
    {
        m_settings->setArrayIndex(i);
        m_settings->setValue("name", sit.key());
        m_settings->setValue("state", sit.value());
        ++sit;
        ++i;
    }
    m_settings->endArray();

    m_settings->setValue("MainWindow/ApplicationTransparency", appTransparency);
    m_settings->setValue("TerminalTransparency", termTransparency);
    m_settings->setValue("TerminalBackgroundImage", backgroundImage);
    m_settings->setValue("ScrollbarPosition", scrollBarPos);
    m_settings->setValue("TabsPosition", tabsPos);
    m_settings->setValue("KeyboardCursorShape", keyboardCursorShape);
    m_settings->setValue("HideTabBarWithOneTab", hideTabBarWithOneTab);
    m_settings->setValue("MotionAfterPaste", m_motionAfterPaste);

    m_settings->setValue("LimitTabWidth", limitTabWidth);
    m_settings->setValue("LimitTabWidthValue", limitTabWidthValue);
    m_settings->setValue("ShowCloseTabButton", showCloseTabButton);

    m_settings->setValue("Borderless", borderless);
    m_settings->setValue("TabBarless", tabBarless);
    m_settings->setValue("MenuVisible", menuVisible);
    m_settings->setValue("AskOnExit", askOnExit);
    m_settings->setValue("SavePosOnExit", savePosOnExit);
    m_settings->setValue("SaveSizeOnExit", saveSizeOnExit);
    m_settings->setValue("UseCWD", useCWD);
    m_settings->setValue("Term", term);

    // bookmarks
    m_settings->setValue("UseBookmarks", useBookmarks);
    m_settings->setValue("BookmarksVisible", bookmarksVisible);
    m_settings->setValue("BookmarksFile", bookmarksFile);

    m_settings->setValue("TerminalsPreset", terminalsPreset);

    m_settings->beginGroup("DropMode");
    m_settings->setValue("ShortCut", dropShortCut.toString());
    m_settings->setValue("KeepOpen", dropKeepOpen);
    m_settings->setValue("ShowOnStart", dropShowOnStart);
    m_settings->setValue("Width", dropWidht);
    m_settings->setValue("Height", dropHeight);
    m_settings->endGroup();

    m_settings->setValue("ChangeWindowTitle", changeWindowTitle);
    m_settings->setValue("ChangeWindowIcon", changeWindowIcon);
    m_settings->setValue("enabledBidiSupport", enabledBidiSupport);


    m_settings->setValue("ConfirmMultilinePaste", confirmMultilinePaste);
    m_settings->setValue("TrimPastedTrailingNewlines", trimPastedTrailingNewlines);
}

void Properties::migrate_settings()
{
    // Deal with rearrangements of settings.
    // If this method becomes unbearably huge we should look at the config-update
    // system used by kde and razor.
    QSettings settings;
    QString lastVersion = settings.value("version", "0.0.0").toString();
    QString currentVersion = STR_VERSION;
    if (currentVersion < lastVersion)
    {
        qDebug() << "Warning: Configuration file was written by a newer version "
                 << "of QTerminal. Some settings might be incompatible";
    }

    if (lastVersion < "0.4.0")
    {
        // ===== Paste Selection -> Paste Clipboard =====
        settings.beginGroup("Shortcuts");
        if(!settings.contains(PASTE_CLIPBOARD))
        {
            QString value = settings.value("Paste Selection", PASTE_CLIPBOARD_SHORTCUT).toString();
            settings.setValue(PASTE_CLIPBOARD, value);
        }
        settings.remove("Paste Selection");
        settings.endGroup();
    }

    if (lastVersion <= "0.6.0")
    {
        // ===== AlwaysShowTabs -> HideTabBarWithOneTab =====
        if(!settings.contains("HideTabBarWithOneTab"))
        {
            QString hideValue = settings.value("AlwaysShowTabs", false).toString();
            settings.setValue("HideTabBarWithOneTab", hideValue);
        }
        settings.remove("AlwaysShowTabs");

        // ===== appOpacity -> ApplicationTransparency =====
        //
        // Note: In 0.6.0 the opacity values had been erroneously
        // restricted to [0,99] instead of [1,100]. We fix this here by
        // setting the opacity to 100 if it was 99 and to 1 if it was 0.
        //
        if(!settings.contains("MainWindow/ApplicationTransparency"))
        {
            int appOpacityValue = settings.value("MainWindow/appOpacity", 100).toInt();
            appOpacityValue = appOpacityValue == 99 ? 100 : appOpacityValue;
            appOpacityValue = appOpacityValue == 0 ? 1 : appOpacityValue;
            settings.setValue("MainWindow/ApplicationTransparency", 100 - appOpacityValue);
        }
        settings.remove("MainWindow/appOpacity");

        // ===== termOpacity -> TerminalTransparency =====
        if(!settings.contains("TerminalTransparency"))
        {
            int termOpacityValue = settings.value("termOpacity", 100).toInt();
            termOpacityValue = termOpacityValue == 99  ? 100 : termOpacityValue;
            settings.setValue("TerminalTransparency", 100 - termOpacityValue);
        }
        settings.remove("termOpacity");
	// geometry -> size, pos
	if (!settings.contains("MainWindow/size"))
	{
	    QWidget geom;
	    geom.restoreGeometry(settings.value("MainWindow/geometry").toByteArray());
            settings.setValue("MainWindow/size", geom.size());
            settings.setValue("MainWindow/pos", geom.pos());
            settings.remove("MainWindow/geometry");
	}
    }

    if (currentVersion > lastVersion)
        settings.setValue("version", currentVersion);
}

