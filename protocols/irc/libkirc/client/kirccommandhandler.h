/*
    kirccommandhandler.h - IRC Client

    Copyright (c) 2004-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2004-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIRCCOMMANDHANDLER_H
#define KIRCCOMMANDHANDLER_H

#include "kircmessage.h"

#include <QtCore/QMultiHash>
#include <QtCore/QObject>

namespace KIrc {
class Command;

class CommandHandler : public QObject
{
    Q_OBJECT

public:
    CommandHandler(QObject *parent = 0);
    ~CommandHandler();

public slots:
    Command *registerCommand(const char *name, Command *command);

    /**
     * Connects the given object member signal/slot to this message redirector.
     * The member signal slot should be looking like:
     * SIGNAL(mysignal(KIrc::Message msg))
     * or
     * SIGNAL(myslot(KIrc::Message msg))
     */
    Command *registerCommand(const char *name, QObject *object);

    virtual void handleMessage(KIrc::Message msg);

    void unregisterCommand(const char *name);

protected:
    QMultiHash<QString, Command *> m_commands;

private:
    Q_DISABLE_COPY(CommandHandler)
};
}

#endif
