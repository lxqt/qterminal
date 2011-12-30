#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QtCore>
#include <QFont>
#include <QAction>

typedef QString Session;

typedef QMap<QString,Session> Sessions;

typedef QMap<QString,QString> ShortcutMap;


class Properties
{
    public:
        static Properties *Instance();

        void saveSettings();

        QByteArray mainWindowGeometry;
        QByteArray mainWindowState;
        //ShortcutMap shortcuts;
        QString shell;
        QFont font;
        QString colorScheme;
        QString guiStyle;

        bool historyLimited;
        unsigned historyLimitedTo;

        QString emulation;

        Sessions sessions;

        int appOpacity;
        int termOpacity;

        int scrollBarPos;
        int tabsPos;

        bool borderless;
        bool tabBarless;

        bool askOnExit;

        QMap< QString, QAction * > actions;

        void loadSettings();

    private:

        // Singleton handling
        static Properties *m_instance;

        Properties();
        Properties(const Properties &) {};
        ~Properties();

};

#endif

