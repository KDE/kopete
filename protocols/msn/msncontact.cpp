/*
    msncontact.cpp - MSN Contact

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002      by Ryan Cumming           <bodnar42@phalynx.dhs.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart at kde.org>
    Copyright (c) 2005      by Michaël Larouche       <michael.larouche@kdemail.net>

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

#include "msncontact.h"

#include <qcheckbox.h>

#undef KDE_NO_COMPAT
#include <kaction.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <krun.h>
#include <ktempfile.h>
#include <kconfig.h>
#include <kglobal.h>
#include <qregexp.h>
#include <kio/job.h>

#include "kopetecontactlist.h"
#include "kopetechatsessionmanager.h"
#include "kopetemetacontact.h"
#include "kopetegroup.h"
#include "kopeteuiglobal.h"
#include "kopeteglobal.h"

#include "msninfo.h"
#include "msnchatsession.h"
#include "msnnotifysocket.h"
#include "msnaccount.h"

MSNContact::MSNContact( Kopete::Account *account, const QString &id, Kopete::MetaContact *parent )
: Kopete::Contact( account, id, parent )
{
	m_deleted = false;
	m_allowed = false;
	m_blocked = false;
	m_reversed = false;
	m_moving = false;
	
	m_clientFlags=0;

	setFileCapable( true );

	// When we are not connected, it's because we are loading the contactlist.
	// so we set the initial status to offline.
	// We set offline directly because modifying the status after is too slow.
	// (notification, contactlist updating,....)
	//
	// FIXME: Hacks like these shouldn't happen in the protocols, but should be
	//        covered properly at the libkopete level instead - Martijn
	//
	// When we are connected, it can be because the user added a contact with the
	// wizard, and it can be because we are creating a temporary contact.
	// if it's added by the wizard, the status will be set immediately after.
	// if it's a temporary contact, better to set the unknown status.
	setOnlineStatus( ( parent && parent->isTemporary() ) ? MSNProtocol::protocol()->UNK : MSNProtocol::protocol()->FLN );

	actionBlock = 0L;

	setProperty(MSNProtocol::protocol()->propEmail, id);
}

MSNContact::~MSNContact()
{
	kdDebug(14140) << k_funcinfo << endl;
}

bool MSNContact::isReachable()
{
	if ( account()->isConnected() && isOnline() && account()->myself()->onlineStatus() != MSNProtocol::protocol()->HDN )
		return true;

	MSNChatSession *kmm=dynamic_cast<MSNChatSession*>(manager(Kopete::Contact::CannotCreate));
	if( kmm && kmm->service() )  //the chat socket is open.  than mean message will be sent
		return true;

	// When we are invisible we can't start a chat with others, make isReachable return false
	// (This is an MSN limitation, not a problem in Kopete)
	if ( !account()->isConnected() || account()->myself()->onlineStatus() == MSNProtocol::protocol()->HDN )
		return false;
		
	//if the contact is offline, it is impossible to send it a message.  but it is impossible
	//to be sure the contact is realy offline. For example, if the contact is not on the contactlist for
	//some reason.
	if( onlineStatus() == MSNProtocol::protocol()->FLN && ( isAllowed() || isBlocked() ) && !serverGroups().isEmpty() )
		return false;
		
	return true;
}

Kopete::ChatSession *MSNContact::manager( Kopete::Contact::CanCreateFlags canCreate )
{
	Kopete::ContactPtrList chatmembers;
	chatmembers.append(this);

	Kopete::ChatSession *_manager = Kopete::ChatSessionManager::self()->findChatSession(  account()->myself(), chatmembers, protocol() );
	MSNChatSession *manager = dynamic_cast<MSNChatSession*>( _manager );
	if(!manager &&  canCreate==Kopete::Contact::CanCreate)
	{
		manager = new MSNChatSession( protocol(), account()->myself(), chatmembers  );
		static_cast<MSNAccount*>( account() )->slotStartChatSession( contactId() );
	}
	return manager;
}

QPtrList<KAction> *MSNContact::customContextMenuActions()
{
	QPtrList<KAction> *m_actionCollection = new QPtrList<KAction>;

	// Block/unblock Contact
	QString label = isBlocked() ? i18n( "Unblock User" ) : i18n( "Block User" );
	if( !actionBlock )
	{
		actionBlock = new KAction( label, "msn_blocked",0, this, SLOT( slotBlockUser() ),
			this, "actionBlock" );

		//show profile
		actionShowProfile = new KAction( i18n("Show Profile") , 0, this, SLOT( slotShowProfile() ),
			this, "actionShowProfile" );

		// Send mail (only available if it is an hotmail account)
		actionSendMail = new KAction( i18n("Send Email...") , "mail_generic",0, this, SLOT( slotSendMail() ),
			this, "actionSendMail" );

		// Invite to receive webcam
		actionWebcamReceive = new KAction( i18n( "View Contact's Webcam" ), "webcamreceive",  0, this, SLOT(slotWebcamReceive() ), this, "msnWebcamReceive" ) ;

		//Send webcam action
		actionWebcamSend = new KAction( i18n( "Send Webcam" ), "webcamsend",  0, this, SLOT(slotWebcamSend() ), this, "msnWebcamSend" ) ;
	}
	else
		actionBlock->setText( label );

	actionSendMail->setEnabled( static_cast<MSNAccount*>(account())->isHotmail());

	m_actionCollection->append( actionBlock );
	m_actionCollection->append( actionShowProfile );
	m_actionCollection->append( actionSendMail );
	m_actionCollection->append( actionWebcamReceive );
	m_actionCollection->append( actionWebcamSend );


	return m_actionCollection;
}

void MSNContact::slotBlockUser()
{
	MSNNotifySocket *notify = static_cast<MSNAccount*>( account() )->notifySocket();
	if( !notify )
	{
		KMessageBox::error( Kopete::UI::Global::mainWidget(),
			i18n( "<qt>Please go online to block or unblock a contact.</qt>" ),
			i18n( "MSN Plugin" ));
		return;
	}

	if( m_blocked )
	{
		notify->removeContact( contactId(), MSNProtocol::BL, QString::null, QString::null );
	}
	else
	{
		if(m_allowed)
			notify->removeContact( contactId(), MSNProtocol::AL, QString::null, QString::null );
		else
			notify->addContact( contactId(), MSNProtocol::BL, QString::null, QString::null, QString::null );
	}
}

void MSNContact::slotUserInfo()
{
	KDialogBase *infoDialog=new KDialogBase( 0l, "infoDialog", /*modal = */false, QString::null, KDialogBase::Close , KDialogBase::Close, false );
	QString nick=property( Kopete::Global::Properties::self()->nickName()).value().toString();
	QString personalMessage=property( MSNProtocol::protocol()->propPersonalMessage).value().toString();
	MSNInfo *info=new MSNInfo ( infoDialog,"info");
	info->m_id->setText( contactId() );
	info->m_displayName->setText(nick);
	info->m_personalMessage->setText(personalMessage);
	info->m_phh->setText(m_phoneHome);
	info->m_phw->setText(m_phoneWork);
	info->m_phm->setText(m_phoneMobile);
	info->m_reversed->setChecked(m_reversed);

	connect( info->m_reversed, SIGNAL(toggled(bool)) , this, SLOT(slotUserInfoDialogReversedToggled()));

	infoDialog->setMainWidget(info);
	infoDialog->setCaption(nick);
	infoDialog->show();
}

void MSNContact::slotUserInfoDialogReversedToggled()
{
	//workaround to make this checkboxe readonly
	const QCheckBox *cb=dynamic_cast<const QCheckBox*>(sender());
	if(cb && cb->isChecked()!=m_reversed)
		const_cast<QCheckBox*>(cb)->setChecked(m_reversed);
}

void MSNContact::deleteContact()
{
	kdDebug( 14140 ) << k_funcinfo << endl;

	MSNNotifySocket *notify = static_cast<MSNAccount*>( account() )->notifySocket();
	if( notify )
	{
		if( hasProperty(MSNProtocol::protocol()->propGuid.key()) )
		{
			// Remove from all groups he belongs (if applicable)
			for( QMap<QString, Kopete::Group*>::Iterator it = m_serverGroups.begin(); it != m_serverGroups.end(); ++it )
			{
				kdDebug(14140) << k_funcinfo << "Removing contact from group \"" << it.key() << "\"" << endl;
				notify->removeContact( contactId(), MSNProtocol::FL, guid(), it.key() );
			}
	
			// Then trully remove it from server contact list, 
			// because only removing the contact from his groups isn't sufficent from MSNP11.
			kdDebug( 14140 ) << k_funcinfo << "Removing contact from top-level." << endl;
			notify->removeContact( contactId(), MSNProtocol::FL, guid(), QString::null);
		}
		else
		{
			kdDebug( 14140 ) << k_funcinfo << "The contact is already removed from server, just delete it" << endl;
			deleteLater();
		}
	}
	else
	{
		// FIXME: This case should be handled by Kopete, not by the plugins :( - Martijn
		// FIXME: We should be able to delete contacts offline, and remove it from server next time we go online - Olivier
		KMessageBox::error( Kopete::UI::Global::mainWidget(), i18n( "<qt>Please go online to remove a contact from your contact list.</qt>" ), i18n( "MSN Plugin" ));
	}
}

bool MSNContact::isBlocked() const
{
	return m_blocked;
}

void MSNContact::setBlocked( bool blocked )
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

bool MSNContact::isAllowed() const
{
	return m_allowed;
}

void MSNContact::setAllowed( bool allowed )
{
	m_allowed = allowed;
}

bool MSNContact::isReversed() const
{
	return m_reversed;
}

void MSNContact::setReversed( bool reversed )
{
	m_reversed= reversed;
}

bool MSNContact::isDeleted() const
{
	return m_deleted;
}

void MSNContact::setDeleted( bool deleted )
{
	m_deleted= deleted;
}

uint MSNContact::clientFlags() const
{
	return m_clientFlags;
}

void MSNContact::setClientFlags( uint flags )
{
	if(m_clientFlags != flags) 
	{
		if(hasProperty( MSNProtocol::protocol()->propClient.key() ))
		{
			if( flags & MSNProtocol::WebMessenger)
				setProperty(  MSNProtocol::protocol()->propClient , i18n("Web Messenger") );
			else if( flags & MSNProtocol::WindowsMobile)
				setProperty(  MSNProtocol::protocol()->propClient , i18n("Windows Mobile") );
			else if( flags & MSNProtocol::MSNMobileDevice)
				setProperty(  MSNProtocol::protocol()->propClient , i18n("MSN Mobile") );
			else if( m_obj.contains("kopete")  )
				setProperty(  MSNProtocol::protocol()->propClient , i18n("Kopete") );
		}

	}
	m_clientFlags=flags;
}

void MSNContact::setInfo(const  QString &type,const QString &data )
{
	if( type == "PHH" )
	{
		m_phoneHome = data;
		setProperty(MSNProtocol::protocol()->propPhoneHome, data);
	}
	else if( type == "PHW" )
	{
		m_phoneWork=data;
		setProperty(MSNProtocol::protocol()->propPhoneWork, data);
	}
	else if( type == "PHM" )
	{
		m_phoneMobile = data;
		setProperty(MSNProtocol::protocol()->propPhoneMobile, data);
	}
	else if( type == "MOB" )
	{
		if( data == "Y" )
			m_phone_mob = true;
		else if( data == "N" )
			m_phone_mob = false;
		else
			kdDebug( 14140 ) << k_funcinfo << "Unknown MOB " << data << endl;
	}
	else if( type == "MFN" )
	{
		setProperty(Kopete::Global::Properties::self()->nickName(), data );
	}
	else
	{
		kdDebug( 14140 ) << k_funcinfo << "Unknow info " << type << " " << data << endl;
	}
}


void MSNContact::serialize( QMap<QString, QString> &serializedData, QMap<QString, QString> & /* addressBookData */ )
{
	// Contact id and display name are already set for us, only add the rest
	QString groups;
	bool firstEntry = true;
	for( QMap<QString, Kopete::Group *>::ConstIterator it = m_serverGroups.begin(); it != m_serverGroups.end(); ++it )
	{
		if( !firstEntry )
		{
			groups += ",";
			firstEntry = true;
		}
		groups += it.key();
	}

	QString lists="C";
	if(m_blocked)
		lists +="B";
	if(m_allowed)
		lists +="A";
	if(m_reversed)
		lists +="R";

	serializedData[ "groups" ]  = groups;
	serializedData[ "PHH" ]  = m_phoneHome;
	serializedData[ "PHW" ]  = m_phoneWork;
	serializedData[ "PHM" ]  = m_phoneMobile;
	serializedData[ "lists" ] = lists;
	serializedData[ "obj" ] = m_obj;
	serializedData[ "contactGuid" ] = guid();
}


QString MSNContact::guid(){ return property(MSNProtocol::protocol()->propGuid).value().toString(); }

QString MSNContact::phoneHome(){ return m_phoneHome ;}
QString MSNContact::phoneWork(){ return m_phoneWork ;}
QString MSNContact::phoneMobile(){ return m_phoneMobile ;}


const QMap<QString, Kopete::Group*>  MSNContact::serverGroups() const
{
	return m_serverGroups;
}
void MSNContact::clearServerGroups() 
{
	m_serverGroups.clear();
}


void MSNContact::sync( unsigned int changed )
{
	if( !  (changed & Kopete::Contact::MovedBetweenGroup) )
		return;  //we are only interested by a change in groups

	if(!metaContact() || metaContact()->isTemporary() )
		return;

	if(m_moving)
	{
		//We need to make sure that syncGroups is not called twice successively
		// because m_serverGroups will be only updated with the reply of the server
		// and then, the same command can be sent twice.
		// FIXME: if this method is called a seconds times, that mean change can be
		//        done in the contactlist. we should found a way to recall this
		//        method later. (a QTimer?)
		kdDebug( 14140 ) << k_funcinfo << " This contact is already moving. Abort sync    id: " << contactId() << endl;
		return;
	}
	
	MSNNotifySocket *notify = static_cast<MSNAccount*>( account() )->notifySocket();
	if( !notify )
	{
		//We are not connected, we will doing it next connection.
		//Force to reload the whole contactlist from server to suync groups when connecting
		account()->configGroup()->writeEntry("serial", 0 );
		return;
	}
	
	if(m_deleted)  //the contact hasn't been synced from server yet.
		return; 

	unsigned int count=m_serverGroups.count();

	//Don't add the contact if it's myself.
	if(count==0 && contactId() == account()->accountId())
		return;

	//STEP ONE : add the contact to every kopetegroups where the MC is
	QPtrList<Kopete::Group> groupList = metaContact()->groups();
	for ( Kopete::Group *group = groupList.first(); group; group = groupList.next() )
	{
		//For each group, ensure it is on the MSN server
		if( !group->pluginData( protocol() , account()->accountId() + " id" ).isEmpty() )
		{
			QString Gid=group->pluginData( protocol(), account()->accountId() + " id" );
			if( !static_cast<MSNAccount*>( account() )->m_groupList.contains(Gid) ) 
			{ // ohoh!   something is corrupted on the contactlist.xml
			  // anyway, we never should add a contact to an unexisting group on the server.
			  //     This shouln't be possible anymore  2004-06-10 -Olivier

				//repair the problem
				group->setPluginData( protocol() , account()->accountId() + " id" , QString::null);
				group->setPluginData( protocol() , account()->accountId() + " displayName" , QString::null);
				kdWarning( 14140 ) << k_funcinfo << " Group " << group->displayName() << " marked with id #" <<Gid << " does not seems to be anymore on the server" << endl;

				if(!group->displayName().isEmpty() && group->type() == Kopete::Group::Normal) //not the top-level
				{
					//Create the group and add the contact
					static_cast<MSNAccount*>( account() )->addGroup( group->displayName(),contactId() );
					count++;
					m_moving=true;
				}
			}
			else if( !m_serverGroups.contains(Gid) )
			{
				//Add the contact to the group on the server
				notify->addContact( contactId(), MSNProtocol::FL, QString::null, guid(), Gid );
				count++;
				m_moving=true;
			}
		}
		else
		{
			if(!group->displayName().isEmpty() && group->type() == Kopete::Group::Normal) //not the top-level
			{
				//Create the group and add the contact
				static_cast<MSNAccount*>( account() )->addGroup( group->displayName(),contactId() );

				//WARNING: if contact is not correctly added (because the group was not aded corrdctly for hinstance),
				// if we increment the count, the contact can be deleted from the old group, and be lost :-(
				count++;
				m_moving=true;
			}
		}
	}

	//STEP TWO : remove the contact from groups where the MC is not, but let it at least in one group

	//contact is not in that group. on the server. we will remove them dirrectly after the loop
	QValueList<QString> removinglist; 
	
	for( QMap<QString, Kopete::Group*>::Iterator it = m_serverGroups.begin();(count > 1 && it != m_serverGroups.end()); ++it )
	{
		if( !static_cast<MSNAccount*>( account() )->m_groupList.contains(it.key()) )
		{ // ohoh!   something is corrupted on the contactlist.xml
		  // anyway, we never should add a contact to an unexisting group on the server.

			//repair the problem ...     //contactRemovedFromGroup( it.key() );
			//         ... later  (we  can't remove it from the map now )
			removinglist.append(it.key());
			count--;

			kdDebug( 14140 ) << k_funcinfo << "the group marked with id #" << it.key() << " does not seems to be anymore on the server" << endl;

			continue;
		}

		Kopete::Group *group=it.data();
		if(!group) //we can't trust the data of it()   see in MSNProtocol::deserializeContact why
			group=static_cast<MSNAccount*>( account() )->m_groupList[it.key()];
		if( !metaContact()->groups().contains(group) )
		{
			m_moving=true;
			notify->removeContact( contactId(), MSNProtocol::FL, guid(), it.key() );
			count--;
		}
	}
	
	for(QValueList<QString>::Iterator it= removinglist.begin() ; it != removinglist.end() ; ++it )
		contactRemovedFromGroup(*it);

	//FINAL TEST: is the contact at least in a group..
	//   this may happens if we just added a temporary contact to top-level
	//   we add the contact to the group #0 (the default one)
	/*if(count==0)
	{
//		notify->addContact( contactId(), MSNProtocol::FL, QString::null, guid(), "0");
	}*/
}

void MSNContact::contactAddedToGroup( const QString& groupId, Kopete::Group *group )
{
	m_serverGroups.insert( groupId, group );
	m_moving=false;
}

void MSNContact::contactRemovedFromGroup( const QString& groupId )
{
	m_serverGroups.remove( groupId );
	if(m_serverGroups.isEmpty() && !m_moving)
	{
		deleteLater();
	}
	m_moving=false;
}


void MSNContact::rename( const QString &newName )
{
	//kdDebug( 14140 ) << k_funcinfo << "From: " << displayName() << ", to: " << newName << endl;

/*	if( newName == displayName() )
		return;*/

	// FIXME: This should be called anymore.
	MSNNotifySocket *notify = static_cast<MSNAccount*>( account() )->notifySocket();
	if( notify )
	{
		notify->changePublicName( newName, contactId() );
	}
}

void MSNContact::slotShowProfile()
{
	KRun::runURL( KURL( QString::fromLatin1("http://members.msn.com/?pgmarket=it-it&mem=") + contactId())  , "text/html" );
}


/**
 * FIXME: Make this a standard KMM API call
 */
void MSNContact::sendFile( const KURL &sourceURL, const QString &altFileName, uint /*fileSize*/ )
{
	QString filePath;

	//If the file location is null, then get it from a file open dialog
	if( !sourceURL.isValid() )
		filePath = KFileDialog::getOpenFileName( QString::null ,"*", 0l  , i18n( "Kopete File Transfer" ));
	else
		filePath = sourceURL.path(-1);

	//kdDebug(14140) << "MSNContact::sendFile: File chosen to send:" << fileName << endl;

	if ( !filePath.isEmpty() )
	{
		Q_UINT32 fileSize = QFileInfo(filePath).size();
		//Send the file
		static_cast<MSNChatSession*>( manager(Kopete::Contact::CanCreate) )->sendFile( filePath, altFileName, fileSize );

	}
}

void MSNContact::setOnlineStatus(const Kopete::OnlineStatus& status)
{
	if(isBlocked() && status.internalStatus() < 15)
	{
		Kopete::Contact::setOnlineStatus(
				Kopete::OnlineStatus(status.status() ,
				(status.weight()==0) ? 0 : (status.weight() -1)  ,
				protocol() ,
				status.internalStatus()+15 ,
				status.overlayIcons() + QStringList("msn_blocked") ,
				i18n("%1|Blocked").arg( status.description() ) ) );
	}
	else if(!isBlocked() && status.internalStatus() >= 15)
	{	//the user is not blocked, but the status is blocked
		switch(status.internalStatus()-15)
		{
			case 1:
				Kopete::Contact::setOnlineStatus(MSNProtocol::protocol()->NLN);
				break;
			case 2:
				Kopete::Contact::setOnlineStatus(MSNProtocol::protocol()->BSY);
				break;
			case 3:
				Kopete::Contact::setOnlineStatus(MSNProtocol::protocol()->BRB);
				break;
			case 4:
				Kopete::Contact::setOnlineStatus(MSNProtocol::protocol()->AWY);
				break;
			case 5:
				Kopete::Contact::setOnlineStatus(MSNProtocol::protocol()->PHN);
				break;
			case 6:
				Kopete::Contact::setOnlineStatus(MSNProtocol::protocol()->LUN);
				break;
			case 7:
				Kopete::Contact::setOnlineStatus(MSNProtocol::protocol()->FLN);
				break;
			case 8:
				Kopete::Contact::setOnlineStatus(MSNProtocol::protocol()->HDN);
				break;
			case 9:
				Kopete::Contact::setOnlineStatus(MSNProtocol::protocol()->IDL);
				break;
			default:
				Kopete::Contact::setOnlineStatus(MSNProtocol::protocol()->UNK);
				break;
		}
	}
	else
		Kopete::Contact::setOnlineStatus(status);
	m_currentStatus=status;
}

void MSNContact::slotSendMail()
{
	MSNNotifySocket *notify = static_cast<MSNAccount*>( account() )->notifySocket();
	if( notify )
	{
		notify->sendMail( contactId() );
	}
}

void MSNContact::setDisplayPicture(KTempFile *f)
{
	//copy the temp file somewere else.
	// in a better world, the file could be dirrectly wrote at the correct location.
	// but the custom emoticon code is to deeply merged in the display picture code while it could be separated.
	QString newlocation=locateLocal( "appdata", "msnpictures/"+ contactId().lower().replace(QRegExp("[./~]"),"-")  +".png"  ) ;

	KIO::Job *j=KIO::file_move( KURL::fromPathOrURL( f->name() ) , KURL::fromPathOrURL( newlocation ) , -1, true /*overwrite*/ , false /*resume*/ , false /*showProgressInfo*/ );
	
	f->setAutoDelete(false);
	delete f;

	//let the time to KIO to copy the file
	connect(j, SIGNAL(result(KIO::Job *)) , this, SLOT(slotEmitDisplayPictureChanged() ));
}

void MSNContact::slotEmitDisplayPictureChanged()
{
	QString newlocation=locateLocal( "appdata", "msnpictures/"+ contactId().lower().replace(QRegExp("[./~]"),"-")  +".png"  ) ;
	setProperty( Kopete::Global::Properties::self()->photo() , newlocation );
	emit displayPictureChanged();
}

void MSNContact::setObject(const QString &obj)
{
	if(m_obj==obj && (obj.isEmpty() || hasProperty(Kopete::Global::Properties::self()->photo().key())))
		return;

	m_obj=obj;

	removeProperty( Kopete::Global::Properties::self()->photo()  ) ;
	emit displayPictureChanged();

	KConfig *config = KGlobal::config();
	config->setGroup( "MSN" );
	if ( config->readNumEntry( "DownloadPicture", 2 ) >= 2 && !obj.isEmpty() 
			 && account()->myself()->onlineStatus().status() != Kopete::OnlineStatus::Invisible )
		manager(Kopete::Contact::CanCreate); //create the manager which will download the photo automatically.
}

#include "msncontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

