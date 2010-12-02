#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QtCore>
#include <QFont>



typedef QString Session;

typedef QMap<QString,Session> Sessions;

typedef QMap<QString,QString> ShortcutMap;


class Properties
{
    public:
        static Properties * Instance();

        void saveSettings();

        QByteArray mainWindowGeometry;
        QByteArray mainWindowState;
        ShortcutMap shortcuts;
        QString shell;
        QFont font;
        int colorScheme;
        QString guiStyle;

        QString emulation;

        Sessions sessions;

        int appOpacity;
        int termOpacity;

    private:

        // Singleton handling
        static Properties * m_instance;

        void loadSettings();

        Properties();
        Properties(const Properties &) {};
        ~Properties();

};

#endif

