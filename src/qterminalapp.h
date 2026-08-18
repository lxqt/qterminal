#ifndef QTERMINALAPP_H
#define QTERMINALAPP_H

#include <QApplication>
#ifdef HAVE_QDBUS
    #include <QtDBus/QtDBus>
#endif


#include "mainwindow.h"


class QTerminalApp : public QApplication
{
Q_OBJECT

public:
    MainWindow *newWindow(bool dropMode, TerminalConfig &cfg, const QString &dbus_id = QString());
    QList<MainWindow*> getWindowList();
    void addWindow(MainWindow *window);
    void removeWindow(MainWindow *window);
    static QTerminalApp *Instance(int &argc, char **argv);
    static QTerminalApp *Instance();
    QString &getWorkingDirectory();
    void setWorkingDirectory(const QString &wd);

    #ifdef HAVE_QDBUS
    void registerOnDbus(bool dropDown, const QString dbus_id);
    QString getDbusService() const { return m_dbusService; }
    QList<QDBusObjectPath> getWindows();
    QDBusObjectPath newWindow(const QString &dbus_id, const QString &shell_command, const QString &workdir, int columns, int lines);
    QDBusObjectPath newWindow(const QHash<QString,QVariant> &termArgs);
    QDBusObjectPath getActiveWindow();
    bool isDropMode();
    bool toggleDropdown();
    void requestDropDown();
    bool isPrimaryInstance();
    #endif

    static void cleanup();

private:
    QString m_workDir;
    QList<MainWindow *> m_windowList;
    static QTerminalApp *m_instance;
    bool m_isPrimaryInstance = true;
#ifdef HAVE_QDBUS
    QString m_dbusService;
#endif
    QTerminalApp(int &argc, char **argv);
    ~QTerminalApp() override{};
};

template <class T> T* findParent(QObject *child)
{
    QObject *maybeT = child;
    while (true)
    {
        if (maybeT == nullptr)
        {
            return nullptr;
        }
        T *holder = qobject_cast<T*>(maybeT);
        if (holder)
            return holder;
        maybeT = maybeT->parent();
    }
}

#endif
