/*
    qqnotifysocket.cpp - Notify Socket for the QQ Protocol
    forked from qqnotifysocket.cpp
    
    Copyright (c) 2006      by Hui Jin <blueangel.jin@gmail.com>
    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart at kde.org>
    Copyright (c) 2005      by Michaël Larouche       <michael.larouche@kdemail.net>
    Copyright (c) 2005      by Gregg Edghill          <gregg.edghill@gmail.com>

    Kopete    (c) 2002-2005 by the Kopete developers  <kopete-devel@kde.org>

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

#include "qqnotifysocket.h"
#include "qqcontact.h"
#include "qqaccount.h"

#include <QDateTime>
#include <QRegExp>
#include <qdom.h>
#include <QTextStream>

#include <kdebug.h>
#include <kdeversion.h>
#include <klocale.h>
#include <kcodecs.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <krun.h>
#include <kio/job.h>
#include <qfile.h>
#include <kconfig.h>
#include <knotification.h>
#include <QHostAddress>

#include "kopeteuiglobal.h"
#include "kopeteglobal.h"
#include "kopetestatusmessage.h"
#include "libeva.h"


QQNotifySocket::QQNotifySocket( QQAccount *account, const QString &password )
: QQSocket( account )
{
	m_account = account;
	// FIXME: Do we really need password ?
	m_password = password;
	m_newstatus = Kopete::OnlineStatus::Offline;
	Eva::ByteArray pwd( password.toAscii().data(), password.size() );
	m_passwordKey = Eva::QQHash(pwd);
	pwd.release(); // the data is handled in QT
	m_loginMode = Eva::NormalLogin;

	// DELME: dump the result
	QByteArray tmp( m_passwordKey.data(), m_passwordKey.size() );
	kDebug(14140) << endl << endl << "!!!!" << k_funcinfo << "passwordKey = " << tmp << m_passwordKey.size() << endl;


	// FIXME: more error-checking.
	m_qqId = account->accountId().toInt();
}

QQNotifySocket::~QQNotifySocket()
{
	kDebug(14140) << k_funcinfo << endl;
}


void QQNotifySocket::doneConnect()
{
	// setup the status first
	QQSocket::doneConnect();

	kDebug( 14140 ) << k_funcinfo << "Negotiating server protocol version" << endl;
	if( m_token.size() )
		sendLogin();
	else
		sendLoginTokenRequest();
}


void QQNotifySocket::disconnect()
{
	kDebug(14140) << k_funcinfo << "online status =" <<
		onlineStatus() << endl;
	// FIXME: double check the logic, please.
	if(	m_disconnectReason==Kopete::Account::Unknown )
		m_disconnectReason=Kopete::Account::Manual;
	// sendGoodbye, shall we setup the status as well ?
	if( onlineStatus() == Connected )
		sendGoodbye();

	// the socket is not connected yet, so I should force the signals
	if ( onlineStatus() == Disconnected || onlineStatus() == Connecting )
		emit socketClosed();
	else
		QQSocket::disconnect();
}

void QQNotifySocket::handleError( uint code, uint id )
{
	kDebug(14140) << k_funcinfo << endl;

	// TODO: Add support for all of these!
	switch( code )
	{
	default:
		QQSocket::handleError( code, id );
		break;
	}
}

// Core functions
void QQNotifySocket::parsePacket( const QByteArray& rawdata )
{
	kDebug( 14140 ) << k_funcinfo << rawdata << endl;
	Eva::Packet packet( rawdata.data(), rawdata.size() );
	Eva::ByteArray text;
	int len;

	Eva::ByteArray initKey((char*) Eva::getInitKey(), 16 );
	initKey.release();

	kDebug( 14140 ) << "command = " << packet.command() << endl;
	switch( packet.command() )
	{
		case Eva::RequestLoginToken :
			text = Eva::loginToken( packet.body() );
			break;

		case Eva::Login :
			text = Eva::decrypt( packet.body(), m_passwordKey );
			if( text.size() == 0 )
				text = Eva::decrypt( packet.body(), initKey );
			break;

		default:
			text = Eva::decrypt( packet.body(), m_sessionKey );
			if ( text.size() == 0 )
				text = Eva::decrypt( packet.body(), m_passwordKey );
	}
			
	kDebug( 14140 ) << "text = " << QByteArray( text.data(), text.size() ) << endl;

	
	switch( packet.command() )
	{
		// FIXME: use table-driven pattern ?
		case Eva::Logout :
		case Eva::KeepAlive :
		case Eva::UpdateInfo :
		case Eva::Search :
		case Eva::UserInfo :
		case Eva::AddFriend :
		case Eva::RemoveFriend :
		case Eva::AuthInvite :
			break;
		case Eva::ChangeStatus :
			if( Eva::Packet::replyCode(text) == Eva::ChangeStatusOK )
			{
				kDebug( 14140 ) << "ChangeStatus ok" << endl;
				emit statusChanged( m_newstatus );
			}
			else // TODO: Debug me.
				disconnect();
			break;

		case Eva::AckSysMsg :
		case Eva::SendMsg :
		case Eva::ReceiveMsg :
		case Eva::RemoveMe :
			break;

		case Eva::RequestKey :
		{
			char type = text.data()[0];
			char reply = text.data()[1];

			if( reply == Eva::RequestKeyOK )
			{
				// NOTE: the type of the key supports TransferKey only.
				if( type == Eva::TransferKey )
				{
					m_transferKey = Eva::Packet::transferKey( text );
					m_transferToken = Eva::Packet::transferToken( text );
					kDebug( 14140 ) << "transferKey =" << QByteArray( m_transferKey.data(), m_transferKey.size()) << endl;
					kDebug( 14140 ) << "transferToken =" << QByteArray( m_transferToken.data(), m_transferToken.size()) << endl;

				}
			}
			break;
		}

		case Eva::GetCell :
			break;

		case Eva::Login :
			switch( Eva::Packet::replyCode(text)  )
			{
				case Eva::LoginOK:
					kDebug( 14140 ) << "Bingo! QQ:#" << m_qqId << " logged in!" << endl;
					// show off some meta data :
					m_sessionKey = Eva::Packet::sessionKey(text);
					kDebug( 14140 ) << "sessionKey = " << 
						QByteArray( m_sessionKey.data(), m_sessionKey.size() ) << endl;

					kDebug( 14140 )  << "remote IP: " << QHostAddress( Eva::Packet::remoteIP(text) ).toString() << endl;
					kDebug( 14140 )  << "remote port: " << Eva::Packet::remotePort(text) << endl;
					kDebug( 14140 )  << "local IP: " << QHostAddress( Eva::Packet::localIP(text) ).toString() << endl;
					kDebug( 14140 )  << "local port: " << Eva::Packet::localPort(text) << endl;
					kDebug( 14140 )  << "login time: " << Eva::Packet::loginTime(text) << endl;
					kDebug( 14140 )  << "last login from: " << QHostAddress( Eva::Packet::lastLoginFrom(text) ).toString() << endl;
					kDebug( 14140 )  << "last login time: " << Eva::Packet::lastLoginTime(text) << endl;

					emit newContactList();
					// FIXME: We might login in as invisible as well.
					m_newstatus = Kopete::OnlineStatus::Online;
					sendChangeStatus( Eva::Online );
					sendRequestTransferKey();

					break;

				case Eva::LoginRedirect :
					kDebug( 14140 ) << "Redirect to " 
						<< QHostAddress(Eva::Packet::redirectedIP(text)).toString()
						<< " : " << Eva::Packet::redirectedPort(text) << endl;
					disconnect();
					connect( QHostAddress( Eva::Packet::redirectedIP(text) ).toString(), Eva::Packet::redirectedPort(text) );
					break;

				case Eva::LoginWrongPassword :
					kDebug( 14140 )  << "password is wrong. " << endl;
					break;

				case Eva::LoginMiscError :
					kDebug( 14140 )  << "unknown error. " << endl;
					break;

				default:
					kDebug( 14140 ) << "Bad, we are not supposed to be here !" << endl;
					break;
			}

			break;

		case Eva::ContactList :
			/*
			{
				len = 2;
				while( len < text.size() )
					emit contactList( Eva::contactInfo( text.data(), len ) );
				short pos = ntohs( Eva::type_cast<short> (text.data()) );

				if( pos != Eva::ContactListEnd )
					sendContactList(pos);
			}
			*/
			break;
		case Eva::ContactsOnline :
		case Eva::GetCell2 :
		case Eva::SIP :
		case Eva::Test :
			break;
		case Eva::GroupNames :
			doGetGroupNames( text );
			break;

		case Eva::UploadGroups :
		case Eva::Memo :
			break;
		case Eva::DownloadGroups :
			doGetCGTs( text );
			break;

		case Eva::GetLevel :
			break;

		case Eva::RequestLoginToken :
			m_token = text;
			kDebug( 14140 ) << "command = " << packet.command() << ": token = " << 
				QByteArray ( m_token.data(), m_token.size() ) << endl;

			sendLogin();
			break;

		case Eva::ExtraInfo :
		case Eva::Signature :
		case Eva::ReceiveSysMsg :
			break;
		case Eva::ContactStausChanged :
			kDebug( 14140 ) << "contact status signal" << endl;
			emit contactStatusChanged( Eva::ContactStatus(text) );
			break;

		default:
			break;

	}
}

// FIXME: Refactor us !!
void QQNotifySocket::sendLoginTokenRequest()
{
	Eva::ByteArray data = Eva::requestLoginToken(m_qqId, m_id++);
	sendPacket( QByteArray( data.data(), data.size()) );
}

void QQNotifySocket::sendLogin()
{
	kDebug( 14140 ) << "QQ = " << m_qqId << endl;
	Eva::ByteArray data = Eva::login( m_qqId, m_id++, m_passwordKey, 
				m_token, m_loginMode );
	sendPacket( QByteArray( data.data(), data.size()) );
}

void QQNotifySocket::sendChangeStatus( char status )
{
	Eva::ByteArray packet = Eva::changeStatus( m_qqId, m_id++, m_sessionKey, status );
	sendPacket( QByteArray( packet.data(), packet.size()) );
}

void QQNotifySocket::sendRequestTransferKey()
{
	Eva::ByteArray packet = Eva::requestTransferKey( m_qqId, m_id++, m_sessionKey);
	sendPacket( QByteArray( packet.data(), packet.size()) );
}

void QQNotifySocket::sendContactList( short pos )
{
	Eva::ByteArray packet = Eva::contactList( m_qqId, m_id++, m_sessionKey, pos );
	sendPacket( QByteArray( packet.data(), packet.size()) );
}

void QQNotifySocket::sendGetGroupNames()
{
	Eva::ByteArray packet = Eva::getGroupNames( m_qqId, m_id++, m_sessionKey );
	sendPacket( QByteArray( packet.data(), packet.size()) );
}

void QQNotifySocket::sendDownloadGroups( int pos )
{
	Eva::ByteArray packet = Eva::downloadGroups( m_qqId, m_id++, m_sessionKey, pos );
	sendPacket( QByteArray( packet.data(), packet.size()) );
}


void QQNotifySocket::doGetGroupNames( const Eva::ByteArray& text )
{
	QStringList ql;
	std::list< std::string > l = Eva::Packet::groupNames( text );
	for( std::list<std::string>::const_iterator it = l.begin(); it != l.end(); it++ )
		ql.append( QString( (*it).c_str() ) );

	kDebug(14140) << k_funcinfo << endl;
	emit groupNames( ql );
}

void QQNotifySocket::doGetCGTs( const Eva::ByteArray& text )
{
	kDebug(14140) << k_funcinfo << endl;
	std::list< Eva::CGT > cgts = Eva::Packet::cgts( text );
	// TODO: send it one by one.
	for( std::list< Eva::CGT >::const_iterator it = cgts.begin();
		it != cgts.end(); it++ )
	{
		kDebug(14140) << "buddy: qqId = " << (*it).qqId << " type = " << (*it).type 
			<< " groupId = " << (*it).groupId << endl;
			emit contactInGroup( (*it).qqId, (*it).type, (*it).groupId );
	}

	int next = Eva::Packet::nextGroupId( text );
	if( next )
		sendDownloadGroups( next );
}

#include "qqnotifysocket.moc"
// vim: set noet ts=4 sts=4 sw=4:
