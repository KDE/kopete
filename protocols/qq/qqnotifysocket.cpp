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
	char eva_token[24] = {
		0xc6, 0x9c, 0x6a, 0x7, 0x81, 0x4a, 0xaf, 0x17, 0xd9, 0x5, 0x5a, 0x45, 0xdf, 0xd7, 0xcd, 0x98, 0x7, 0xae, 0x9e, 0x83, 0x97, 0x57, 0x9, 0x7e };

	switch( packet.command() )
	{
		case Eva::RequestLoginToken:
			m_token = Eva::loginToken( packet.body() );

			QByteArray tmp( m_token.data(), m_token.size() );
			kDebug( 14140 ) << packet.command() << ": token = " << tmp << endl;

			for( int i = 0; i< 24; i++ )
				m_token.data()[i] = eva_token[i];

			QByteArray tmp2( m_token.data(), m_token.size() );
			kDebug( 14140 ) << packet.command() << ": token = " << tmp2 << endl;

			
			sendLogin();
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

