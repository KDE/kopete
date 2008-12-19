/*
    wlmaccount.cpp - Kopete Wlm Protocol

    Copyright (c) 2008      by Tiago Salem Herrmann <tiagosh@gmail.com>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "wlmaccount.h"

#include <QTextCodec>
#include <QFile>
#include <QImage>
#include <QDomDocument>
#include <QtCore/QCryptographicHash>

#include <kaction.h>
#include <kactionmenu.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <knotification.h>
#include <KCodecs>
#include <KInputDialog>
#include <KStandardDirs>
#include <KToolInvocation>

#include "kopetechatsessionmanager.h"
#include "kopetemetacontact.h"
#include "kopetecontactlist.h"
#include "kopetegroup.h"
#include "kopetepassword.h"
#include "kopeteuiglobal.h"
#include "kopetepicture.h"
#include "kopeteutils.h"
#include "kopetetransfermanager.h"
#include "kopeteidentity.h"
#include "kopeteavatarmanager.h"
#include "kopeteaddedinfoevent.h"

#include "wlmcontact.h"
#include "wlmprotocol.h"
#include "wlmchatsession.h"

WlmAccount::WlmAccount (WlmProtocol * parent, const QString & accountID):
Kopete::PasswordedAccount (parent, accountID.toLower ()),
m_server (NULL),
m_transferManager (NULL),
m_chatManager (NULL),
clientid (0)
{
    // Init the myself contact
    setMyself (new
               WlmContact (this, accountId (), QString(),
                           accountId (),
                           Kopete::ContactList::self ()->myself ()));
    myself ()->setOnlineStatus (WlmProtocol::protocol ()->wlmOffline);
    clientid += MSN::MSNC7;
    clientid += MSN::MSNC6;
    clientid += MSN::MSNC5;
    clientid += MSN::MSNC4;
    clientid += MSN::MSNC3;
    clientid += MSN::MSNC2;
    clientid += MSN::MSNC1;
    clientid += MSN::SupportWinks;
    clientid += MSN::VoiceClips;
    clientid += MSN::InkGifSupport;
    clientid += MSN::SIPInvitations;
    clientid += MSN::SupportMultiPacketMessaging;

    m_openInboxAction = new KAction(KIcon("mail-folder-inbox"), i18n("Open Inbo&x..."), this);
    QObject::connect(m_openInboxAction, SIGNAL(triggered(bool)), this, SLOT(slotOpenInbox()));

    m_changeDNAction = new KAction(i18n("&Change Display Name..."), this);
    QObject::connect(m_changeDNAction, SIGNAL(triggered(bool)), this, SLOT(slotChangePublicName()));

//     m_startChatAction = new KAction(KIcon("mail-message-new"), i18n("&Start Chat..."), this);
//     QObject::connect(m_startChatAction, SIGNAL(triggered(bool)), this, SLOT(slotStartChat()));
    
    m_openStatusAction = new KAction(i18n("Open MS&N service status site..."), this);
    QObject::connect(m_openStatusAction, SIGNAL(triggered(bool)), this, SLOT(slotOpenStatus()));
}

WlmAccount::~WlmAccount ()
{
    disconnect ();
}

void
WlmAccount::fillActionMenu (KActionMenu * actionMenu)
{
    Kopete::Account::fillActionMenu (actionMenu);
    const bool connected = isConnected();
    m_openInboxAction->setEnabled(connected);
//     m_startChatAction->setEnabled(connected);
    m_changeDNAction->setEnabled(connected);

    actionMenu->addSeparator();

    actionMenu->addAction(m_changeDNAction);
//     actionMenu->addAction(m_startChatAction);

//     actionMenu->addAction(m_openInboxAction);
    actionMenu->addAction(m_openStatusAction);
}

bool
WlmAccount::createContact (const QString & contactId,
                           Kopete::MetaContact * parentContact)
{
    kDebug() << "contact " << contactId;
    WlmContact *newContact = new WlmContact (this, contactId, QString(), parentContact->displayName (), parentContact);

    if (parentContact->isTemporary())
        return true;

    if (m_serverSideContactsPassports.contains(contactId))
    {
        kDebug() << "contact " << contactId << " already on server list. Do nothing.";
        return true;
    }

    QString groupName;
    Kopete::GroupList kopeteGroups = parentContact->groups(); //get the group list

    if (kopeteGroups.isEmpty() || kopeteGroups.first() == Kopete::Group::topLevel())
        groupName = i18n("Buddies");
    else
        groupName = kopeteGroups.first() ? kopeteGroups.first()->displayName() : i18n("Buddies");

    // emergency exit, should never occur
    if (groupName.isEmpty())
        return false;

    m_contactAddQueue.insert(contactId.toAscii(), groupName.toAscii());
    if (!m_groupToGroupId.contains(groupName.toAscii()))
    {
        kDebug() << "group \'" << groupName << "\' not found adding group";
        m_server->cb.mainConnection->addGroup (groupName.toAscii().data());
    }
    else
    {
        kDebug() << "group \'" << groupName << "\' found adding contact";
        m_server->cb.mainConnection->addToAddressBook (contactId.toAscii().data(), contactId.toAscii().data());
    }

	return newContact != 0L;
}

void WlmAccount::setPersonalMessage (const Kopete::StatusMessage & reason)
{
    kDebug (14210) << k_funcinfo;
    myself()->setStatusMessage(reason);
    if (isConnected ())
    {
        MSN::personalInfo pInfo;
        pInfo.mediaIsEnabled = 0;
        QTextCodec::setCodecForCStrings (QTextCodec::codecForName ("utf8"));
        if (reason.message().isEmpty ())
            pInfo.PSM = "";
        else
            pInfo.PSM = reason.message().toAscii ().data ();

        // we have both artist and title
        if( reason.hasMetaData("artist") && reason.hasMetaData("title") )
        {
            pInfo.mediaIsEnabled = 1;
            pInfo.mediaType="Music";
            pInfo.mediaLines.push_back( reason.metaData("artist").toString().toAscii().data() );
            pInfo.mediaLines.push_back( reason.metaData("title").toString().toAscii().data() );
            pInfo.mediaFormat="{0} - {1}";
            m_server->cb.mainConnection->setPersonalStatus (pInfo);
            return;
        }
    
        // we have only the title
        if( reason.hasMetaData("title") )
        {
            pInfo.mediaIsEnabled = 1;
            pInfo.mediaType="Music";
            pInfo.mediaFormat="{0}";
            pInfo.mediaLines.push_back( reason.metaData("title").toString().toAscii().data() );
            m_server->cb.mainConnection->setPersonalStatus (pInfo);
            return;
        }
        m_server->cb.mainConnection->setPersonalStatus (pInfo);
    }
}

void
WlmAccount::setOnlineStatus (const Kopete::OnlineStatus & status,
                             const Kopete::StatusMessage & reason)
{
    kDebug (14210) << k_funcinfo;

    setPersonalMessage(reason);

    temporaryStatus = status;

    if (status == WlmProtocol::protocol ()->wlmConnecting &&
        myself ()->onlineStatus () == WlmProtocol::protocol ()->wlmOffline)
        slotGoOnline ();
    else if (status == WlmProtocol::protocol ()->wlmOnline)
        slotGoOnline ();
    else if (status == WlmProtocol::protocol ()->wlmOffline)
        slotGoOffline ();
    else if (status == WlmProtocol::protocol ()->wlmInvisible)
        slotGoInvisible ();
    else if (status.status () == Kopete::OnlineStatus::Away)
        slotGoAway (status);
}

void
WlmAccount::setStatusMessage (const Kopete::StatusMessage & statusMessage)
{
    setPersonalMessage(statusMessage);
}

void
WlmAccount::slotChangePublicName()
{
    if ( !isConnected() )
    {
        return;
        //TODO:  change it anyway, and sync at the next connection
    }

    bool ok;
    const QString name = KInputDialog::getText( i18n( "Change Display Name - MSN Plugin" ), //TODO rename MSN to WLM (see also following strings)
        i18n( "Enter the new display name by which you want to be visible to your friends on MSN:" ),
        myself()->property( Kopete::Global::Properties::self()->nickName()).value().toString(), &ok );

    if ( ok )
    {
        if ( name.length() > 387 )
        {
            KMessageBox::error( Kopete::UI::Global::mainWidget(),
                i18n( "<qt>The display name you entered is too long. Please use a shorter name.\n"
                    "Your display name has <b>not</b> been changed.</qt>" ),
                i18n( "Change Display Name - MSN Plugin" ) );
            return;
        }

        m_server->cb.mainConnection->setFriendlyName(name.toAscii().data(), true);
    }
}

void
WlmAccount::slotOpenInbox()
{
//     if (m_notifySocket)
//         m_notifySocket->slotOpenInbox();
}

void
WlmAccount::slotOpenStatus()
{
    KToolInvocation::invokeBrowser(QLatin1String("http://messenger.msn.com/Status.aspx")) ;
}

void
WlmAccount::connectWithPassword (const QString & pass)
{
    kDebug (14210) << k_funcinfo;
    if (myself ()->onlineStatus () != WlmProtocol::protocol ()->wlmOffline)
        return;

    if (pass.isEmpty ())
    {
        password ().setWrong (true);
        password ().setWrong (false);
        return;
    }

    password ().setWrong (false);

    QString id = accountId ();
    QString pass1 = pass;

    enableInitialList ();

    m_server = new WlmServer (this, id, pass1);
    m_server->WlmConnect ( serverName(), serverPort() );

    m_transferManager = new WlmTransferManager (this);

    m_chatManager = new WlmChatManager (this);

    QObject::connect (&m_server->cb, SIGNAL (connectionCompleted ()),
                      this, SLOT (connectionCompleted ()));
    QObject::connect (&m_server->cb, SIGNAL (connectionFailed ()),
                      this, SLOT (connectionFailed ()));
    QObject::connect (&m_server->cb,
                      SIGNAL (gotDisplayName (const QString &)), this,
                      SLOT (gotDisplayName (const QString &)));
    QObject::connect (&m_server->cb,
                      SIGNAL (receivedOIMList
                              (std::vector < MSN::eachOIM > &)), this,
                      SLOT (receivedOIMList
                            (std::vector < MSN::eachOIM > &)));
    QObject::connect (&m_server->cb,
                      SIGNAL (receivedOIM (const QString &, const QString &)),
                      this,
                      SLOT (receivedOIM (const QString &, const QString &)));

    QObject::connect (&m_server->cb,
                      SIGNAL (deletedOIM(const QString&, const bool&)), this,
                      SLOT (deletedOIM(const QString&, const bool &)));
    QObject::connect (&m_server->cb,
                      SIGNAL (NotificationServerConnectionTerminated
                              (MSN::NotificationServerConnection *)), this,
                      SLOT (NotificationServerConnectionTerminated
                            (MSN::NotificationServerConnection *)));
    QObject::connect (&m_server->cb, SIGNAL (wrongPassword ()), this,
                      SLOT (wrongPassword ()));

    myself ()->setOnlineStatus (WlmProtocol::protocol ()->wlmConnecting);
}

QString WlmAccount::serverName() const
{
    return configGroup()->readEntry( "serverName" , "messenger.hotmail.com" );
}

uint WlmAccount::serverPort() const
{
    return configGroup()->readEntry( "serverPort" , 1863 );
}

void
WlmAccount::gotNewContact (const MSN::ContactList & list,
                           const QString & passport,
                           const QString & friendlyname)
{
    kDebug() << "contact " << passport;
    if (list == MSN::LST_RL)
    {
        Kopete::AddedInfoEvent* event = new Kopete::AddedInfoEvent(passport, this);
        QObject::connect(event, SIGNAL(actionActivated(uint)), this, SLOT(addedInfoEventActionActivated(uint)));

        Kopete::AddedInfoEvent::ShowActionOptions actions = Kopete::AddedInfoEvent::AuthorizeAction;
        actions |= Kopete::AddedInfoEvent::BlockAction;
        //actions |= Kopete::AddedInfoEvent::InfoAction;

        WlmContact * ct = qobject_cast<WlmContact*>(contacts().value(passport));
        if (!ct || !ct->metaContact() || ct->metaContact()->isTemporary())
            actions |= Kopete::AddedInfoEvent::AddAction;

        event->setContactNickname(friendlyname);
        event->showActions(actions);
        event->sendEvent();
    }
    else if (list == MSN::LST_BL)
    {
        kDebug() << "contact " << passport << " added to block list";
        m_allowList.remove(passport);
        m_blockList.insert(passport);
        WlmContact * ct = qobject_cast<WlmContact*>(contacts().value(passport));
        if(ct)
            ct->setOnlineStatus(ct->onlineStatus());
    }
    else if (list == MSN::LST_AL)
    {
        kDebug() << "contact " << passport << " added to allow list";
        m_blockList.remove(passport);
        m_allowList.insert(passport);
        WlmContact * ct = qobject_cast<WlmContact*>(contacts().value(passport));
        if(ct)
            ct->setOnlineStatus(ct->onlineStatus());
    }
}

void WlmAccount::gotRemovedContactFromList (const MSN::ContactList & list, const QString & contact)
{
    kDebug() << "contact " << contact;
    if (list == MSN::LST_BL)
    {
        kDebug() << "contact " << contact << " removed from block list";
        m_blockList.remove( contact );
    }
    else if (list == MSN::LST_AL)
    {
        kDebug() << "contact " << contact << " removed from allow list";
        m_allowList.remove( contact );
    }
}

void WlmAccount::addedInfoEventActionActivated(uint actionId)
{
    Kopete::AddedInfoEvent *event = qobject_cast<Kopete::AddedInfoEvent *>(sender());
    if (!event || !isConnected())
        return;

    switch (actionId)
    {
    case Kopete::AddedInfoEvent::AddContactAction:
        event->addContact();
        break;
    case Kopete::AddedInfoEvent::AuthorizeAction:
        blockContact(event->contactId(), false);
        break;
    case Kopete::AddedInfoEvent::BlockAction:
        blockContact(event->contactId(), true);
        break;
/*    case Kopete::AddedInfoEvent::InfoAction:
        break;*/
    }
}

void
WlmAccount::wrongPassword ()
{
    kDebug (14210) << k_funcinfo;
    password ().setWrong (true);
}

void
WlmAccount::scheduleConnect ()
{
    connect (temporaryStatus);
}

void
WlmAccount::gotDisplayPicture (const QString & contactId,
                               const QString & filename)
{
    kDebug (14210) << k_funcinfo;
    WlmContact * contact = qobject_cast<WlmContact*>(contacts ()[contactId]);
    if (contact)
    {
        // remove from pending display pictures list if applicable
        m_pendingDisplayPictureList.remove(contactId);

        QFile f(filename);
        if (!f.exists () || !f.size ())
        {
            f.remove ();
            contact->removeProperty (Kopete::Global::Properties::self ()->
                                     photo ());
            //dynamic_cast < WlmContact * >(contact)->setMsnObj ("0");
            return;
        }
        // remove old picture
        if (contact->
            hasProperty (Kopete::Global::Properties::self()->photo ().key ()))
        {
            QString file =
                contact->property (
			Kopete::Global::Properties::self ()->photo()).value ().toString ();
            contact->removeProperty (Kopete::Global::Properties::self ()->photo ());
            if (QFile (file).exists () && file != filename)
                QFile::remove (file);
        }

        // check file integrity (SHA1D)
        QDomDocument xmlobj;
        xmlobj.setContent (contact->getMsnObj());
        QString SHA1D_orig = xmlobj.documentElement ().attribute ("SHA1D");

        if (SHA1D_orig.isEmpty ())
            return;

        // open the file to generate the SHA1D
        if (!f.open(QIODevice::ReadOnly))
        {
            kDebug(14140) << "Could not open avatar picture.";
            contact->removeProperty( Kopete::Global::Properties::self()->photo() );
            QFile::remove (filename);
            return;
        }

        QByteArray ar = f.readAll();
        QByteArray SHA1D = QCryptographicHash::hash(ar, QCryptographicHash::Sha1).toBase64();

        // remove corrupted files
        if(SHA1D != SHA1D_orig)
        {
            QFile::remove (filename);
            return;
        }

        QImage contactPhoto = QImage( filename );
        if(contactPhoto.format()!=QImage::Format_Invalid)
        {
            contact->setProperty (Kopete::Global::Properties::self ()->photo (),
                              filename);
        }
        else
        {
            f.remove();
            contact->removeProperty (Kopete::Global::Properties::self ()->
                                     photo ());
        }
    }
}

void
WlmAccount::gotDisplayName (const QString & displayName)
{
    kDebug (14210) << k_funcinfo;
    myself ()->setProperty (Kopete::Global::Properties::self ()->nickName (),
                            displayName);
}

void
WlmAccount::gotContactPersonalInfo (const MSN::Passport & fromPassport,
                                    const MSN::personalInfo & pInfo)
{
    kDebug (14210) << k_funcinfo;
    WlmContact * contact = qobject_cast<WlmContact*>(contacts ()[fromPassport.c_str ()]);
    if (contact)
    {
        // TODO - handle the other fields of pInfo
        contact->setStatusMessage(Kopete::StatusMessage(QString(pInfo.PSM.c_str())));
        QString type (pInfo.mediaType.c_str ());
        if (pInfo.mediaIsEnabled && type == "Music")
        {
            QString song_line (pInfo.mediaFormat.c_str ());
            int num = pInfo.mediaLines.size ();
            for (int i = 0; i < num; i++)
            {
                song_line.replace ('{' + QString::number (i) + '}',
                                   pInfo.mediaLines[i].c_str ());
            }
            contact->setProperty (WlmProtocol::protocol ()->currentSong,
                                  song_line.toAscii ().constData ());
        }
        else
        {
            contact->setProperty (WlmProtocol::protocol ()->currentSong,
                                  QVariant (QString()));
        }
    }
}

void
WlmAccount::contactChangedStatus (const MSN::Passport & buddy,
                                  const QString & friendlyname,
                                  const MSN::BuddyStatus & state,
                                  const unsigned int &clientID,
                                  const QString & msnobject)
{
    Q_UNUSED( clientID );

    kDebug (14210) << k_funcinfo;
    WlmContact *contact = qobject_cast<WlmContact*>(contacts ()[buddy.c_str ()]);
    if (contact)
    {
        contact->setProperty (Kopete::Global::Properties::self ()->
                              nickName (), friendlyname);

        if (state == MSN::STATUS_AWAY)
            contact->setOnlineStatus (WlmProtocol::protocol ()->wlmAway);
        else if (state == MSN::STATUS_AVAILABLE)
            contact->setOnlineStatus (WlmProtocol::protocol ()->wlmOnline);
        else if (state == MSN::STATUS_INVISIBLE)
            contact->setOnlineStatus (WlmProtocol::protocol ()->wlmInvisible);
        else if (state == MSN::STATUS_BUSY)
            contact->setOnlineStatus (WlmProtocol::protocol ()->wlmBusy);
        else if (state == MSN::STATUS_OUTTOLUNCH)
            contact->setOnlineStatus (WlmProtocol::protocol ()->
                                      wlmOutToLunch);
        else if (state == MSN::STATUS_ONTHEPHONE)
            contact->setOnlineStatus (WlmProtocol::protocol ()->
                                      wlmOnThePhone);
        else if (state == MSN::STATUS_BERIGHTBACK)
            contact->setOnlineStatus (WlmProtocol::protocol ()->
                                      wlmBeRightBack);
        else if (state == MSN::STATUS_IDLE)
            contact->setOnlineStatus (WlmProtocol::protocol ()->wlmIdle);

        qobject_cast <WlmContact *>(contact)->setMsnObj (msnobject.toAscii().data());

        if (msnobject.isEmpty () || msnobject == "0")   // no picture
        {
            if (contact && contact->
                hasProperty (Kopete::Global::Properties::self()->photo ().key ()))
            {
                QString file = contact->property (
                    Kopete::Global::Properties::self ()->
                        photo()).value ().toString ();
                contact->removeProperty (Kopete::Global::Properties::self ()->photo ());
                if (QFile (file).exists ())
                    QFile::remove (file);
            }
            return;
        }

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
            gotDisplayPicture (contact->contactId (), newlocation);
            return;
        }

        // do not request all pictures at once when you are just connected
        if (isInitialList ())
        {
            // schedule to retrieve this picture later
            m_pendingDisplayPictureList.insert(buddy.c_str ());
            return;
        }

        if ((myself ()->onlineStatus () !=
                WlmProtocol::protocol ()->wlmOffline)
               && (myself ()->onlineStatus () !=
                   WlmProtocol::protocol ()->wlmInvisible)
               && (myself ()->onlineStatus () !=
                   WlmProtocol::protocol ()->wlmUnknown))
        {
            chatManager ()->requestDisplayPicture (buddy.c_str ());
        }
    }
}

void
WlmAccount::contactDisconnected (const MSN::Passport & buddy)
{
    kDebug (14210) << k_funcinfo;
    WlmContact * contact = qobject_cast<WlmContact*>(contacts ()[buddy.c_str ()]);
    if (contact)
    {
        contact->setOnlineStatus (WlmProtocol::protocol ()->wlmOffline);
    }
}

void
WlmAccount::groupListReceivedFromServer (std::map < std::string,
                                         MSN::Group > &list)
{
    kDebug (14210) << k_funcinfo;
    // add server groups on local list
    std::map < std::string, MSN::Group >::iterator it;
    for (it = list.begin (); it != list.end (); ++it)   // groups from server
    {
        MSN::Group * g = &(*it).second;
        Kopete::Group * b =
            Kopete::ContactList::self ()->findGroup (g->name.c_str ());
        QTextCodec::setCodecForCStrings (QTextCodec::codecForName ("utf8"));
        if (!b)
        {
            b = new Kopete::Group (QString (g->name.c_str ()).toAscii ().data ());
            Kopete::ContactList::self ()->addGroup ( b );
        }

        m_groupToGroupId.insert (QString(g->name.c_str()), QString(g->groupID.c_str()));
    }
/*
	// remove local groups which are not on server
	QPtrList<Kopete::Group>::Iterator it1 =  Kopete::ContactList::self()->groups().begin();
	Kopete::Group *group;
	for ( group = Kopete::ContactList::self()->groups().first(); group; group = Kopete::ContactList::self()->groups().next() )
	{
		bool ok=false;
		std::map<std::string, MSN::Group>::iterator it;
		for ( it = list.begin(); it != list.end(); ++it ) // groups from server
		{
			MSN::Group *g = &(*it).second;
			if(Kopete::ContactList::self()->findGroup(QString(g->name.c_str()).toAscii().data()))
				ok=true;
		}
		if(!ok)
		{
			if(!group->members().count())
				Kopete::ContactList::self()->removeGroup(group);
		}
	}
*/
}

void
WlmAccount::addressBookReceivedFromServer (std::map < std::string,
                                           MSN::Buddy * >&list)
{
    kDebug (14210) << k_funcinfo;

    // Clear server side passports
    m_serverSideContactsPassports.clear ();
    m_allowList.clear();
    m_blockList.clear();

    // local contacts which dont exist on server should be deleted
    std::map < std::string, MSN::Buddy * >::iterator it;
    for (it = list.begin (); it != list.end (); ++it)
    {
        Kopete::MetaContact * metacontact = 0L;
        MSN::Buddy * b = (*it).second;
        QString passport = (*it).first.c_str();

        if (b->lists & MSN::LST_AB)
            m_serverSideContactsPassports.insert (b->userName.c_str());
        if ( b->lists & MSN::LST_AL )
            m_allowList.insert( passport );
        if ( b->lists & MSN::LST_BL )
            m_blockList.insert( passport );
        if ( b->lists & MSN::LST_PL )
            m_pendingList.insert( passport );

        if ( !contacts().value( passport ) )
        {
            if (!b->friendlyName.length ())
                b->friendlyName = b->userName;

            QTextCodec::setCodecForCStrings (QTextCodec::codecForName ("utf8"));
            std::list < MSN::Group * >::iterator i = b->groups.begin ();
            //bool ok = false;

            // no groups, add to top level
            if (!b->groups.size ())
            {
                // only add users in forward list and messenger users
                if (b->lists & MSN::LST_AB && b->properties["isMessengerUser"] == "true" )
                {
                    metacontact = addContact (b->userName.c_str (), QString(), 0L, Kopete::Account::DontChangeKABC);

                    WlmContact * newcontact = qobject_cast<WlmContact*>(contacts ()[b->userName.c_str ()]);
                    if(!newcontact)
                        return;

                    newcontact->setProperty (Kopete::Global::Properties::self ()->nickName (), QString (b->friendlyName.c_str ()));
                }

                if (metacontact)
                {
                    WlmContact *contact = qobject_cast<WlmContact*>(contacts().value( passport ));
                    if (contact)
                    {
                        contact->setContactSerial (b->properties["contactId"].c_str ());
                        kDebug (14210) << "ContactID: " << b->properties["contactId"].c_str ();
                    }
                }
                continue;
            }

            for (; i != b->groups.end (); ++i)
            {
                Kopete::Group * g = Kopete::ContactList::self ()->findGroup (QString ((*i)->name.c_str ()).toAscii ());

                if (g)
                    metacontact = addContact (b->userName.c_str (), QString(), g, Kopete::Account::DontChangeKABC);
                else
                    metacontact = addContact (b->userName.c_str (), QString(), Kopete::Group::topLevel (), Kopete::Account::DontChangeKABC);

                if (metacontact)
                {
                    WlmContact *contact = qobject_cast<WlmContact*>(contacts().value( passport ));
                    if (contact)
                    {
                        contact->setProperty (Kopete::Global::Properties::self ()->nickName (), QString (b->friendlyName.c_str ()));
                        contact->setContactSerial (b->properties["contactId"].c_str ());
                        kDebug (14210) << "ContactID: " << b->properties["contactId"].c_str ();
                    }
                }
            }
        }
    }
}

void
WlmAccount::slotGlobalIdentityChanged (Kopete::PropertyContainer *,
                                       const QString & key, const QVariant &,
                                       const QVariant & newValue)
{
    kDebug (14210) << k_funcinfo;
    if (key == Kopete::Global::Properties::self ()->photo ().key ())
    {
        m_pictureFilename = newValue.toString ();
        // TODO - Set no photo on server
        if (m_pictureFilename.isEmpty ())
        {
            myself()->removeProperty(Kopete::Global::Properties::self()->photo ());
            if(m_server && isConnected ())
            {
                m_server->cb.mainConnection->change_DisplayPicture ("");
                setOnlineStatus (myself ()->onlineStatus ());
            }
            return;
        }

        QImage contactPhoto = QImage( m_pictureFilename );
        Kopete::AvatarManager::AvatarEntry entry;
        entry.name = myself ()->contactId();
        entry.image = contactPhoto;
        entry.category = Kopete::AvatarManager::Contact;
        entry.contact = myself();
        entry = Kopete::AvatarManager::self()->add(entry);

        kDebug (14140) << k_funcinfo << m_pictureFilename;
        if(!entry.path.isNull())
        {
            if(m_server)
               m_server->cb.mainConnection->
                   change_DisplayPicture (entry.path.toLatin1 ().data ());
               myself()->setProperty(Kopete::Global::Properties::self()->photo (), entry.path);
        }
        setOnlineStatus (myself ()->onlineStatus ());
    }
    else if (key == Kopete::Global::Properties::self ()->nickName ().key ())
    {
        QString oldNick =
            myself ()->property (Kopete::Global::Properties::self ()->
                                 nickName ()).value ().toString ();
        QString newNick = newValue.toString ();

        if (newNick != oldNick)
        {
            QTextCodec::setCodecForCStrings (QTextCodec::codecForName ("utf8"));
            if(m_server && isConnected())
                m_server->cb.mainConnection->setFriendlyName (newNick.toAscii ().
                                                          data ());
        }
    }
}

void
WlmAccount::changedStatus (MSN::BuddyStatus & state)
{
    kDebug (14210) << k_funcinfo;
    if (state == MSN::STATUS_AWAY)
        myself ()->setOnlineStatus (WlmProtocol::protocol ()->wlmAway);
    else if (state == MSN::STATUS_AVAILABLE)
        myself ()->setOnlineStatus (WlmProtocol::protocol ()->wlmOnline);
    else if (state == MSN::STATUS_INVISIBLE)
        myself ()->setOnlineStatus (WlmProtocol::protocol ()->wlmInvisible);
    else if (state == MSN::STATUS_BUSY)
        myself ()->setOnlineStatus (WlmProtocol::protocol ()->wlmBusy);
    else if (state == MSN::STATUS_OUTTOLUNCH)
        myself ()->setOnlineStatus (WlmProtocol::protocol ()->wlmOutToLunch);
    else if (state == MSN::STATUS_ONTHEPHONE)
        myself ()->setOnlineStatus (WlmProtocol::protocol ()->wlmOnThePhone);
    else if (state == MSN::STATUS_BERIGHTBACK)
        myself ()->setOnlineStatus (WlmProtocol::protocol ()->wlmBeRightBack);
    if (state == MSN::STATUS_IDLE)
        myself ()->setOnlineStatus (WlmProtocol::protocol ()->wlmIdle);

}

void
WlmAccount::connectionFailed ()
{
    kDebug (14210) << k_funcinfo;
    myself ()->setOnlineStatus (WlmProtocol::protocol ()->wlmOffline);
    Kopete::Utils::notifyCannotConnect (this);
}

void
WlmAccount::connectionCompleted ()
{
    kDebug (14210) << k_funcinfo;
    if (identity ()->
        hasProperty (Kopete::Global::Properties::self ()->photo ().key()))
    {
        m_server->cb.mainConnection->change_DisplayPicture (identity ()->
                                                            customIcon ().
                                                            toLatin1 ().
                                                            data ());
        QImage contactPhoto = QImage( identity ()->customIcon() );
        Kopete::AvatarManager::AvatarEntry entry;
        entry.name = myself ()->contactId();
        entry.image = contactPhoto;
        entry.category = Kopete::AvatarManager::Contact;
        entry.contact = myself();
        entry = Kopete::AvatarManager::self()->add(entry);
        if(!entry.path.isNull())
            myself()->setProperty(Kopete::Global::Properties::self()->photo (), entry.path);
    }

    if ( identity()->hasProperty( Kopete::Global::Properties::self()->nickName().key() ))
    {
        // use the identity nickname instead of the one stored on the server side
        QString nick = identity()->property(
                Kopete::Global::Properties::self()->nickName()).value().toString();
        m_server->cb.mainConnection->setFriendlyName(nick.toAscii().data());
    }

    password ().setWrong (false);

    QObject::connect (&m_server->cb,
                      SIGNAL (changedStatus (MSN::BuddyStatus &)), this,
                      SLOT (changedStatus (MSN::BuddyStatus &)));

    QObject::connect (&m_server->cb,
                      SIGNAL (contactChangedStatus
                              (const MSN::Passport &, const QString &,
                               const MSN::BuddyStatus &, const unsigned int &,
                               const QString &)), this,
                      SLOT (contactChangedStatus
                            (const MSN::Passport &, const QString &,
                             const MSN::BuddyStatus &, const unsigned int &,
                             const QString &)));

    QObject::connect (&m_server->cb,
                      SIGNAL (contactDisconnected (const MSN::Passport &)),
                      this,
                      SLOT (contactDisconnected (const MSN::Passport &)));

    QObject::connect (identity (),
                      SIGNAL (propertyChanged
                              (Kopete::PropertyContainer *, const QString &,
                               const QVariant &, const QVariant &)),
                      SLOT (slotGlobalIdentityChanged
                            (Kopete::PropertyContainer *, const QString &,
                             const QVariant &, const QVariant &)));

    QObject::connect (&m_server->cb,
                      SIGNAL (gotDisplayPicture
                              (const QString &, const QString &)),
                      SLOT (gotDisplayPicture
                            (const QString &, const QString &)));

    QObject::connect (&m_server->cb,
                      SIGNAL (gotContactPersonalInfo
                              (const MSN::Passport &,
                               const MSN::personalInfo &)),
                      SLOT (gotContactPersonalInfo
                            (const MSN::Passport &,
                             const MSN::personalInfo &)));

    QObject::connect (&m_server->cb, SIGNAL(gotNewContact(const MSN::ContactList&, const QString&, const QString&)),
                      SLOT (gotNewContact(const MSN::ContactList&, const QString&, const QString&)));

    QObject::connect (&m_server->cb, SIGNAL(gotRemovedContactFromList(const MSN::ContactList&, const QString&)),
                      this, SLOT(gotRemovedContactFromList(const MSN::ContactList&, const QString&)) );

    QObject::connect (&m_server->cb, SIGNAL(gotAddedContactToGroup(bool, const QString&, const QString&)),
                      this, SLOT(gotAddedContactToGroup(bool, const QString&, const QString&)) );

    QObject::connect (&m_server->cb, SIGNAL(gotRemovedContactFromGroup(bool, const QString&, const QString&)),
                      this, SLOT(gotRemovedContactFromGroup(bool, const QString&, const QString&)) );

    QObject::connect (&m_server->cb, SIGNAL(gotAddedGroup(bool, const QString&, const QString&)),
                      this, SLOT(gotAddedGroup(bool, const QString&, const QString&)) );

    QObject::connect (&m_server->cb, SIGNAL(gotRemovedGroup(bool, const QString&)),
                      this, SLOT(gotRemovedGroup(bool, const QString&)) );


    QObject::connect (&m_server->cb, SIGNAL(gotAddedContactToAddressBook (bool, const QString&, const QString&, const QString&)),
                      this, SLOT(gotAddedContactToAddressBook(bool, const QString&, const QString&, const QString&)));

    QObject::connect (&m_server->cb, SIGNAL(gotRemovedContactFromAddressBook(const bool&, const QString&, const QString&)),
                      this, SLOT(gotRemovedContactFromAddressBook(bool, const QString&, const QString&)));

    MSN::BuddyStatus state = MSN::STATUS_AVAILABLE;

    if (temporaryStatus == WlmProtocol::protocol ()->wlmOnline)
        state = MSN::STATUS_AVAILABLE;
    else if (temporaryStatus == WlmProtocol::protocol ()->wlmAway)
        state = MSN::STATUS_AWAY;
    else if (temporaryStatus == WlmProtocol::protocol ()->wlmInvisible)
        state = MSN::STATUS_INVISIBLE;
    else if (temporaryStatus == WlmProtocol::protocol ()->wlmBusy)
        state = MSN::STATUS_BUSY;
    else if (temporaryStatus == WlmProtocol::protocol ()->wlmOutToLunch)
        state = MSN::STATUS_OUTTOLUNCH;
    else if (temporaryStatus == WlmProtocol::protocol ()->wlmOnThePhone)
        state = MSN::STATUS_ONTHEPHONE;
    else if (temporaryStatus == WlmProtocol::protocol ()->wlmBeRightBack)
        state = MSN::STATUS_BERIGHTBACK;

    m_server->cb.mainConnection->setState (state, clientid);
    // this prevents our client from downloading display pictures
    // when is just connected.
    QTimer::singleShot (10 * 1000, this, SLOT (disableInitialList ()));
    setPersonalMessage(myself()->statusMessage());

    // download a pending picture every 20 seconds
    m_pendingDisplayPicturesTimer = new QTimer(this);

    QObject::connect(m_pendingDisplayPicturesTimer, SIGNAL(timeout()), 
            this, SLOT(downloadPendingDisplayPicture()));

    m_pendingDisplayPicturesTimer->start(30 * 1000);

    // manage pending list
    foreach ( const QString &contact, pendingList() )
    {
        // if we do not have this contact yet, so ask for add it
        if(!isOnServerSideList(contact))
        {
            // fake this contact in RL to prompt the user to add it
            gotNewContact (MSN::LST_RL, contact, contact);
        }
    }
}

void WlmAccount::downloadPendingDisplayPicture()
{
    if(!m_pendingDisplayPicturesTimer)
        return;

    if (m_pendingDisplayPictureList.isEmpty())
    {
        m_pendingDisplayPicturesTimer->stop();
        m_pendingDisplayPicturesTimer = NULL;
        return;
    }

    QString passport = m_pendingDisplayPictureList.toList().first();
    m_pendingDisplayPictureList.remove(passport);

    WlmContact * contact = qobject_cast<WlmContact*>(contacts ()[passport]);
    if(!contact)
        return;

    // we only download the display picture if we and the contact are online
    if ((myself ()->onlineStatus () != WlmProtocol::protocol ()->wlmOffline)
     && (myself ()->onlineStatus () != WlmProtocol::protocol ()->wlmInvisible)
     && (myself ()->onlineStatus () != WlmProtocol::protocol ()->wlmUnknown)
     && (contact->onlineStatus () != WlmProtocol::protocol ()->wlmOffline)
     && (contact->onlineStatus () != WlmProtocol::protocol ()->wlmInvisible)
     && (contact->onlineStatus () != WlmProtocol::protocol ()->wlmUnknown))
 
    {
        chatManager ()->requestDisplayPicture (passport);
    }
}

void WlmAccount::gotAddedGroup (bool added,
                                const QString & groupName,
                                const QString & groupId)
{
    kDebug() << "groupName: " << groupName << "groupId: " << groupId << " added:" << added;
    const QStringList contactIdList = m_contactAddQueue.keys (groupName);
    if (!added)
    {
        // Remove contact from add queue. FIXME: We should somehow sync the contact list here
        foreach ( const QString &contactId, contactIdList )
            m_contactAddQueue.remove(contactId);

        return;
    }

    // Insert new group
    m_groupToGroupId.insert(groupName, groupId);

    // Add contact to the new group
    foreach ( const QString &contactId, contactIdList )
    {
        kDebug() << "adding contact " << contactId;
        m_server->cb.mainConnection->addToAddressBook (contactId.toAscii().data(), contactId.toAscii().data());
    }
}

void WlmAccount::gotRemovedGroup (bool removed,
                      const QString & groupId)
{
    kDebug() << "groupId: " << groupId << " removed:" << removed;
    if ( !removed )
        return;

    // remove group
    m_groupToGroupId.remove(m_groupToGroupId.key(groupId));
}

void
WlmAccount::gotAddedContactToGroup (bool added,
                        const QString & groupId,
                        const QString & contactId)
{
    kDebug() << "groupId: " << groupId << " contactId: " << contactId << " added:" << added;
}

void
WlmAccount::gotRemovedContactFromGroup (bool removed,
                            const QString & groupId,
                            const QString & contactId)
{
    kDebug() << "groupId: " << groupId << " contactId: " << contactId << " removed:" << removed;
}

void
WlmAccount::gotAddedContactToAddressBook (bool added, const QString & passport, const QString & displayName, const QString & guid)
{
    kDebug() << "contact: " << passport << " added:" << added << " guid: " << guid;
    if (added)
    {
        m_serverSideContactsPassports.insert (passport);
        addContact (passport, QString(), Kopete::Group::topLevel (), Kopete::Account::DontChangeKABC);

        WlmContact * newcontact = qobject_cast <WlmContact *>(contacts ()[passport]);
        if (!newcontact)
            return;

        newcontact->setContactSerial (guid);
        newcontact->setProperty (Kopete::Global::Properties::self()->nickName(), displayName);

        QString groupName = m_contactAddQueue.value (passport);
        if( !groupName.isEmpty() && m_groupToGroupId.contains (groupName.toAscii()) )
        {
            kDebug() << "Adding contact \'" << passport << "\' to group \'" << groupName << "\'";
            QString groupId = m_groupToGroupId.value (groupName.toAscii());
            m_server->cb.mainConnection->addToGroup (groupId.toAscii().data(), guid.toAscii().data());
        }
    }
    else
    {
        // TODO: Raise an error
    }

    // Remove contact from add queue
    m_contactAddQueue.remove(passport);
}

void
WlmAccount::gotRemovedContactFromAddressBook (bool removed, const QString & passport, const QString & contactId)
{
    Q_UNUSED( contactId );

    kDebug() << "contact: " << passport << " removed:" << removed;
    if (removed)
        m_serverSideContactsPassports.remove( passport );

}

void
WlmAccount::NotificationServerConnectionTerminated (MSN::
                                                    NotificationServerConnection
                                                    * conn)
{
    Q_UNUSED( conn );

    kDebug (14210) << k_funcinfo;

    if (myself ()->onlineStatus () == WlmProtocol::protocol ()->wlmConnecting
        && !password ().isWrong ())
    {
        connectionFailed ();
        return;
    }
    if (password ().isWrong ())
    {
        myself ()->setOnlineStatus (WlmProtocol::protocol ()->wlmOffline);
        QTimer::singleShot (2 * 1000, this, SLOT (scheduleConnect ()));
        return;
    }
    if (isConnected ())
    {
        myself ()->setOnlineStatus (WlmProtocol::protocol ()->wlmOffline);
    }
}

void
WlmAccount::disconnect ()
{
    kDebug (14210) << k_funcinfo;
    if (m_server)
        m_server->WlmDisconnect ();

    myself ()->setOnlineStatus (WlmProtocol::protocol ()->wlmOffline);

    QObject::disconnect (Kopete::ContactList::self (), 0, 0, 0);
    QObject::disconnect (Kopete::TransferManager::transferManager (), 0, 0, 0);

    if (m_transferManager)
    {
        delete m_transferManager;
        m_transferManager = NULL;
    }
    if (m_chatManager)
    {
        delete m_chatManager;
        m_chatManager = NULL;
    }
    if (m_server)
    {
        QObject::disconnect (&m_server->cb, 0, 0, 0);
        delete m_server;
        m_server = NULL;
    }
}

WlmServer *
WlmAccount::server ()
{
    return m_server;
}

bool WlmAccount::isBlocked(const QString& passport) const
{
    return (isOnBlockList(passport) || (blockUnknownUsers() && !isOnAllowList(passport)));
}

void WlmAccount::blockContact(const QString& passport, bool block)
{
    if (!isConnected() || isBlocked(passport) == block)
        return;

    if (block)
    {
        if (isOnAllowList(passport))
            server()->mainConnection->removeFromList(MSN::LST_AL, passport.toAscii().data());

        server()->mainConnection->addToList(MSN::LST_BL, passport.toAscii().data());
    }
    else
    {
        if (isOnBlockList(passport))
            server()->mainConnection->removeFromList(MSN::LST_BL, passport.toAscii().data());

        server()->mainConnection->addToList(MSN::LST_AL, passport.toAscii().data());
    }
}

void
WlmAccount::slotGoOnline ()
{
    kDebug (14210) << k_funcinfo;

    if (!isConnected ())
        connect (WlmProtocol::protocol ()->wlmOnline);
    else
        m_server->cb.mainConnection->setState (MSN::STATUS_AVAILABLE,
                                               clientid);
}

void
WlmAccount::slotGoInvisible ()
{
    kDebug (14210) << k_funcinfo;

    if (!isConnected ())
        connect (WlmProtocol::protocol ()->wlmInvisible);
    else
        m_server->cb.mainConnection->setState (MSN::STATUS_INVISIBLE,
                                               clientid);
}

void
WlmAccount::slotGoAway (const Kopete::OnlineStatus & status)
{
    kDebug (14210) << k_funcinfo;

    if (!isConnected ())
        connect (status);
    else
    {
        if (status == WlmProtocol::protocol ()->wlmIdle)
            m_server->cb.mainConnection->setState (MSN::STATUS_IDLE,
                                                   clientid);
        if (status == WlmProtocol::protocol ()->wlmAway)
            m_server->cb.mainConnection->setState (MSN::STATUS_AWAY,
                                                   clientid);
        else if (status == WlmProtocol::protocol ()->wlmOutToLunch)
            m_server->cb.mainConnection->setState (MSN::STATUS_OUTTOLUNCH,
                                                   clientid);
        else if (status == WlmProtocol::protocol ()->wlmBusy)
            m_server->cb.mainConnection->setState (MSN::STATUS_BUSY,
                                                   clientid);
        else if (status == WlmProtocol::protocol ()->wlmOnThePhone)
            m_server->cb.mainConnection->setState (MSN::STATUS_ONTHEPHONE,
                                                   clientid);
        else if (status == WlmProtocol::protocol ()->wlmBeRightBack)
            m_server->cb.mainConnection->setState (MSN::STATUS_BERIGHTBACK,
                                                   clientid);
    }
}

void
WlmAccount::slotGoOffline ()
{
    kDebug (14210) << k_funcinfo;

    if (isConnected ()
        || myself ()->onlineStatus ().status () ==
        Kopete::OnlineStatus::Connecting)
        disconnect ();
    myself ()->setOnlineStatus (WlmProtocol::protocol ()->wlmOffline);

    foreach ( Kopete::Contact *kc , contacts() )
    {
        WlmContact *c = static_cast<WlmContact *>( kc );
        c->setOnlineStatus (WlmProtocol::protocol ()->wlmOffline);
    }
}

void
WlmAccount::updateContactStatus ()
{
}

void
WlmAccount::receivedOIMList (std::vector < MSN::eachOIM > &oimlist)
{
    kDebug (14210) << k_funcinfo;
    std::vector < MSN::eachOIM >::iterator i = oimlist.begin ();
    for (; i != oimlist.end (); i++)
    {
        m_oimList[(*i).id.c_str ()] = (*i).from.c_str ();
        m_server->cb.mainConnection->get_oim ((*i).id.c_str(), true);
    }
}

void
WlmAccount::deletedOIM(const QString& id, const bool deleted)
{
    kDebug() << " deleted OIM " << id << " " << deleted;
}

void
WlmAccount::receivedOIM (const QString & id, const QString & message)
{
    kDebug (14210) << k_funcinfo;
    QString contactId = m_oimList[id];
    WlmContact * contact = qobject_cast<WlmContact*>(contacts ()[contactId]);

    Kopete::Message msg = Kopete::Message (contact, myself ());
    msg.setPlainBody (message);
    msg.setDirection (Kopete::Message::Inbound);

    if (contact)
        contact->manager (Kopete::Contact::CanCreate)->appendMessage (msg);

    m_oimList.remove (id);
    m_server->cb.mainConnection->delete_oim (id.toLatin1 ().data ());
}

#include "wlmaccount.moc"
