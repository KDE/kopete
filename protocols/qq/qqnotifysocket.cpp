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
	m_password=password;
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
	if( onlineStatus() != Offline )
	{
		sendGoodbye();
		QQSocket::disconnect();
	}
	else
		emit socketClosed();
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

void QQNotifySocket::parsePacket( const QByteArray& data )
{
	// TODO: develop me!
	// dump the data:
	kDebug( 14140 ) << data << endl;
}

void QQNotifySocket::sendLoginTokenRequest()
{
	QByteArray& packet = Eva::loginToken(m_qqId, m_id++);
	sendPacket( packet );
	setOnlineStatus( LoginToken );

}

#include "qqnotifysocket.moc"

// vim: set noet ts=4 sts=4 sw=4:

