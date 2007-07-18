/*
 * messengercontact.cpp - Windows Live Messenger Kopete Contact.
 *
 * Copyright (c) 2007 by Zhang Panyong <pyzhang@gmail.com>
 * 
 * Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>
 * 
 * Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>
 *
 *************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
 */
#include "messengercontact.h"

// KDE includes
#include <kaction.h>

// Kopete includes
#include "kopetechatsessionmanager.h"
#include "kopetechatsession.h"
#include "kopetemetacontact.h"

// Messenger includes
#include "messengeraccount.h"

MessengerContact::MessengerContact(MessengerAccount *account, const QString &contactId, Kopete::MetaContact *parent)
 : Kopete::Contact(account, contactId, parent)
{

}

bool MessengerContact::isReachable()
{
	return false;
}

void MessengerContact::serialize(QMap< QString, QString >& serializedData, QMap< QString, QString >& addressBookData)
{
	// Contact id and display name are already set for us, only add the rest
}

QList<KAction *> *MessengerContact::customContextMenuActions()
{
	QList<KAction*> *m_actionCollection = new QList<KAction*>;

	// Block/unblock Contact
	QString blockLabel = isBlocked() ? i18n( "Unblock User" ) : i18n( "Block User" );
	if( !actionBlock )
	{
		actionBlock = new KAction( KIcon("messenger_blocked"), blockLabel, this );
                //, "actionBlock" );
		connect( actionBlock, SIGNAL(triggered(bool)), this, SLOT( slotBlockUser()) );

		//show profile
		actionShowProfile = new KAction( i18n("View User's Live Profile"), this );
                //, "actionShowProfile" );
		connect( actionBlock, SIGNAL(triggered(bool)), this, SLOT(slotUserProfile()) );

		// Send mail (only available if it is an hotmail account)
		actionSendMail = new KAction( KIcon("mail"), i18n("Send Email..."), this );
                //, "actionSendMail" );
		connect( actionSendMail, SIGNAL(triggered(bool)), this, SLOT(slotSendMail()) );

		// Invite to receive webcam
		actionWebcamReceive = new KAction( KIcon("webcamreceive"), i18n( "View Contact's Webcam" ), this );
                //, "msnWebcamReceive" ) ;
		connect( actionWebcamReceive, SIGNAL(triggered(bool)), this, SLOT(slotWebcamReceive()) );

		//Send webcam action
		actionWebcamSend = new KAction( KIcon("webcamsend"), i18n( "Send Webcam" ), this );
                //, "msnWebcamSend" ) ;
		connect( actionWebcamSend, SIGNAL(triggered(bool)), this, SLOT(slotWebcamSend()) );
	}
	else
	{
		actionBlock->setText( blockLabel );
	}

	actionSendMail->setEnabled( static_cast<MSNAccount*>(account())->isLivemail());

	m_actionCollection->append( actionBlock );
	m_actionCollection->append( actionShowProfile );
	m_actionCollection->append( actionSendMail );
	m_actionCollection->append( actionWebcamReceive );
	m_actionCollection->append( actionWebcamSend );


	return m_actionCollection;

	return 0;
}

Kopete::ChatSession *MessengerContact::manager(CanCreateFlags canCreate)
{
	return 0;
}

void MessengerContact::slotUserInfo()
{
	m_infoWidget = new MessengerUserInfoWidget( Kopete::UI::Global::mainWidget() );
	QObject::connect( m_infoWidget, SIGNAL( finished() ), this, SLOT( closeUserInfoDialog() ) );
	m_infoWidget->setContact( this );
	m_infoWidget->show();
}

void MessengerContact::closeUserInfoDialog()
{
	QObject::disconnect( this, 0, m_infoWidget, 0 );
	m_infoWidget->delayedDestruct();
	m_infoWidget = 0L;
}

void MessengerContact::sendFile( const KUrl &sourceURL, const QString &altFileName, uint /*fileSize*/ )
{
	QString filePath;

	//If the file location is null, then get it from a file open dialog
	if( !sourceURL.isValid() )
	{
		filePath = KFileDialog::getOpenFileName( KUrl(), "*", 0l  , i18n( "Kopete File Transfer" ));
	}
	else
	{
		filePath = sourceURL.path(KUrl::RemoveTrailingSlash);
	}

	//kDebug(14140) << "MSNContact::sendFile: File chosen to send:" << fileName << endl;

	if ( filePath.isEmpty() )
	{
		kDebug(MESSENGER_DEBUG) << "files empty, assuming cancel" << endl;
		return;
	}

	kDebug(MESSENGER_DEBUG) << "files: '" << files << "' " << endl;

	//TODO Send the file
	mAccount->engine()->sendFiles(filePath, altFileName, fileSize);
}

void MessengerContact::slotUserProfile()
{   
	kDebug(MESSENGER_DEBUG) << k_funcinfo << endl;

	QString profileSiteString = QLatin1String("http://spaces.live.com/profile.aspx?mem=") + userId();
	KToolInvocation::invokeBrowser( profileSiteString );
} 

void MessengerContact::slotBlockUser()
{
}

void MessengerContact::slotSendMail()
{
}

#include "messengercontact.moc"
