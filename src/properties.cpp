#include <qtermwidget.h>

#include "properties.h"
#include "config.h"


Properties * Properties::m_instance = 0;


Properties * Properties::Instance()
{
    if (!m_instance)
        m_instance = new Properties();
    return m_instance;
}

Properties::Properties()
{
    qDebug("Properties constructor called");
}

Properties::~Properties()
{
    qDebug("Properties destructor called");
    saveSettings();
    delete m_instance;
    m_instance = 0;
}

void Properties::loadSettings()
{
    //qDebug("Properties::loadSettings");

    QSettings settings;

    guiStyle = settings.value("guiStyle", QString()).toString();
    if (!guiStyle.isNull())
        QApplication::setStyle(guiStyle);

    colorScheme = settings.value("colorScheme", "Linux").toString();

    highlightCurrentTerminal = settings.value("highlightCurrentTerminal", true).toBool();

    QFont default_font = QApplication::font();
    default_font.setFamily("Monospace");
    default_font.setPointSize(10);
    default_font.setStyleHint(QFont::TypeWriter);

    font = qvariant_cast<QFont>(settings.value("font", default_font));

    settings.beginGroup("Shortcuts");
    QStringList keys = settings.childKeys();
    foreach( QString key, keys )
    {
        QKeySequence sequence = QKeySequence( settings.value( key ).toString() );
        if( Properties::Instance()->actions.contains( key ) )
            Properties::Instance()->actions[ key ]->setShortcut( sequence );
    }
    settings.endGroup();

    mainWindowGeometry = settings.value("MainWindow/geometry").toByteArray();
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

    appOpacity = settings.value("MainWindow/appOpacity", 100).toInt();
    termOpacity = settings.value("termOpacity", 100).toInt();

    /* default to Right. see qtermwidget.h */
    scrollBarPos = settings.value("ScrollbarPosition", 2).toInt();
    /* default to South */
    tabsPos = settings.value("TabsPosition", 1).toInt();

    /* toggles */
    borderless = settings.value("Borderless", false).toBool();
    tabBarless = settings.value("TabBarless", false).toBool();
    askOnExit = settings.value("AskOnExit", true).toBool();

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
    qDebug("Properties::saveSettings");
    QSettings settings;

    settings.setValue("guiStyle", guiStyle);
    settings.setValue("colorScheme", colorScheme);
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

    settings.setValue("MainWindow/geometry", mainWindowGeometry);
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

    settings.setValue("MainWindow/appOpacity", appOpacity);
    settings.setValue("termOpacity", termOpacity);
    settings.setValue("ScrollbarPosition", scrollBarPos);
    settings.setValue("TabsPosition", tabsPos);
    settings.setValue("Borderless", borderless);
    settings.setValue("TabBarless", tabBarless);
    settings.setValue("AskOnExit", askOnExit);

    settings.beginGroup("DropMode");
    settings.setValue("ShortCut", dropShortCut.toString());
    settings.setValue("KeepOpen", dropKeepOpen);
    settings.setValue("ShowOnStart", dropShowOnStart);
    settings.setValue("Width", dropWidht);
    settings.setValue("Height", dropHeight);
    settings.endGroup();

}

