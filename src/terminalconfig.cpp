

#include <QHash>
#include <QString>

#include "qterminalapp.h"
#include "terminalconfig.h"
#include "properties.h"
#include "termwidget.h"

TerminalConfig::TerminalConfig(const QString & wdir, const QString & shell)
{
    m_workingDirectory = wdir;
    m_shell = shell;
}

TerminalConfig::TerminalConfig()
{
}

TerminalConfig::TerminalConfig(const TerminalConfig &cfg)
    : m_currentDirectory(cfg.m_currentDirectory),
      m_workingDirectory(cfg.m_workingDirectory),
      m_shell(cfg.m_shell) {}

QString TerminalConfig::getWorkingDirectory()
{
    if (!m_workingDirectory.isEmpty())
        return m_workingDirectory;
    if (Properties::Instance()->useCWD && !m_currentDirectory.isEmpty())
        return m_currentDirectory;
    return QTerminalApp::Instance()->getWorkingDirectory();
}

QString TerminalConfig::getShell()
{
    if (!m_shell.isEmpty())
        return m_shell;
    if (!Properties::Instance()->shell.isEmpty())
        return Properties::Instance()->shell;
    QByteArray envShell = qgetenv("SHELL");
    if (envShell.constData() != NULL)
    {
        QString shellString = QString(envShell);
        if (!shellString.isEmpty())
            return shellString;
    }
    return QString();
}

void TerminalConfig::setWorkingDirectory(const QString &val)
{
    m_workingDirectory = val;
}

void TerminalConfig::setShell(const QString &val)
{
    m_shell = val;
}

void TerminalConfig::provideCurrentDirectory(const QString &val)
{
    m_currentDirectory = val;
}


