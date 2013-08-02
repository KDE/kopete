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
#include <QFileInfo>
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
clientid (0),
m_lastMainConnectionError(Callbacks::NoError)
{
    // Init the myself contact
    setMyself (new
               WlmContact (this, accountId (), accountId (),
                           Kopete::ContactList::self ()->myself ()));
    myself ()->setOnlineStatus (WlmProtocol::protocol ()->wlmOffline);
    clientid += MSN::MSNC7;
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

    tmpMailFile = 0L;
    m_tmpMailFileTimer = new QTimer();
    QObject::connect(m_tmpMailFileTimer, SIGNAL(timeout()), this, SLOT(slotRemoveTmpMailFile()));
}

WlmAccount::~WlmAccount ()
{
    slotRemoveTmpMailFile();
    delete m_tmpMailFileTimer;
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

    actionMenu->addAction(m_openInboxAction);
    actionMenu->addAction(m_openStatusAction);
}

bool
WlmAccount::createContact (const QString & contactId,
                           Kopete::MetaContact * parentContact)
{
    if ( !m_server )
        return false;

    kDebug() << "contact " << contactId;
    WlmContact *newContact = new WlmContact (this, contactId, QString(), parentContact);

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

    m_contactAddQueue.insert(contactId, groupName);
	if (!m_groupToGroupId.contains(groupName))
    {
        kDebug() << "group \'" << groupName << "\' not found adding group";
        m_server->cb.mainConnection->addGroup (groupName.toUtf8().constData());
    }
    else
    {
        kDebug() << "group \'" << groupName << "\' found adding contact";
        m_server->cb.mainConnection->addToAddressBook (contactId.toLatin1().constData(), contactId.toUtf8().constData());
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
        if (reason.message().isEmpty ())
            pInfo.PSM = "";
        else
            pInfo.PSM = reason.message().toUtf8().constData();

        // we have both artist and title
        if( reason.hasMetaData("artist") && reason.hasMetaData("title") )
        {
            pInfo.mediaIsEnabled = 1;
            pInfo.mediaType="Music";
            pInfo.mediaLines.push_back( reason.metaData("artist").toString().toUtf8().constData() );
            pInfo.mediaLines.push_back( reason.metaData("title").toString().toUtf8().constData() );
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
            pInfo.mediaLines.push_back( reason.metaData("title").toString().toUtf8().constData() );
            m_server->cb.mainConnection->setPersonalStatus (pInfo);
            return;
        }
        m_server->cb.mainConnection->setPersonalStatus (pInfo);
    }
}

void
WlmAccount::setOnlineStatus (const Kopete::OnlineStatus & status,
                             const Kopete::StatusMessage & reason,
                             const OnlineStatusOptions& /*options*/)
{
    kDebug (14210) << k_funcinfo;

    setPersonalMessage(reason);

    temporaryStatus = status;

    if (status == WlmProtocol::protocol ()->wlmConnecting &&
        myself ()->onlineStatus () == WlmProtocol::protocol ()->wlmOffline)
        slotGoOnline ();
    else if (status == WlmProtocol::protocol ()->wlmOnline || status.status () == Kopete::OnlineStatus::Online)
        slotGoOnline ();
    else if (status == WlmProtocol::protocol ()->wlmOffline)
        slotGoOffline ();
    else if (status == WlmProtocol::protocol ()->wlmInvisible)
        slotGoInvisible ();
    else if (status.status () == Kopete::OnlineStatus::Away ||
              status.status () == Kopete::OnlineStatus::Busy)
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
        myself()->displayName(), &ok );

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

        m_server->cb.mainConnection->setFriendlyName(name.toUtf8().constData(), true);
    }
}

void
WlmAccount::slotOpenInbox()
{
    if (isConnected ())
        m_server->cb.mainConnection->getInboxUrl ();
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
        // User has cancelled password prompt.
        return;
    }

    password ().setWrong (false);

    QString id = accountId ();
    QString pass1 = pass;

    enableInitialList ();

    m_lastMainConnectionError = Callbacks::NoError;
    m_server = new WlmServer (this, id, pass1);
    m_server->WlmConnect ( serverName(), serverPort() );

    m_transferManager = new WlmTransferManager (this);

    m_chatManager = new WlmChatManager (this);

    QObject::connect (&m_server->cb, SIGNAL (connectionCompleted()),
                      this, SLOT (connectionCompleted()));
    QObject::connect (&m_server->cb, SIGNAL (connectionFailed()),
                      this, SLOT (connectionFailed()));
    QObject::connect (&m_server->cb, SIGNAL(socketError(int)),
                      this, SLOT(error(int)));
    QObject::connect (&m_server->cb, SIGNAL(mainConnectionError(int)),
                      this, SLOT(mainConnectionError(int)));
    QObject::connect (&m_server->cb,
                      SIGNAL (gotDisplayName(QString)), this,
                      SLOT (gotDisplayName(QString)));
    QObject::connect (&m_server->cb,
                      SIGNAL (receivedOIMList
                              (std::vector < MSN::eachOIM > &)), this,
                      SLOT (receivedOIMList
                            (std::vector < MSN::eachOIM > &)));
    QObject::connect (&m_server->cb,
                      SIGNAL (receivedOIM(QString,QString)),
                      this,
                      SLOT (receivedOIM(QString,QString)));

    QObject::connect (&m_server->cb,
                      SIGNAL (deletedOIM(QString,bool)), this,
                      SLOT (deletedOIM(QString,bool)));
    QObject::connect (&m_server->cb,
                      SIGNAL (NotificationServerConnectionTerminated
                              (MSN::NotificationServerConnection *)), this,
                      SLOT (NotificationServerConnectionTerminated
                            (MSN::NotificationServerConnection *)));
    QObject::connect (&m_server->cb, SIGNAL (initialEmailNotification(int)), this,
                      SLOT (slotInitialEmailNotification(int)));
    QObject::connect (&m_server->cb, SIGNAL (newEmailNotification(QString,QString)), this,
                      SLOT (slotNewEmailNotification(QString,QString)));
    QObject::connect (&m_server->cb, SIGNAL (inboxUrl(MSN::hotmailInfo&)), this,
                      SLOT (slotInboxUrl(MSN::hotmailInfo&)));

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

QString WlmAccount::proxyUsername() const
{
    return configGroup()->readEntry( "proxyUsername" );
}

QString WlmAccount::proxyPassword() const
{
    return configGroup()->readEntry( "proxyPassword" );
}

QString WlmAccount::proxyHost() const
{
    return configGroup()->readEntry( "proxyHost" );
}

uint WlmAccount::proxyType() const
{
    return configGroup()->readEntry( "proxyType", 0 );
}

uint WlmAccount::proxyPort() const
{
    return configGroup()->readEntry( "proxyPort", 8080 );
}

bool WlmAccount::isProxyEnabled() const
{
    return configGroup()->readEntry( "enableProxy", false );
}

bool WlmAccount::doNotRequestEmoticons() const
{
    return configGroup()->readEntry( "doNotRequestEmoticons", false );
}

bool WlmAccount::doNotSendEmoticons() const
{
    return configGroup()->readEntry( "doNotSendEmoticons", false );
}

void
WlmAccount::gotNewContact (const MSN::ContactList & list,
                           const QString & passport,
                           const QString & friendlyname)
{
    kDebug() << "contact " << passport;
    if (list == MSN::LST_RL)
    {
        kDebug() << "contact " << passport << " added to reverse list";
        m_reverseList.insert(passport);
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
    else if (list == MSN::LST_RL)
    {
        kDebug() << "contact " << contact << " removed from reverse list";
        m_reverseList.remove( contact );
        // force overlayIcons to be updated
	    WlmContact * ct = qobject_cast<WlmContact*>(contacts().value( contact ));
        if(ct)
            ct->setOnlineStatus(ct->onlineStatus());
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
        if (isOnAllowList(event->contactId()))
            server()->mainConnection->removeFromList(MSN::LST_AL, event->contactId().toLatin1().constData());
        if(!isOnBlockList(event->contactId()))
            server()->mainConnection->addToList(MSN::LST_BL, event->contactId().toLatin1().constData());
        break;
/*    case Kopete::AddedInfoEvent::InfoAction:
        break;*/
    }
}

void
WlmAccount::mainConnectionError(int errorCode)
{
    kDebug (14210) << k_funcinfo;
    m_lastMainConnectionError = errorCode;
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
    // FIXME: Why we get local file and not just QByteArray data?
    kDebug (14210) << k_funcinfo;
    WlmContact * contact = qobject_cast<WlmContact*>(contacts().value(contactId));
    if (contact)
    {
        // remove from pending display pictures list if applicable
        m_pendingDisplayPictureList.remove(contactId);

        // check file integrity (SHA1D)
        QDomDocument xmlobj;
        xmlobj.setContent (contact->getMsnObj());
        QString SHA1D_orig = xmlobj.documentElement ().attribute ("SHA1D");

        QFile f(filename);
        QByteArray avatarData;
        if (f.exists() && f.size() > 0 && f.open(QIODevice::ReadOnly))
        {
            avatarData = f.readAll();
            f.close();
        }
        QFile::remove(filename);

        if (!avatarData.isEmpty() && !SHA1D_orig.isEmpty() &&
            SHA1D_orig == QCryptographicHash::hash(avatarData, QCryptographicHash::Sha1).toBase64())
        {
            QImage img;
            img.loadFromData(avatarData);

            Kopete::AvatarManager::AvatarEntry entry;
            entry.name = contact->contactId();
            entry.category = Kopete::AvatarManager::Contact;
            entry.contact = contact;
            entry.image = img;
            entry = Kopete::AvatarManager::self()->add(entry);
            if (!entry.dataPath.isNull())
            {
                contact->removeProperty(Kopete::Global::Properties::self()->photo());
                contact->setProperty(Kopete::Global::Properties::self()->photo(), entry.dataPath);
                contact->setProperty(WlmProtocol::protocol()->displayPhotoSHA1, SHA1D_orig);
            }
        }
        else
        {
            contact->removeProperty(WlmProtocol::protocol()->displayPhotoSHA1);
            contact->removeProperty(Kopete::Global::Properties::self()->photo());
        }
    }
}

void
WlmAccount::gotDisplayName (const QString & displayName)
{
    kDebug (14210) << k_funcinfo;
    myself ()->setNickName (displayName);
}

void
WlmAccount::gotContactPersonalInfo (const QString & fromPassport,
                                    const MSN::personalInfo & pInfo)
{
    kDebug (14210) << k_funcinfo;
	WlmContact * contact = qobject_cast<WlmContact*>(contacts().value(fromPassport));
    if (contact)
    {
        // TODO - handle the other fields of pInfo
        contact->setStatusMessage(Kopete::StatusMessage(WlmUtils::utf8(pInfo.PSM)));
        QString type(WlmUtils::utf8(pInfo.mediaType));
        if (pInfo.mediaIsEnabled && type == "Music")
        {
            QString song_line (WlmUtils::utf8(pInfo.mediaFormat));
            int num = pInfo.mediaLines.size ();
            for (int i = 0; i < num; i++)
            {
                song_line.replace ('{' + QString::number (i) + '}', WlmUtils::utf8(pInfo.mediaLines[i]));
            }
            contact->setProperty (WlmProtocol::protocol ()->currentSong, song_line);
        }
        else
        {
            contact->removeProperty (WlmProtocol::protocol ()->currentSong);
        }
    }
}

void
WlmAccount::contactChangedStatus (const QString & buddy,
                                  const QString & friendlyname,
                                  const MSN::BuddyStatus & state,
                                  const unsigned int &clientID,
                                  const QString & msnobject)
{
    kDebug (14210) << k_funcinfo;
	WlmContact *contact = qobject_cast<WlmContact*>(contacts().value(buddy));
    if (contact)
    {
        contact->setNickName (friendlyname);

        // set contact properties
        contact->setProperty (WlmProtocol::protocol ()->contactCapabilities, QString::number(clientID));

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

        qobject_cast <WlmContact *>(contact)->setMsnObj(msnobject);

        if (msnobject.isEmpty () || msnobject == "0")   // no picture
        {
            contact->removeProperty(WlmProtocol::protocol()->displayPhotoSHA1);
            contact->removeProperty(Kopete::Global::Properties::self()->photo());
            return;
        }

        QDomDocument xmlobj;
        xmlobj.setContent (msnobject);

        // track display pictures by SHA1D field
        QString SHA1D = xmlobj.documentElement ().attribute ("SHA1D");

        if (SHA1D.isEmpty ())
            return;

        QString currentSHA1D = contact->property(WlmProtocol::protocol()->displayPhotoSHA1).value().toString();
        QString photoPath = contact->property(Kopete::Global::Properties::self()->photo().key()).value().toString();
        if (SHA1D == currentSHA1D && QFileInfo(photoPath).size() > 0)
            return;

        // do not request all pictures at once when you are just connected
        if (isInitialList ())
        {
            // schedule to retrieve this picture later
            m_pendingDisplayPictureList.insert(buddy);
            return;
        }

        if ((myself ()->onlineStatus () !=
                WlmProtocol::protocol ()->wlmOffline)
               && (myself ()->onlineStatus () !=
                   WlmProtocol::protocol ()->wlmInvisible)
               && (myself ()->onlineStatus () !=
                   WlmProtocol::protocol ()->wlmUnknown))
        {
            // do not open many switchboards in a short period of time
            if(!m_recentDPRequests.contains(buddy))
            {
                m_recentDPRequests.append(buddy);
                QTimer::singleShot(10 * 1000, this, SLOT(slotRemoveRecentDPRequests()));
                chatManager ()->requestDisplayPicture(buddy);
            }
        }
    }
}

void
WlmAccount::contactDisconnected (const QString & buddy)
{
    kDebug (14210) << k_funcinfo;
	WlmContact * contact = qobject_cast<WlmContact*>(contacts().value(buddy));
    if (contact)
    {
        contact->setOnlineStatus (WlmProtocol::protocol ()->wlmOffline);
    }
}

void
WlmAccount::groupListReceivedFromServer (std::map < std::string, MSN::Group > &list)
{
    kDebug (14210) << k_funcinfo;
    // add server groups on local list
    std::map < std::string, MSN::Group >::iterator it;
    for (it = list.begin (); it != list.end (); ++it)   // groups from server
    {
        MSN::Group * g = &(*it).second;

        QString gName = WlmUtils::utf8(g->name);
        Kopete::Group * b = Kopete::ContactList::self ()->findGroup(gName);
        if (!b)
        {
            b = new Kopete::Group(gName);
            Kopete::ContactList::self ()->addGroup( b );
        }

        m_groupToGroupId.insert(gName, WlmUtils::latin1(g->groupID));
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
			if(Kopete::ContactList::self()->findGroup(WlmUtils::utf8(g->name)))
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
    m_pendingList.clear();
    m_reverseList.clear();

    // local contacts which do not exist on server should be deleted
    std::map < std::string, MSN::Buddy * >::iterator it;
    for (it = list.begin (); it != list.end (); ++it)
    {
        Kopete::MetaContact * metacontact = 0L;
        MSN::Buddy * b = (*it).second;
        QString passport = WlmUtils::passport(b->userName);

        if (b->lists & MSN::LST_AB)
            m_serverSideContactsPassports.insert(passport);
        if ( b->lists & MSN::LST_AL )
            m_allowList.insert( passport );
        if ( b->lists & MSN::LST_BL )
            m_blockList.insert( passport );
        if ( b->lists & MSN::LST_PL )
            m_pendingList.insert( passport );
        if ( b->lists & MSN::LST_RL )
            m_reverseList.insert(passport);

        // disabled users (not in list)
        if(b->properties["isMessengerUser"] == "false")
        {
            // disable this contact
	        WlmContact *contact = qobject_cast<WlmContact*>(contacts().value( passport ));
            if (!contact)
            {
                addContact (passport, QString(), Kopete::Group::topLevel (), Kopete::Account::DontChangeKABC);
	            contact = qobject_cast<WlmContact*>(contacts().value( passport ));
            }
            if(contact)
            {
                contact->setContactSerial(WlmUtils::latin1(b->properties["contactId"]));
                contact->setCurrentGroup(Kopete::Group::topLevel());
                contact->setDisabled(true, false);
            }
            continue;
        }

	    if ( !contacts().value( passport ) )
        {
            if (!b->friendlyName.length ())
                b->friendlyName = b->userName;

            std::list < MSN::Group * >::iterator i = b->groups.begin ();

            // no groups, add to top level
            if (!b->groups.size ())
            {
                // only add users in forward list
                if (b->lists & MSN::LST_AB)
                {
                    metacontact = addContact (passport, QString(), Kopete::Group::topLevel (), Kopete::Account::DontChangeKABC);

                    WlmContact * newcontact = qobject_cast<WlmContact*>(contacts().value(passport));
                    if(!newcontact)
                        return;

                    newcontact->setNickName (WlmUtils::utf8(b->friendlyName));
                }

                if (metacontact)
                {
	                WlmContact *contact = qobject_cast<WlmContact*>(contacts().value( passport ));
                    if (contact)
                    {
                        contact->setContactSerial(WlmUtils::latin1(b->properties["contactId"]));
                        kDebug (14210) << "ContactID: " << WlmUtils::latin1(b->properties["contactId"]);
                    }
                }
                continue;
            }

            for (; i != b->groups.end (); ++i)
            {
                Kopete::Group * g = Kopete::ContactList::self ()->findGroup (WlmUtils::utf8((*i)->name));

                if (g)
                    metacontact = addContact (passport, QString(), g, Kopete::Account::DontChangeKABC);
                else
                    metacontact = addContact (passport, QString(), Kopete::Group::topLevel (), Kopete::Account::DontChangeKABC);

                if (metacontact)
                {
	                WlmContact *contact = qobject_cast<WlmContact*>(contacts().value( passport ));
                    if (contact)
                    {
                        contact->setNickName(WlmUtils::utf8(b->friendlyName));
                        contact->setContactSerial(WlmUtils::latin1(b->properties["contactId"]));
                        kDebug (14210) << "ContactID: " << WlmUtils::latin1(b->properties["contactId"]);
                    }
                }
            }
        }
        else
        {
            // check if this contact has changed groups while we were offline.
            // users in the toplevel group on server side
            if( b->groups.size() == 0)
            {
	            Kopete::Group *current = contacts().value( passport )->metaContact()->groups().first();
                // the contact has no group, so put it on top level group
                if(current != Kopete::Group::topLevel ())
                {
	                contacts().value( passport )->metaContact()->
                        moveToGroup(current, Kopete::Group::topLevel ());
	                qobject_cast<WlmContact*>(contacts().value( passport ))->setCurrentGroup(Kopete::Group::topLevel());
                }
                continue;
            }
            // users in only one group on server side
            if( b->groups.size() == 1)
            {
                // users in only one group in the local list
                if(contacts().value( passport )->metaContact()->groups().size() == 1) 
                {
	                Kopete::Group *current = contacts().value( passport )->metaContact()->groups().first();
                    Kopete::Group *newgroup = Kopete::ContactList::self ()->findGroup(WlmUtils::utf8(b->groups.front()->name));

                    if(!current || !newgroup)
                        continue;

                    if(current != newgroup)
                    {
	                    contacts().value( passport )->metaContact()->moveToGroup(current, newgroup);
	                    qobject_cast<WlmContact*>(contacts().value( passport ))->setCurrentGroup(newgroup);
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
                m_server->cb.mainConnection->change_DisplayPicture(QFile::encodeName(entry.path).constData());

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
            if(m_server && isConnected())
                m_server->cb.mainConnection->setFriendlyName (newNick.toUtf8().constData());
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
    logOff( Kopete::Account::Unknown );
    Kopete::Utils::notifyCannotConnect (this);
}

void
WlmAccount::connectionCompleted ()
{
    kDebug (14210) << k_funcinfo;

    // set all users as offline
    foreach ( Kopete::Contact *kc , contacts() )
    {
        WlmContact *c = static_cast<WlmContact *>( kc );
        c->setOnlineStatus (WlmProtocol::protocol ()->wlmOffline);
    }

    if (identity ()->hasProperty (Kopete::Global::Properties::self ()->photo ().key()))
    {
        m_server->cb.mainConnection->change_DisplayPicture(QFile::encodeName(identity()->customIcon()).constData());
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
        m_server->cb.mainConnection->setFriendlyName(nick.toUtf8().constData());
    }
    else
    {
        // Set myself contact display name here
        // This information come along with the address book
        // Fix BUG 182366
        m_server->cb.mainConnection->setFriendlyName( m_server->mainConnection->myDisplayName );
    }

    password ().setWrong (false);

    QObject::connect (&m_server->cb,
                      SIGNAL (changedStatus(MSN::BuddyStatus&)), this,
                      SLOT (changedStatus(MSN::BuddyStatus&)));

    QObject::connect (&m_server->cb,
                      SIGNAL (contactChangedStatus
                              (const QString &, const QString &,
                               const MSN::BuddyStatus &, const unsigned int &,
                               const QString &)), this,
                      SLOT (contactChangedStatus
                            (const QString &, const QString &,
                             const MSN::BuddyStatus &, const unsigned int &,
                             const QString &)));

    QObject::connect (&m_server->cb,
                      SIGNAL (contactDisconnected(QString)),
                      this,
                      SLOT (contactDisconnected(QString)));

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
                              (const QString &,
                               const MSN::personalInfo &)),
                      SLOT (gotContactPersonalInfo
                            (const QString &,
                             const MSN::personalInfo &)));

    QObject::connect (&m_server->cb, SIGNAL(gotNewContact(MSN::ContactList,QString,QString)),
                      SLOT (gotNewContact(MSN::ContactList,QString,QString)));

    QObject::connect (&m_server->cb, SIGNAL(gotRemovedContactFromList(MSN::ContactList,QString)),
                      this, SLOT(gotRemovedContactFromList(MSN::ContactList,QString)) );

    QObject::connect (&m_server->cb, SIGNAL(gotAddedContactToGroup(bool,QString,QString)),
                      this, SLOT(gotAddedContactToGroup(bool,QString,QString)) );

    QObject::connect (&m_server->cb, SIGNAL(gotRemovedContactFromGroup(bool,QString,QString)),
                      this, SLOT(gotRemovedContactFromGroup(bool,QString,QString)) );

    QObject::connect (&m_server->cb, SIGNAL(gotAddedGroup(bool,QString,QString)),
                      this, SLOT(gotAddedGroup(bool,QString,QString)) );

    QObject::connect (&m_server->cb, SIGNAL(gotRemovedGroup(bool,QString)),
                      this, SLOT(gotRemovedGroup(bool,QString)) );


    QObject::connect (&m_server->cb, SIGNAL(gotAddedContactToAddressBook(bool,QString,QString,QString)),
                      this, SLOT(gotAddedContactToAddressBook(bool,QString,QString,QString)));

    QObject::connect (&m_server->cb, SIGNAL(gotRemovedContactFromAddressBook(bool,QString,QString)),
                      this, SLOT(gotRemovedContactFromAddressBook(bool,QString,QString)));

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
    QTimer::singleShot (10 * 1000, this, SLOT (disableInitialList()));
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
        if(!isOnServerSideList(contact) &&
                !isOnAllowList(contact) &&
                !isOnBlockList(contact))
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
        m_pendingDisplayPicturesTimer->deleteLater();
        m_pendingDisplayPicturesTimer = NULL;
        return;
    }

    QString passport = m_pendingDisplayPictureList.toList().first();
    m_pendingDisplayPictureList.remove(passport);

	WlmContact * contact = qobject_cast<WlmContact*>(contacts().value(passport));
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
        // do not open many switchboards in a short period of time
        if(!m_recentDPRequests.contains(passport))
        {
            m_recentDPRequests.append(passport);
            QTimer::singleShot(10 * 1000, this, SLOT(slotRemoveRecentDPRequests()));
            chatManager ()->requestDisplayPicture (passport);
        }
    }
}

void
WlmAccount::slotInitialEmailNotification (const int unread_inbox)
{
    if ( isBusy() )
        return;

    KNotification *notification= new KNotification ("msn_mail", Kopete::UI::Global::mainWidget());

    notification->setText(i18np( "You have one unread message in your Hotmail inbox.",
                                 "You have %1 unread messages in your Hotmail inbox.", unread_inbox));
    notification->setActions(( QStringList() << i18nc("@action", "Open Inbox" ) << i18nc("@action", "Close" )) );
    notification->setFlags(KNotification::Persistent);
    notification->setPixmap(accountIcon(KIconLoader::SizeMedium));
    QObject::connect(notification,SIGNAL(activated()), this , SLOT(slotOpenInbox()) );
    QObject::connect(notification,SIGNAL(action1Activated()), this, SLOT(slotOpenInbox()) );
    QObject::connect(notification,SIGNAL(action2Activated()), notification, SLOT(close()) );
    QObject::connect(notification,SIGNAL(ignored()), notification, SLOT(close()) );
    notification->sendEvent();
}

void
WlmAccount::slotNewEmailNotification (const QString from, const QString subject)
{
    if ( isBusy() )
        return;

    KNotification *notification= new KNotification ("msn_mail", Kopete::UI::Global::mainWidget());

    notification->setText(i18n( "New message from %1 in your Hotmail inbox.<p>Subject: %2", from, subject));
    notification->setActions(( QStringList() << i18nc("@action", "Open Inbox" ) << i18nc("@action", "Close" )) );
    notification->setFlags(KNotification::Persistent);
    notification->setPixmap(accountIcon(KIconLoader::SizeMedium));
    QObject::connect(notification,SIGNAL(activated()), this , SLOT(slotOpenInbox()) );
    QObject::connect(notification,SIGNAL(action1Activated()), this, SLOT(slotOpenInbox()) );
    QObject::connect(notification,SIGNAL(action2Activated()), notification, SLOT(close()) );
    QObject::connect(notification,SIGNAL(ignored()), notification, SLOT(close()) );
    notification->sendEvent();
}

void
WlmAccount::slotInboxUrl (MSN::hotmailInfo & info)
{
    //write the tmp file
    QString UserID = accountId();

    QString hotmailRequest = "<html>\n"
        "<head>\n"
            "<noscript>\n"
                "<meta http-equiv=Refresh content=\"0; url=http://www.hotmail.com\">\n"
            "</noscript>\n"
        "</head>\n"
        "<body onload=\"document.pform.submit(); \">\n"
            "<form name=\"pform\" action=\"" + WlmUtils::utf8(info.url) + "\" method=\"POST\">\n"
                "<input type=\"hidden\" name=\"mode\" value=\"ttl\">\n"
                "<input type=\"hidden\" name=\"login\" value=\"" + UserID.left( UserID.indexOf('@') ) + "\">\n"
                "<input type=\"hidden\" name=\"username\" value=\"" + UserID + "\">\n"
                "<input type=\"hidden\" name=\"sid\" value=\"" + WlmUtils::utf8(info.sid) + "\">\n"
                "<input type=\"hidden\" name=\"kv\" value=\"" + WlmUtils::utf8(info.kv) + "\">\n"
                "<input type=\"hidden\" name=\"id\" value=\""+ WlmUtils::utf8(info.id) +"\">\n"
                "<input type=\"hidden\" name=\"sl\" value=\"" + WlmUtils::utf8(info.sl) +"\">\n"
                "<input type=\"hidden\" name=\"rru\" value=\"" + WlmUtils::utf8(info.rru) + "\">\n"
                "<input type=\"hidden\" name=\"auth\" value=\"" + WlmUtils::utf8(info.MSPAuth) + "\">\n"
                "<input type=\"hidden\" name=\"creds\" value=\"" + WlmUtils::utf8(info.creds) + "\">\n"
                "<input type=\"hidden\" name=\"svc\" value=\"mail\">\n"
                "<input type=\"hidden\" name=\"js\" value=\"yes\">\n"
            "</form></body>\n</html>\n";

    slotRemoveTmpMailFile();
    tmpMailFile = new KTemporaryFile();
    tmpMailFile->setSuffix(".html");

    if (tmpMailFile->open())
    {
	    tmpMailFile->write(hotmailRequest.toUtf8());
        tmpMailFile->flush();

        /* tmpMailFile->close() erases tmpMailFile->fileName property(), so use it before closing file. */
        KToolInvocation::invokeBrowser( tmpMailFile->fileName(), "0" ); // "0" means we don't need startup notification
        tmpMailFile->close();
        m_tmpMailFileTimer->start(30000);
        m_tmpMailFileTimer->setSingleShot(true);
    }
    else
        kDebug(14140) << "Error opening temporary file";
}

void WlmAccount::slotRemoveTmpMailFile()
{
    if (tmpMailFile)
    {
        delete tmpMailFile;
        tmpMailFile = 0L;
    }

    m_tmpMailFileTimer->stop();
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
        m_server->cb.mainConnection->addToAddressBook (contactId.toLatin1().constData(), contactId.toUtf8().constData());
    }

    // Sync contact belonging to the new group
    foreach ( Kopete::Contact *kc , contacts() )
    {
        WlmContact *c = static_cast<WlmContact *>( kc );
        if ( c->metaContact()->groups().first()->displayName() == groupName )
            c->sync( Kopete::Contact::MovedBetweenGroup );
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

	    WlmContact * newcontact = qobject_cast <WlmContact *>(contacts().value(passport));
        if (!newcontact)
            return;

        newcontact->setContactSerial (guid);
        newcontact->setNickName (displayName);

        QString groupName = m_contactAddQueue.value (passport);
        if( !groupName.isEmpty() && m_groupToGroupId.contains (groupName) )
        {
            kDebug() << "Adding contact \'" << passport << "\' to group \'" << groupName << "\'";
            QString groupId = m_groupToGroupId.value (groupName);
            m_server->cb.mainConnection->addToGroup (groupId.toLatin1().constData(), guid.toLatin1().constData());
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

    if (m_lastMainConnectionError == Callbacks::WrongPassword)
        logOff( Kopete::Account::BadPassword );
    else if (m_lastMainConnectionError == Callbacks::OtherClient)
        logOff( Kopete::Account::OtherClient );
    else if (myself ()->onlineStatus () == WlmProtocol::protocol ()->wlmConnecting)
        connectionFailed ();
    else if (isConnected ())
        logOff( Kopete::Account::Unknown );
}

void WlmAccount::disconnect()
{
    logOff( Kopete::Account::Manual );
}

void WlmAccount::error( int /*errCode*/ )
{
    logOff( Kopete::Account::ConnectionReset );
}


void WlmAccount::logOff( Kopete::Account::DisconnectReason reason )
{
    kDebug (14210) << k_funcinfo;
    if (m_server)
        m_server->WlmDisconnect ();

    if (myself ())
        myself ()->setOnlineStatus (WlmProtocol::protocol ()->wlmOffline);

    foreach ( Kopete::Contact *kc , contacts() )
    {
        WlmContact *c = static_cast<WlmContact *>( kc );
        c->setOnlineStatus (WlmProtocol::protocol ()->wlmOffline);
    }

    delete m_transferManager;
    m_transferManager = NULL;
    delete m_chatManager;
    m_chatManager = NULL;
    if (m_server)
    {
        QObject::disconnect (&m_server->cb, 0, 0, 0);
        m_server->deleteLater();
        m_server = NULL;
    }

    disconnected( reason );
}

WlmServer *WlmAccount::server ()
{
    return m_server;
}

bool WlmAccount::isContactBlocked(const QString& passport) const
{
    return (isOnBlockList(passport) || (blockUnknownUsers() && !isOnAllowList(passport)));
}

void WlmAccount::blockContact(const QString& passport, bool block)
{
    if (!isConnected() || isContactBlocked(passport) == block)
        return;

    if (block)
    {
        if (isOnAllowList(passport))
            server()->mainConnection->removeFromList(MSN::LST_AL, passport.toLatin1().constData());

        server()->mainConnection->addToList(MSN::LST_BL, passport.toLatin1().constData());
    }
    else
    {
        if (isOnBlockList(passport))
            server()->mainConnection->removeFromList(MSN::LST_BL, passport.toLatin1().constData());

        server()->mainConnection->addToList(MSN::LST_AL, passport.toLatin1().constData());
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

    if ( isConnected() || myself ()->onlineStatus ().status () == Kopete::OnlineStatus::Connecting )
        disconnect();
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
        m_oimList[WlmUtils::latin1((*i).id)] = WlmUtils::passport((*i).from);
        m_server->cb.mainConnection->get_oim((*i).id, true);
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
	WlmContact * contact = qobject_cast<WlmContact*>(contacts().value( contactId));

    Kopete::Message msg = Kopete::Message (contact, myself ());
    msg.setPlainBody (message);
    msg.setDirection (Kopete::Message::Inbound);

    if (contact)
        contact->manager (Kopete::Contact::CanCreate)->appendMessage (msg);

    m_oimList.remove (id);
    m_server->cb.mainConnection->delete_oim (id.toLatin1().constData ());
}

void WlmAccount::slotRemoveRecentDPRequests()
{
    m_recentDPRequests.pop_front();
}

#include "wlmaccount.moc"
