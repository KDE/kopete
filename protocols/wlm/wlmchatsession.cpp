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

#include "wlmchatsession.h"

#include <QLabel>
#include <QImage>
#include <QToolTip>
#include <QFile>
#include <QIcon>
#include <QTextCodec>
#include <QRegExp>
#include <QDomDocument>
#include <QFileInfo>
#include <QBuffer>
#include <QPainter>

#include <kconfig.h>
#include <kdebug.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <kaction.h>
#include <kactionmenu.h>
#include <kactioncollection.h>
#include <kmainwindow.h>
#include <ktoolbar.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>
#include <kemoticons.h>
#include <kcodecs.h>

#include "kopetecontactaction.h"
#include "kopeteonlinestatus.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetechatsessionmanager.h"
#include "kopeteuiglobal.h"
#include "kopeteglobal.h"
#include "kopeteview.h"
#include "kopeteutils.h"
#include "private/kopeteemoticons.h"

#include "wlmcontact.h"
#include "wlmprotocol.h"
#include "wlmaccount.h"


WlmChatSession::WlmChatSession (Kopete::Protocol * protocol,
                                const Kopete::Contact * user,
                                Kopete::ContactPtrList others,
                                MSN::SwitchboardServerConnection * conn):
Kopete::ChatSession (user, others, protocol),
m_chatService (conn),
m_downloadDisplayPicture (false),
m_sendNudge (false),
m_tries (0),
m_oimid (1),
m_sessionID(1)
{
    Kopete::ChatSessionManager::self ()->registerChatSession (this);

    setComponentData (protocol->componentData ());

    connect (this, SIGNAL (messageSent (Kopete::Message &,
                                        Kopete::ChatSession *)),
             this, SLOT (slotMessageSent (Kopete::Message &,
                                          Kopete::ChatSession *)));

    connect (this, SIGNAL (myselfTyping (bool)),
             this, SLOT (sendTypingMsg (bool)));

    m_keepalivetimer = new QTimer (this);
    connect (m_keepalivetimer, SIGNAL (timeout ()), SLOT (sendKeepAlive ()));

    if (getChatService ()
        && getChatService ()->connectionState () ==
        MSN::SwitchboardServerConnection::SB_READY)
    {
        setReady (true);
    }

    m_actionNudge = new KAction (KIcon ("preferences-desktop-notification-bell"), i18n ("Send Nudge"), this);
    actionCollection ()->addAction ("wlmSendNudge", m_actionNudge);
    connect (m_actionNudge, SIGNAL (triggered (bool)), this,
             SLOT (sendNudge ()));


    m_actionInvite =
        new KActionMenu (KIcon ("system-users"), i18n ("&Invite"), this);
    actionCollection ()->addAction ("wlmInvite", m_actionInvite);
    connect (m_actionInvite->menu (), SIGNAL (aboutToShow ()), this,
             SLOT (slotActionInviteAboutToShow ()));

    setXMLFile ("wlmchatui.rc");
    setMayInvite (true);
}

void
WlmChatSession::sendKeepAlive ()
{
    if(isReady ())
        getChatService ()->sendKeepAlive ();
}

void
WlmChatSession::inviteContact (const QString & passport)
{
    if (!isReady () && !isConnecting ())
    {
        m_pendingInvitations.append (passport);
        requestChatService ();
        return;
    }
    WlmContact * c = qobject_cast<WlmContact*>(account ()->contacts ()[passport]);
    if (c)
        slotInviteContact (c);
}

unsigned int
WlmChatSession::generateSessionID()
{
    m_sessionID++;
    QTime midnight(0, 0, 0);
    qsrand(midnight.secsTo(QTime::currentTime()));
    return (unsigned int)(qrand() % 4294967295)+m_sessionID;

}

void
WlmChatSession::sendFile (const QString & fileLocation,
                          long unsigned int fileSize)
{
    Q_UNUSED( fileSize );

    MSN::fileTransferInvite ft;
    ft.type = MSN::FILE_TRANSFER_WITHOUT_PREVIEW;
    ft.sessionId = generateSessionID();
    ft.filename = fileLocation.toLatin1 ().data ();
    ft.friendlyname =
        QFileInfo (fileLocation).fileName ().toLatin1 ().data ();
    ft.filesize = QFile (fileLocation).size ();
    ft.userPassport = members ().first ()->contactId ().toLatin1 ().data ();

    // do not generate preview for big pictures
    if(ft.filesize < 2097152)
    {   
        QImage tryImage( fileLocation );
        if(tryImage.format() != QImage::Format_Invalid)
        {
            ft.type = MSN::FILE_TRANSFER_WITH_PREVIEW;
            QByteArray ba;
            QBuffer buffer(&ba);
            buffer.open(QIODevice::WriteOnly);
            tryImage = tryImage.scaled(64,64, Qt::KeepAspectRatio);
            // some clients are stretching the image, so make sure it is really 64x64
            if (tryImage.size() != QSize(64,64))
            {
                QImage temp(64,64, QImage::Format_ARGB32_Premultiplied);
                temp.fill(Qt::transparent);
                QRect r = tryImage.rect();
                r.moveCenter(temp.rect().center());
                QPainter p(&temp);
                p.drawImage(r.topLeft(), tryImage);
                tryImage = temp;
            }
            tryImage.save(&buffer, "PNG"); 
            ft.preview = QString::fromUtf8(KCodecs::base64Encode(ba)).toAscii().data();
        }
    }

    // TODO create a switchboard to send the file is one if not available.
    if (isReady ())
    {
        if (!account ())
            return;
        WlmAccount *acc = qobject_cast < WlmAccount * >(account ());
        if (!acc)
            return;
        Kopete::Transfer * transf =
            Kopete::TransferManager::transferManager ()->
            addTransfer (members ().first (), fileLocation,
                         QFile (fileLocation).size (),
                         members ().first ()->contactId (),
                         Kopete::FileTransferInfo::Outgoing);

        connect (transf, SIGNAL (transferCanceled ()),
                 acc->transferManager (), SLOT (slotCanceled ()));
        acc->transferManager ()->addTransferSession (ft.sessionId, transf,
                                                     account ()->myself ()->
                                                     contactId (),
                                                     members ().first ()->
                                                     contactId ());

        setCanBeDeleted (false);
        getChatService ()->sendFile (ft);
    }
    else
    {
        m_pendingFiles.append (fileLocation);
        if (!isConnecting ())
            requestChatService ();
    }
}


void
WlmChatSession::slotSendFile ()
{
   qobject_cast < WlmContact * >(members ().first ())->sendFile ();
}

void
WlmChatSession::slotInviteContact (Kopete::Contact * contact)
{
    // if we have a session, just invite the new contact
    if (isReady ())
    {
        getChatService ()->inviteUser (contact->contactId ().toLatin1 ().data ());
        return;
    }
    // if we are not in a session or connecting, add this contact to be invited later
    if (!isReady () && !isConnecting ())
    {
        m_pendingInvitations.append (contact->contactId ());
        requestChatService ();
        return;
    }
    // finally if we have a connection in progress, only add this user to be invited later
    if (isConnecting ())
        m_pendingInvitations.append (contact->contactId ());
}

void
WlmChatSession::slotActionInviteAboutToShow ()
{
    // We can't simply insert  KAction in this menu bebause we don't know when to delete them.
    //  items inserted with insert items are automatically deleted when we call clear

    qDeleteAll (m_inviteactions);
    m_inviteactions.clear ();

    m_actionInvite->menu ()->clear ();


    QHash < QString, Kopete::Contact * >contactList = account ()->contacts ();
    QHash < QString, Kopete::Contact * >::Iterator it, itEnd =
        contactList.end ();
    for (it = contactList.begin (); it != itEnd; ++it)
    {
        if (!members ().contains (it.value ()) && it.value ()->isOnline ()
            && it.value ()->onlineStatus ().status () ==
            Kopete::OnlineStatus::Online && it.value () != myself ())
        {
            KAction *a =
                new Kopete::UI::ContactAction (it.value (),
                                               actionCollection ());
			connect( a, SIGNAL(triggered(Kopete::Contact*,bool)),
					this, SLOT(slotInviteContact(Kopete::Contact*)) );

            m_actionInvite->addAction (a);
            m_inviteactions.append (a);
        }
    }

    // We can't simply insert  KAction in this menu bebause we don't know when to delete them.
    //  items inserted with insert items are automatically deleted when we call clear
/*
    m_inviteactions.setAutoDelete(true);
    m_inviteactions.clear();

    m_actionInvite->popupMenu()->clear();

    QListIterator<Kopete::Contact> it( account()->contacts() );
    for( ; it.current(); ++it )
    {
        if( !members().contains( it.current() ) && it.current()->isOnline() && it.current() != myself() )
        {
            KAction *a=new KopeteContactAction( it.current(), this,
                SLOT( slotInviteContact( Kopete::Contact * ) ), m_actionInvite );
            m_actionInvite->insert( a );
            m_inviteactions.append( a ) ;
        }
    }
//    KAction *b=new KAction( i18n ("Other..."), 0, this, SLOT( slotInviteOtherContact() ), m_actionInvite, "actionOther" );
//    m_actionInvite->insert( b );
//    m_inviteactions.append( b ) ;
*/
}


void
WlmChatSession::sendNudge ()
{
    if (isReady ())
    {
        getChatService ()->sendNudge ();
        Kopete::Message msg = Kopete::Message (myself (), members ());
        msg.setDirection (Kopete::Message::Outbound);
        msg.setType (Kopete::Message::TypeAction);
        msg.setPlainBody (i18n ("has sent a nudge"));

        appendMessage (msg);
        return;
    }

    if (!isConnecting ())
    {
        setSendNudge (true);
        requestChatService ();
    }
}

WlmChatSession::~WlmChatSession ()
{
    if (!account ())
        return;

    WlmAccount *acc = qobject_cast < WlmAccount * >(account ());

    if (!acc)
        return;

    WlmChatManager *manager = acc->chatManager ();

    if (manager && getChatService ())
        manager->chatSessions.remove (getChatService ());

    stopSendKeepAlive();

    if (isReady () && getChatService ())
    {
        delete getChatService ();
        setChatService(NULL);
    }
}

bool
WlmChatSession::isConnecting()
{
    if(!getChatService ())
        return false;

    if(getChatService()->connectionState () !=
        MSN::SwitchboardServerConnection::SB_READY &&
            getChatService ()->connectionState () !=
             MSN::SwitchboardServerConnection::SB_DISCONNECTED)
        return true;
    return false;
}

void
WlmChatSession::setChatService (MSN::SwitchboardServerConnection * conn)
{
    m_chatService = conn;
    if (!getChatService())
    {
        setReady (false);
        return;
    }
    if (getChatService ()
        && getChatService ()->connectionState () ==
            MSN::SwitchboardServerConnection::SB_READY)
    {
        setReady (true);
    }
}

void
WlmChatSession::setReady (bool value)
{
    Q_UNUSED( value );

    if (isReady ())
    {
        m_tries = 0;
        if (isDownloadDisplayPicture ())
        {
            setDownloadDisplayPicture (false);
            requestDisplayPicture ();
        }
        if (isSendNudge ())
        {
            sendNudge ();
            setSendNudge (false);
        }

        // invite pending contacts
        QLinkedList < QString >::iterator it;
        for (it = m_pendingInvitations.begin ();
             it != m_pendingInvitations.end (); ++it)
        {
            WlmContact * c = qobject_cast<WlmContact*>(account ()->contacts ()[(*it)]);
            if (c)
                slotInviteContact (c);
        }
        m_pendingInvitations.clear ();

        // send queued messages first
        QLinkedList < Kopete::Message >::iterator it2;
        for (it2 = m_messagesQueue.begin (); it2 != m_messagesQueue.end ();
             ++it2)
        {
            int fontEffects = 0;
            QTextCodec::setCodecForCStrings (QTextCodec::
                                             codecForName ("utf8"));
            MSN::Message mmsg ((*it2).plainBody ().toAscii ().data ());
            mmsg.setFontName ((*it2).font ().family ().toAscii ().data ());
            if ((*it2).font ().bold ())
                fontEffects |= MSN::Message::BOLD_FONT;
            if ((*it2).font ().italic ())
                fontEffects |= MSN::Message::ITALIC_FONT;
            if ((*it2).font ().underline ())
                fontEffects |= MSN::Message::UNDERLINE_FONT;
            if ((*it2).font ().strikeOut ())
                fontEffects |= MSN::Message::STRIKETHROUGH_FONT;

            mmsg.setFontEffects (fontEffects);
            QColor color = (*it2).foregroundColor ();
            mmsg.setColor (color.red (), color.green (), color.blue ());

            // stolen from msn plugin
            const QHash<QString, QStringList> emap = Kopete::Emoticons::self()->theme().emoticonsMap();

            // Check the list for any custom emoticons
            for (QHash<QString, QStringList>::const_iterator itr = emap.begin(); itr != emap.end(); itr++)
            {
                for ( QStringList::const_iterator itr2 = itr.value().constBegin(); itr2 != itr.value().constEnd(); ++itr2 )
                {
                    if ( (*it2).plainBody().contains( *itr2 ) )
                    {
                        getChatService ()->sendEmoticon((*itr2).toAscii().data(), itr.key().toAscii().data());
                    }
                }
            }

            int trid = getChatService ()->sendMessage (&mmsg);

            m_messagesSentQueue[trid] = (*it2);
        }
        m_messagesQueue.clear ();

        // send pending files
        QLinkedList < QString >::iterator it3;
        for (it3 = m_pendingFiles.begin (); it3 != m_pendingFiles.end ();
             ++it3)
        {
            sendFile ((*it3), 0);
        }
        m_pendingFiles.clear ();
    }
    else
    {
        stopSendKeepAlive();
    }
}

bool
WlmChatSession::requestChatService ()
{
    // check if the other contact is offline
    if (members ().count () > 0 &&
        members ().first ()->onlineStatus () ==
        WlmProtocol::protocol ()->wlmOffline)
        return false;

    if (!isReady () && account ()->isConnected () && !isConnecting ())
    {
        const std::string rcpt_ =
            members ().first ()->contactId ().toLatin1 ().data ();
        const std::string msg_ = "";
        const std::pair < std::string,
          std::string > *ctx = new std::pair < std::string,
            std::string > (rcpt_, msg_);
        // request a new switchboard connection
        static_cast <WlmAccount *>(account ())->server ()->
            cb.mainConnection->requestSwitchboardConnection (ctx);
        QTimer::singleShot (15 * 1000, this,
                            SLOT (switchboardConnectionTimeout ()));
        return true;
    }
    // probably we are about to connect
    return true;
}

void
WlmChatSession::switchboardConnectionTimeout ()
{
    if (!isReady ())
    {
        // try 3 times
        if (m_tries < 3)
        {
            m_tries++;
            requestChatService ();
            return;
        }
        Kopete::Utils::notifyCannotConnect(account(), "Could not open switchboard connection");
        messageSucceeded ();
    }
}

void
WlmChatSession::slotMessageSent (Kopete::Message & msg,
                                 Kopete::ChatSession * chat)
{
    Q_UNUSED( chat );
    if (!account ()->isConnected ())
    {
        KMessageBox::queuedMessageBox (Kopete::UI::Global::mainWidget (),
                                       KMessageBox::Information,
                                       "You cannot send a message while in offline status",
                                       "Information");
        messageSucceeded ();
        return;
    }

    if (isReady ())
    {
        // send the message and wait for the ACK 
        int fontEffects = 0;
        QTextCodec::setCodecForCStrings (QTextCodec::codecForName ("utf8"));
        MSN::Message mmsg (msg.plainBody ().toAscii ().data ());

        // FIXME: Can we add FontFamily FF_DONTCARE ?
        if ( msg.format() == Qt::RichText )
        {
            mmsg.setFontName (msg.font ().family ().toAscii ().data ());
            if (msg.font ().bold ())
                fontEffects |= MSN::Message::BOLD_FONT;
            if (msg.font ().italic ())
                fontEffects |= MSN::Message::ITALIC_FONT;
            if (msg.font ().underline ())
                fontEffects |= MSN::Message::UNDERLINE_FONT;
            if (msg.font ().strikeOut ())
                fontEffects |= MSN::Message::STRIKETHROUGH_FONT;

            mmsg.setFontEffects (fontEffects);
            QColor color = msg.foregroundColor ();
            mmsg.setColor (color.red (), color.green (), color.blue ());
        }

        // stolen from msn plugin
        const QHash<QString, QStringList> emap = Kopete::Emoticons::self()->theme().emoticonsMap();

        // Check the list for any custom emoticons
        for (QHash<QString, QStringList>::const_iterator itr = emap.begin(); itr != emap.end(); itr++)
        {
            for ( QStringList::const_iterator itr2 = itr.value().constBegin(); itr2 != itr.value().constEnd(); ++itr2 )
            {
                if ( msg.plainBody().contains( *itr2 ) )
                {
                    getChatService ()->sendEmoticon((*itr2).toAscii().data(), itr.key().toAscii().data());
                }
            }
        }

        int trid = getChatService ()->sendMessage (&mmsg);
        m_messagesSentQueue[trid] = msg;
        return;
    }

    if (!isConnecting () && !isReady ())
    {
        // request switchboard
        if (!requestChatService ())
        {
            MSN::Soap::OIM oim;
            oim.myFname =
                myself ()->property (Kopete::Global::Properties::self ()->
                                     nickName ()).value ().toString ().
                toLatin1 ().data ();
            oim.toUsername =
                members ().first ()->contactId ().toLatin1 ().data ();
            QTextCodec::setCodecForCStrings (QTextCodec::
                                             codecForName ("utf8"));
            oim.message = msg.plainBody ().toAscii ().data ();
            oim.myUsername = myself ()->contactId ().toLatin1 ().data ();
            oim.id = m_oimid++;

            static_cast <
                WlmAccount *
                >(account ())->server ()->cb.mainConnection->send_oim (oim);
            appendMessage (msg);
            messageSucceeded ();
            return;
        }
        // put the message in a queue
        m_messagesQueue.append (msg);
        return;
    }

    if (isConnecting ())
    {
        // put the message in the queue, we are trying to connect to the
        // switchboard server
        m_messagesQueue.append (msg);
        return;
    }

}
bool
WlmChatSession::isReady ()
{
    if(!getChatService ())
        return false;

    // check in libmsn if we are really ready
    if(getChatService ()->connectionState () ==
            MSN::SwitchboardServerConnection::SB_READY)
        return true;
    else
        return false;
}

void
WlmChatSession::sendTypingMsg (bool istyping)
{
    if (!istyping || isConnecting ())
        return;

    // do not send notification if we 
    // are alone in the session
    if (!isReady ())
        return;

    getChatService ()->sendTypingNotification ();

    startSendKeepAlive();
}

void
WlmChatSession::messageSentACK (unsigned int trID)
{
    appendMessage (m_messagesSentQueue[trID]);
    m_messagesSentQueue.remove (trID);
    // remove the blinking icon when there are no messages
    // waiting for delivery
    if (m_messagesSentQueue.empty ())
        messageSucceeded ();
}

void 
WlmChatSession::startSendKeepAlive()
{
    // send keepalive each 50 seconds.
    if(m_keepalivetimer && isReady())
        m_keepalivetimer->start (50 * 1000);
}

void 
WlmChatSession::stopSendKeepAlive()
{
    if(m_keepalivetimer)
        m_keepalivetimer->stop ();
}

void
WlmChatSession::receivedNudge (QString passport)
{
    WlmContact * c = qobject_cast<WlmContact*>(account ()->contacts ()[passport]);
    if (!c)
        c = qobject_cast<WlmContact*>(members ().first ());

    Kopete::Message msg = Kopete::Message (c, myself ());
    msg.setPlainBody (i18n ("has sent you a nudge"));
    msg.setDirection (Kopete::Message::Inbound);
    msg.setType (Kopete::Message::TypeAction);
    appendMessage (msg);
    emitNudgeNotification ();   // notifies with system message close to tray icon
    startSendKeepAlive();

}

void
WlmChatSession::requestDisplayPicture ()
{
    // only request picture in a 2 session people only
    if (members ().count () != 1)
        return;

    WlmContact *contact = qobject_cast < WlmContact * >(members ().first ());

    if (!contact)
        return;

    if (contact->getMsnObj ().isEmpty () || contact->getMsnObj () == "0")
        return;

    QString msnobj = contact->getMsnObj ();

    QDomDocument xmlobj;
    xmlobj.setContent (msnobj);

    // track display pictures by SHA1D field
    QString SHA1D = xmlobj.documentElement ().attribute ("SHA1D");

    if (SHA1D.isEmpty ())
        return;

    QString newlocation =
        KGlobal::dirs ()->locateLocal ("appdata",
                                       "wlmpictures/" +
                                       QString (SHA1D.replace ("/", "_")));

    if (QFile (newlocation).exists () && QFile (newlocation).size ())
    {
        qobject_cast <WlmAccount *>(account ())->gotDisplayPicture (contact->contactId (),
                                              newlocation);
        return;
    }

    // request switchboard connection
    // and ask for the display picture
    if (!isReady () && !isConnecting ())
    {
        requestChatService ();
        setDownloadDisplayPicture (true);
        return;                 // TODO - schedule this action
    }
    if (isReady ())
    {
        getChatService ()->requestDisplayPicture (generateSessionID(),
                                              newlocation.toLatin1 ().data (),
                                              contact->getMsnObj ().
                                              toAscii ().data ());
        setDownloadDisplayPicture (false);
    }
}

#include "wlmchatsession.moc"
