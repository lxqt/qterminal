#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QtCore>
#include <QFont>


typedef QMap<QString, QString> ShortcutMap;


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
   
    private:

        // Singleton handling
        static Properties * m_instance;

        void loadSettings();

        Properties();
        Properties(const Properties &) {};
        ~Properties(); 

};

#endif

