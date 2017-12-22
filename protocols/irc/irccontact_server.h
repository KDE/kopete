/*
    ircservercontact.h - IRC User Contact

    Copyright (c) 2003-2007 by Michel Hermier <michel.hermier@gmail.com>

    Kopete    (c) 2003-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef IRCCONTACT_SERVER_H
#define IRCCONTACT_SERVER_H

#include "irccontact.h"

#include "kircengine.h"

#include "kopetechatsessionmanager.h"

#include <qvaluelist.h>
#include <qlist.h>

class KopeteView;

class IRCContactManager;
class IRCChannelContact;

/**
 * @author Michel Hermier <michel.hermier@gmail.com>
 *
 * This class is the @ref Kopete::Contact object representing IRC Servers.
 * It is derrived from @ref IRCContact where much of its functionality is shared with @ref IRCChannelContact and @ref IRCUserContact.
 */
class IRCServerContact : public IRCContact
{
public:
    // This class provides a Kopete::Contact for each server of a given IRC connection.
    IRCServerContact(IRCContactManager *, const QString &servername, Kopete::MetaContact *mc);

    virtual const QString caption() const;

    virtual void appendMessage(Kopete::Message &);
    void appendMessage(const QString &message);

protected slots:
    void engineInternalError(KIRC::Engine::Error error, KIRC::Message &ircmsg);
    virtual void slotSendMsg(Kopete::Message &message, Kopete::ChatSession *);

private slots:
    virtual void updateStatus();
    void slotViewCreated(KopeteView *);
    void slotDumpMessages();

    void slotIncomingUnknown(const QString &message);
    void slotIncomingConnect(const QString &message);
    void slotIncomingMotd(const QString &motd);
    void slotIncomingNotice(const QString &orig, const QString &notice);
    void slotCannotSendToChannel(const QString &channel, const QString &msg);

private:
    QList<Kopete::Message> mMsgBuffer;
};

#endif // IRCCONTACT_SERVER_H
