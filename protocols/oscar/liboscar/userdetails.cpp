/*
	Kopete Oscar Protocol
	userdetails.cpp - user details from the extended status packet

	Copyright (c) 2004 by Matt Rogers <mattr@kde.org>

	Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

	*************************************************************************
	*                                                                       *
	* This library is free software; you can redistribute it and/or         *
	* modify it under the terms of the GNU Lesser General Public            *
	* License as published by the Free Software Foundation; either          *
	* version 2 of the License, or (at your option) any later version.      *
	*                                                                       *
	*************************************************************************
*/
#include "userdetails.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <kdebug.h>
#include <klocale.h>
#include "buffer.h"
#include "oscarutils.h"
#include "oscardebug.h"
#include <QtCore/QTextCodec>

#define OSCAR_USERINFO_DEBUG

using namespace Oscar;

UserDetails::UserDetails()
{
    m_capabilities.resize(CAP_LAST);
	clear();
}


UserDetails::~UserDetails()
{
}

void UserDetails::clear()
{
	m_capabilities.fill( false );
	m_warningLevel = 0;
	m_userClass = 0;
	m_idleTime = 0;
	m_extendedStatus = 0;
	m_xtrazStatus = -1;
	m_statusMood = -1;
	m_dcPort = 0;
	m_dcType = 0;
	m_dcProtoVersion = 0;
	m_dcAuthCookie = 0;
	m_dcWebFrontPort = 0;
	m_dcClientFeatures = 0;
	m_dcLastInfoUpdateTime = 0;
	m_dcLastExtInfoUpdateTime = 0;
	m_dcLastExtStatusUpdateTime = 0;
	m_onlineStatusMsgSupport = false;

	m_userClassSpecified = false;
	m_memberSinceSpecified = false;
	m_onlineSinceSpecified = false;
	m_awaySinceSpecified = false;
	m_numSecondsOnlineSpecified = false;
	m_idleTimeSpecified = false;
	m_extendedStatusSpecified = false;
	m_xtrazStatusSpecified = false;
	m_statusMoodSpecified = false;
	m_capabilitiesSpecified = false;
	m_dcOutsideSpecified = false;
	m_dcInsideSpecified = false;
	m_iconSpecified = false;
}

int UserDetails::warningLevel() const
{
	return m_warningLevel;
}

QString UserDetails::userId() const
{
	return m_userId;
}

Oscar::WORD UserDetails::idleTime() const
{
	return m_idleTime;
}

QHostAddress UserDetails::dcInternalIp() const
{
	return m_dcInsideIp;
}

QHostAddress UserDetails::dcExternalIp() const
{
	return m_dcOutsideIp;
}

Oscar::DWORD UserDetails::dcPort() const
{
	return m_dcPort;
}

Oscar::WORD UserDetails::dcProtoVersion() const
{
    return m_dcProtoVersion;
}

QDateTime UserDetails::onlineSinceTime() const
{
	return m_onlineSince;
}

QDateTime UserDetails::awaySinceTime() const
{
	return m_awaySince;
}

QDateTime UserDetails::memberSinceTime() const
{
	return m_memberSince;
}

int UserDetails::userClass() const
{
	return m_userClass;
}

Oscar::DWORD UserDetails::extendedStatus() const
{
	return m_extendedStatus;
}

int UserDetails::xtrazStatus() const
{
	return m_xtrazStatus;
}

int UserDetails::statusMood() const
{
	return m_statusMood;
}

Oscar::WORD UserDetails::iconType() const
{
	return m_iconType;
}

QString UserDetails::personalMessage() const
{
	return m_personalMessage;
}

Oscar::BYTE UserDetails::iconCheckSumType() const
{
	return m_iconChecksumType;
}

QByteArray UserDetails::buddyIconHash() const
{
	return m_md5IconHash;
}

QString UserDetails::clientName() const
{
	return m_clientName;
}

void UserDetails::parseCapabilities( Buffer &inbuf, int &xStatus )
{
	xStatus = -1;
	QString dbgCaps = "CAPS: ";
	while ( inbuf.bytesAvailable() >= 16 )
	{
		bool found = false;
		Guid cap( inbuf.getGuid() );
		int i;
		for ( i=0; i < CAP_LAST; ++i )
		{
			if ( (i == CAP_KOPETE && cap.isEqual ( oscar_caps[i], 12 ) )    ||
			     (i == CAP_MIRANDA && cap.isEqual ( oscar_caps[i], 8 ) )    ||
			     (i == CAP_QIP && cap.isEqual ( oscar_caps[i], 16 ) )       ||
			     (i == CAP_QIPINFIUM && cap.isEqual ( oscar_caps[i], 16 ) ) ||
			     (i == CAP_QIPPDA && cap.isEqual ( oscar_caps[i], 16 ) )    ||
			     (i == CAP_QIPSYMBIAN && cap.isEqual ( oscar_caps[i], 16 ) )||
			     (i == CAP_QIPMOBILE && cap.isEqual ( oscar_caps[i], 16 ) ) ||
			     (i == CAP_JIMM && cap.isEqual ( oscar_caps[i], 5 ) )       ||
			     (i == CAP_MICQ && cap.isEqual ( oscar_caps[i], 12 ) )      ||
			     (i == CAP_SIMNEW && cap.isEqual ( oscar_caps[i], 12 ) )    ||
			     (i == CAP_SIMOLD && cap.isEqual ( oscar_caps[i], 15 ) )    ||
			     (i == CAP_VMICQ && cap.isEqual ( oscar_caps[i], 6 ) )      ||
			     (i == CAP_LICQ && cap.isEqual ( oscar_caps[i], 12 ) )      ||
			     (i == CAP_ANDRQ && cap.isEqual ( oscar_caps[i], 9 ) )      ||
			     (i == CAP_RANDQ && cap.isEqual ( oscar_caps[i], 9 ) )      ||
			     (i == CAP_MCHAT && cap.isEqual ( oscar_caps[i], 10 ) ) )
			{
				m_capabilities[i] = true;
				dbgCaps += capName(i);
				m_identCap = cap;
				found = true;
				break;
			}
			else if(oscar_caps[i] == cap)
			{
				m_capabilities[i] = true;
				dbgCaps += capName(i);
				found = true;
				break;
			}
		}
		if(!found && xStatus == -1)
		{
			for(i = 0; i < XSTAT_LAST; ++i)
			{
				if(oscar_xStatus[i] == cap)
				{
					xStatus = i;
					found = true;
					break;
				}
			}
		}
	}
	kDebug(OSCAR_RAW_DEBUG) << dbgCaps;
}

void UserDetails::parseNewCapabilities( Buffer &inbuf )
{
	QString dbgCaps = "NEW CAPS: ";
	QByteArray cap = Guid( QLatin1String("094600004c7f11d18222444553540000"));
	while ( inbuf.bytesAvailable() >= 2 )
	{
		cap[2] = inbuf.getByte();
		cap[3] = inbuf.getByte();
		
		for ( int i = 0; i < CAP_LAST; i++ )
		{
			if ( oscar_caps[i].data() == cap )
			{
				m_capabilities[i] = true;
				dbgCaps += capName( i );
				break;
			}
		}
	}
	kDebug(OSCAR_RAW_DEBUG) << dbgCaps;
}

void UserDetails::fill( Buffer * buffer )
{
	QString user = QString( buffer->getBUIN() );
	if ( !user.isEmpty() )
		m_userId = user;
	m_warningLevel = buffer->getWord();
	Oscar::WORD numTLVs = buffer->getWord();

	kDebug( OSCAR_RAW_DEBUG ) << "Got user info for " << user;
#ifdef OSCAR_USERINFO_DEBUG
	kDebug( OSCAR_RAW_DEBUG ) << "Warning level is " << m_warningLevel;
#endif
	//start parsing TLVs
	for( int i = 0; i < numTLVs; ++i  )
	{
		TLV t = buffer->getTLV();
		if ( t )
		{
			Buffer b( t.data, t.length );
			switch( t.type )
			{
			case 0x0001: //user class
				m_userClass = b.getWord();
				m_userClassSpecified = true;
#ifdef OSCAR_USERINFO_DEBUG
				kDebug(OSCAR_RAW_DEBUG) << "User class is " << m_userClass;
#endif
				break;
			case 0x0002: //member since
			case 0x0005: //member since
				m_memberSince.setTime_t( b.getDWord() );
				m_memberSinceSpecified = true;
#ifdef OSCAR_USERINFO_DEBUG
				kDebug(OSCAR_RAW_DEBUG) << "Member since " << m_memberSince;
#endif
				break;
			case 0x0003: //sigon time
				m_onlineSince.setTime_t( b.getDWord() );
				m_onlineSinceSpecified = true;
#ifdef OSCAR_USERINFO_DEBUG
				kDebug(OSCAR_RAW_DEBUG) << "Signed on at " << m_onlineSince;
#endif
				break;
			case 0x0004: //idle time
				m_idleTime = b.getWord() * 60;
				m_idleTimeSpecified = true;
#ifdef OSCAR_USERINFO_DEBUG
				kDebug(OSCAR_RAW_DEBUG) << "Idle time is " << m_idleTime;
#endif
				break;
			case 0x0006: //extended user status
				m_extendedStatus = b.getDWord();
				m_extendedStatusSpecified = true;
#ifdef OSCAR_USERINFO_DEBUG
				kDebug(OSCAR_RAW_DEBUG) << "Extended status is " << QString::number( m_extendedStatus, 16 );
#endif
                break;
			case 0x0008:
				m_onlineStatusMsgSupport = (b.getWord() == 0x0A06);
#ifdef OSCAR_USERINFO_DEBUG
				kDebug(OSCAR_RAW_DEBUG) << "Online status messages support";
#endif
				break;
			case 0x000A: //external IP address
				m_dcOutsideIp.setAddress( b.getDWord() );
				m_dcOutsideSpecified = true;
#ifdef OSCAR_USERINFO_DEBUG
				kDebug(OSCAR_RAW_DEBUG) << "External IP address is " << m_dcOutsideIp.toString();
#endif
				break;
			case 0x000C: //DC info
				m_dcInsideIp.setAddress( b.getDWord() );
				m_dcPort = b.getDWord();
#ifdef OSCAR_USERINFO_DEBUG
    			kDebug(OSCAR_RAW_DEBUG) << "Internal IP address is " << m_dcInsideIp.toString();
    			kDebug(OSCAR_RAW_DEBUG) << "Port number is " << m_dcPort;
#endif
    			m_dcType = b.getByte();
				m_dcProtoVersion = b.getWord();
				m_dcAuthCookie = b.getDWord();
				m_dcWebFrontPort = b.getDWord();
				m_dcClientFeatures = b.getDWord();
				m_dcLastInfoUpdateTime = b.getDWord();
				m_dcLastExtInfoUpdateTime = b.getDWord();
				m_dcLastExtStatusUpdateTime = b.getDWord();
  			    b.getWord(); //unknown.
				m_dcInsideSpecified = true;
#ifdef OSCAR_USERINFO_DEBUG
				kDebug(OSCAR_RAW_DEBUG) << "Got DC info";
#endif
				break;
			case 0x000D: //capability info
				parseCapabilities(b, m_xtrazStatus);
				m_capabilitiesSpecified = true;
				m_xtrazStatusSpecified = true;
				break;
			case 0x0010:
			case 0x000F: //online time
				m_numSecondsOnline = b.getDWord();
				m_numSecondsOnlineSpecified = true;
#ifdef OSCAR_USERINFO_DEBUG
				kDebug(OSCAR_RAW_DEBUG) << "Online for " << m_numSecondsOnline;
#endif
				break;
			case 0x0019: //new capability info
				parseNewCapabilities( b );
				m_capabilitiesSpecified = true;
				break;
			case 0x001D:
			{
				if ( t.length == 0 )
					break;

				while ( b.bytesAvailable() > 0 )
				{
#ifdef OSCAR_USERINFO_DEBUG
					kDebug(OSCAR_RAW_DEBUG) << "Icon and status message info";
#endif
					Oscar::WORD type2 = b.getWord();
					Oscar::BYTE number = b.getByte();
					Oscar::BYTE length = b.getByte();
					switch( type2 )
					{
					case 0x0000:
						b.skipBytes(length);
						break;
					case 0x0001: // AIM/ICQ avatar hash
					case 0x000C: // ICQ contact photo 
					//case 0x0008: // ICQ Flash avatar hash
						if ( length > 0 && ( number == 0x01 || number == 0x00 ) &&
						     ( m_iconSpecified == false || m_iconType < type2 ) ) // Check priority
						{
							m_iconType = type2;
							m_iconChecksumType = number;
							m_md5IconHash = b.getBlock( length );
							m_iconSpecified = true;
#ifdef OSCAR_USERINFO_DEBUG
							kDebug(OSCAR_RAW_DEBUG) << "checksum:" << m_md5IconHash.toHex();
#endif
						}
 						else
 						{
	 						kWarning(OSCAR_RAW_DEBUG) << "icon checksum indicated"
		 						<< " but unable to parse checksum" << endl;
							b.skipBytes( length );
 						}
						break;
					case 0x0002:
						if ( length > 0 )
						{
							Buffer pmBuffer( b.getBBlock( length ) );
							QByteArray personalMessageData = pmBuffer.getBSTR();
							
							QTextCodec *codec = 0;
							if ( pmBuffer.bytesAvailable() >= 4 && pmBuffer.getWord() == 0x0001 )
							{
								pmBuffer.skipBytes( 2 );
								QByteArray encoding = pmBuffer.getBSTR();
								codec = QTextCodec::codecForName( encoding );
								kDebug(OSCAR_RAW_DEBUG) << "Encoding:" << encoding;
							}

							if (codec)
								m_personalMessage = codec->toUnicode( personalMessageData );
							else
								m_personalMessage = QString::fromUtf8( personalMessageData );

#ifdef OSCAR_USERINFO_DEBUG
							kDebug(OSCAR_RAW_DEBUG) << "personal message:" << m_personalMessage;
#endif
						}
						else
							kDebug(OSCAR_RAW_DEBUG) << "not enough bytes for status message";
						break;
					case 0x000E:
						if ( length > 0 )
						{
							QString mood( b.getBlock( length ) );
							m_statusMood = mood.mid( 7 ).toInt();
						}
						m_statusMoodSpecified = true;
#ifdef OSCAR_USERINFO_DEBUG
						kDebug(OSCAR_RAW_DEBUG) << "status mood:" << m_statusMood;
#endif
						break;
					default:
						b.skipBytes( length );
						break;
					}
				}
				break;
			}
			case 0x0029:
				m_awaySince.setTime_t( b.getDWord() );
				m_awaySinceSpecified = true;
#ifdef OSCAR_USERINFO_DEBUG
				kDebug(OSCAR_RAW_DEBUG) << "Away since " << m_awaySince;
#endif
				break;
			default:
				kDebug(OSCAR_RAW_DEBUG) << "Unknown TLV, type=" << t.type << ", length=" << t.length
					<< " in userinfo" << endl;
				break;
			};
			//detach buffer and free TLV data
			b.clear();
		}
	}

	//do client detection on fill
	if ( m_capabilitiesSpecified )
	{
		detectClient();
	}
}

static QString mirandaVersionToString( Oscar::DWORD v )
{
	QString ver;
	ver.sprintf( "%d.%d.%d.%d", (v >> 0x18) & 0x7F, (v >> 0x10) & 0xFF, (v >> 0x08) & 0xFF, v & 0xFF );
	if ( v & 0x80000000 )
		ver += " alpha";
	return ver;
}

static QString getMirandaVersion( Oscar::DWORD iver, Oscar::DWORD mver, bool isUnicode )
{
	if ( !iver )
		return QString();
	QString ver;
	if ( !mver && iver == 1 )
	{
		ver = mirandaVersionToString( 0x80010200 );
	}
	else if ( !mver && ( iver & 0x7FFFFFFF ) <= 0x030301 )
	{
		ver = mirandaVersionToString( iver );
	}
	else
	{
		if ( mver )
			ver = mirandaVersionToString( mver );
		if ( isUnicode )
			ver += " Unicode";
		ver += " (ICQ v" + mirandaVersionToString( iver ) + ')';
	}
	return ver;
}

static QString getVersionFromCap( Guid &cap, int s, int f = 16 )
{
	const char *c = cap.data().data();
	int len = 0;
	for ( int i = s; i < f; i++, len++ )
	{
		if ( c[i] == '\0' )
			break;
	}
	return QString::fromLatin1(c + s, len);
}

void UserDetails::detectClient()
{
	// 1 m_dcLastInfoUpdateTime
	// 2 m_dcLastExtInfoUpdateTime
	// 3 m_dcLastExtStatusUpdateTime
	/*
		Most of this code is based on Miranda ICQ plugin code
	*/
	m_clientName = QString::fromLatin1("");
	if ( m_dcLastInfoUpdateTime == 0xFFFFFFFF )
	{
		if ( m_dcLastExtInfoUpdateTime == 0xffffffff )
		{
			m_clientName = QString::fromLatin1( "Gaim" );
		}
		else if ( !m_dcLastExtInfoUpdateTime && m_dcProtoVersion == 7 )
		{
			m_clientName = QString::fromLatin1( "WebICQ" );
		}
		else if ( !m_dcLastExtInfoUpdateTime && m_dcLastExtStatusUpdateTime == 0x3B7248ED )
		{
			m_clientName = QString::fromLatin1( "Spam Bot" );
		}
		else
		{
			m_clientName = QString::fromLatin1( "Miranda IM" );
			m_clientName += ' ' + getMirandaVersion( m_dcLastExtInfoUpdateTime, 0, false );
		}
	}
	else if ( m_dcLastInfoUpdateTime == 0x7FFFFFFF )
	{
		// Miranda with unicode core
		m_clientName = QString::fromLatin1( "Miranda IM" );
		m_clientName += ' ' + getMirandaVersion( m_dcLastExtInfoUpdateTime, 0, true );
	}
	else if ( ( m_dcLastInfoUpdateTime & 0xFF7F0000 ) == 0x7D000000 )
	{
        //licq
	}
	else if ( m_dcLastInfoUpdateTime == 0xFFFFFF8F )
	{
		m_clientName = QString::fromLatin1( "StrICQ" );
	}
	else if ( m_dcLastInfoUpdateTime == 0xFFFFFF42 )
	{
		m_clientName = QString::fromLatin1( "mICQ" );
	}
	else if ( m_dcLastInfoUpdateTime == 0xFFFFFFBE )
	{
		m_clientName = QString::fromLatin1("Alicq %1.%2.%3").arg((m_dcLastExtInfoUpdateTime >> 0x18) & 0xFF).arg((m_dcLastExtInfoUpdateTime >> 0x10) & 0xFF).arg((m_dcLastExtInfoUpdateTime >> 0x08) & 0xFF);
	}
	else if ( m_dcLastInfoUpdateTime == 0xFFFFFF7F )
	{
		m_clientName = QString::fromLatin1( "&RQ" );
	}
	else if ( m_dcLastInfoUpdateTime == 0xFFFFFFAB )
	{
		m_clientName = QString::fromLatin1( "YSM" );
	}
	else if ( m_dcLastInfoUpdateTime == 0x04031980 )
	{
		m_clientName = QString::fromLatin1( "vICQ" );
	}
	else if ( m_dcLastInfoUpdateTime == 0x3AA773EE && m_dcLastExtInfoUpdateTime == 0x3AA66380 )
	{
		m_clientName = QString::fromLatin1( "libicq2000" );
	}
	else if ( m_dcLastInfoUpdateTime == 0x3B75AC09 )
	{
		m_clientName = QString::fromLatin1( "Trillian" );
	}
	else if ( m_dcLastInfoUpdateTime == 0xFFFFFFFE && m_dcLastExtStatusUpdateTime == 0xFFFFFFFE )
	{
		m_clientName = QString::fromLatin1( "Jimm" );
	}
	else if ( m_dcLastInfoUpdateTime == 0xFFFFF666 && !m_dcLastExtStatusUpdateTime )
	{
        // this is R&Q (Rapid Edition)
		m_clientName = QString::fromLatin1( "R&Q" );
		m_clientVersion.sprintf( "%u", m_dcLastExtInfoUpdateTime );
		m_clientName += ' ' + m_clientVersion;
	}
    // parse capabilities
	if ( hasCap( CAP_KOPETE ) )
	{
		m_clientName = i18n( "Kopete" );
		m_clientVersion.sprintf( "%d.%d.%d", m_identCap.data().at(12), m_identCap.data().at(13), m_identCap.data().at(14) * 100 + m_identCap.data().at(15) );
		m_clientName += ' ' + m_clientVersion;
		return;
	}
	if ( hasCap ( CAP_MIRANDA ) )
	{
		m_clientName = QString::fromLatin1( "Miranda IM" );
		Oscar::DWORD iver = m_identCap.data().at(12) << 0x18 | m_identCap.data().at(13) << 0x10 | m_identCap.data().at(14) << 0x08 | m_identCap.data().at(15);
		Oscar::DWORD mver = m_identCap.data().at(8) << 0x18 | m_identCap.data().at(9) << 0x10 | m_identCap.data().at(10) << 0x08 | m_identCap.data().at(11);
		m_clientName += ' ' + getMirandaVersion( iver, mver, m_dcLastInfoUpdateTime == 0x7FFFFFFF );
		return;
	}
	if  ( hasCap( CAP_QIP ) )
	{
		m_clientName = QString::fromLatin1( "QIP" );
		if ( m_dcLastExtStatusUpdateTime == 0x0F )
			m_clientVersion = QString::fromLatin1( "2005" );
		else
			m_clientVersion = getVersionFromCap( m_identCap, 11 );
		QString build;
		if ( m_dcLastInfoUpdateTime && m_dcLastExtInfoUpdateTime == 0x0E )
		{
			build.sprintf( "(%d%d%d%d)", m_dcLastInfoUpdateTime >> 0x18, (m_dcLastInfoUpdateTime >> 0x10) & 0xFF, (m_dcLastInfoUpdateTime >> 0x08) & 0xFF, m_dcLastInfoUpdateTime & 0xFF );
		}
		m_clientName += ' ' + m_clientVersion + ' ' + build;
		return;
	}
	if ( hasCap( CAP_QIPINFIUM ) )
	{
		m_clientName = QString::fromLatin1( "QIP Infium" );
		if ( m_dcLastInfoUpdateTime )
		{
			QString build;
			build.sprintf(" (%d)", m_dcLastInfoUpdateTime );
			m_clientName += build;
		}
		if ( m_dcLastExtInfoUpdateTime == 0x0B )
			m_clientName += " Beta";
		return;
	}
	if ( hasCap( CAP_QIPPDA ) )
	{
		m_clientName = QString::fromLatin1( "QIP PDA (Windows)" );
		return;
	}
	if ( hasCap( CAP_QIPSYMBIAN ) )
	{
		m_clientName = QString::fromLatin1( "QIP PDA (Symbian)" );
		return;
	}
	if ( hasCap( CAP_QIPMOBILE ) )
	{
		m_clientName = QString::fromLatin1( "QIP Mobile (Java)" );
		return;
	}
	if ( hasCap( CAP_JIMM ) )
	{
		m_clientName = QString::fromLatin1( "Jimm" );
		m_clientName += ' ' + getVersionFromCap( m_identCap, 5 );
		return;
	}
	if ( hasCap( CAP_SIMNEW ) )
	{
		m_clientName = QString::fromLatin1( "SIM" );
		m_clientVersion.sprintf( "%d.%d.%d.%d", m_identCap.data().at(12), m_identCap.data().at(13), m_identCap.data().at(14), m_identCap.data().at(15) & 0x0F );
		if ( m_identCap.data().at(15) & 0x80 )
			m_clientVersion += QString::fromLatin1( " (Win32)" );
		else if ( m_identCap.data().at(15) & 0x40 )
			m_clientVersion += QString::fromLatin1( " (MacOS X)" );
        // Linux version? Fix last number
		m_clientName += ' ' + m_clientVersion;
		return;
	}
	if ( hasCap( CAP_SIMOLD ) )
	{
		m_clientName = QString::fromLatin1( "SIM" );
        /*int hiVersion = (cap.data()[15] >> 6) - 1;
					unsigned loVersion = cap.data()[15] & 0x1F;
					kDebug(14150) << "OLD SIM version : <" <<
						hiVersion << ":" << loVersion << endl;
				    m_capabilities[i] = true;
					versionString.sprintf("%d.%d", (unsigned int)hiVersion, loVersion);
					versionString.insert( 0, "SIM " );*/
		return;
	}
	if ( hasCap( CAP_VMICQ ) )
	{
		m_clientName = QString::fromLatin1( "VmICQ" );
		return;
	}
	if ( hasCap( CAP_LICQ ) )
	{
		m_clientName = QString::fromLatin1( "Licq" );
		m_clientVersion.sprintf( "%d.%d.%d", m_identCap.data().at(12), m_identCap.data().at(13) % 100, m_identCap.data().at(14) );
		if ( m_identCap.data().at(15) )
			m_clientVersion += " SSL";
		m_clientName += ' ' + m_clientVersion;
		return;
	}
	if ( hasCap( CAP_ANDRQ ) )
	{
		m_clientName = QString::fromLatin1( "&RQ" );
		m_clientVersion.sprintf( "%d.%d.%d.%d", m_identCap.data().at(12), m_identCap.data().at(11), m_identCap.data().at(10), m_identCap.data().at(9) );
		m_clientName += ' ' + m_clientVersion;
		return;
	}
	if ( hasCap( CAP_RANDQ ) )
	{
		m_clientName = QString::fromLatin1("R&Q");
		m_clientVersion.sprintf("%d.%d.%d.%d", m_identCap.data().at(12), m_identCap.data().at(11), m_identCap.data().at(10), m_identCap.data().at(9));
		m_clientName += ' ' + m_clientVersion;
		return;
	}
	if ( hasCap( CAP_MCHAT ) )
	{
		m_clientName = QString::fromLatin1( "mChat" );
		m_clientVersion = getVersionFromCap( m_identCap, 10 );
		m_clientName += ' ' + m_clientVersion;
		return;
	}
	
	if ( m_dcProtoVersion == 9 )
	{
		if ( hasCap( CAP_XTRAZ ) )
		{
			if ( hasCap( CAP_SENDFILE ) )
			{
				if ( hasCap( CAP_TZERS ) )
				{
					if ( hasCap( CAP_HTMLMSGS ) )
						m_clientName = QString::fromLatin1( "ICQ 6" );
					else
						m_clientName = QString::fromLatin1( "ICQ 5.1" );
				}
				else
				{
					m_clientName = QString::fromLatin1( "ICQ 5" );
				}
				if ( hasCap( CAP_ICQ_RAMBLER ) )
				{
					m_clientName += QString::fromLatin1( " (Rambler)" );
				}
				if ( hasCap( CAP_ICQ_ABV ) )
				{
					m_clientName += QString::fromLatin1( " (Abv)" );
				}
				if ( hasCap( CAP_ICQ_NETVIGATOR ) )
				{
					m_clientName += QString::fromLatin1( " (Netvigator)" );
				}
				return;
			}
		}
		else if ( !hasCap( CAP_DIRECT_ICQ_COMMUNICATION ) )
		{
			if ( hasCap( CAP_UTF8 ) && !hasCap( CAP_RTFMSGS ) )
			{
				m_clientName = QString::fromLatin1( "pyICQ" );
			}
		}
		
	}
	else if ( m_dcProtoVersion == 0 )
	{
		
	}
	if ( !m_clientName.isEmpty() )
		return;
	if ( m_dcProtoVersion == 6 )
	{
		m_clientName = QString::fromLatin1( "ICQ 99" );
	}
	else if ( m_dcProtoVersion == 7 )
	{
		m_clientName = QString::fromLatin1( "ICQ 2000/Icq2Go" );
	}
	else if ( m_dcProtoVersion == 8 )
	{
		m_clientName = QString::fromLatin1( "ICQ 2001-2003a" );
	}
	else if ( m_dcProtoVersion == 9 )
	{
		m_clientName = QString::fromLatin1( "ICQ Lite" );
	}
	else if ( m_dcProtoVersion == 10 )
	{
		m_clientName = QString::fromLatin1( "ICQ 2003b" );
	}
}

bool UserDetails::hasCap( int capNumber ) const
{
	return m_capabilities[capNumber];
}

bool UserDetails::onlineStatusMsgSupport() const
{
	return m_onlineStatusMsgSupport;
}

void UserDetails::merge( const UserDetails& ud )
{
	m_userId = ud.m_userId;
	m_warningLevel = ud.m_warningLevel;
	if ( ud.m_userClassSpecified )
	{
		m_userClass = ud.m_userClass;
		m_userClassSpecified = true;
	}
	if ( ud.m_memberSinceSpecified )
	{
		m_memberSince = ud.m_memberSince;
		m_memberSinceSpecified = true;
	}
	if ( ud.m_onlineSinceSpecified )
	{
		m_onlineSince = ud.m_onlineSince;
		m_onlineSinceSpecified = true;
	}
	if ( ud.m_awaySinceSpecified )
	{
		m_awaySince = ud.m_awaySince;
		m_awaySinceSpecified = true;
	}
	if ( ud.m_numSecondsOnlineSpecified )
	{
		m_numSecondsOnline = ud.m_numSecondsOnline;
		m_numSecondsOnlineSpecified = true;
	}
	if ( ud.m_idleTimeSpecified )
	{
		m_idleTime = ud.m_idleTime;
		m_idleTimeSpecified = true;
	}
	if ( ud.m_extendedStatusSpecified )
	{
		m_extendedStatus = ud.m_extendedStatus;
		m_extendedStatusSpecified = true;
	}
	if ( ud.m_xtrazStatusSpecified )
	{
		m_xtrazStatus = ud.m_xtrazStatus;
		m_xtrazStatusSpecified = true;
	}
	if ( ud.m_statusMoodSpecified )
	{
		m_statusMood = ud.m_statusMood;
		m_statusMoodSpecified = true;
	}
	if ( ud.m_capabilitiesSpecified )
	{
		m_capabilities = ud.m_capabilities;
		m_clientVersion = ud.m_clientVersion;
		m_clientName = ud.m_clientName;
		m_capabilitiesSpecified = true;
	}
	if ( ud.m_dcOutsideSpecified )
	{
		m_dcOutsideIp = ud.m_dcOutsideIp;
		m_dcOutsideSpecified = true;
	}
	if ( ud.m_dcInsideSpecified )
	{
		m_dcInsideIp = ud.m_dcInsideIp;
		m_dcPort = ud.m_dcPort;
		m_dcType = ud.m_dcType;
		m_dcProtoVersion = ud.m_dcProtoVersion;
		m_dcAuthCookie = ud.m_dcAuthCookie;
		m_dcWebFrontPort = ud.m_dcWebFrontPort;
		m_dcClientFeatures = ud.m_dcClientFeatures;
		m_dcLastInfoUpdateTime = ud.m_dcLastInfoUpdateTime;
		m_dcLastExtInfoUpdateTime = ud.m_dcLastExtInfoUpdateTime;
		m_dcLastExtStatusUpdateTime = ud.m_dcLastExtStatusUpdateTime;
		m_dcInsideSpecified = true;
	}
	if ( ud.m_iconSpecified )
	{
		m_iconType = ud.m_iconType;
		m_iconChecksumType = ud.m_iconChecksumType;
		m_md5IconHash = ud.m_md5IconHash;
		m_iconSpecified = true;
	}
	m_personalMessage = ud.m_personalMessage;
	m_onlineStatusMsgSupport = ud.m_onlineStatusMsgSupport;
}

//kate: tab-width 4; indent-mode csands;
