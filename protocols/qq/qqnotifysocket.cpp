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
	kDebug( 14140 ) << k_funcinfo << "Negotiating server protocol version" << endl;
	sendLoginTokenRequest();
}


void QQNotifySocket::disconnect()
{
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
	
	char login_test[] = {
		0x1, 0xdf, 0x25, 0xd2, 0x9e, 0xdb, 0x9, 0xef, 0xd4, 0xf1, 0x12, 0x8f, 0x61, 0x21, 0x63, 0x68, 0x37, 0xed, 0x88, 0x36, 0x3, 0xb2, 0x6c, 0x44 };
		
	Eva::ByteArray test( login_test, 24);
	test.release();

	Eva::ByteArray initKey((char*) Eva::getInitKey(), 16 );
	initKey.release();

	switch( packet.command() )
	{
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
		case Eva::Login :
			//kDebug( 14140 ) << packet.command() << ": crypted body = " <<
			//	QByteArray( packet.body().data(), packet.body().size() ) << endl;
			// insert the testing:

			kDebug( 14140 ) << packet.command() << ": crypted body = " <<
				QByteArray( test.data(), test.size() ) << endl;
			// text = Eva::decrypt( packet.body(), m_passwordKey );
			text = Eva::decrypt( test, m_passwordKey );
			if ( text.size() == 0 )
			{
			kDebug( 14140 ) << "initKey size = " << initKey.size() << ", data =" << QByteArray( initKey.data(), initKey.size() ) << endl;
				text = Eva::decrypt( test, initKey );
			}
			
			kDebug( 14140 ) << "text size = " << text.size() << ", data =" <<
				QByteArray( text.data(), text.size() ) << endl;
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

