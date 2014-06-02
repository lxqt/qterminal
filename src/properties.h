#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QApplication>
#include <QtCore>
#include <QFont>
#include <QAction>

typedef QString Session;

typedef QMap<QString,Session> Sessions;

typedef QMap<QString,QString> ShortcutMap;


class Properties
{
    public:
        static Properties *Instance(const QString& filename = QString(""));

        QFont defaultFont();
        void saveSettings();

        QByteArray mainWindowGeometry;
        QByteArray mainWindowState;
        //ShortcutMap shortcuts;
        QString shell;
        QFont font;
        QString colorScheme;
        QString guiStyle;
        bool highlightCurrentTerminal;

        bool historyLimited;
        unsigned historyLimitedTo;

        QString emulation;

        Sessions sessions;

        int appOpacity;
        int termOpacity;

        int scrollBarPos;
        int tabsPos;
        bool alwaysShowTabs;
        int m_motionAfterPaste;

        bool borderless;
        bool tabBarless;
        bool menuVisible;

        bool askOnExit;

        bool useCWD;

        bool useBookmarks;
        bool bookmarksVisible;
        QString bookmarksFile;

        QKeySequence dropShortCut;
        bool dropKeepOpen;
        bool dropShowOnStart;
        int dropWidht;
        int dropHeight;

        QMap< QString, QAction * > actions;

        void loadSettings();


    private:

        // Singleton handling
        static Properties *m_instance;
        QString filename;

        explicit Properties(const QString& filename);
        Properties(const Properties &) {};
        ~Properties();

};

#endif

