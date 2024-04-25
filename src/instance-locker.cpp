/***************************************************************************
*   Copyright (C) 2024 by Marcus Britanicus                               *
*   marcusbritanicus@gmail.com                                            *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
***************************************************************************/

#include <unistd.h>

#include "instance-locker.h"

static inline QString getSocketName(QString appId)
{
    /** Get the env-var XDG_RUNTIME_DIR */
    QString sockName = QString::fromUtf8(qgetenv("XDG_RUNTIME_DIR"));

    /** The env-var was not set. We will use /tmp/ */
    if (not sockName.length())
    {
        sockName = QString::fromUtf8("/tmp/");
    }

    if (not sockName.endsWith(QString::fromUtf8("/")))
    {
        sockName += QString::fromUtf8("/");
    }

    /** Append a random number */
    sockName += QString::fromUtf8("%1-Scoket-%2").arg(appId).arg(getuid());

    return sockName;
}


InstanceLocker::InstanceLocker(QString appId, QObject *parent) : QObject(parent)
{
    mServer = nullptr;

    /* App ID */
    mAppId = appId;

    /** Get the env-var XDG_RUNTIME_DIR */
    mSocketName = getSocketName(mAppId);

    /* Lock File */
    lockFile = new QLockFile(mSocketName + QString::fromUtf8(".lock"));

    /* Try to lock the @lockFile, if it fails, then we're not the first instance */
    if (lockFile->tryLock())
    {
        /* Local Server for communication */
        mServer = new QLocalServer(this);

        /* Start the server */
        bool res = mServer->listen(mSocketName);

        /* @res can't be false at the moment, because we're the first instance. */
        /* The only reason why @res is false, the socket file exists from a previous */
        /* crash. So delete it and try again. */
        if (not res && (mServer->serverError() == QAbstractSocket::AddressInUseError))
        {
            QLocalServer::removeServer(mSocketName);
            res = mServer->listen(mSocketName);

            if (!res)
            {
                qWarning("InstanceLocker: listen on local socket failed, %s", qPrintable(mServer->errorString()));
            }
        }
    }
}


InstanceLocker::~InstanceLocker()
{
    disconnect();

    delete lockFile;
}


bool InstanceLocker::isRunning()
{
    /* If we have the lock, we're the server */
    /* In other words, if we're not there, there is no server */
    if (lockFile->isLocked())
    {
        return false;
    }

    /* If we cannot get the lock then the server is running elsewhere */
    if (not lockFile->tryLock())
    {
        return true;
    }

    /* Be default, we'll assume that the server is running elsewhere */
    return true;
}


bool InstanceLocker::sendMessage(const QString& message, int timeout)
{
    if (not isRunning())
    {
        return false;
    }

    /* Preparing socket */
    QLocalSocket socket(this);

    /* Connecting to server */
    socket.connectToServer(mSocketName);

    /* Wait for ACK (500 ms) */
    if (not socket.waitForConnected(timeout))
    {
        return false;
    }

    /* Send the message to the server */
    socket.write(message.toUtf8());

    /** Should finish writing in 500 ms */
    return socket.waitForBytesWritten(timeout);
}


void InstanceLocker::disconnect()
{
    if (mServer)
    {
        mServer->close();
    }

    lockFile->unlock();
}


QString InstanceLocker::handleConnection()
{
    /* Preparing socket */
    QLocalSocket *socket = mServer->nextPendingConnection();

    if (socket == nullptr)
    {
        return QString();
    }

    socket->waitForReadyRead(2000);
    QByteArray msg = socket->readAll();

    /** Close the connection */
    socket->close();

    return QString::fromUtf8(msg);
}
