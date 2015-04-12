#include <qtermwidget.h>

#include "properties.h"
#include "config.h"


Properties * Properties::m_instance = 0;


Properties * Properties::Instance(const QString& filename)
{
    if (!m_instance)
        m_instance = new Properties(filename);
    return m_instance;
}

Properties::Properties(const QString& filename) : filename(filename)
{
    if (filename.isEmpty()) {
        QSettings settings;
        this->filename = settings.fileName();
    }
    qDebug("Properties constructor called");
}

Properties::~Properties()
{
    qDebug("Properties destructor called");
    saveSettings();
    delete m_instance;
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
    QSettings settings(filename, QSettings::IniFormat);

    guiStyle = settings.value("guiStyle", QString()).toString();
    if (!guiStyle.isNull())
        QApplication::setStyle(guiStyle);

    colorScheme = settings.value("colorScheme", "Linux").toString();

    highlightCurrentTerminal = settings.value("highlightCurrentTerminal", true).toBool();

    font = qvariant_cast<QFont>(settings.value("font", defaultFont()));

    settings.beginGroup("Shortcuts");
    QStringList keys = settings.childKeys();
    foreach( QString key, keys )
    {
        QKeySequence sequence = QKeySequence( settings.value( key ).toString() );
        if( Properties::Instance()->actions.contains( key ) )
            Properties::Instance()->actions[ key ]->setShortcut( sequence );
    }
    settings.endGroup();

    mainWindowSize = settings.value("MainWindow/size").toSize();
    mainWindowPosition = settings.value("MainWindow/pos").toPoint();
    mainWindowState = settings.value("MainWindow/state").toByteArray();

    historyLimited = settings.value("HistoryLimited", true).toBool();
    historyLimitedTo = settings.value("HistoryLimitedTo", 1000).toUInt();

    emulation = settings.value("emulation", "default").toString();

    // sessions
    int size = settings.beginReadArray("Sessions");
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        QString name(settings.value("name").toString());
        if (name.isEmpty())
            continue;
        sessions[name] = settings.value("state").toByteArray();
    }
    settings.endArray();

    appTransparency = settings.value("MainWindow/ApplicationTransparency", 0).toInt();
    termTransparency = settings.value("TerminalTransparency", 0).toInt();

    /* default to Right. see qtermwidget.h */
    scrollBarPos = settings.value("ScrollbarPosition", 2).toInt();
    /* default to North. I'd prefer South but North is standard (they say) */
    tabsPos = settings.value("TabsPosition", 0).toInt();
    hideTabBarWithOneTab = settings.value("HideTabBarWithOneTab", false).toBool();
    m_motionAfterPaste = settings.value("MotionAfterPaste", 0).toInt();

    /* toggles */
    borderless = settings.value("Borderless", false).toBool();
    tabBarless = settings.value("TabBarless", false).toBool();
    menuVisible = settings.value("MenuVisible", true).toBool();
    askOnExit = settings.value("AskOnExit", true).toBool();
    saveSizeOnExit = settings.value("SaveSizeOnExit", true).toBool();
    savePosOnExit = settings.value("SavePosOnExit", true).toBool();
    useCWD = settings.value("UseCWD", false).toBool();

    // bookmarks
    useBookmarks = settings.value("UseBookmarks", false).toBool();
    bookmarksVisible = settings.value("BookmarksVisible", true).toBool();
    bookmarksFile = settings.value("BookmarksFile", QFileInfo(settings.fileName()).canonicalPath()+"/qterminal_bookmarks.xml").toString();

    terminalsPreset = settings.value("TerminalsPreset", 0).toInt();

    settings.beginGroup("DropMode");
    dropShortCut = QKeySequence(settings.value("ShortCut", "F12").toString());
    dropKeepOpen = settings.value("KeepOpen", false).toBool();
    dropShowOnStart = settings.value("ShowOnStart", true).toBool();
    dropWidht = settings.value("Width", 70).toInt();
    dropHeight = settings.value("Height", 45).toInt();
    settings.endGroup();
}

void Properties::saveSettings()
{
    QSettings settings(filename, QSettings::IniFormat);

    settings.setValue("guiStyle", guiStyle);
    settings.setValue("colorScheme", colorScheme);
    settings.setValue("highlightCurrentTerminal", highlightCurrentTerminal);
    settings.setValue("font", font);

    settings.beginGroup("Shortcuts");
    QMapIterator< QString, QAction * > it(actions);
    while( it.hasNext() )
    {
        it.next();
        QKeySequence shortcut = it.value()->shortcut();
        settings.setValue( it.key(), shortcut.toString() );
    }
    settings.endGroup();

    settings.setValue("MainWindow/size", mainWindowSize);
    settings.setValue("MainWindow/pos", mainWindowPosition);
    settings.setValue("MainWindow/state", mainWindowState);

    settings.setValue("HistoryLimited", historyLimited);
    settings.setValue("HistoryLimitedTo", historyLimitedTo);

    settings.setValue("emulation", emulation);

    // sessions
    settings.beginWriteArray("Sessions");
    int i = 0;
    Sessions::iterator sit = sessions.begin();
    while (sit != sessions.end())
    {
        settings.setArrayIndex(i);
        settings.setValue("name", sit.key());
        settings.setValue("state", sit.value());
        ++sit;
        ++i;
    }
    settings.endArray();

    settings.setValue("MainWindow/ApplicationTransparency", appTransparency);
    settings.setValue("TerminalTransparency", termTransparency);
    settings.setValue("ScrollbarPosition", scrollBarPos);
    settings.setValue("TabsPosition", tabsPos);
    settings.setValue("HideTabBarWithOneTab", hideTabBarWithOneTab);
    settings.setValue("MotionAfterPaste", m_motionAfterPaste);
    settings.setValue("Borderless", borderless);
    settings.setValue("TabBarless", tabBarless);
    settings.setValue("MenuVisible", menuVisible);
    settings.setValue("AskOnExit", askOnExit);
    settings.setValue("SavePosOnExit", savePosOnExit);
    settings.setValue("SaveSizeOnExit", saveSizeOnExit);
    settings.setValue("UseCWD", useCWD);

    // bookmarks
    settings.setValue("UseBookmarks", useBookmarks);
    settings.setValue("BookmarksVisible", bookmarksVisible);
    settings.setValue("BookmarksFile", bookmarksFile);

    settings.setValue("TerminalsPreset", terminalsPreset);

    settings.beginGroup("DropMode");
    settings.setValue("ShortCut", dropShortCut.toString());
    settings.setValue("KeepOpen", dropKeepOpen);
    settings.setValue("ShowOnStart", dropShowOnStart);
    settings.setValue("Width", dropWidht);
    settings.setValue("Height", dropHeight);
    settings.endGroup();

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
        // Paste Selection -> Paste Clipboard
        settings.beginGroup("Shortcuts");
        QString value = settings.value("Paste Selection", PASTE_CLIPBOARD_SHORTCUT).toString();
        settings.setValue(PASTE_CLIPBOARD, value);
        settings.remove("Paste Selection");
        settings.endGroup();
    }
    if (lastVersion <= "0.6.0")
    {

        // AlwaysShowTabs -> HideTabBarWithOneTab
        QString hideValue = settings.value("AlwaysShowTabs", false).toString();
        settings.setValue("HideTabBarWithOneTab", hideValue);
        settings.remove("AlwaysShowTabs");
        // appOpacity -> ApplicationTransparency
        /*
         * Note: In 0.6.0 the opacity values had been erroneously
         * restricted to [0,99] instead of [1,100]. We fix this here by
         * setting the opacity to 100 if it was 99 and to 1 if it was 0.
         */
        int appOpacityValue = settings.value("MainWindow/appOpacity", 100).toInt();
        appOpacityValue = appOpacityValue == 99 ? 100 : appOpacityValue;
        appOpacityValue = appOpacityValue == 0 ? 1 : appOpacityValue;
        settings.setValue("MainWindow/ApplicationTransparency", 100 - appOpacityValue);
        settings.remove("MainWindow/appOpacity");
        // termOpacity -> TerminalTransparency
        int termOpacityValue = settings.value("termOpacity", 100).toInt();
        termOpacityValue = termOpacityValue == 99  ? 100 : termOpacityValue;
        settings.setValue("TerminalTransparency", 100 - termOpacityValue);
        settings.remove("termOpacity");
    }

    if (currentVersion > lastVersion)
        settings.setValue("version", currentVersion);
}

