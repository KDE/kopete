/*
    qqnotifysocket.cpp - Notify Socket for the QQ Protocol
    forked from msnnotifysocket.cpp
    
    Copyright (c) 2006      by Hui Jin <blueangel.jin@gmail.com>
    Copyright (c) 2002      by Duncan Mac-Vicar Prett <duncan@kde.org>
    Copyright (c) 2002-2003 by Martijn Klingens       <klingens@kde.org>
    Copyright (c) 2002-2005 by Olivier Goffart        <ogoffart at kde.org>
    Copyright (c) 2005      by Michaël Larouche       <larouche@kde.org>
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

#include <kdebug.h>
#include <QHostAddress>

#include "kopetestatusmessage.h"
#include "libeva.h"

#include "qqaccount.h"

QQNotifySocket::QQNotifySocket( QQAccount *account, const QString &password )
: QQSocket( account )
{
	m_account = account;
	m_newstatus = Kopete::OnlineStatus::Offline;
	Eva::ByteArray pwd( password.toAscii().data(), password.size() );
	m_passwordKey = Eva::Packet::QQHash(pwd);
	pwd.release(); // the data is handled in QT
	m_loginMode = Eva::NormalLogin;

	// FIXME: more error-checking.
	m_qqId = account->accountId().toInt();

	m_heartbeat = new QTimer(this);
	QObject::connect( m_heartbeat, SIGNAL(timeout()), SLOT(heartbeat()) );
}

QQNotifySocket::~QQNotifySocket()
{
	kDebug(14140) ;
	if( m_heartbeat->isActive() )
		m_heartbeat->stop();

	delete m_heartbeat;
}


void QQNotifySocket::doneConnect()
{
	// setup the status first
	QQSocket::doneConnect();

	kDebug( 14140 ) << "Negotiating server protocol version";
	if( m_token.size() )
		sendPacket( Eva::login( m_qqId, m_id++, m_passwordKey, m_token, m_loginMode ) );
	else
		sendPacket( Eva::loginToken(m_qqId, m_id++) );
}


void QQNotifySocket::disconnect()
{
	kDebug(14140) << "online status =" <<
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
	kDebug(14140) ;

	// TODO: Add support for all of these!
	switch( code )
	{
	default:
		QQSocket::handleError( code, id );
		break;
	}
}

// Core functions
void QQNotifySocket::handleIncomingPacket( const QByteArray& rawData )
{
	kDebug( 14140 ) << rawData;
	Eva::Packet packet( rawData.data(), rawData.size() );
	Eva::ByteArray text;

	Eva::ByteArray initKey((char*) Eva::Packet::getInitKey(), 16 );
	initKey.release();

	kDebug( 14140 ) << "command = " << packet.command();
	switch( packet.command() )
	{
		case Eva::Command::RequestLoginToken :
			text = Eva::Packet::loginToken( packet.body() );
			break;

		case Eva::Command::Login :
			text = Eva::Packet::decrypt( packet.body(), m_passwordKey );
			if( text.size() == 0 )
				text = Eva::Packet::decrypt( packet.body(), initKey );
			break;

		default:
			text = Eva::Packet::decrypt( packet.body(), m_sessionKey );
			if ( text.size() == 0 )
				text = Eva::Packet::decrypt( packet.body(), m_passwordKey );
	}
			
	kDebug( 14140 ) << "text = " << QByteArray( text.c_str(), text.size() );

	
	switch( packet.command() )
	{
		// FIXME: use table-driven pattern ?
		case Eva::Command::Logout :
		case Eva::Command::Heartbeat:
			break;
		case Eva::Command::UpdateInfo :
		case Eva::Command::Search :
		case Eva::Command::UserInfo :
		{
			// map std::map to QMap
			std::map<const char*, std::string, Eva::ltstr> dict = Eva::Packet::contactDetail(text);
			QMap<const char*, QByteArray> qmap;
			
			QString id = QString( dict["qqId"].c_str() );
			std::map<const char*, std::string, Eva::ltstr>::const_iterator it = dict.begin();

			for( ; it != dict.end(); it++ )
				qmap.insert( (*it).first, QByteArray((*it).second.c_str() ) );

			emit contactDetailReceived(id, qmap);
		}

			
			break;
		case Eva::Command::AddBuddy:
		case Eva::Command::RemoveBuddy:
		case Eva::Command::AuthInvite :
			break;
		case Eva::Command::ChangeStatus :
			if( Eva::Packet::replyCode(text) == Eva::ChangeStatusOK )
			{
				kDebug( 14140 ) << "ChangeStatus ok";
				emit statusChanged( m_newstatus );
			}
			else // TODO: Debug me.
				disconnect();
			break;

		case Eva::Command::AckSysMsg :
		case Eva::Command::SendMsg :
			break;
		case Eva::Command::ReceiveMsg :
		{
			Eva::MessageEnvelop envelop(text);
			kDebug(14140) << "Received message from " << envelop.sender << " to " << envelop.receiver << " type=" << envelop.type;
			kDebug(14140) << "seq = " << envelop.sequence << " from " << envelop.ip << ":" << envelop.port;
			
			sendPacket( Eva::messageReply(m_qqId, packet.sequence(), m_sessionKey, Eva::Packet::replyKey(text) ));
			Eva::ByteArray body( text.data() + sizeof(envelop), text.size() - sizeof(envelop) );
			body.release();

			// TODO: check whether this is a duplicated message
			switch( envelop.type )
			{
				case 0x0010:
					kDebug(14140) << "command 0x0010: " << QByteArray( body.c_str(), body.size() );
					break;
				case Eva::RcvFromBuddy:
				{
					Eva::MessageHeader mh(body);
					kDebug(14140) << "message header:";
					kDebug(14140) << "ver:" << mh.version << " sender:" << mh.sender 
						<< " receiver:" << mh.receiver 
						<< " type:" << mh.type << " seq:" << mh.sequence 
						<< " timestamp:" << mh.timestamp << " avatar:" << mh.avatar 
						<< endl;

					if( mh.receiver != m_qqId )
					{
						kDebug(14140) << "receive other(" << mh.receiver <<")'s message";
						break;
					}

					// FIXME: replace the magic number!
					// FIXME: the code stinks!
					Eva::uchar* p = body.data()+36;
					bool hasFontStyle = p[3] != 0;
					Eva::uchar replyType = p[8];
					
					// clear compiler warnings
					Q_UNUSED(hasFontStyle);
					Q_UNUSED(replyType);

					Eva::ByteArray msg(body.size());
					p += 9;

					while( *p )
						msg += *p++;
					msg += char(0x0);

					kDebug(14140) << "message received: " << msg.data();
					// FIXME: use a function to generate guid!
					emit messageReceived(mh, msg);
					
					break;
				}
				default:
					break;
			}
			break;
		}

		case Eva::Command::RemoveMe :
			break;

		case Eva::Command::RequestKey :
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
					kDebug( 14140 ) << "transferKey =" << QByteArray( m_transferKey.c_str(), m_transferKey.size());
					kDebug( 14140 ) << "transferToken =" << QByteArray( m_transferToken.c_str(), m_transferToken.size());

				}
			}
			break;
		}

		case Eva::Command::GetCell :
			break;

		case Eva::Command::Login :
			switch( Eva::Packet::replyCode(text)  )
			{
				case Eva::LoginOK:
					kDebug( 14140 ) << "Bingo! QQ:#" << m_qqId << " logged in!";
					// show off some meta data :
					m_sessionKey = Eva::Packet::sessionKey(text);
					kDebug( 14140 ) << "sessionKey = " << 
						QByteArray( m_sessionKey.c_str(), m_sessionKey.size() ) << endl;

					kDebug( 14140 )  << "remote IP: " << QHostAddress( Eva::Packet::remoteIP(text) ).toString();
					kDebug( 14140 )  << "remote port: " << Eva::Packet::remotePort(text);
					kDebug( 14140 )  << "local IP: " << QHostAddress( Eva::Packet::localIP(text) ).toString();
					kDebug( 14140 )  << "local port: " << Eva::Packet::localPort(text);
					kDebug( 14140 )  << "login time: " << Eva::Packet::loginTime(text);
					kDebug( 14140 )  << "last login from: " << QHostAddress( Eva::Packet::lastLoginFrom(text) ).toString();
					kDebug( 14140 )  << "last login time: " << Eva::Packet::lastLoginTime(text);

					// start the heartbeat
					if( !m_heartbeat->isActive() )
					{
						m_heartbeat->setSingleShot(false);
						m_heartbeat->start(60000);
					}

					// FIXME: refactor me!
					emit newContactList();
					// FIXME: We might login in as invisible as well.
					m_newstatus = Kopete::OnlineStatus::Online;
					sendPacket( Eva::statusUpdate( m_qqId, m_id++, m_sessionKey, Eva::Online) );
					sendPacket( Eva::transferKey( m_qqId, m_id++, m_sessionKey) );

					// get the meta data for myself
					contactDetail(m_qqId);

					// fetch the online contacts
					sendListOnlineContacts();



					break;

				case Eva::LoginRedirect :
					kDebug( 14140 ) << "Redirect to " 
						<< QHostAddress(Eva::Packet::redirectedIP(text)).toString()
						<< " : " << Eva::Packet::redirectedPort(text) << endl;
					disconnect();
					connect( QHostAddress( Eva::Packet::redirectedIP(text) ).toString(), Eva::Packet::redirectedPort(text) );
					break;

				case Eva::LoginWrongPassword :
					kDebug( 14140 )  << "password is wrong. ";
					break;

				case Eva::LoginMiscError :
					kDebug( 14140 )  << "unknown error. ";
					break;

				default:
					kDebug( 14140 ) << "Bad, we are not supposed to be here !";
					break;
			}

			break;

		case Eva::Command::AllContacts:
			/*
			{
				len = 2;
				while( len < text.size() )
					emit contactList( Eva::contactInfo( text.data(), len ) );
				short pos = ntohs( Eva::type_cast<short> (text.data()) );

				if( pos != Eva::ContactListEnd )
					sendPacket( Eva::allContacts( m_qqId, m_id++, m_sessionKey, pos ) );
			}
			*/
			break;
		case Eva::Command::ContactsOnline :
			
			break;
		case Eva::Command::GetCell2 :
		case Eva::Command::SIP :
		case Eva::Command::Test :
			break;
		case Eva::Command::GroupNames :
			groupNames( text );
			break;

		case Eva::Command::UploadGroups :
		case Eva::Command::Memo :
			break;
		case Eva::Command::DownloadGroups :
			groupInfos( text );
			break;

		case Eva::Command::GetLevel :
			break;

		case Eva::Command::RequestLoginToken :
			m_token = text;
			kDebug( 14140 ) << "command = " << packet.command() << ": token = " << 
				QByteArray ( m_token.c_str(), m_token.size() ) << endl;
			sendPacket( Eva::login( m_qqId, m_id++, m_passwordKey, m_token, m_loginMode ) );
			break;

		case Eva::Command::ExtraInfo :
		case Eva::Command::Signature :
		case Eva::Command::ReceiveSysMsg :
			break;
		case Eva::Command::ContactStausChanged :
		{
			kDebug( 14140 ) << "contact status signal";
			Eva::ContactStatus cs(text.data());
			kDebug( 14140 ) << "contact status detail:";
			kDebug( 14140 ) << "id = " << cs.qqId << " status = " << cs.status;
			emit contactStatusChanged( cs );
			break;
		}
		default:
			break;

	}
}


void QQNotifySocket::contactDetail(Eva::uint qqId)
{
	sendPacket( Eva::contactDetail( m_qqId, m_id++, m_sessionKey, qqId) );
}
	
void QQNotifySocket::sendTextMessage( const uint toId, const QByteArray& message )
{
	// Translate the message to Eva::ByteArray
	// TODO: color and font
	kDebug( 14140 ) << "Send the message: " << message << " from " << m_qqId << " to " << toId;
	// attach the ByteArray to QString:
	// FIXME: Add an adapter to ByteArray
	Eva::ByteArray text( (char*)message.data(), message.size() );
	text.release();

	Eva::ByteArray packet = Eva::textMessage(m_qqId, m_id++, m_sessionKey, toId, m_transferKey, text );
	QQSocket::sendPacket( QByteArray( packet.c_str(), packet.size()) );
}


void QQNotifySocket::heartbeat()
{
	sendPacket( Eva::heartbeat( m_qqId, m_id++, m_sessionKey )); 
}

void QQNotifySocket::sendListOnlineContacts(uint pos)
{
	sendPacket( Eva::onlineContacts( m_qqId, m_id++, m_sessionKey, pos) );
}

void QQNotifySocket::groupNames( const Eva::ByteArray& text )
{
	QStringList ql;
	std::list< std::string > l = Eva::Packet::groupNames( text );
	for( std::list<std::string>::const_iterator it = l.begin(); it != l.end(); it++ )
		ql.append( QString( (*it).c_str() ) );

	kDebug(14140) ;
	emit groupNames( ql );
}

void QQNotifySocket::groupInfos( const Eva::ByteArray& text )
{
	kDebug(14140) ;
	std::list< Eva::GroupInfo > gis = Eva::Packet::groupInfos( text );
	// TODO: send it one by one.
	for( std::list< Eva::GroupInfo >::const_iterator it = gis.begin();
		it != gis.end(); it++ )
	{
		kDebug(14140) << "buddy: qqId = " << (*it).qqId << " type = " << (*it).type 
			<< " groupId = " << (*it).groupId << endl;
			emit contactInGroup( (*it).qqId, (*it).type, (*it).groupId );
	}

	int next = Eva::Packet::nextGroupId( text );
	if( next )
		sendDownloadGroups( next );
}

void QQNotifySocket::doGetContactStatuses( const Eva::ByteArray& text )
{
	kDebug(14140) ;
	Eva::uchar pos = Eva::ContactListBegin;
	std::list< Eva::ContactStatus > css = Eva::Packet::onlineContacts( text, pos );
	for( std::list< Eva::ContactStatus >::const_iterator it = css.begin();
		it != css.end(); it++ )
	{
		kDebug(14140) << "buddy: qqId = " << (*it).qqId << " status = " << (*it).status;
		emit contactStatusChanged(*it);
	}

	if( pos != 0xff )	
		sendListOnlineContacts(pos);
}

#include "qqnotifysocket.moc"
// vim: set noet ts=4 sts=4 sw=4:
