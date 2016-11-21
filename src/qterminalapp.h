#ifndef QTERMINALAPP_H
#define QTERMINALAPP_H

#include <QApplication>

#include "mainwindow.h"


class QTerminalApp : public QApplication
{
Q_OBJECT

public:
    MainWindow *newWindow(bool dropMode, const QString& workdir, const QString& shell_command);
    QList<MainWindow*> getWindowList();
    void addWindow(MainWindow *window);
    void removeWindow(MainWindow *window);
	static QTerminalApp *Instance(int &argc, char **argv);
	static QTerminalApp *Instance();

private:
	QList<MainWindow *> m_windowList;
	static QTerminalApp *m_instance;
	QTerminalApp(int &argc, char **argv);
	~QTerminalApp(){};
};

template <class T> T* findParent(QObject *child)
{
    QObject *maybeT = child;
    while (true)
    {
        if (maybeT == NULL)
        {
            return NULL;
        }
        T *holder = qobject_cast<T*>(maybeT);
        if (holder)
            return holder;
        maybeT = maybeT->parent();
    }
}

#endif