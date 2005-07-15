/*
    msnnotifysocket.cpp - Notify Socket for the MSN Protocol

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart at kde.org>
	Copyright (c) 2005      by Michaël Larouche       <shock@shockdev.ca.tc>
	Copyright (c) 2005      by Gregg Edghill          <gregg.edghill@gmail.com>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    Portions taken from
    KMerlin   (c) 2001      by Olaf Lueg              <olueg@olsd.de>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "msnnotifysocket.h"
#include "msncontact.h"
#include "msnaccount.h"
#include "msnsecureloginhandler.h"
#include "msnchallengehandler.h"

#include <qdatetime.h>
#include <qregexp.h>

#include <kdebug.h>
#include <kdeversion.h>
#include <klocale.h>
#include <kmdcodec.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <krun.h>
#include <kio/job.h>
#include <qfile.h>
#include <kconfig.h>
#include <knotification.h>


#include "kopeteuiglobal.h"
#include "kopeteglobal.h"

#include <ctime>

MSNNotifySocket::MSNNotifySocket( MSNAccount *account, const QString& /*msnId*/, const QString &password )
: MSNSocket( account )
{
	m_newstatus = MSNProtocol::protocol()->NLN;
	m_secureLoginHandler=0L;
	m_challengeHandler = new MsnChallengeHandler("YMM8C_H7KCQ2S_KL", "PROD0090YUAUV{2B");

	m_isHotmailAccount=false;
	m_ping=false;
	m_disconnectReason=Kopete::Account::Unknown;

	m_account = account;
	m_password=password;
	QObject::connect( this, SIGNAL( blockRead( const QString & ) ),
		this, SLOT( slotReadMessage( const QString & ) ) );

	m_keepaliveTimer = new QTimer( this, "m_keepaliveTimer" );
	QObject::connect( m_keepaliveTimer, SIGNAL( timeout() ), SLOT( slotSendKeepAlive() ) );
}

MSNNotifySocket::~MSNNotifySocket()
{
	delete m_secureLoginHandler;
	delete m_challengeHandler;

	kdDebug(14140) << k_funcinfo << endl;
}

void MSNNotifySocket::doneConnect()
{
//	kdDebug( 14140 ) << k_funcinfo << "Negotiating server protocol version" << endl;
	sendCommand( "VER", "MSNP11 MSNP10 CVR0" );
}


void MSNNotifySocket::disconnect()
{
	if(	m_disconnectReason==Kopete::Account::Unknown )
		m_disconnectReason=Kopete::Account::Manual;
	if( onlineStatus() == Connected )
		sendCommand( "OUT", QString::null, false );

	m_keepaliveTimer->stop();

	// the socket is not connected yet, so I should force the signals
	if ( onlineStatus() == Disconnected || onlineStatus() == Connecting )
		emit socketClosed();
	else
		MSNSocket::disconnect();
}

void MSNNotifySocket::handleError( uint code, uint id )
{
	QString handle;
	if(m_tmpHandles.contains(id))
		handle=m_tmpHandles[id];

	// See http://www.hypothetic.org/docs/msn/basics.php for a
	// description of all possible error codes.
	// TODO: Add support for all of these!
	switch( code )
	{
	case 201:
	case 205:
	case 208:
	{
		QString msg = i18n( "<qt>The MSN user '%1' does not exist.<br>Please check the MSN ID.</qt>" ).arg( handle );
		KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry, msg, i18n( "MSN Plugin" ) );
		break;
	}
	case 207:
	case 218:
	case 540:
	{
		KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Information,
			i18n( "<qt>An internal error occurred in the MSN plugin.<br>"
			      "MSN Error: %1<br>"
			      "please send us a detailed bug report "
			      "at kopete-devel@kde.org containing the raw debug output on the "
			      "console (in gzipped format, as it is probably a lot of output.)" ).arg(code) ,
			i18n( "MSN Internal Error" ) );
		break;

	}
	case 209:
	{
		if(handle==m_account->accountId())
		{
			QString msg = i18n( "Unable to change your display name.\n"
				"Please ensure your display is not too long and does not contains censored words." );
			KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry, msg, i18n( "MSN Plugin" ) );
		}
		/*else
		 {
			QString msg = i18n( "You are trying to change the display name of a user who has not "
				"confirmed his or her email address;\n"
				"the contact was not renamed on the server." );
			KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Error, msg, i18n( "MSN Plugin" ) );
		}*/
		break;
	}
	case 210:
	{
		KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry,
			i18n("Your contact list is full; you cannot add any new contacts."),
			i18n( "MSN Contact List Full" ) );
		break;
	}
	case 215:
	{
		QString msg = i18n( "<qt>The user '%1' already exists in this group on the MSN server;<br>"
			"if Kopete does not show the user, please send us a detailed bug report "
			"at kopete-devel@kde.org containing the raw debug output on the "
			"console (in gzipped format, as it is probably a lot of output.)</qt>" ).arg(handle);
		KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Information, msg, i18n( "MSN Plugin" ) );
		break;
	}
	case 216:
	{
		//This might happen is you rename an user if he is not in the contactlist
		//currently, we just iniore;
		//TODO: try to don't rename user not in the list
		//actualy, the bug is in MSNChatSession::slotUserJoined()
		break;
	}
	case 219:
	{
		QString msg = i18n( "The user '%1' seems to already be blocked or allowed on the server." ).arg(handle);
		KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry, msg, i18n( "MSN Plugin" ) );
		break;
	}
	case 223:
	{
		QString msg = i18n( "You have reached the maximum number of groups:\n"
			"MSN does not support more than 30 groups." );
		KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry, msg, i18n( "MSN Plugin" ) );
		break;
	}
	case 224:
	case 225:
	case 230:
	{
		KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Information,
			i18n("Kopete is trying to perform an operation on a group or a contact that does not exists on the server.\n"
			"This might happen if the Kopete contact list and the MSN-server contact list are not correctly synchronized; if this is the case, you probably should send a bug report."),
			i18n( "MSN Plugin" ) );
		break;
	}

	case 229:
	{
		KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry,
			i18n("The group name is too long; it has not been changed on the MSN server."),
			i18n( "Invalid group name - MSN Plugin" ) );
		break;
	}
	case 710:
	{
		QString msg = i18n( "You cannot open a Hotmail inbox because you do not have an MSN account with a valid "
			"Hotmail or MSN mailbox." );
		KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry, msg, i18n( "MSN Plugin" ) );
		break;
	}
	case 715:
	{
		/*
		//if(handlev==m_account->accountId())
		QString msg = i18n( "Your email address has not been verified with the MSN server.\n"
			"You should have received a mail with a link to confirm your email address.\n"
			"Some functions will be restricted if you do not confirm your email address." );
		KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry, msg, i18n( "MSN Plugin" ) );//TODO don't show again
		*/
		break;
	}
	case 800:
	{
		//This happen when too much commends are sent to the server.
		//the command will not be executed, too bad.
		// ignore it for now, as we don't really know what command it was.
/*		QString msg = i18#n( "You are trying to change your status, or your display name too rapidly.\n"
	 		"This might happen if you added yourself to your own contact list." );
		KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry, msg, i18n( "MSN Plugin" ) );
		//FIXME: try to fix this problem*/
		break;
	}
	case 911:
		m_disconnectReason=Kopete::Account::BadPassword;
		disconnect();
		break;
	case 913:
	{
		QString msg = i18n( "You can not send messages when you are offline or when you are invisible." );
		KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry, msg, i18n( "MSN Plugin" ) );
		break;
	}
	case 923:
		KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Sorry,
			i18n( "You are trying to perform an action you are not allowed to perform in 'kid mode'." ) ,
			i18n( "MSN Plugin" ) );
		break;

	default:
		MSNSocket::handleError( code, id );
		break;
	}
}

void MSNNotifySocket::parseCommand( const QString &cmd, uint id, const QString &data )
{
	//kdDebug(14140) << "MSNNotifySocket::parseCommand: Command: " << cmd << endl;

	if ( cmd == "VER" )
	{
		sendCommand( "CVR", "0x0409 winnt 5.1 i386 MSNMSGR 7.0.0813 MSMSGS " + m_account->accountId() );
/*
		struct utsname utsBuf;
		uname ( &utsBuf );

		sendCommand( "CVR", i18n( "MS Local code, see http://www.microsoft.com/globaldev/reference/oslocversion.mspx", "0x0409" ) +
			" " + escape( utsBuf.sysname ) + " " + escape( utsBuf.release ) + " " + escape( utsBuf.machine ) + " Kopete " +
			escape( kapp->aboutData()->version() ) + " Kopete " + m_msnId );
*/
	}
	else if ( cmd == "CVR" ) //else if ( cmd == "INF" )
	{
		sendCommand( "USR", "TWN I " + m_account->accountId() );
	}
	else if( cmd == "USR" ) //// here follow the auth processus
	{
		if( data.section( ' ', 1, 1 ) == "S" )
		{
			m_secureLoginHandler = new MSNSecureLoginHandler(m_account->accountId(), m_password, data.section( ' ' , 2 , 2 ));

			QObject::connect(m_secureLoginHandler, SIGNAL(loginFailed()), this, SLOT(sslLoginFailed()));
			QObject::connect(m_secureLoginHandler, SIGNAL(loginSuccesful(QString )), this, SLOT(sslLoginSucceeded(QString )));

			m_secureLoginHandler->login();
		}
		else
		{
			// Successful authentication.
			m_disconnectReason=Kopete::Account::Unknown;

			// Synchronize with the server.
			QString lastSyncTime, lastChange;
			
			// Retrieve the last synchronization timestamp, and last change timestamp.
			lastSyncTime = m_account->configGroup()->readEntry( "lastsynctime", "0");
			lastChange = m_account->configGroup()->readEntry("lastchange", "0");
			
			sendCommand( "SYN", lastChange + " " + lastSyncTime);

			// We are connected start to ping
			slotSendKeepAlive();
		}
	}
	else if( cmd == "LST" ) 
	{
		// MSNP11 changed command. Now it's: 
		// LST N=passport@hotmail.com F=Display%20Name C=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx 13 xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
		// But can be
		// LST N=passport@hotmail.com 10
		QString publicName, contactGuid, groups;
		uint lists;

		QRegExp regex("N=(\\S+)\\s?(?:F=(\\S+))?\\s?(?:C=([0-9a-fA-F]{8}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{12}))?\\s?(\\d+)\\s?((?:[0-9a-fA-F]{8}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{12},?)*)$");
		regex.search(data);
		
		// Capture passport email.
		m_tmpLastHandle = regex.cap(1);
		// Capture public name.
		publicName = unescape( regex.cap(2) );
		// Capture contact guid.
		contactGuid = regex.cap(3);
		// Capture list enum type.
		lists = regex.cap(4).toUInt();
		// Capture contact group(s) guid(s)
		groups = regex.cap(5);
		
// 		kdDebug(14140) << k_funcinfo << " msnId: " << m_tmpLastHandle << " publicName: " << publicName << " contactGuid: " << guid << " list: " << lists << " groupGuid: " << groups << endl;

		// handle, publicName, Contact GUID, lists, Group GUID
		emit contactList(  m_tmpLastHandle , publicName, contactGuid, lists, groups );
	}
	else if( cmd == "GCF" )
	{
		m_configFile = data.section(' ', 0, 0);
		readBlock( data.section( ' ', 1, 1 ).toUInt() );
	}
	else if( cmd == "MSG" )
	{
		readBlock( data.section( ' ', 2, 2 ).toUInt() );
	}
	else if( cmd == "ILN" ||  cmd == "NLN" )
	{
		// status handle publicName strangeNumber MSNOBJECT
		MSNContact *c = static_cast<MSNContact*>( m_account->contacts()[ data.section( ' ', 1, 1 ) ] );
		if( c && c->contactId() != m_account->accountId() )
		{
			QString publicName=unescape( data.section( ' ', 2, 2 ) );
			if ( (publicName!=c->contactId() ||  c->hasProperty(Kopete::Global::Properties::self()->nickName().key())  ) &&
						 publicName!=c->property( Kopete::Global::Properties::self()->nickName()).value().toString() )
						 
				changePublicName(publicName,c->contactId());
			QString obj=unescape(data.section( ' ', 4, 4 ));
			c->setObject( obj );
			c->setOnlineStatus( convertOnlineStatus( data.section( ' ', 0, 0 ) ) );

			if(!c->hasProperty( MSNProtocol::protocol()->propClient.key() ))
			{
				unsigned int clientID=data.section( ' ', 3, 3 ).toUInt();
				if( clientID & 512)
					c->setProperty(  MSNProtocol::protocol()->propClient , i18n("Web Messenger") );
				else if(clientID & 1)
					c->setProperty(  MSNProtocol::protocol()->propClient , i18n("Windows Mobile") );
				else if(clientID & 64)
					c->setProperty(  MSNProtocol::protocol()->propClient , i18n("MSN Mobile") );
				else if(obj.contains("kopete")  )
					c->setProperty(  MSNProtocol::protocol()->propClient , i18n("Kopete") );
			}
		}
	}
	else if( cmd == "UBX" )
	{
		m_tmpLastHandle = data.section(' ', 0, 0);
		uint length = data.section( ' ', 1, 1 ).toUInt();
		if(length > 0) {
			readBlock( length );
		}
	}
	else if( cmd == "UUX" )
	{
		// Do nothing.
	}
	else if( cmd == "FLN" )
	{
		MSNContact *c = static_cast<MSNContact*>( m_account->contacts()[ data.section( ' ', 0, 0 ) ] );
		if( c && c->contactId() != m_account->accountId() )
		{
			c->setOnlineStatus( MSNProtocol::protocol()->FLN );
			c->removeProperty(  MSNProtocol::protocol()->propClient );
		}
	}
	else if( cmd == "XFR" )
	{
		QString stype=data.section( ' ', 0, 0 );
		if( stype=="SB" ) //switchboard connection (chat)
		{
			// Address, AuthInfo
			emit startChat( data.section( ' ', 1, 1 ), data.section( ' ', 3, 3 ) );
		}
		else if( stype=="NS" ) //notifysocket  ; Got our notification server
		{ //we are connecting and we receive the initial NS, or the msn server encounter a problem, and we are switching to another switchboard
			QString host = data.section( ' ', 1, 1 );
			QString server = host.section( ':', 0, 0 );
			uint port = host.section( ':', 1, 1 ).toUInt();
			setOnlineStatus( Connected );
			emit receivedNotificationServer( server, port );
			disconnect();
		}

	}
	else if( cmd == "RNG" )
	{
		// SessionID, Address, AuthInfo, handle, publicName
		emit invitedToChat( QString::number( id ), data.section( ' ', 0, 0 ), data.section( ' ', 2, 2 ),
			data.section( ' ', 3, 3 ), unescape( data.section( ' ', 4, 4 ) ) );
	}
	else if( cmd == "ADC" )
	{
		QString msnId, list, publicName, contactGuid, groupGuid;

		// Retrieve the list parameter (FL/AL/BL/RL)
		list = data.section( ' ', 0, 0 );

		// Examples of received data
		// ADC TrID xL N=example@passport.com
		// ADC TrID FL C=contactGuid groupdGuid
		// ADC TrID RL N=example@passport.com F=friednly%20name
		// ADC TrID FL N=ex@pas.com F=My%20Name C=contactGuid
		// Thanks Gregg for that complex RegExp.
		QRegExp regex("(?:N=(\\S+))?\\s?(?:F=(\\S+))?\\s?(?:C=([0-9a-fA-F]{8}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{12}))?\\s?((?:[0-9a-fA-F]{8}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{12},?)*)$");
		regex.search( data.section( ' ', 1 ) );
		
		// Capture passport email.
		msnId = regex.cap(1);
		// Capture public name.
		publicName = unescape( regex.cap(2) );
		// Capture contact guid.
		contactGuid = regex.cap(3);
		// Capture contact group(s) guid(s)
		groupGuid = regex.cap(4);

// 		kdDebug(14140) << k_funcinfo << list << " msnId: " << msnId << " publicName: " << publicName << " contactGuid: " << contactGuid << " groupGuid: " << groupGuid << endl;

		// handle, list, publicName, contactGuid, groupGuid
		emit contactAdded( msnId, list, publicName, contactGuid, groupGuid );
	}
	else if( cmd == "REM" ) // someone is removed from a list
	{
		QString handle, list, contactGuid, groupGuid;
		list = data.section( ' ', 0, 0 );
		if( list  == "FL" )
		{
			// Removing a contact
		 	if( data.contains( ' ' ) < 2 )
			{
				contactGuid = data.section( ' ', 1, 1 );
			}
			// Removing a contact from a group
			else if( data.contains( ' ' ) < 3 )
			{
				contactGuid = data.section( ' ', 1, 1 );
				groupGuid = data.section( ' ', 2, 2 );
			}
		}
		else
		{
			handle = data.section( ' ', 1, 1);
		}

		// handle, list, contactGuid, groupGuid
		emit contactRemoved( handle, list, contactGuid, groupGuid );
		
	}
	else if( cmd == "OUT" )
	{
		if( data.section( ' ', 0, 0 ) == "OTH" )
		{
			m_disconnectReason=Kopete::Account::OtherClient;
		}
		disconnect();
	}
	else if( cmd == "CHG" )
	{
		QString status = data.section( ' ', 0, 0 );
		setOnlineStatus( Connected );
		emit statusChanged( convertOnlineStatus( status ) );
	}
	else if( cmd == "SBP" )
	{
		QString contactGuid, type, publicName;
		contactGuid = data.section( ' ', 0, 0 );
		type = data.section( ' ', 1, 1 );
		if(type == "MFN" )
		{
			publicName = unescape( data.section( ' ', 2, 2 ) );
			MSNContact *c = m_account->findContactByGuid( contactGuid );
			if(c != 0L)
			{
				c->setProperty( Kopete::Global::Properties::self()->nickName(), publicName );
			}
		}
	}
	else if( cmd == "LSG" )
	{
		// New Format: LSG Friends xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
		// groupDisplayName, groupGuid
		emit groupListed( unescape( data.section( ' ', 0, 0 ) ), data.section( ' ', 1, 1) );
	}
	else if( cmd == "ADG" )
	{
		// groupName, groupGuid
		emit groupAdded( unescape( data.section( ' ', 0, 0 ) ),
			data.section( ' ', 1, 1 ) );
	}
	else if( cmd == "REG" )
	{
		// groupGuid, groupName
		emit groupRenamed( data.section( ' ', 0, 0 ), unescape( data.section( ' ', 1, 1 ) ) );
	}
	else if( cmd == "RMG" )
	{
		// groupGuid
		emit groupRemoved( data.section( ' ', 1, 1 ) );
	}
	else if( cmd  == "CHL" )
	{
		QString chlResponse = m_challengeHandler->computeHash(data.section(' ', 0, 0));
		sendCommand("QRY", m_challengeHandler->productId(), true, chlResponse.utf8());
	}
	else if( cmd == "SYN" )
	{
		// Retrieve the last synchronization timestamp known to the server.
		QString lastSyncTime = data.section( ' ', 1, 1 );
		if( lastSyncTime != m_account->configGroup()->readEntry("lastsynctime") )
		{
			// If the server timestamp and the local timestamp are different,
			// prepare to receive the contact list.
			emit newContactList();  // remove all contacts datas, msn sends a new contact list
			m_account->configGroup()->writeEntry( "lastsynctime" , data.section( ' ', 1, 1 ) );
			m_account->configGroup()->writeEntry( "lastchange", data.section (' ', 0, 0 ) );
		}else
			kdDebug(14140) << k_funcinfo << "Contact list up-to-date." << endl;
			
		// set the status
		setStatus( m_newstatus );
	}
	else if( cmd == "BPR" )
	{
		MSNContact *c = static_cast<MSNContact*>( m_account->contacts()[ m_tmpLastHandle ] );
		if( c )
			c->setInfo(data.section( ' ', 0, 0 ),unescape(data.section( ' ', 1, 1 )));
	}
	else if( cmd == "PRP" )
	{
		MSNContact *c = static_cast<MSNContact*>( m_account->myself() );
		if( c )
		{
			QString type = data.section( ' ', 0, 0 );
			QString prpData = unescape( data.section( ' ', 1, 1 ) ); //SECURITY????????
			c->setInfo( type, prpData );
			m_account->configGroup()->writeEntry( type, prpData ); 
		}
	}
	else if( cmd == "BLP" )
	{
		if( id > 0 ) //FROM BLP
		{
			m_account->configGroup()->writeEntry( "serial" , data.section( ' ', 0, 0 ) );
			m_account->configGroup()->writeEntry( "BLP" , data.section( ' ', 1, 1 ) );
		}
		else //FROM SYN
			m_account->configGroup()->writeEntry( "BLP" , data.section( ' ', 0, 0 ) );
	}
	else if( cmd == "QRY" )
	{
		// Do nothing
	}
	else if( cmd == "QNG" )
	{
		//this is a reply from a ping
		m_ping=false;

		// id is the timeout in fact, and we remove 5% of it
		m_keepaliveTimer->start( id * 950, true );
		kdDebug( 14140 ) << k_funcinfo << "timerTimeout=" << id << "sec"<< endl;
	}
	else if( cmd == "URL" )
	{
		// URL 6 /cgi-bin/HoTMaiL https://loginnet.passport.com/ppsecure/md5auth.srf?lc=1033 2
		//example of reply: URL 10 /cgi-bin/HoTMaiL https://msnialogin.passport.com/ppsecure/md5auth.srf?lc=1036 3
		QString from_action_url = data.section( ' ', 1, 1 );
		QString rru = data.section( ' ', 0, 0 );
		QString id = data.section( ' ', 2, 2 );

		//write the tmp file
		QString UserID=m_account->accountId();

		time_t actualTime;
		time(&actualTime);
		QString sl = QString::number( ( unsigned long ) actualTime - m_loginTime.toULong() );

		QString md5this( m_MSPAuth + sl + m_password );
		KMD5 md5( md5this.utf8() );

		QString hotmailRequest = "<html>\n"
			"<head>\n"
				"<noscript>\n"
					"<meta http-equiv=Refresh content=\"0; url=http://www.hotmail.com\">\n"
				"</noscript>\n"
			"</head>\n"
			"<body onload=\"document.pform.submit(); \">\n"
				"<form name=\"pform\" action=\"" + from_action_url  + "\" method=\"POST\">\n"
					"<input type=\"hidden\" name=\"mode\" value=\"ttl\">\n"
					"<input type=\"hidden\" name=\"login\" value=\"" + UserID.left( UserID.find('@') ) + "\">\n"
					"<input type=\"hidden\" name=\"username\" value=\"" + UserID + "\">\n"
					"<input type=\"hidden\" name=\"sid\" value=\"" + m_sid + "\">\n"
					"<input type=\"hidden\" name=\"kv\" value=\"" + m_kv + "\">\n"
					"<input type=\"hidden\" name=\"id\" value=\""+ id +"\">\n"
					"<input type=\"hidden\" name=\"sl\" value=\"" + sl +"\">\n"
					"<input type=\"hidden\" name=\"rru\" value=\"" + rru + "\">\n"
					"<input type=\"hidden\" name=\"auth\" value=\"" + m_MSPAuth + "\">\n"
					"<input type=\"hidden\" name=\"creds\" value=\"" + QString::fromLatin1( md5.hexDigest() ) + "\">\n"
					"<input type=\"hidden\" name=\"svc\" value=\"mail\">\n"
					"<input type=\"hidden\" name=\"js\" value=\"yes\">\n"
				"</form></body>\n</html>\n";

		KTempFile tmpMailFile( locateLocal( "tmp", "kopetehotmail-" ), ".html" );
		*tmpMailFile.textStream() << hotmailRequest;
		tmpMailFile.file()->flush();

		KRun::runURL( KURL::fromPathOrURL( tmpMailFile.name() ), "text/html" , true );

	}
	else
	{
		// Let the base class handle the rest
		//MSNSocket::parseCommand( cmd, id, data );
		kdDebug( 14140 ) << k_funcinfo << "Unimplemented command '" << cmd << " " << id << " " << data << "' from server!" << endl;
	}
}


void MSNNotifySocket::sslLoginFailed()
{
	m_disconnectReason=Kopete::Account::InvalidHost;
	disconnect();
}

void MSNNotifySocket::sslLoginIncorrect()
{
	m_disconnectReason=Kopete::Account::BadPassword;
	disconnect();
}

void MSNNotifySocket::sslLoginSucceeded(QString ticket)
{
	sendCommand("USR" , "TWN S " + ticket);

	m_secureLoginHandler->deleteLater();
	m_secureLoginHandler = 0L;
}


void MSNNotifySocket::slotOpenInbox()
{
	sendCommand("URL", "INBOX" );
}

void MSNNotifySocket::sendMail(const QString &email)
{
	sendCommand("URL", QString("COMPOSE " + email).utf8() );
}

void MSNNotifySocket::slotReadMessage( const QString &msg )
{
	if(msg.contains("text/x-msmsgsinitialmdatanotification"))
	{
		//Mail-Data: <MD><E><I>301</I><IU>1</IU><O>4</O><OU>2</OU></E><Q><QTM>409600</QTM><QNM>204800</QNM></Q></MD>
		// MD - Mail Data
		// E  - email
		// I  - initial mail
		// IU - initial unread
		// O  - other mail
		// OU - other unread.
		QRegExp regex("<MD><E><I>(\\d+)?</I>(?:<IU>(\\d+)?</IU>)<O>(\\d+)?</O><OU>(\\d+)?</OU></E><Q>.*</Q></MD>");
		regex.search(msg);
		
		bool unread;
		// Retrieve the number of unread email messages.
		mailCount = regex.cap(2).toUInt(&unread);
		if(unread && mailCount > 0)
		{
			// If there are new email message available, raise the unread email event.
			QObject::connect(KNotification::event( "msn_mail", i18n( "You have one unread message in your MSN inbox.",
					"You have %n unread messages in your MSN inbox.", mailCount ), 0 , 0 , i18n( "Open &Inbox..." ) ),
				SIGNAL(activated(unsigned int ) ) , this, SLOT( slotOpenInbox() ) );
		}
	}
	else if(msg.contains("text/x-msmsgsactivemailnotification"))
	{
		 //this sends the server if mails are deleted
		 QString m = msg.right(msg.length() - msg.find("Message-Delta:") );
		 m = m.left(msg.find("\r\n"));
		 mailCount = mailCount - m.right(m.length() -m.find(" ")-1).toUInt();
	}
	else if(msg.contains("text/x-msmsgsemailnotification"))
	{
		 //this sends the server if a new mail has arrived
		QRegExp rx("From-Addr: ([A-Za-z0-9@._\\-]*)");
		rx.search(msg);
		QString m=rx.cap(1);

		mailCount++;

		//TODO:  it is also possible to get the subject  (but warning about the encoding)
		QObject::connect(KNotification::event( "msn_mail",i18n( "You have one new email from %1 in your MSN inbox." ).arg(m),
										0 , 0 , i18n( "Open &Inbox..." ) ),
				SIGNAL(activated(unsigned int ) ) , this, SLOT( slotOpenInbox() ) );
	}
	else if(msg.contains("text/x-msmsgsprofile"))
	{
		//Hotmail profile
		if(msg.contains("MSPAuth:"))
		{
			QRegExp rx("MSPAuth: ([A-Za-z0-9$!*]*)");
			rx.search(msg);
			m_MSPAuth=rx.cap(1);
		}
		if(msg.contains("sid:"))
		{
			QRegExp rx("sid: ([0-9]*)");
			rx.search(msg);
			m_sid=rx.cap(1);
		}
		if(msg.contains("kv:"))
		{
			QRegExp rx("kv: ([0-9]*)");
			rx.search(msg);
			m_kv=rx.cap(1);
		}
		if(msg.contains("LoginTime:"))
		{
			QRegExp rx("LoginTime: ([0-9]*)");
			rx.search(msg);
			m_loginTime=rx.cap(1);
		}
			else //IN MSNP9  there are no logintime it seems, so set it manualy
			{
				time_t actualTime;
				time(&actualTime);
				m_loginTime=QString::number((unsigned long)actualTime);
			}
		if(msg.contains("EmailEnabled:"))
		{
			QRegExp rx("EmailEnabled: ([0-9]*)");
			rx.search(msg);
			m_isHotmailAccount = (rx.cap(1).toUInt() == 1);
			emit hotmailSeted(m_isHotmailAccount);
		}
		if(msg.contains("ClientIP:"))
		{
			QRegExp rx("ClientIP: ([0-9.]*)");
			rx.search(msg);
			m_localIP = rx.cap(1);
		}
	}

	if(!m_configFile.isNull())
	{
		// TODO Global configuration file.
	}

	if(!m_tmpLastHandle.isNull())
	{
		QString personalMessage;
		QDomDocument psm;
		if( psm.setContent(msg) )
		{
			// Get the first child of the xml "document";
			QDomElement psmElement = psm.documentElement().firstChild().toElement();
	
			while( !psmElement.isNull() )
			{
				if(psmElement.tagName() == QString::fromUtf8("PSM"))
				{
					personalMessage = psmElement.text();
					kdDebug(14140) << k_funcinfo << "Personnal Message received: " << personalMessage << endl;
				}
				else if(psmElement.tagName() == QString::fromUtf8("CurrentMedia"))
				{
					//TODO: Process CurrentMedia
				}
				psmElement = psmElement.nextSibling().toElement();
			}
	
			MSNContact *contact = static_cast<MSNContact*>(m_account->contacts()[ m_tmpLastHandle ]);
			if(contact)
			{
				contact->setProperty(MSNProtocol::protocol()->propPersonalMessage, personalMessage);
			}
		}
		m_tmpLastHandle = QString::null;
	}
}

void MSNNotifySocket::addGroup(const QString& groupName)
{
	// escape spaces
	sendCommand( "ADG", escape( groupName ) );
}

void MSNNotifySocket::renameGroup( const QString& groupName, const QString& groupGuid )
{
	// escape spaces
	sendCommand( "REG", groupGuid + " " + escape( groupName ) );
}

void MSNNotifySocket::removeGroup( const QString& groupGuid )
{
	sendCommand( "RMG",  groupGuid );
}

void MSNNotifySocket::addContact( const QString &handle, int list, const QString& publicName, const QString& contactGuid, const QString& groupGuid )
{
	QString args;
	switch( list )
	{
		case MSNProtocol::FL:
		{
			// Adding the contact to a group
			if( !contactGuid.isEmpty() )
			{
				args = QString("FL C=%1 %2").arg( contactGuid ).arg( groupGuid );
				kdDebug(14140) << k_funcinfo << "In adding contact to a group" << endl;
			}
			// Adding a new contact
			else
			{
				args = QString("FL N=%1 F=%2").arg( handle ).arg( escape( publicName ) );
				kdDebug(14140) << k_funcinfo << "In adding contact to a new contact" << endl;
			}
			break;
		}	
		case MSNProtocol::AL:
			args = QString("AL N=%1").arg( handle );
			break;
		case MSNProtocol::BL:
			args = QString("BL N=%1").arg( handle );
			break;
		case MSNProtocol::RL:
			args = QString("RL N=%1").arg( handle );
			break;
		default:
			kdDebug(14140) << k_funcinfo <<"WARNING! Unknown list " << list << "!" << endl;
			return;
	}
	unsigned int id=sendCommand( "ADC", args );
	m_tmpHandles[id]=handle;
}

void MSNNotifySocket::removeContact( const QString &handle, int list, const QString& contactGuid, const QString& groupGuid )
{
	QString args;
	switch( list )
	{
	case MSNProtocol::FL:
		args = "FL " + contactGuid;
		// Removing a contact from a group
		if( !groupGuid.isEmpty() )
			args += " " + groupGuid;
		break;
	case MSNProtocol::AL:
		args = "AL " + handle;
		break;
	case MSNProtocol::BL:
		args = "BL " + handle;
		break;
	case MSNProtocol::PL:
		args = "PL " + handle;
		break;
	default:
		kdDebug(14140) <<k_funcinfo  << "WARNING! Unknown list " << list << "!" << endl;
		return;
	}
	unsigned int id=sendCommand( "REM", args );
	m_tmpHandles[id]=handle;
}

void MSNNotifySocket::setStatus( const Kopete::OnlineStatus &status )
{
//	kdDebug( 14140 ) << k_funcinfo << statusToString( status ) << endl;

	if( onlineStatus() == Disconnected )
		m_newstatus = status;
	else
		sendCommand( "CHG", statusToString( status ) + " " + MSNProtocol::protocol()->clientId() + " " + escape(m_account->pictureObject()) );
}

void MSNNotifySocket::changePublicName( const QString &publicName, const QString &handle )
{
	QString tempPublicName = publicName;

	if( escape(publicName).length() > 387 )
	{
		tempPublicName = publicName.left(387);
	}

	if( handle.isNull() )
	{
		unsigned int id = sendCommand( "PRP", "MFN " + escape( tempPublicName ) );
		m_tmpHandles[id] = m_account->accountId();
	}
	else
	{
		MSNContact *currentContact = static_cast<MSNContact *>(m_account->contacts()[handle]);
		if(currentContact)
		{
			unsigned int id = sendCommand( "SBP", currentContact->guid() + " MFN " + escape( tempPublicName ) );
			m_tmpHandles[id] = handle;
		}
	}
}

void MSNNotifySocket::changePersonalMessage( const QString& type, const QString &personalMessage )
{
	QString tempPersonalMessage = personalMessage;
	
	//Magic number : 129 characters
	if( escape(personalMessage).length() > 129 )
	{
		// We cut. for now.
		tempPersonalMessage = personalMessage.left(129);
	}

	QDomDocument xmlMessage;
	xmlMessage.appendChild( xmlMessage.createElement( "Data" ) );
	
	QDomElement psm = xmlMessage.createElement("PSM");
	psm.appendChild( xmlMessage.createTextNode( tempPersonalMessage ) );
	xmlMessage.documentElement().appendChild( psm );

	// TODO : Music type, Office type. Integrate with KOffice and the NowListening thingy.
	QDomElement currentMedia = xmlMessage.createElement("CurrentMedia");
	currentMedia.appendChild( xmlMessage.createTextNode( "" ) );
	xmlMessage.documentElement().appendChild( currentMedia );

	unsigned int id = sendCommand("UUX","",true, xmlMessage.toString().utf8(), false);
	m_tmpHandles[id] = m_account->accountId();

}

void MSNNotifySocket::changePhoneNumber( const QString &key, const QString &data )
{
	sendCommand( "PRP", key + " " + escape ( data ) );
}


void MSNNotifySocket::createChatSession()
{
	sendCommand( "XFR", "SB" );
}

QString MSNNotifySocket::statusToString( const Kopete::OnlineStatus &status ) const
{
	if( status == MSNProtocol::protocol()->NLN )
		return "NLN";
	else if( status == MSNProtocol::protocol()->BSY )
		return "BSY";
	else if( status == MSNProtocol::protocol()->BRB )
		return "BRB";
	else if( status == MSNProtocol::protocol()->AWY )
		return "AWY";
	else if( status == MSNProtocol::protocol()->PHN )
		return "PHN";
	else if( status == MSNProtocol::protocol()->LUN )
		return "LUN";
	else if( status == MSNProtocol::protocol()->FLN )
		return "FLN";
	else if( status == MSNProtocol::protocol()->HDN )
		return "HDN";
	else if( status == MSNProtocol::protocol()->IDL )
		return "IDL";
	else
	{
		kdWarning( 14140 ) << k_funcinfo << "Unknown status " << status.internalStatus() << "!" << endl;
		return "UNK";
	}
}

void MSNNotifySocket::slotSendKeepAlive()
{
	if(getTransport() == MSNSocket::HttpTransport)
	{
		// If the base socket is using the http transport,
		// disable the keep alive timer because it is not
		// needed since PNG is not supported in Msn http.
		m_keepaliveTimer->stop();
		return;
	}

	//we did not received the previous QNG
	if(m_ping)
	{
		m_disconnectReason=Kopete::Account::ConnectionReset;
		disconnect();
		/*KMessageBox::queuedMessageBox( Kopete::UI::Global::mainWidget(), KMessageBox::Information,
		i18n( "The connection with the MSN network has been lost." ) , i18n ("MSN Plugin") );*/
		return;
	}
	else
	{
		// Send a dummy command to fake activity. This makes sure MSN doesn't
		// disconnect you when the notify socket is idle.
		sendCommand( "PNG" , QString::null , false );
		m_ping=true;
	}

	//at least 90 second has been ellapsed since the last messages
	// we shouldn't receive error from theses command anymore
	m_tmpHandles.clear();
}

Kopete::OnlineStatus MSNNotifySocket::convertOnlineStatus( const QString &status )
{
	if( status == "NLN" )
		return MSNProtocol::protocol()->NLN;
	else if( status == "FLN" )
		return MSNProtocol::protocol()->FLN;
	else if( status == "HDN" )
		return MSNProtocol::protocol()->HDN;
	else if( status == "PHN" )
		return MSNProtocol::protocol()->PHN;
	else if( status == "LUN" )
		return MSNProtocol::protocol()->LUN;
	else if( status == "BRB" )
		return MSNProtocol::protocol()->BRB;
	else if( status == "AWY" )
		return MSNProtocol::protocol()->AWY;
	else if( status == "BSY" )
		return MSNProtocol::protocol()->BSY;
	else if( status == "IDL" )
		return MSNProtocol::protocol()->IDL;
	else
		return MSNProtocol::protocol()->UNK;
}

#include "msnnotifysocket.moc"

// vim: set noet ts=4 sts=4 sw=4:

