/*
    wlmcontact.cpp - Kopete Wlm Protocol

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

#include "wlmcontact.h"

#include <QPointer>
#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <KToolInvocation>
#include <KUrl>

#include "kopeteaccount.h"
#include "kopetechatsessionmanager.h"
#include "kopetemetacontact.h"
#include "kopeteuiglobal.h"
#include "kopetegroup.h"

#include "ui_wlminfo.h"
#include "wlmaccount.h"
#include "wlmprotocol.h"

WlmContact::WlmContact (Kopete::Account * _account,
                        const QString & uniqueName,
                        const QString & contactSerial,
                        Kopete::MetaContact * parent):
Kopete::Contact (_account, uniqueName, parent)
{
    kDebug (14210) << k_funcinfo << " uniqueName: " << uniqueName;
    m_msgManager = 0L;
    m_account = qobject_cast<WlmAccount*>(account());
    setFileCapable (true);
    setOnlineStatus (WlmProtocol::protocol ()->wlmOffline);
    m_contactSerial = contactSerial;
    m_disabled = false;
    m_dontShowEmoticons = false;

    if ( metaContact() )
        m_currentGroup = metaContact()->groups().first();

    m_actionBlockContact = new KToggleAction(KIcon("wlm_blocked"), i18n("Block Contact"), this );
    QObject::connect( m_actionBlockContact, SIGNAL(triggered(bool)), this, SLOT(blockContact(bool)) );

    m_actionShowProfile = new KAction(i18n("Show Profile"), this);
    QObject::connect(m_actionShowProfile, SIGNAL(triggered(bool)), this, SLOT(slotShowProfile()));

    m_actionUpdateDisplayPicture = new KAction(i18n("Update Photo"), this);
    QObject::connect(m_actionUpdateDisplayPicture, SIGNAL(triggered(bool)), this, SLOT(slotUpdateDisplayPicture()));

    m_actionDontShowEmoticons = new KToggleAction (KIcon ("wlm_fakefriend"), 
           i18n ("&Block custom emoticons"), this);
    QObject::connect (m_actionDontShowEmoticons, SIGNAL(triggered(bool)), this, SLOT(slotDontShowEmoticons(bool)));
}

void WlmContact::slotDontShowEmoticons(bool block)
{
    m_actionDontShowEmoticons->setChecked(block);
    m_dontShowEmoticons = block;
}

bool WlmContact::dontShowEmoticons()
{
    return m_dontShowEmoticons;
}


void WlmContact::setDisabled(bool disabled, bool updateServer)
{
    WlmAccount* acc = qobject_cast<WlmAccount*>(account());
    if(!acc)
        return;

    if(disabled)
    {
        // already disabled
        if(isDisabled())
            return;

        m_disabled = true;

        if(!metaContact())
            return;

        metaContact()->setTemporary(m_disabled);

        setOnlineStatus(WlmProtocol::protocol()->wlmOffline);

        if(updateServer && account ()->isConnected ())
            acc->server ()->mainConnection->disableContactOnAddressBook( m_contactSerial.toLatin1().constData(), contactId ().toLatin1().constData() );
    }
    else
    {
        // already enabled
        if(!isDisabled())
            return;

        m_disabled = false;

        if(!metaContact())
            return;

        metaContact()->setTemporary(m_disabled);
        setOnlineStatus(WlmProtocol::protocol()->wlmOffline);

        if(updateServer && account ()->isConnected ())
            acc->server()->mainConnection->enableContactOnAddressBook(m_contactSerial.toLatin1().constData(), contactId().toLatin1().constData());
    }
}

WlmContact::~WlmContact ()
{
}

bool
WlmContact::isReachable ()
{
	// always true, as we can send offline messages
    return true;
}

void
WlmContact::slotShowProfile()
{
    KToolInvocation::invokeBrowser(QString::fromLatin1("http://members.msn.com/default.msnw?mem=") + contactId()) ;
}

void
WlmContact::slotUpdateDisplayPicture()
{
    if(!account()->isConnected())
        return;

	WlmAccount* acc = qobject_cast<WlmAccount*>(account());
    if(!acc)
        return;

    if ((onlineStatus() != WlmProtocol::protocol ()->wlmOffline)
           && (onlineStatus() != WlmProtocol::protocol ()->wlmInvisible)
           && (onlineStatus() != WlmProtocol::protocol ()->wlmUnknown))
    {
        acc->chatManager ()->requestDisplayPicture (contactId());
    }

}

void
WlmContact::sendFile (const KUrl & sourceURL, const QString & fileName,
                      uint fileSize)
{
    Q_UNUSED( fileName );
    Q_UNUSED( fileSize );

    QString filePath;

    if (!sourceURL.isValid ())
        filePath =
            KFileDialog::getOpenFileName (KUrl (), "*", 0l,
                                          i18n ("Kopete File Transfer"));
    else
        filePath = sourceURL.path (KUrl::RemoveTrailingSlash);

    if (!filePath.isEmpty ())
    {
        quint32 fileSize = QFileInfo (filePath).size ();
        //Send the file
        static_cast <WlmChatSession *>
			(manager (Kopete::Contact::CanCreate))->sendFile (filePath,
                                                               fileSize);
    }
}

void
WlmContact::serialize (QMap < QString, QString > &serializedData,
                       QMap < QString, QString > & /* addressBookData */ )
{
    serializedData["displayPicture"] =
        property (Kopete::Global::Properties::self ()->photo ()).value ().
        toString ();
    serializedData["contactSerial"] = m_contactSerial;
    serializedData["dontShowEmoticons"] = m_dontShowEmoticons ? "true" : "false";
}

Kopete::ChatSession *
    WlmContact::manager (Kopete::Contact::CanCreateFlags canCreate)
{
    Kopete::ContactPtrList chatmembers;
    chatmembers.append (this);

    Kopete::ChatSession * _manager =
        Kopete::ChatSessionManager::self ()->
				findChatSession (account ()->myself (), chatmembers, protocol ());
    WlmChatSession *manager = qobject_cast <WlmChatSession *>(_manager);
    if (!manager && canCreate == Kopete::Contact::CanCreate)
    {
        manager =
            new WlmChatSession (protocol (), account ()->myself (),
                                chatmembers);
    }
    return manager;
}

QList < KAction * >* WlmContact::customContextMenuActions ()     //OBSOLETE
{
    QList<KAction*> *actions = new QList<KAction*>();

    m_actionBlockContact->setEnabled(m_account->isConnected());
    m_actionBlockContact->setChecked(m_account->isContactBlocked(contactId()));
    actions->append(m_actionBlockContact);
    actions->append(m_actionShowProfile);
    actions->append(m_actionUpdateDisplayPicture);
    actions->append(m_actionDontShowEmoticons);

    // temporary action collection, used to apply Kiosk policy to the actions
    KActionCollection tempCollection((QObject*)0);
    tempCollection.addAction(QLatin1String("contactBlock"), m_actionBlockContact);
    tempCollection.addAction(QLatin1String("contactViewProfile"), m_actionShowProfile);
    tempCollection.addAction(QLatin1String("updateDisplayPicture"), m_actionUpdateDisplayPicture);
    tempCollection.addAction(QLatin1String("dontShowEmoticons"), m_actionDontShowEmoticons);

    return actions;
}

void WlmContact::sync(unsigned int flags)
{
    if (!account ()->isConnected ())
        return;
    /* 
     * If the contact has changed groups, then we update the server.
     */
    if( !metaContact() )
        return;

    // Don't add the contact if it's myself.
    if(contactId() == account()->accountId())
        return;

    if ( (flags & Kopete::Contact::MovedBetweenGroup) == Kopete::Contact::MovedBetweenGroup )
    {
        Kopete::Group* newGroup = metaContact()->groups().first();
        WlmAccount* acc = qobject_cast<WlmAccount*>(account());
        if(!acc)
            return;

        // if this contact is not in the contact list, add it
        if(!acc->isOnServerSideList(contactId()))
        {
            acc->createContact(contactId(), metaContact());
            m_currentGroup = newGroup;
            return;
        }

        if(isDisabled())
        {
            // enable this contact and update on server side
            setDisabled(false,true);
        }

        if(newGroup == m_currentGroup)
            return;

        if(newGroup == Kopete::Group::topLevel())
        {
            acc->server ()->mainConnection->removeFromGroup (
                acc->groupToGroupId().value(m_currentGroup->displayName()).toLatin1().constData(),
                m_contactSerial.toLatin1().constData ());
            m_currentGroup = newGroup;
            return;
        }

        // if the group don't exist on server side, we create it
        if(!acc->groupToGroupId().contains(newGroup->displayName()))
        {
            acc->server ()->mainConnection->addGroup(newGroup->displayName().toUtf8().constData());
                return;
        }

        // if we have both groups on server side, just move this contact
        if(acc->groupToGroupId().contains(newGroup->displayName())
            && acc->groupToGroupId().contains(m_currentGroup->displayName()))
        {
            acc->server ()->mainConnection->removeFromGroup (
                acc->groupToGroupId().value(m_currentGroup->displayName()).toLatin1().constData(),
                m_contactSerial.toLatin1().constData ());
            acc->server ()->mainConnection->addToGroup (
                acc->groupToGroupId().value(newGroup->displayName()).toLatin1().constData(),
                m_contactSerial.toLatin1().constData ());
            m_currentGroup = newGroup;
            return;
        }
        if(m_currentGroup == Kopete::Group::topLevel() &&
                acc->groupToGroupId().contains(newGroup->displayName()))
        {
            acc->server ()->mainConnection->addToGroup (
                acc->groupToGroupId().value(newGroup->displayName()).toLatin1().constData(),
                m_contactSerial.toLatin1().constData ());
            m_currentGroup = newGroup;
            return;
        }
    }
}

void WlmContact::blockContact(bool block)
{
    if (!m_account->isConnected())
        return;

    m_account->blockContact(contactId(), block);
}

void WlmContact::slotUserInfo()
{
    QPointer <KDialog> infoDialog = new KDialog;
    infoDialog->setButtons( KDialog::Close);
    infoDialog->setDefaultButton(KDialog::Close);
    const QString name = displayName();
    const QString personalMessage = statusMessage().message();
    Ui::WLMInfo info;
    info.setupUi(infoDialog->mainWidget());
    info.m_id->setText(contactId());
    info.m_displayName->setText(name);
    info.m_personalMessage->setText(personalMessage);
//     info.m_phh->setText(m_phoneHome); //TODO enable and fill
//     info.m_phw->setText(m_phoneWork);
//     info.m_phm->setText(m_phoneMobile);
//     info.m_reversed->setChecked(m_reversed);

//     connect( info.m_reversed, SIGNAL(toggled(bool)) , this, SLOT(slotUserInfoDialogReversedToggled()));

    info.groupBox->setVisible(false);
    info.m_reversed->setVisible(false);

    infoDialog->setCaption(name);
    infoDialog->exec();
    delete infoDialog;
}

void
WlmContact::showContactSettings ()
{
    //WlmContactSettings* p = new WlmContactSettings( this );
    //p->show();
}

void
WlmContact::sendMessage (Kopete::Message & message)
{
    kDebug (14210) << k_funcinfo;
    // give it back to the manager to display
    manager ()->appendMessage (message);
    // tell the manager it was sent successfully
    manager ()->messageSucceeded ();
}

void
WlmContact::deleteContact ()
{
    if (account ()->isConnected ())
    {
        qobject_cast <WlmAccount *>(account ())->server ()->mainConnection->
            delFromAddressBook (m_contactSerial.toLatin1 ().constData (), contactId ().toLatin1 ().constData ());
        deleteLater ();
    }
    else
    {
        KMessageBox::error (Kopete::UI::Global::mainWidget (),
                            i18n
                            ("<qt>You need to go online to remove a contact from your contact list. This contact will appear again when you reconnect.</qt>"),
                            i18n ("WLM Plugin"));
    }
}

void
WlmContact::receivedMessage (const QString & message)
{
    // Create a Kopete::Message
    Kopete::ContactPtrList contactList;
    account ();
    contactList.append (account ()->myself ());
    Kopete::Message newMessage (this, contactList);
    newMessage.setPlainBody (message);
    newMessage.setDirection (Kopete::Message::Inbound);

    // Add it to the manager
    manager ()->appendMessage (newMessage);
}

void
WlmContact::slotChatSessionDestroyed ()
{
    //FIXME: the chat window was closed?  Take appropriate steps.
    m_msgManager = 0L;
}

void
WlmContact::setOnlineStatus(const Kopete::OnlineStatus& status)
{
#if 0
	bool isBlocked = qobject_cast <WlmAccount *>(account())->isOnBlockList(contactId());
	bool isOnForwardList = qobject_cast <WlmAccount *>(account())->isOnServerSideList(contactId());
	bool isOnReverseList = qobject_cast <WlmAccount *>(account())->isOnReverseList(contactId());
	
	// if this contact is blocked, and currently has a regular status,
	// create a custom status and add wlm_blocked to ovelayIcons
	if(isBlocked || (isOnForwardList && !isOnReverseList))
	{
		QStringList overelayIconsList;
		QString reason;
		if(isOnForwardList && !isOnReverseList)
		{
			overelayIconsList << "wlm_fakefriend";
			reason = i18n("This contact does not have you in his/her list");
		}

		if(isBlocked)
		{
			overelayIconsList << "wlm_blocked";
			if(reason.isEmpty())
				reason = i18n("This contact is blocked");
			else
				reason = i18n("This contact does not have you in his/her list and is blocked");
		}
		// set the new status
		Kopete::Contact::setOnlineStatus(
				Kopete::OnlineStatus(status.status() ,
				(status.weight()==0) ? 0 : (status.weight() -1),
				protocol(),
				status.internalStatus()+15,
				status.overlayIcons() + overelayIconsList,
				reason ) );
		return;
	}

	if (status.internalStatus() >= 15)
	{
		// if this contact was previously blocked, set a regular status again
		switch(status.internalStatus()-15)
		{
			case 1:
				Kopete::Contact::setOnlineStatus(WlmProtocol::protocol()->wlmOnline);
				break;
			case 2:
				Kopete::Contact::setOnlineStatus(WlmProtocol::protocol()->wlmAway);
				break;
			case 3:
				Kopete::Contact::setOnlineStatus(WlmProtocol::protocol()->wlmBusy);
				break;
			case 4:
				Kopete::Contact::setOnlineStatus(WlmProtocol::protocol()->wlmBeRightBack);
				break;
			case 5:
				Kopete::Contact::setOnlineStatus(WlmProtocol::protocol()->wlmOnThePhone);
				break;
			case 6:
				Kopete::Contact::setOnlineStatus(WlmProtocol::protocol()->wlmOutToLunch);
				break;
			case 7:
				Kopete::Contact::setOnlineStatus(WlmProtocol::protocol()->wlmInvisible);
				break;
			case 8:
				Kopete::Contact::setOnlineStatus(WlmProtocol::protocol()->wlmOffline);
				break;
			case 9:
				Kopete::Contact::setOnlineStatus(WlmProtocol::protocol()->wlmIdle);
				break;
			default:
				Kopete::Contact::setOnlineStatus(WlmProtocol::protocol()->wlmUnknown);
				break;
		}

	}
	else
#endif
		Kopete::Contact::setOnlineStatus(status);
}

#include "wlmcontact.moc"

// vim: set noet ts=4 sts=4 sw=4:
