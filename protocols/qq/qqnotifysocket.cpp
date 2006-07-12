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
	Eva::ByteArray pwd( password.toAscii().data(), password.size() );
	m_passwordKey = Eva::QQHash(pwd);
	pwd.release(); // the data is handled in QT
	m_lastCmd = NA;
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

void QQNotifySocket::setStatus( const Kopete::OnlineStatus &status )
{
	emit statusChanged( status );
}

// Core functions
void QQNotifySocket::parsePacket( const QByteArray& rawdata )
{
	kDebug( 14140 ) << k_funcinfo << rawdata << endl;
	Eva::Packet packet( rawdata.data(), rawdata.size() );
	Eva::ByteArray text;

	Eva::ByteArray initKey((char*) Eva::getInitKey(), 16 );
	initKey.release();

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
		case Eva::ChangeStatus :
		case Eva::AckSysMsg :
		case Eva::SendMsg :
		case Eva::ReceiveMsg :
		case Eva::RemoveMe :
		case Eva::RequestKey :
		case Eva::GetCell :
			break;

		case Eva::Login :
			text = Eva::decrypt( packet.body(), m_passwordKey );
			if ( text.size() == 0 )
			{
			kDebug( 14140 ) << "initKey size = " << initKey.size() << ", data =" << QByteArray( initKey.data(), initKey.size() ) << endl;
				text = Eva::decrypt( packet.body(), initKey );
			}
			
			kDebug( 14140 ) << "text size = " << text.size() << ", data =" <<
				QByteArray( text.data(), text.size() ) << endl;

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
					// TODO: set the client status to connected.
					
					break;

				case Eva::LoginRedirect :
					kDebug( 14140 ) << "Redirect to " 
						<< QHostAddress(Eva::Packet::redirectedIP(text)).toString()
						<< " : " << Eva::Packet::redirectedPort(text) << endl;
					disconnect();
					connect( QHostAddress( Eva::Packet::redirectedIP(text) ).toString(), Eva::Packet::redirectedPort(text) );
					break;

				case Eva::LoginWrongPassword :
					break;

				case Eva::LoginMiscError :
					break;

				default:
					kDebug( 14140 ) << "Bad, we are not supposed to be here !" << endl;
					break;
			}

			break;

		case Eva::BuddyList :
		case Eva::BuddyOnline :
		case Eva::GetCell2 :
		case Eva::SIP :
		case Eva::Test :
		case Eva::UpdateGroup :
		case Eva::UploadGroup :
		case Eva::Memo :
		case Eva::DownloadGroup :
		case Eva::GetLevel :
		case Eva::RequestLoginToken :
			m_token = Eva::loginToken( packet.body() );
			
			kDebug( 14140 ) << packet.command() << ": token = " << 
				QByteArray ( m_token.data(), m_token.size() ) << endl;

			sendLogin();
			break;

		case Eva::ExtraInfo :
		case Eva::Signature :
		case Eva::ReceiveSysMsg :
		case Eva::FriendStausChange :

		default:
			break;

	}
}


void QQNotifySocket::sendLoginTokenRequest()
{
	kDebug( 14140 ) << k_funcinfo << endl;
	Eva::ByteArray data = Eva::requestLoginToken(m_qqId, m_id++);
	m_lastCmd = LoginTokenRequest;
	sendPacket( QByteArray( data.data(), data.size()) );
}

void QQNotifySocket::sendLogin()
{
	Eva::ByteArray data = Eva::login( m_qqId, m_id++, m_passwordKey, 
				m_token, m_loginMode );
	m_lastCmd = Login;
	sendPacket( QByteArray( data.data(), data.size()) );
}



#include "qqnotifysocket.moc"

// vim: set noet ts=4 sts=4 sw=4:

