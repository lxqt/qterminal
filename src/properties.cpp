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
    loadSettings();
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
        qDebug("Properties::loadSettings");
        
    QSettings settings(QDir::homePath()+"/.qterminal", QSettings::IniFormat);

    guiStyle = settings.value("guiStyle", QString()).toString();
    if (!guiStyle.isNull())
        QApplication::setStyle(guiStyle);
        
    colorScheme = settings.value("color_scheme", QVariant(COLOR_SCHEME_GREEN_ON_BLACK)).toInt();

    QFont default_font = QApplication::font();
    default_font.setFamily("Monospace");
    default_font.setPointSize(10);
    default_font.setStyleHint(QFont::TypeWriter);

    font = qvariant_cast<QFont>(settings.value("font", default_font));
    
    settings.beginGroup("Shortcuts");
    shortcuts[COPY_SELECTION] = settings.value(COPY_SELECTION, COPY_SELECTION_SHORTCUT).toString();
    shortcuts[PASTE_SELECTION] = settings.value(PASTE_SELECTION, PASTE_SELECTION_SHORTCUT).toString();
    shortcuts[RENAME_SESSION] = settings.value(RENAME_SESSION, RENAME_SESSION_SHORTCUT).toString();
    shortcuts[ADD_TAB] = settings.value(ADD_TAB, ADD_TAB_SHORTCUT).toString();
    shortcuts[CLOSE_TAB] = settings.value(CLOSE_TAB, CLOSE_TAB_SHORTCUT).toString();
    shortcuts[TAB_RIGHT] = settings.value(TAB_RIGHT, TAB_RIGHT_SHORTCUT).toString();
    shortcuts[TAB_LEFT] = settings.value(TAB_LEFT, TAB_LEFT_SHORTCUT).toString();
    shortcuts[MOVE_LEFT] = settings.value(MOVE_LEFT, MOVE_LEFT_SHORTCUT).toString();
    shortcuts[MOVE_RIGHT] = settings.value(MOVE_RIGHT, MOVE_RIGHT_SHORTCUT).toString();
    settings.endGroup();

    mainWindowGeometry = settings.value("MainWindow/geometry").toByteArray();
    mainWindowState = settings.value("MainWindow/state").toByteArray();

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
}

void Properties::saveSettings()
{
        qDebug("Properties::saveSettings");
    QSettings settings(QDir::homePath()+"/.qterminal", QSettings::IniFormat);

    settings.setValue("guiStyle", guiStyle);
    settings.setValue("color_scheme", colorScheme);
    settings.setValue("font", font);
    
    settings.beginGroup("Shortcuts");
    QMapIterator<QString, QString> it(shortcuts);
    while (it.hasNext()) {
	    it.next();
	    settings.setValue(it.key(), it.value());
    }
    settings.endGroup();

    settings.setValue("MainWindow/geometry", mainWindowGeometry);
    settings.setValue("MainWindow/state", mainWindowState);

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
}

