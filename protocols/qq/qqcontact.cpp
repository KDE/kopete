/*
    qqcontact.cpp - QQ Contact

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002      by Ryan Cumming           <bodnar42@phalynx.dhs.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart at kde.org>
    Copyright (c) 2005      by Michaël Larouche       <larouche@kde.org>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "qqcontact.h"

#include <qcheckbox.h>
#include <QList>

#undef KDE_NO_COMPAT
#include <kaction.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <ktoolinvocation.h>
#include <ktemporaryfile.h>
#include <kconfig.h>
#include <kglobal.h>
#include <qregexp.h>
#include <kio/job.h>
#include <kdialog.h>
#include <kicon.h>

#include "kopetecontactlist.h"
#include "kopetechatsessionmanager.h"
#include "kopetemetacontact.h"
#include "kopetegroup.h"
#include "kopeteuiglobal.h"
#include "kopeteglobal.h"

#include "qqnotifysocket.h"
#include "qqaccount.h"
#include "qqprotocol.h"
#include "qqchatsession.h"
#include "dlgqqvcard.h"

QQContact::QQContact( Kopete::Account *account, const QString &id, Kopete::MetaContact *parent )
: Kopete::Contact( account, id, parent )
{
	m_deleted = false;
	m_allowed = false;
	m_blocked = false;
	m_reversed = false;
	m_moving = false;

	m_clientFlags=0;

	setFileCapable( true );

	// When we are not connected, it's because we are loading the contact list.
	// so we set the initial status to offline.
	// We set offline directly because modifying the status after is too slow.
	// (notification, contact list updating,....)
	//
	// FIXME: Hacks like these shouldn't happen in the protocols, but should be
	//        covered properly at the libkopete level instead - Martijn
	//
	// When we are connected, it can be because the user added a contact with the
	// wizard, and it can be because we are creating a temporary contact.
	// if it's added by the wizard, the status will be set immediately after.
	// if it's a temporary contact, better to set the unknown status.
	setOnlineStatus( ( parent && parent->isTemporary() ) ? QQProtocol::protocol()->UNK : QQProtocol::protocol()->Offline );

	actionBlock = 0L;
}

QQContact::~QQContact()
{
	kDebug(14140) ;
}

bool QQContact::isReachable()
{
	// QQ supports offline chat.
	return true;
	if ( account()->isConnected() && isOnline() && account()->myself()->onlineStatus() != QQProtocol::protocol()->HDN )
		return true;
/*
	QQChatSession *kmm=dynamic_cast<QQChatSession*>(manager(Kopete::Contact::CannotCreate));
	if( kmm && kmm->service() )  //the chat socket is open.  than mean message will be sent
		return true;
*/
	// When we are invisible we can't start a chat with others, make isReachable return false
	// (This is an QQ limitation, not a problem in Kopete)
	if ( !account()->isConnected() || account()->myself()->onlineStatus() == QQProtocol::protocol()->HDN )
		return false;

	//if the contact is offline, it is impossible to send it a message.  but it is impossible
	//to be sure the contact is really offline. For example, if the contact is not on the contact list for
	//some reason.
	if( onlineStatus() == QQProtocol::protocol()->Offline && ( isAllowed() || isBlocked() ) && !serverGroups().isEmpty() )
		return false;

	return true;
}

Kopete::ChatSession *QQContact::manager( Kopete::Contact::CanCreateFlags canCreate )
{
	Kopete::ContactPtrList chatMembers;
	chatMembers.append(this);
	QString guid(QString::null);	//krazy:exclude=nullstrassign for old broken gcc

	// 1 to 1 chat session
	if( chatMembers.count() == 1 )
		// FIXME: Use a function to override the hard hack!
		guid = account()->myself()->contactId() + ':' + this->contactId();

	return static_cast<QQAccount*>(account())->chatSession( chatMembers, guid, canCreate );
}

QList<KAction*> *QQContact::customContextMenuActions()
{
	QList<KAction*> *m_actionCollection = new QList<KAction*>;

	// Block/unblock Contact
	QString label = isBlocked() ? i18n( "Unblock User" ) : i18n( "Block User" );
	if( !actionBlock )
	{
		actionBlock = new KAction( KIcon("qq_blocked"), label, this );
		connect( actionBlock, SIGNAL(triggered(bool)), this, SLOT(slotBlockUser()) );

		//show profile
		actionShowProfile = new KAction( i18n("Show Profile"), this );
		connect( actionBlock, SIGNAL(triggered(bool)), this, SLOT(slotShowProfile()) );

		// Send mail (only available if it is an hotmail account)
		actionSendMail = new KAction( KIcon("mail-message-new"), i18n("Send Email..."), this );
		connect( actionSendMail, SIGNAL(triggered(bool)), this, SLOT(slotSendMail()) );

		// Invite to receive webcam
		actionWebcamReceive = new KAction( KIcon("webcamreceive"), i18n( "View Contact's Webcam" ), this );
		connect( actionWebcamReceive, SIGNAL(triggered(bool)), this, SLOT(slotWebcamReceive()) );

		//Send webcam action
		actionWebcamSend = new KAction( KIcon("webcamsend"), i18n( "Send Webcam" ), this );
		connect( actionWebcamSend, SIGNAL(triggered(bool)), this, SLOT(slotWebcamSend()) );
	}
	else
		actionBlock->setText( label );

	m_actionCollection->append( actionBlock );
	m_actionCollection->append( actionShowProfile );
	m_actionCollection->append( actionSendMail );
	m_actionCollection->append( actionWebcamReceive );
	m_actionCollection->append( actionWebcamSend );


	return m_actionCollection;
}

void QQContact::slotBlockUser()
{
}

void QQContact::slotUserInfo()
{
	new dlgQQVCard(static_cast<QQAccount*>(account()), this, Kopete::UI::Global::mainWidget() );
}

void QQContact::slotUserInfoDialogReversedToggled()
{
	//workaround to make this checkboxe readonly
	const QCheckBox *cb=dynamic_cast<const QCheckBox*>(sender());
	if(cb && cb->isChecked()!=m_reversed)
		const_cast<QCheckBox*>(cb)->setChecked(m_reversed);
}

void QQContact::deleteContact()
{
	kDebug( 14140 ) ;
}

bool QQContact::isBlocked() const
{
	return m_blocked;
}

void QQContact::setBlocked( bool blocked )
{
	if( m_blocked != blocked )
	{
		m_blocked = blocked;
		//update the status
		setOnlineStatus(m_currentStatus);
		//m_currentStatus is used here.  previously it was  onlineStatus()  but this may cause problem when
		// the account is offline because of the  Kopete::Contact::OnlineStatus()  account offline hack.
	}
}

bool QQContact::isAllowed() const
{
	return m_allowed;
}

void QQContact::setAllowed( bool allowed )
{
	m_allowed = allowed;
}

bool QQContact::isReversed() const
{
	return m_reversed;
}

void QQContact::setReversed( bool reversed )
{
	m_reversed= reversed;
}

bool QQContact::isDeleted() const
{
	return m_deleted;
}

void QQContact::setDeleted( bool deleted )
{
	m_deleted= deleted;
}

uint QQContact::clientFlags() const
{
	return m_clientFlags;
}

void QQContact::setClientFlags( uint flags )
{
	if(m_clientFlags != flags)
	{
		/*
		if(hasProperty( QQProtocol::protocol()->propClient.key() ))
		{
			if( flags & QQProtocol::WebMessenger)
				setProperty(  QQProtocol::protocol()->propClient , i18n("Web Messenger") );
			else if( flags & QQProtocol::WindowsMobile)
				setProperty(  QQProtocol::protocol()->propClient , i18n("Windows Mobile") );
			else if( flags & QQProtocol::QQMobileDevice)
				setProperty(  QQProtocol::protocol()->propClient , i18n("QQ Mobile") );
			else if( m_obj.contains("kopete")  )
				setProperty(  QQProtocol::protocol()->propClient , i18n("Kopete") );
		}
		*/

	}
	m_clientFlags=flags;
}

void QQContact::setInfo(const  QString &type,const QString &data )
{
	if( type == "PHH" )
	{
		m_phoneHome = data;
		//setProperty(QQProtocol::protocol()->propPhoneHome, data);
	}
	else if( type == "PHW" )
	{
		m_phoneWork=data;
		// setProperty(QQProtocol::protocol()->propPhoneWork, data);
	}
	else if( type == "PHM" )
	{
		m_phoneMobile = data;
		// setProperty(QQProtocol::protocol()->propPhoneMobile, data);
	}
	else if( type == "MOB" )
	{
		if( data == "Y" )
			m_phone_mob = true;
		else if( data == "N" )
			m_phone_mob = false;
		else
			kDebug( 14140 ) << "Unknown MOB " << data;
	}
	else if( type == "MFN" )
	{
		setNickName(data);
	}
	else
	{
		kDebug( 14140 ) << "Unknown info " << type << ' ' << data;
	}
}


void QQContact::serialize( QMap<QString, QString> &serializedData, QMap<QString, QString> & /* addressBookData */ )
{
	// Contact id and display name are already set for us, only add the rest
	/*  TODO: use groups later
	QString groups;
	for( QMap<QString, Kopete::Group *>::ConstIterator it = m_serverGroups.begin(); it != m_serverGroups.end(); ++it )
	{
		groups += it.key();
		groups += ',';
	}
    if(groups.length() > 0)
        groups.truncate(groups.length()-1);

	QString lists="C";
	if(m_blocked)
		lists +='B';
	if(m_allowed)
		lists +='A';
	if(m_reversed)
		lists +='R';
	*/
	if( m_contactDetail.empty() )
		return;

	int size = sizeof(Eva::contactDetailIndex)/sizeof(Eva::contactDetailIndex[0]);

	for( int i=1; i< size; i++)
	{
		serializedData[ Eva::contactDetailIndex[i] ] = m_contactDetail[ Eva::contactDetailIndex[i] ];
	}

	return;

	/* TODO: delete us!
	serializedData[ "groups" ]  = groups;
	serializedData[ "PHH" ]  = m_phoneHome;
	serializedData[ "PHW" ]  = m_phoneWork;
	serializedData[ "PHM" ]  = m_phoneMobile;
	serializedData[ "lists" ] = lists;
	serializedData[ "obj" ] = m_obj;
	serializedData[ "contactGuid" ] = guid();
	*/
}


QString QQContact::guid(){ return 0; }//property(QQProtocol::protocol()->propGuid).value().toString(); }

QString QQContact::phoneHome(){ return m_phoneHome ;}
QString QQContact::phoneWork(){ return m_phoneWork ;}
QString QQContact::phoneMobile(){ return m_phoneMobile ;}


const QMap<QString, Kopete::Group*>  QQContact::serverGroups() const
{
	return m_serverGroups;
}
void QQContact::clearServerGroups()
{
	m_serverGroups.clear();
}


void QQContact::sync( unsigned int changed )
{
	Q_UNUSED(changed);
	return;
}

void QQContact::contactAddedToGroup( const QString& groupId, Kopete::Group *group )
{
	m_serverGroups.insert( groupId, group );
	m_moving=false;
}

void QQContact::contactRemovedFromGroup( const QString& groupId )
{
	m_serverGroups.remove( groupId );
	if(m_serverGroups.isEmpty() && !m_moving)
	{
		deleteLater();
	}
	m_moving=false;
}


void QQContact::rename( const QString &newName )
{
	Q_UNUSED(newName);
	//kDebug( 14140 ) << "From: " << displayName() << ", to: " << newName;

/*	if( newName == displayName() )
		return;*/

	// FIXME: This should be called anymore.
		// notify->changePublicName( newName, contactId() );
}

void QQContact::slotShowProfile()
{
	KToolInvocation::invokeBrowser( QString::fromLatin1("http://members.qq.com/default.qqw?mem=") + contactId()) ;
}


/**
 * FIXME: Make this a standard KMM API call
 */
void QQContact::sendFile( const KUrl &sourceURL, const QString &altFileName, uint /*fileSize*/ )
{
	Q_UNUSED(altFileName);

	QString filePath;

	//If the file location is null, then get it from a file open dialog
	if( !sourceURL.isValid() )
		filePath = KFileDialog::getOpenFileName( KUrl(),"*", 0l  , i18n( "Kopete File Transfer" ));
	else
		filePath = sourceURL.path(KUrl::RemoveTrailingSlash);

	//kDebug(14140) << "QQContact::sendFile: File chosen to send:" << fileName;

	if ( !filePath.isEmpty() )
	{
		quint32 fileSize = QFileInfo(filePath).size();
		Q_UNUSED(fileSize);
		//Send the file
		// static_cast<QQChatSession*>( manager(Kopete::Contact::CanCreate) )->sendFile( filePath, altFileName, fileSize );

	}
}

void QQContact::setOnlineStatus(const Kopete::OnlineStatus& status)
{
	Kopete::Contact::setOnlineStatus(status);
	m_currentStatus=status;
}

void QQContact::slotSendMail()
{
}

void QQContact::setDisplayPicture(KTemporaryFile *f)
{
	//copy the temp file somewere else.
	// in a better world, the file could be dirrectly wrote at the correct location.
	// but the custom emoticon code is to deeply merged in the display picture code while it could be separated.
	QString newlocation=KStandardDirs::locateLocal( "appdata", "qqpictures/"+ contactId().toLower().replace(QRegExp("[./~]"),"-")  +".png"  ) ;

	QString fileName = f->fileName();
	f->setAutoRemove(false);
	delete f;

	KIO::Job *j=KIO::file_move( KUrl( fileName ), KUrl( newlocation ), -1, KIO::Overwrite | KIO::HideProgressInfo );


	//let the time to KIO to copy the file
	connect(j, SIGNAL(result(KJob*)) , this, SLOT(slotEmitDisplayPictureChanged()));
}

void QQContact::slotEmitDisplayPictureChanged()
{
	QString newlocation=KStandardDirs::locateLocal( "appdata", "qqpictures/"+ contactId().toLower().replace(QRegExp("[./~]"),"-")  +".png"  ) ;
	setProperty( Kopete::Global::Properties::self()->photo() , newlocation );
	emit displayPictureChanged();
}


void QQContact::setObject(const QString &obj)
{
	if(m_obj==obj && (obj.isEmpty() || hasProperty(Kopete::Global::Properties::self()->photo().key())))
		return;

	m_obj=obj;

	removeProperty( Kopete::Global::Properties::self()->photo()  ) ;
	emit displayPictureChanged();

	KConfigGroup config(KGlobal::config(), "QQ");
	if ( config.readEntry( "DownloadPicture", 2 ) >= 2 && !obj.isEmpty()
			 && account()->myself()->onlineStatus().status() != Kopete::OnlineStatus::Invisible )
		manager(Kopete::Contact::CanCreate); //create the manager which will download the photo automatically.
}


#include "qqcontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

