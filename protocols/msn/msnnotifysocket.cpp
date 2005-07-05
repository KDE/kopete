/*
    msnnotifysocket.cpp - Notify Socket for the MSN Protocol

    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart at kde.org>

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

#ifdef OLDSSLLOGIN
#include "sslloginhandler.h"
#else
#include "msnsecureloginhandler.h"
#endif

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
#ifdef OLDSSLLOGIN
	m_sslLoginHandler=0l;
#else
	m_secureLoginHandler=0L;
#endif

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
#ifdef OLDSSLLOGIN
	delete m_sslLoginHandler;
#else
	delete m_secureLoginHandler;
#endif
	kdDebug(14140) << k_funcinfo << endl;
}

void MSNNotifySocket::doneConnect()
{
//	kdDebug( 14140 ) << k_funcinfo << "Negotiating server protocol version" << endl;
	sendCommand( "VER", "MSNP9" );
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

void MSNNotifySocket::parseCommand( const QString &cmd, uint id,
	const QString &data )
{
	//kdDebug(14140) << "MSNNotifySocket::parseCommand: Command: " << cmd << endl;


	if ( cmd == "VER" )
	{
		sendCommand( "CVR", "0x0409 winnt 5.1 i386 MSNMSGR 6.2.0205 MSMSGS " + m_account->accountId() );
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
			#ifdef OLDSSLLOGIN
			m_sslLoginHandler = new SslLoginHandler();
			QObject::connect( m_sslLoginHandler, SIGNAL(       loginFailed()        ),
					 this,            SLOT  (    sslLoginFailed()        ) );
			QObject::connect( m_sslLoginHandler, SIGNAL(    loginIncorrect()        ),
					 this,            SLOT  ( sslLoginIncorrect()        ) );
			QObject::connect( m_sslLoginHandler, SIGNAL(    loginSucceeded(QString) ),
					 this,            SLOT  ( sslLoginSucceeded(QString) ) );

			m_sslLoginHandler->login( data.section( ' ' , 2 , 2 ), m_account->accountId() , m_password );
			#else
			m_secureLoginHandler = new MSNSecureLoginHandler(m_account->accountId(), m_password, data.section( ' ' , 2 , 2 ));

			QObject::connect(m_secureLoginHandler, SIGNAL(loginFailed()), this, SLOT(sslLoginFailed()));
			QObject::connect(m_secureLoginHandler, SIGNAL(loginSuccesful(const QString& )), this, SLOT(sslLoginSucceeded(const QString& )));

			m_secureLoginHandler->login();
			#endif
	
		}
		else
		{
			// Successful auth
			m_disconnectReason=Kopete::Account::Unknown;

			// sync contact list
			QString serial=m_account->configGroup()->readEntry( "serial" , "0" );
			if(serial.isEmpty()) //0.9 comatibility
				serial= "0";
			sendCommand( "SYN", serial );

			// this is our current user and friendly name
			// do some nice things with it  :-)
			QString publicName = unescape( data.section( ' ', 2, 2 ) );
			emit publicNameChanged( publicName );

			// We are connected start to ping
			slotSendKeepAlive();
		}
	}
	else if( cmd == "LST" )
	{
		//The hanlde is used if we receive some BRP
		m_tmpLastHandle=data.section( ' ', 0, 0 );

		// handle, publicName, lists, group
		emit contactList(  m_tmpLastHandle ,
			unescape( data.section( ' ', 1, 1 ) ), data.section( ' ', 2, 2 ).toUInt(),
			data.section( ' ', 3, 3 ) );
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
			if (publicName!=c->property( Kopete::Global::Properties::self()->nickName()).value().toString())
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
	else if( cmd == "ADD" )
	{
		QString msnId = data.section( ' ', 2, 2 );
		uint group;
		if( data.section( ' ', 0, 0 ) == "FL" )
			group = data.section( ' ', 4, 4 ).toUInt();
		else
			group = 0;

		// handle, publicName, List,  group
		emit contactAdded( msnId, unescape( data.section( ' ', 2, 2 ) ),
			data.section( ' ', 0, 0 ), group );
		m_account->configGroup()->writeEntry( "serial" , data.section( ' ', 1, 1 ) );
	}
	else if( cmd == "REM" ) // someone is removed from a list
	{
		uint group;
		if( data.section( ' ', 0, 0 ) == "FL" )
			group = data.section( ' ', 3, 3 ).toUInt();
		else
			group = 0;

		// handle, list, group
		emit contactRemoved( data.section( ' ', 2, 2 ), data.section( ' ', 0, 0 ), group );
		m_account->configGroup()->writeEntry( "serial" , data.section( ' ', 1, 1 ) );
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
	else if( cmd == "REA" )
	{
		QString handle=data.section( ' ', 1, 1 );
		if( handle == m_account->accountId() )
			emit publicNameChanged( unescape( data.section( ' ', 2, 2 ) ) );
		else
		{
			MSNContact *c = static_cast<MSNContact*>( m_account->contacts()[ handle ] );
			if( c )
				c->setProperty( Kopete::Global::Properties::self()->nickName() , unescape( data.section( ' ', 2, 2 ) ) );
		}
		m_account->configGroup()->writeEntry( "serial" , data.section( ' ', 0, 0 ) );
	}
	else if( cmd == "LSG" )
	{
		//the LSG syntax depends if it is called from SYN or from LSG
		if(data.contains(' ') > 4) //FROM LSG
		{ //   --NOTE:  since 2003-11-14 , The MSN Server does not accept anymere the LSG command.  So this is maybe useless now.
			emit groupListed( unescape( data.section( ' ', 4, 4 ) ), data.section( ' ', 3, 3 ).toUInt() );
		}
		else //from SYN
		{
			//this command has not id, and the first param is a number, so MSNSocket think it is the ID
			emit groupListed( unescape( data.section( ' ', 0, 0 ) ), id );
		}
	}
	else if( cmd == "ADG" )
	{
		// groupName, group
		emit groupAdded( unescape( data.section( ' ', 1, 1 ) ),
			data.section( ' ', 2, 2 ).toUInt() );
		m_account->configGroup()->writeEntry( "serial" , data.section( ' ', 0, 0 ) );
	}
	else if( cmd == "REG" )
	{
		// groupName, group
		emit groupRenamed( unescape( data.section( ' ', 2, 2 ) ),
			data.section( ' ', 1, 1 ).toUInt() );
		m_account->configGroup()->writeEntry( "serial" , data.section( ' ', 0, 0 ) );
	}
	else if( cmd == "RMG" )
	{
		// group
		emit groupRemoved( data.section( ' ', 1, 1 ).toUInt() );
		m_account->configGroup()->writeEntry( "serial" , data.section( ' ', 0, 0 ) );
	}
	else if( cmd  == "CHL" )
	{
//		kdDebug(14140) << k_funcinfo <<"Sending final Authentication" << endl;
		KMD5 context( ( data.section( ' ', 0, 0 ) + "Q1P7W2E4J9R8U3S5" ).utf8() );
		sendCommand( "QRY", "msmsgs@msnmsgr.com", true,
			context.hexDigest());
	}
	else if( cmd == "SYN" )
	{
		// this is the current serial on the server, if its different with the own we can get the user list
		QString serial = data.section( ' ', 0, 0 );
		if( serial != m_account->configGroup()->readEntry("serial") )
		{
			emit newContactList();  // remove all contacts datas, msn sends a new contact list
			m_account->configGroup()->writeEntry( "serial" , data.section( ' ', 0, 0 ) );
		}
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
			if( id > 0 ) //FROM PRP
			{
				m_account->configGroup()->writeEntry( "serial" , data.section( ' ', 0, 0 ) );
				m_account->configGroup()->writeEntry( data.section( ' ', 1, 1 ),unescape(data.section( ' ', 2, 2 ) )); //SECURITY????????
				c->setInfo(data.section( ' ', 1, 1 ),unescape(data.section( ' ', 2, 2 )));
			}
			else //FROM SYN
			{
				c->setInfo(data.section( ' ', 0, 0 ),unescape(data.section( ' ', 1, 1 )));
				m_account->configGroup()->writeEntry(data.section( ' ', 0, 0 ),unescape(data.section( ' ', 1, 1 ) )); //SECURITY????????
			}
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
		//do nothing
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

#ifdef OLDSSLLOGIN
	m_sslLoginHandler->deleteLater();
	m_sslLoginHandler = 0L;
#else
	m_secureLoginHandler->deleteLater();
	m_secureLoginHandler = 0L;
#endif
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
	if(msg.contains("text/x-msmsgsinitialemailnotification"))
	{
		 //this sends the server if we are going online, contains the unread message count
		QString m = msg.right(msg.length() - msg.find("Inbox-Unread:") );
		m = m.left(msg.find("\r\n"));
		mailCount = m.right(m.length() -m.find(" ")-1).toUInt();

		if(mailCount > 0 )
		{
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
}

void MSNNotifySocket::addGroup(const QString& groupName)
{
	// escape spaces
	sendCommand( "ADG", escape( groupName ) + " 0" );
}

void MSNNotifySocket::renameGroup( const QString& groupName, uint group )
{
	// escape spaces
	sendCommand( "REG", QString::number( group ) + " " +
		escape( groupName ) + " 0" );
}

void MSNNotifySocket::removeGroup( uint group )
{
	sendCommand( "RMG", QString::number( group ) );
}

void MSNNotifySocket::addContact( const QString &handle, const QString& publicName, uint group, int list )
{
	QString args;
	switch( list )
	{
	case MSNProtocol::FL:
		args = "FL " + handle + " " + escape( publicName ) + " " + QString::number( group );
		break;
	case MSNProtocol::AL:
		args = "AL " + handle + " " + escape( publicName );
		break;
	case MSNProtocol::BL:
		args = "BL " + handle + " " + escape( publicName );
		break;
	default:
		kdDebug(14140) << k_funcinfo <<"WARNING! Unknown list " <<
			list << "!" << endl;
		return;
	}
	unsigned int id=sendCommand( "ADD", args );
	m_tmpHandles[id]=handle;
}

void MSNNotifySocket::removeContact( const QString &handle, uint group,	int list )
{
	QString args;
	switch( list )
	{
	case MSNProtocol::FL:
		args = "FL " + handle + " " + QString::number( group );
		break;
	case MSNProtocol::AL:
		args = "AL " + handle;
		break;
	case MSNProtocol::BL:
		args = "BL " + handle;
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
		sendCommand( "CHG", statusToString( status ) + " 268435492 " + escape(m_account->pictureObject()) );
}

void MSNNotifySocket::changePublicName(  QString publicName, const QString &handle )
{
	if( escape(publicName).length() > 387 )
	{
		publicName=publicName.left(387);
	}

	if( handle.isNull() )
	{
		unsigned int id=sendCommand( "REA", m_account->accountId() + " " + escape ( publicName ) );
		m_tmpHandles[id]=m_account->accountId();
	}
	else
	{
		unsigned int id=sendCommand( "REA", handle + " " + escape ( publicName ) );
		m_tmpHandles[id]=handle;
	}
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

