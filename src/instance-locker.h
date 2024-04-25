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

#pragma once

#include <QObject>
#include <QString>
#include <QLockFile>
#include <QLocalServer>
#include <QLocalSocket>

class InstanceLocker : public QObject {
Q_OBJECT

public:
InstanceLocker(QString appId, QObject *parent = nullptr);
~InstanceLocker();

/** Check if another isntance is running */
bool isRunning();

/** Send a message to the original instance */
bool sendMessage(const QString& message, int timeout = 500);

/** Disconnect from the server */
void disconnect();

QString handleConnection();

QLockFile *lockFile = nullptr;
bool mLocked        = false;

QString mSocketName;
QString mAppId;

QLocalServer *mServer;
};
