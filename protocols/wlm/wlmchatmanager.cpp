/*
    wlmchatsession.cpp - MSN Message Manager

    Copyright (c) 2008      by Tiago Salem Herrmann <tiagosh@gmail.com>
    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "wlmchatmanager.h"

#include <QLabel>
#include <QImage>
#include <QToolTip>
#include <QFile>
#include <QIcon>

#include <kconfig.h>
#include <kdebug.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <kmainwindow.h>
#include <ktoolbar.h>
#include <krun.h>

#include "kopetecontactaction.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetechatsessionmanager.h"
#include "kopeteuiglobal.h"
#include "kopeteglobal.h"
#include "kopeteview.h"

#include "wlmcontact.h"
#include "wlmprotocol.h"
#include "wlmaccount.h"


WlmChatManager::WlmChatManager (WlmAccount * account1):
m_account (account1)
{
    QObject::connect (&account ()->server ()->cb,
                      SIGNAL (messageReceived
                              (MSN::SwitchboardServerConnection *,
                               const QString &, const Kopete::Message &)),
                      this,
                      SLOT (receivedMessage
                            (MSN::SwitchboardServerConnection *,
                             const QString &, const Kopete::Message &)));

    QObject::connect (&account ()->server ()->cb,
                      SIGNAL (joinedConversation
                              (MSN::SwitchboardServerConnection *,
                               const QString &, const QString &)), this,
                      SLOT (joinedConversation
                            (MSN::SwitchboardServerConnection *,
                             const QString &, const QString &)));

    QObject::connect (&account ()->server ()->cb,
                      SIGNAL (leftConversation
                              (MSN::SwitchboardServerConnection *,
                               const QString &)), this,
                      SLOT (leftConversation
                            (MSN::SwitchboardServerConnection *,
                             const QString &)));

    QObject::connect (&account ()->server ()->cb,
                      SIGNAL (gotNewSwitchboard
                              (MSN::SwitchboardServerConnection *,
                               const void *)), this,
                      SLOT (gotNewSwitchboard
                            (MSN::SwitchboardServerConnection *,
                             const void *)));

    QObject::connect (&account ()->server ()->cb,
                      SIGNAL (SwitchboardServerConnectionTerminated
                              (MSN::SwitchboardServerConnection *)), this,
                      SLOT (SwitchboardServerConnectionTerminated
                            (MSN::SwitchboardServerConnection *)));

    QObject::connect (&account ()->server ()->cb,
                      SIGNAL (messageSentACK
                              (MSN::SwitchboardServerConnection *,
                               const unsigned int &)), this,
                      SLOT (messageSentACK
                            (MSN::SwitchboardServerConnection *,
                             const unsigned int &)));

    QObject::connect (&account ()->server ()->cb,
                      SIGNAL (receivedNudge
                              (MSN::SwitchboardServerConnection *,
                               const QString &)), this,
                      SLOT (receivedNudge
                            (MSN::SwitchboardServerConnection *,
                             const QString &)));

    QObject::connect (&account ()->server ()->cb,
                      SIGNAL (receivedTypingNotification
                              (MSN::SwitchboardServerConnection *,
                               const QString &)), this,
                      SLOT (receivedTypingNotification
                            (MSN::SwitchboardServerConnection *,
                             const QString &)));
}

WlmChatManager::~WlmChatManager ()
{
    QMap < MSN::SwitchboardServerConnection *,
        WlmChatSession * >::Iterator it;
    for (it = chatSessions.begin (); it != chatSessions.end (); ++it)
    {
        if (it.value ())
            it.value ()->setChatService (NULL);
    }
    chatSessions.clear ();
}

void
WlmChatManager::receivedNudge (MSN::SwitchboardServerConnection * conn,
                               const QString & passport)
{
    createChat (conn);

    if(conn)
        if(chatSessions[conn])
            chatSessions[conn]->receivedNudge (passport);

}

void
WlmChatManager::leftConversation (MSN::SwitchboardServerConnection * conn,
                                  const QString & passport)
{
    kDebug (14210) << k_funcinfo << " " << conn;
    WlmChatSession *chat = chatSessions[conn];

    if (chat)
    {
        Kopete::Contact * contact = account ()->contacts ()[passport];
        if (!contact)
            return;
        chat->removeContact (contact);
    }
}

void
WlmChatManager::removeChatSession (QObject * obj)
{
    QMap <MSN::SwitchboardServerConnection *, WlmChatSession * >::Iterator it;
    for (it = chatSessions.begin (); it != chatSessions.end (); ++it)
    {
        if (it.value () == obj)
        {
            it.value ()->deleteLater();
            chatSessions.remove (it.key ());
            return;
        }
    }
}

void
WlmChatManager::createChat (MSN::SwitchboardServerConnection * conn)
{
    Kopete::ContactPtrList chatmembers;
    kDebug (14210) << k_funcinfo << " " << conn;

    if (chatSessions[conn])
        return;

    std::list < MSN::Passport >::iterator users = conn->users.begin ();
    for (; users != conn->users.end (); users++)
    {
        Kopete::Contact * contact =
            account ()->contacts ()[(*users).c_str ()];
        if (!contact)
        {
            account ()->addContact ((*users).c_str (), QString::null, 0L,
                                    Kopete::Account::Temporary);
            contact = account ()->contacts ()[(*users).c_str ()];
            contact->setOnlineStatus (WlmProtocol::protocol ()->wlmUnknown);
        }
        chatmembers.append (contact);
    }

    Kopete::ChatSession * _manager =
        Kopete::ChatSessionManager::self ()->findChatSession (account ()->myself (),
                                                              chatmembers,
                                                              account ()->protocol ());
    if (_manager)
        chatSessions[conn] = dynamic_cast < WlmChatSession * >(_manager);
    else
        // create a new chat window
        chatSessions[conn] = new WlmChatSession (account ()->protocol (),
                                account ()->myself (), chatmembers, conn);
    if(chatSessions[conn])
        chatSessions[conn]->setChatService (conn);
}

void
WlmChatManager::joinedConversation (MSN::SwitchboardServerConnection * conn,
                                    const QString & passport,
                                    const QString & friendlyname)
{
    Kopete::ContactPtrList chatmembers;
    Kopete::Contact * contact = account ()->contacts ()[passport];
    if (!contact)
    {
        account ()->addContact (passport, QString::null, 0L,
                                Kopete::Account::Temporary);
        contact = account ()->contacts ()[passport];
        contact->setOnlineStatus (WlmProtocol::protocol ()->wlmUnknown);
    }
    // populate chatmembers from SwitchboardServerConnection
    std::list < MSN::Passport >::iterator users = conn->users.begin ();
    for (; users != conn->users.end (); users++)
    {
        Kopete::Contact * contact =
            account ()->contacts ()[(*users).c_str ()];
        if (!contact)
        {
            account ()->addContact ((*users).c_str (), QString::null, 0L,
                                    Kopete::Account::Temporary);
            contact = account ()->contacts ()[(*users).c_str ()];
            contact->setOnlineStatus (WlmProtocol::protocol ()->wlmUnknown);
        }
        chatmembers.append (contact);
    }

    // check if we already have a similar connection
    WlmChatSession *_manager =
        dynamic_cast <WlmChatSession *>(Kopete::ChatSessionManager::self ()->
          findChatSession (account ()->myself (), chatmembers,
                           account ()->protocol ()));

    if (_manager)
    {
        // drop the old and replace by the new one
        MSN::SwitchboardServerConnection * conn_current =
            chatSessions.key (_manager);
        if (conn_current && conn_current != conn)
        {
            chatSessions.remove (conn_current);
            chatSessions[conn] = _manager;
            chatSessions[conn]->setChatService(conn);
            conn_current->disconnect();
            delete conn_current;
            return;
        }
    }

    createChat (conn);
    chatSessions[conn]->addContact (contact);
    // the session is ready when there are at least 2 contacts, me and somebody
    if (!chatSessions[conn]->isReady ())
        chatSessions[conn]->setReady (true);
}

void
WlmChatManager::receivedMessage (MSN::SwitchboardServerConnection * conn,
                                 const QString & from,
                                 const Kopete::Message & message)
{
    kDebug (14210) << k_funcinfo << " " << conn;

    createChat (conn);

    WlmChatSession *chat = chatSessions[conn];

    // Pass it on to the contact to process and display via a KMM
    if (chat)
    {
        Kopete::Contact * contact = account ()->contacts ()[from];
        Kopete::Message * newMessage =
            new Kopete::Message (contact, chat->members ());
        newMessage->setFont (message.font ());
        newMessage->setPlainBody (message.plainBody ());
        newMessage->setForegroundColor (message.foregroundColor ());
        newMessage->setDirection (Kopete::Message::Inbound);
        // Add it to the manager
        chat->appendMessage (*newMessage);

        delete newMessage;
        // send keepalive each 50 seconds.
        chat->startSendKeepAlive();
    }
    else
    {
        kWarning (14210) << k_funcinfo <<
            "unable to look up contact for delivery";
    }
}

void
WlmChatManager::SwitchboardServerConnectionTerminated (
        MSN::SwitchboardServerConnection * conn)
{
    if(!conn)
        return;

    WlmChatSession *chat = chatSessions[conn];
    if (chat)
    {
        chat->setChatService (NULL);
        chatSessions.remove (conn);
/*      if(chat->view())
        {
            if(!chat->view()->isVisible())
                delete chat;
        }
        else
            delete chat;
*/
    }
}

void
WlmChatManager::gotNewSwitchboard (MSN::SwitchboardServerConnection * conn,
                                   const void *tag)
{
    Kopete::ContactPtrList chatmembers;
    const std::pair < std::string, std::string > *ctx = 
	    static_cast < const std::pair < std::string, std::string > *>(tag);

    if (!ctx)
        return;

    conn->inviteUser (ctx->first);
    delete ctx;
    conn->auth.tag = NULL;
    kDebug (14210) << k_funcinfo << " " << conn;
}

void
WlmChatManager::messageSentACK (MSN::SwitchboardServerConnection * conn,
                                const unsigned int &trID)
{
    WlmChatSession *chat = chatSessions[conn];
    if (chat)
    {
        chat->messageSentACK (trID);
    }
}

void
WlmChatManager::requestDisplayPicture (QString contactId)
{
    Kopete::Contact * contact = account ()->contacts ()[contactId];

    if (!contact)
        return;

    WlmChatSession *session =
        dynamic_cast <WlmChatSession *>(contact->manager (Kopete::Contact::CanCreate));

    if (session)
        session->requestDisplayPicture ();

}

void
WlmChatManager::receivedTypingNotification (MSN::SwitchboardServerConnection *
                                            conn, const QString & contactId)
{
    Kopete::Contact * contact = account ()->contacts ()[contactId];

    if (!contact)
        return;

    WlmChatSession *chat = chatSessions[conn];
    if (chat)
    {
        chat->receivedTypingMsg (contact, true);
    }
}

#include "wlmchatmanager.moc"
