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
#include <QImageReader>
#include <QToolTip>
#include <QFile>
#include <QIcon>
#include <QRegExp>
#include <QDomDocument>
#include <QTextDocument>
#include <QTimerEvent>

#include <kconfig.h>
#include <kdebug.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <kmainwindow.h>
#include <ktoolbar.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>

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

const int EmoticonsTimeoutCheckInterval = 2; //seconds
const int EmoticonsTimeoutThreshold = 5; //seconds

WlmChatManager::WlmChatManager (WlmAccount * account1):
m_account (account1), m_emoticonsTimeoutTimerId(0)
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

    QObject::connect (&account ()->server ()->cb,
                      SIGNAL (slotGotEmoticonNotification (MSN::SwitchboardServerConnection *,
                               const QString &, const QString &,
                               const QString &)), this,
                      SLOT (slotGotEmoticonNotification (MSN::SwitchboardServerConnection *,
                               const QString &, const QString &,
                               const QString &)));

    QObject::connect (&account ()->server ()->cb,
                      SIGNAL (slotGotVoiceClipNotification (MSN::SwitchboardServerConnection *,
                                const QString &,
                                const QString &)), this,
                      SLOT(slotGotVoiceClipNotification (MSN::SwitchboardServerConnection *,
                                 const QString &,
                                 const QString &)));

    QObject::connect (&account ()->server ()->cb,
                      SIGNAL(slotGotWinkNotification (MSN::SwitchboardServerConnection *,
                                const QString &,
                                const QString &)), this,
                      SLOT(slotGotWinkNotification (MSN::SwitchboardServerConnection *,
                                const QString &,
                                const QString &)));

    QObject::connect (&account ()->server ()->cb,
                      SIGNAL(slotGotInk (MSN::SwitchboardServerConnection *,
                                    const QString &,
                                    const QByteArray &)), this,
                      SLOT(slotGotInk (MSN::SwitchboardServerConnection *,
                                    const QString &,
                                    const QByteArray &)));

    QObject::connect (&account ()->server ()->cb,
                      SIGNAL(slotGotVoiceClipFile(MSN::SwitchboardServerConnection *,
                                const unsigned int &,
                                const QString &)), this,
                      SLOT(slotGotVoiceClipFile(MSN::SwitchboardServerConnection *,
                                const unsigned int &,
                                const QString &)));

    QObject::connect (&account ()->server ()->cb,
                      SIGNAL(slotGotEmoticonFile(MSN::SwitchboardServerConnection *,
                                const unsigned int &,
                                const QString &,
                                const QString &)), this,
                      SLOT(slotGotEmoticonFile(MSN::SwitchboardServerConnection *,
                                const unsigned int &,
                                const QString &,
                                const QString &)));

    QObject::connect (&account ()->server ()->cb,
                      SIGNAL(slotGotWinkFile(MSN::SwitchboardServerConnection *,
                                    const unsigned int &,
                                    const QString &)), this,
                      SLOT(slotGotWinkFile(MSN::SwitchboardServerConnection *,
                                    const unsigned int &,
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
	    WlmContact * contact = qobject_cast<WlmContact*>(account ()->contacts().value(passport));
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
    for (; users != conn->users.end (); ++users)
    {
        QString userPassport = WlmUtils::passport(*users);
        Kopete::Contact * contact = account ()->contacts().value(userPassport);
        if (!contact)
        {
            account ()->addContact (userPassport, QString(), 0L, Kopete::Account::Temporary);
            contact = account ()->contacts().value(userPassport);
            contact->setOnlineStatus (WlmProtocol::protocol ()->wlmUnknown);
        }
        chatmembers.append (contact);
    }

    Kopete::ChatSession * _manager =
        Kopete::ChatSessionManager::self ()->findChatSession (account ()->myself (),
                                                              chatmembers,
                                                              account ()->protocol ());
    if (_manager)
        chatSessions[conn] = qobject_cast < WlmChatSession * >(_manager);
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
    Q_UNUSED( friendlyname );

    Kopete::ContactPtrList chatmembers;
	Kopete::Contact * contact = account ()->contacts().value(passport);
    if (!contact)
    {
        account ()->addContact (passport, QString(), 0L, Kopete::Account::Temporary);
	    contact = account ()->contacts().value(passport);
        contact->setOnlineStatus (WlmProtocol::protocol ()->wlmUnknown);
    }
    // populate chatmembers from SwitchboardServerConnection
    std::list < MSN::Passport >::iterator users = conn->users.begin ();
    for (; users != conn->users.end (); ++users)
    {
        QString userPassport = WlmUtils::passport(*users);
        Kopete::Contact * contact = account ()->contacts().value(userPassport);
        if (!contact)
        {
            account ()->addContact (userPassport, QString(), 0L, Kopete::Account::Temporary);
            contact = account ()->contacts().value(userPassport);
            contact->setOnlineStatus (WlmProtocol::protocol ()->wlmUnknown);
        }
        chatmembers.append (contact);
    }

    // check if we already have a similar connection
    WlmChatSession *_manager =
        qobject_cast <WlmChatSession *>(Kopete::ChatSessionManager::self ()->
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
	    Kopete::Contact * contact = account ()->contacts().value(from);
        if(!contact)
        {
            account ()->addContact (from, QString(), 0L,
                                            Kopete::Account::Temporary);
	        contact = account ()->contacts().value(from);
        }
        Kopete::Message * newMessage =
            new Kopete::Message (contact, chat->members ());
        newMessage->setPlainBody (message.plainBody ());
        newMessage->setFont (message.font ());
        newMessage->setForegroundColor (message.foregroundColor ());
        newMessage->setDirection (Kopete::Message::Inbound);

        WlmContact *contact_from = qobject_cast<WlmContact*>(contact);
        if(!contact_from)
            return;

        if(!contact_from->dontShowEmoticons())
        {
            if (!fillEmoticons(chat, newMessage))
            {
                pendingMessages[conn].append(PendingMessage(newMessage));
                if (m_emoticonsTimeoutTimerId == 0)
                    m_emoticonsTimeoutTimerId = startTimer(EmoticonsTimeoutCheckInterval * 1000);
                return;
            }
        }
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
	Kopete::Contact * contact = account ()->contacts().value(contactId);

    if (!contact)
        return;

    WlmChatSession *session =
        qobject_cast <WlmChatSession *>(contact->manager (Kopete::Contact::CanCreate));

    if (session)
        session->requestDisplayPicture ();

}

void
WlmChatManager::receivedTypingNotification (MSN::SwitchboardServerConnection *
                                            conn, const QString & contactId)
{
	Kopete::Contact * contact = account ()->contacts().value(contactId);

    if (!contact)
        return;

    WlmChatSession *chat = chatSessions[conn];
    if (chat)
    {
        chat->receivedTypingMsg (contact, true);
    }
}

void
WlmChatManager::slotGotVoiceClipNotification (MSN::SwitchboardServerConnection * conn, 
                                              const QString & from,
                                              const QString & msnobject)
{
    Q_UNUSED( from );
    WlmChatSession *chat = chatSessions[conn];
    if (!chat)
        return;
    unsigned int sessionID = chat->generateSessionID();

    KTemporaryFile voiceClip;
    voiceClip.setPrefix("kopete_voiceclip-");
    voiceClip.setSuffix(".wav");
    voiceClip.setAutoRemove(false);
    voiceClip.open();
    chat->addFileToRemove(voiceClip.fileName());
    conn->requestVoiceClip(sessionID, QFile::encodeName(voiceClip.fileName()).constData(),
                           msnobject.toUtf8().constData());
}

void
WlmChatManager::slotGotWinkNotification (MSN::SwitchboardServerConnection * conn, 
                                         const QString & from,
                                         const QString & msnobject)
{
    Q_UNUSED( conn );
    Q_UNUSED( from );
    Q_UNUSED( msnobject );
}

void
WlmChatManager::slotGotInk (MSN::SwitchboardServerConnection * conn, 
                            const QString & from,
                            const QByteArray & image)
{
    QByteArray ink;
    WlmChatSession *chat = chatSessions[conn];
    if (!chat)
    {
        return;
    }
    Kopete::Contact * contact = account ()->contacts().value(from);
    if(!contact)
    {
        account ()->addContact (from, QString(), 0L, Kopete::Account::Temporary);
        contact = account ()->contacts().value(from);
    }
    if(!contact)
        return;

    ink = QByteArray::fromBase64(image);
    KTemporaryFile *inkImage = new KTemporaryFile();
    inkImage->setPrefix("inkformatgif-");
    inkImage->setSuffix(".gif");
    inkImage->open();
    inkImage->write(ink.constData(), ink.size());
    QString msg=QString ("<img src=\"%1\" />").arg ( inkImage->fileName() );
    inkImage->close();

    Kopete::Message kmsg( contact, chat->members() );
    kmsg.setHtmlBody( msg );
    kmsg.setDirection( Kopete::Message::Inbound );
    chat->appendMessage ( kmsg );
    chat->addFileToRemove(inkImage->fileName());
    //TODO mem leak ? inkImage ?
    inkImage = 0l;

}

void 
WlmChatManager::slotGotVoiceClipFile(MSN::SwitchboardServerConnection * conn, 
                                const unsigned int & sessionID, 
                                const QString & file)
{
    Q_UNUSED( sessionID );

    WlmChatSession *chat = chatSessions[conn];
    if(!chat)
        return;

    Kopete::Message kmsg( chat->members().first(), chat->members() );
    kmsg.setType(Kopete::Message::TypeVoiceClipRequest);
    kmsg.setDirection( Kopete::Message::Inbound );
    kmsg.setFileName(file);
    chat->appendMessage ( kmsg );
}

void WlmChatManager::slotGotEmoticonNotification (MSN::SwitchboardServerConnection * conn,
                                const QString & buddy,
                                const QString & alias,
                                const QString & msnobject)
{
    WlmContact *contact = qobject_cast<WlmContact*>(m_account->contacts().value(buddy));

    if(!contact)
        return;

    if(m_account->doNotRequestEmoticons() || contact->dontShowEmoticons())
        return;

    WlmChatSession *chat = chatSessions[conn];
    if(!chat)
        return;

    unsigned int sessionID = chat->generateSessionID();

    QDomDocument xmlobj;
    xmlobj.setContent (msnobject);

    // track display pictures by SHA1D field
    QString SHA1D = xmlobj.documentElement ().attribute ("SHA1D");
    if (SHA1D.isEmpty ())
        return;

    QString newlocation =
        KGlobal::dirs ()->locateLocal ("appdata",
                                       "wlmpictures/" +
                                       QString (SHA1D.replace ('/', '_')));
    QFile f(newlocation);
    if (f.exists () && f.size ())
    {
        chat->emoticonsList[alias] = newlocation;
        return;
    }

    // pending emoticon
    chat->emoticonsList[alias].clear();

    conn->requestEmoticon(sessionID, QFile::encodeName(newlocation).constData(),
                          msnobject.toUtf8().constData(), alias.toUtf8().constData());
}

void 
WlmChatManager::slotGotEmoticonFile(MSN::SwitchboardServerConnection * conn, 
                                const unsigned int & sessionID,
                                const QString & alias,
                                const QString & file)
{
    Q_UNUSED( sessionID );

    WlmChatSession *chat = chatSessions[conn];
    if(!chat)
        return;

    chat->emoticonsList[alias] = file;

    if(pendingMessages.value(conn).isEmpty())
        return;

    {
        QMutableLinkedListIterator<PendingMessage> it(pendingMessages[conn]);
        while (it.hasNext())
        {
            PendingMessage pendingMsg = it.next();
            if (fillEmoticons(chat, pendingMsg.message))
            {
                chat->appendMessage(*pendingMsg.message);
                it.remove();
                delete pendingMsg.message;
            }
        }
    }

    if (pendingMessages.value(conn).isEmpty())
        pendingMessages.remove(conn);
}

void 
WlmChatManager::slotGotWinkFile(MSN::SwitchboardServerConnection * conn, 
                                    const unsigned int & sessionID, 
                                    const QString & file)
{
    Q_UNUSED( conn );
    Q_UNUSED( sessionID );
    Q_UNUSED( file );
}

void WlmChatManager::timerEvent(QTimerEvent *event)
{
    if (m_emoticonsTimeoutTimerId == event->timerId() )
    {
        QTime thresholdTime = QTime::currentTime().addSecs(-EmoticonsTimeoutThreshold);
        QMutableMapIterator<MSN::SwitchboardServerConnection*, QLinkedList<PendingMessage> > connIt(pendingMessages);
        while (connIt.hasNext())
        {
            connIt.next();

            {
                QMutableLinkedListIterator<PendingMessage> it(connIt.value());
                while (it.hasNext())
                {
                    PendingMessage pendingMsg = it.next();
                    if (pendingMsg.receiveTime < thresholdTime)
                    {
                        kDebug(14210) << "Did not get emoticons in time!";
                        WlmChatSession *chat = chatSessions[connIt.key()];
                        if (chat)
                            chat->appendMessage(*pendingMsg.message);

                        it.remove();
                        delete pendingMsg.message;
                    }
                }
            }
            if (connIt.value().isEmpty())
                connIt.remove();
        }

        if (pendingMessages.isEmpty())
        {
            killTimer(m_emoticonsTimeoutTimerId);
            m_emoticonsTimeoutTimerId = 0;
        }
    }
}

bool WlmChatManager::fillEmoticons(WlmChatSession *chat, Kopete::Message* message)
{
    QString escapedMessage = message->escapedBody();

    // for each emoticon in our list
    QMap<QString,QString>::iterator it2 = chat->emoticonsList.begin();
    for (;it2!=chat->emoticonsList.end(); ++it2)
    {
        QString es = Qt::escape(it2.key());
        if (escapedMessage.contains(es))
        {
            if (!QFile::exists(it2.value()))
            {   // an emoticon is still missing, so wait for it
                message->setHtmlBody(escapedMessage);
                return false;
            }

            QImage iconImage = QImageReader(it2.value()).read();

            escapedMessage.replace( QRegExp(QString::fromLatin1("%1(?![^><]*>)").arg(QRegExp::escape(es))),
                                    QString::fromLatin1("<img align=\"center\" width=\"") +
                                    QString::number(iconImage.width()) +
                                    QString::fromLatin1("\" height=\"") +
                                    QString::number(iconImage.height()) +
                                    QString::fromLatin1("\" src=\"") + it2.value() +
                                    QString::fromLatin1("\" title=\"") + es +
                                    QString::fromLatin1("\" alt=\"") + es +
                                    QString::fromLatin1( "\"/>" ) );
        }
    }
    message->setHtmlBody(escapedMessage);
    return true;
}
#include "wlmchatmanager.moc"
