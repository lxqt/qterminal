
#ifndef TERMINALCONFIG_H
#define TERMINALCONFIG_H

#include <QHash>
#include <QString>
#include <QVariant>

class TermWidget;

class TerminalConfig
{
    public:
        TerminalConfig(const QString & wdir, const QStringList & shell);
        TerminalConfig(const TerminalConfig &cfg);
        TerminalConfig();

        QString getWorkingDirectory();
        QStringList getShell();
        bool hasCommand() const;

        void setWorkingDirectory(const QString &val);
        void setShell(const QStringList &val);
        void provideCurrentDirectory(const QString &val);

        #ifdef HAVE_QDBUS
        static TerminalConfig fromDbus(const QHash<QString,QVariant> &termArgs);
        static TerminalConfig fromDbus(const QHash<QString,QVariant> &termArgs, TermWidget *toSplit);
        #endif


    private:
    	// True when
    	QString m_currentDirectory;
    	QString m_workingDirectory;
        QStringList m_shell;
};

#endif
