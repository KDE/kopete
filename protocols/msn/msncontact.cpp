/*
    msncontact.cpp - MSN Contact

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002      by Ryan Cumming           <bodnar42@phalynx.dhs.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2004 by Olivier Goffart        <ogoffart@tiscalinet.be>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

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
#include <kmessagebox.h>
#include <krun.h>
#include <ktempfile.h>

#include "kopetecontactlist.h"
#include "kopetemessagemanagerfactory.h"
#include "kopetemetacontact.h"
#include "kopetegroup.h"
#include "kopeteuiglobal.h"
#include "kopeteglobal.h"

#include "msninfo.h"
#include "msnmessagemanager.h"
#include "msnnotifysocket.h"
#include "msnaccount.h"

MSNContact::MSNContact( KopeteAccount *account, const QString &id, KopeteMetaContact *parent )
: KopeteContact( account, id, parent )
{
	m_displayPicture = 0L;

	//m_deleted = false;
	m_allowed = false;
	m_blocked = false;
	m_reversed = false;
	m_moving = false;

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
	delete m_displayPicture;
}

bool MSNContact::isReachable()
{
	if ( account()->isConnected() && isOnline() && account()->myself()->onlineStatus() != MSNProtocol::protocol()->HDN )
		return true;

	MSNMessageManager *kmm=dynamic_cast<MSNMessageManager*>(manager(false));
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

KopeteMessageManager *MSNContact::manager( bool canCreate )
{
	KopeteContactPtrList chatmembers;
	chatmembers.append(this);

	KopeteMessageManager *_manager = KopeteMessageManagerFactory::factory()->findKopeteMessageManager(  account()->myself(), chatmembers, protocol() );
	MSNMessageManager *manager = dynamic_cast<MSNMessageManager*>( _manager );
	if(!manager &&  canCreate)
	{
		manager = new MSNMessageManager( protocol(), account()->myself(), chatmembers  );
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
	}
	else
		actionBlock->setText( label );

	actionSendMail->setEnabled( static_cast<MSNAccount*>(account())->isHotmail());

	m_actionCollection->append( actionBlock );
	m_actionCollection->append( actionShowProfile );
	m_actionCollection->append( actionSendMail );

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
		notify->removeContact( contactId(), 0, MSNProtocol::BL );
	}
	else
	{
		if(m_allowed)
			notify->removeContact( contactId(), 0, MSNProtocol::AL);
		else
			notify->addContact( contactId(), contactId(), 0, MSNProtocol::BL );
	}
}

void MSNContact::slotUserInfo()
{
	KDialogBase *infoDialog=new KDialogBase( 0l, "infoDialog", /*modal = */false, QString::null, KDialogBase::Close , KDialogBase::Close, false );
	QString nick=property( Kopete::Global::Properties::self()->nickName()).value().toString();
	MSNInfo *info=new MSNInfo ( infoDialog,"info");
	info->m_id->setText( contactId() );
	info->m_displayName->setText(nick);
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

void MSNContact::slotDeleteContact()
{
	kdDebug( 14140 ) << k_funcinfo << endl;

	MSNNotifySocket *notify = static_cast<MSNAccount*>( account() )->notifySocket();
	if( notify )
	{
		if( m_serverGroups.isEmpty() || onlineStatus() == MSNProtocol::protocol()->UNK )
		{
			kdDebug( 14140 ) << k_funcinfo << "The contact is already removed from server, just delete it" << endl;
			deleteLater();
			return;
		}

		for( QMap<uint, KopeteGroup*>::Iterator it = m_serverGroups.begin(); it != m_serverGroups.end(); ++it )
			notify->removeContact( contactId(), it.key(), MSNProtocol::FL );
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
		setOnlineStatus(onlineStatus());
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
	for( QMap<uint, KopeteGroup *>::ConstIterator it = m_serverGroups.begin(); it != m_serverGroups.end(); ++it )
	{
		if( !firstEntry )
		{
			groups += ",";
			firstEntry = true;
		}
		groups += QString::number( it.key() );
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
}

QString MSNContact::phoneHome(){ return m_phoneHome ;}
QString MSNContact::phoneWork(){ return m_phoneWork ;}
QString MSNContact::phoneMobile(){ return m_phoneMobile ;}


const QMap<uint, KopeteGroup*>  MSNContact::serverGroups() const
{
	return m_serverGroups;
}
void MSNContact::clearServerGroups() 
{
	m_serverGroups.clear();
}


void MSNContact::syncGroups( )
{
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
		account()->setPluginData(protocol(),"serial", QString::null );
		return;
	}

	unsigned int count=m_serverGroups.count();

	//STEP ONE : add the contact to every kopetegroups where the MC is
	QPtrList<KopeteGroup> groupList = metaContact()->groups();
	for ( KopeteGroup *group = groupList.first(); group; group = groupList.next() )
	{
		//For each group, ensure it is on the MSN server
		if( !group->pluginData( protocol() , account()->accountId() + " id" ).isEmpty() )
		{
			int Gid=group->pluginData( protocol(), account()->accountId() + " id" ).toUInt();
			if( !static_cast<MSNAccount*>( account() )->m_groupList.contains(Gid) ) 
			{ // ohoh!   something is corrupted on the contactlist.xml
			  // anyway, we never should add a contact to an unexisting group on the server.
			  //     This shouln't be possible anymore  2004-06-10 -Olivier

				//repair the problem
				group->setPluginData( protocol() , account()->accountId() + " id" , QString::null);
				group->setPluginData( protocol() , account()->accountId() + " displayName" , QString::null);
				kdWarning( 14140 ) << k_funcinfo << " Group " << group->displayName() << " marked with id #" <<Gid << " does not seems to be anymore on the server" << endl;

				if(!group->displayName().isEmpty() && group->type() == KopeteGroup::Normal) //not the top-level
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
				QString nick=property( Kopete::Global::Properties::self()->nickName()).value().toString();
				notify->addContact( contactId(), nick.isEmpty() ? contactId() : nick, Gid, MSNProtocol::FL );
				count++;
				m_moving=true;
			}
		}
		else
		{
			if(!group->displayName().isEmpty() && group->type() == KopeteGroup::Normal) //not the top-level
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
	
	QValueList<int> removinglist; //contact is not in that group. on the server. we will remove them dirrectly after the loop
	
	for( QMap<uint, KopeteGroup*>::Iterator it = m_serverGroups.begin();(count > 1 && it != m_serverGroups.end()); ++it )
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

		KopeteGroup *group=it.data();
		if(!group) //we can't trust the data of it()   see in MSNProtocol::deserializeContact why
			group=static_cast<MSNAccount*>( account() )->m_groupList[it.key()];
		if( !metaContact()->groups().contains(group) )
		{
			m_moving=true;
			notify->removeContact( contactId(), it.key(), MSNProtocol::FL );
			count--;
		}
	}
	
	for(QValueList<int>::Iterator it= removinglist.begin() ; it != removinglist.end() ; ++it )
		contactRemovedFromGroup(*it);

	//FINAL TEST: is the contact at least in a group..
	//   this may happens if we just added a temporary contact to top-level
	//   we add the contact to the group #0 (the default one)
	if(count==0)
	{
		QString nick=property( Kopete::Global::Properties::self()->nickName()).value().toString();
		notify->addContact( contactId(), nick.isEmpty() ? contactId() : nick, 0, MSNProtocol::FL );
	}


}

void MSNContact::contactAddedToGroup( uint groupNumber, KopeteGroup *group )
{
	m_serverGroups.insert( groupNumber, group );
	m_moving=false;
}

void MSNContact::contactRemovedFromGroup( unsigned int group )
{
	m_serverGroups.remove( group );
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

	MSNNotifySocket *notify = static_cast<MSNAccount*>( account() )->notifySocket();
	if( notify )
	{
		notify->changePublicName( newName, contactId() );
	}
}

void MSNContact::slotShowProfile()
{
	KRun::runURL( KURL( QString::fromLatin1("http://members.msn.com/default.msnw?mem=") + contactId())  , "text/html" );
}


/**
 * FIXME: Make this a standard KMM API call
 */
void MSNContact::sendFile( const KURL &sourceURL, const QString &altFileName, uint fileSize )
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
		//Send the file
		static_cast<MSNMessageManager*>( manager(true) )->sendFile( filePath, altFileName, fileSize );

	}
}

void MSNContact::setOnlineStatus(const KopeteOnlineStatus& status)
{
	if(isBlocked() && status.internalStatus() < 15)
	{
		KopeteContact::setOnlineStatus(KopeteOnlineStatus(status.status() , (status.weight()==0) ? 0 : (status.weight() -1)  ,
			protocol() , status.internalStatus()+15 , QString::fromLatin1("msn_blocked"),
			status.caption() ,  i18n("%1|Blocked").arg( status.description() ) ) );
	}
	else
	{
		if(status.internalStatus() >= 15)
		{	//the user is not blocked, but the status is blocked
			switch(status.internalStatus()-15)
			{
				case 1:
					KopeteContact::setOnlineStatus(MSNProtocol::protocol()->NLN);
					break;
				case 2:
					KopeteContact::setOnlineStatus(MSNProtocol::protocol()->BSY);
					break;
				case 3:
					KopeteContact::setOnlineStatus(MSNProtocol::protocol()->BRB);
					break;
				case 4:
					KopeteContact::setOnlineStatus(MSNProtocol::protocol()->AWY);
					break;
				case 5:
					KopeteContact::setOnlineStatus(MSNProtocol::protocol()->PHN);
					break;
				case 6:
					KopeteContact::setOnlineStatus(MSNProtocol::protocol()->LUN);
					break;
				case 7:
					KopeteContact::setOnlineStatus(MSNProtocol::protocol()->FLN);
					break;
				case 8:
					KopeteContact::setOnlineStatus(MSNProtocol::protocol()->HDN);
					break;
				case 9:
					KopeteContact::setOnlineStatus(MSNProtocol::protocol()->IDL);
					break;
				default:
					KopeteContact::setOnlineStatus(MSNProtocol::protocol()->UNK);
					break;
			}
		}
		else
			KopeteContact::setOnlineStatus(status);
	}
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
	if(m_displayPicture && m_displayPicture!=f)
		delete m_displayPicture;
	m_displayPicture=f;
	emit displayPictureChanged();
}

void MSNContact::setObject(const QString &obj)
{
	if(m_obj==obj)
		return;

	m_obj=obj;
	delete m_displayPicture;
	m_displayPicture=0;
	emit displayPictureChanged();
}

#include "msncontact.moc"

// vim: set noet ts=4 sts=4 sw=4:

