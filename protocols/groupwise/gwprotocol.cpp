/*
    gwprotocol.cpp - Kopete GroupWise Protocol

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Testbed   
    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
    rtfizeTest from nm_rtfize_text, from Gaim src/protocols/novell/nmuser.c
    Copyright (c) 2004 Novell, Inc. All Rights Reserved
    
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#include <qregexp.h>
#include <qstringlist.h>

#include <kgenericfactory.h>
#include <kdebug.h>

#include "kopeteaccountmanager.h"
#include "kopeteglobal.h"

#include "gwaccount.h"
#include "gwerror.h"
#include "gwcontact.h"
#include "gwprotocol.h"
#include "ui/gwaddcontactpage.h"
#include "ui/gweditaccountwidget.h"

typedef KGenericFactory<GroupWiseProtocol> GroupWiseProtocolFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_groupwise, GroupWiseProtocolFactory( "kopete_groupwise" )  )

GroupWiseProtocol *GroupWiseProtocol::s_protocol = 0L;

GroupWiseProtocol::GroupWiseProtocol( QObject* parent, const char *name, const QStringList &/*args*/ )
	: KopeteProtocol( GroupWiseProtocolFactory::instance(), parent, name ),
	  groupwiseUnknown    ( KopeteOnlineStatus::Unknown, 25, this, 0, "status_unknown",
	  		"FIXME: Make this unselectable", i18n( "Unknown" ) ),
	  groupwiseOffline ( KopeteOnlineStatus::Offline,    0,  this, 1, QString::null, 
	  		i18n( "O&ffline" ), i18n( "Offline" ) ),
	  groupwiseAvailable  ( KopeteOnlineStatus::Online,  25, this, 2, QString::null, 
	  		i18n( "A&vailable" ),   i18n( "Available" ) ),
	  groupwiseBusy       ( KopeteOnlineStatus::Away,    20, this, 3, "groupwise_busy", 
	  		i18n( "&Busy" ), i18n( "Busy" ) ),
	  groupwiseAway       ( KopeteOnlineStatus::Away,    18, this, 4, "groupwise_away", 
	  		i18n( "Go &Away" ), i18n( "Away" ) ),
	  groupwiseAwayIdle   ( KopeteOnlineStatus::Away,    15, this, 5, "groupwise_away", 
	  		"FIXME: Make this unselectable", i18n( "Idle" ) ),
	  groupwiseInvalid( KopeteOnlineStatus::Unknown, 25, this, 6, "status_unknown",
	  		"FIXME: Make this unselectable", i18n( "Invalid Status" ) ),
	  groupwiseConnecting( KopeteOnlineStatus::Unknown, 0, this, 99, "status_connecting",
	  		"FIXME: Make this unselectable", i18n( "Connecting" ) ),
	  groupwiseAppearOffline( KopeteOnlineStatus::Online,22, this, 98, "groupwise_invisible",
	  		i18n( "A&ppear Offline" ), i18n( "Appear Offline" ) ),
	  propGivenName( Kopete::Global::Properties::self()->firstName() ),
	  propLastName( Kopete::Global::Properties::self()->lastName() ),
	  propFullName( Kopete::Global::Properties::self()->fullName() ),
	  propAwayMessage( Kopete::Global::Properties::self()->awayMessage() ),
	  propAutoReply( "groupwiseAutoReply", i18n( "Auto Reply Message" ), QString::null, false, false ),
	  propCN( "groupwiseCommonName", i18n( "Common Name" ), QString::null, true, false )
{
	// ^^ That is all member initialiser syntax, not broken indentation!
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << endl;

	s_protocol = this;
	
}

GroupWiseProtocol::~GroupWiseProtocol()
{
}

KopeteContact *GroupWiseProtocol::deserializeContact(
	KopeteMetaContact *metaContact, const QMap<QString, QString> &serializedData,
	const QMap<QString, QString> &/* addressBookData */)
{
	QString dn = serializedData[ "DN" ];
	QString accountId = serializedData[ "accountId" ];
	QString displayName = serializedData[ "displayName" ];
	int objectId = serializedData[ "objectId" ].toInt();
	int parentId = serializedData[ "parentId" ].toInt();
	int sequence = serializedData[ "sequenceNumber" ].toInt();
	
	QDict<KopeteAccount> accounts = KopeteAccountManager::manager()->accounts( this );

	KopeteAccount *account = accounts[ accountId ];
	if ( !account )
	{
		kdDebug(GROUPWISE_DEBUG_GLOBAL) << "Account doesn't exist, skipping" << endl;
		return 0;
	}

	// FIXME: creating a contact with a userId here
	return new GroupWiseContact(account, dn, metaContact, objectId, parentId, sequence );
}

AddContactPage * GroupWiseProtocol::createAddContactWidget( QWidget *parent, KopeteAccount *  account )
{
	kdDebug( GROUPWISE_DEBUG_GLOBAL ) << "Creating Add Contact Page" << endl;
	return new GroupWiseAddContactPage( account, parent, "addcontactpage");
}

KopeteEditAccountWidget * GroupWiseProtocol::createEditAccountWidget( KopeteAccount *account, QWidget *parent )
{
	kdDebug(GROUPWISE_DEBUG_GLOBAL) << "Creating Edit Account Page" << endl;
	return new GroupWiseEditAccountWidget( parent, account );
}

KopeteAccount *GroupWiseProtocol::createNewAccount( const QString &accountId )
{
	return new GroupWiseAccount( this, accountId );
}

GroupWiseProtocol *GroupWiseProtocol::protocol()
{
	return s_protocol;
}

KopeteOnlineStatus GroupWiseProtocol::gwStatusToKOS( const int gwInternal )
{
	KopeteOnlineStatus status;
	switch ( gwInternal )
	{
		case GroupWise::Unknown:
			status = groupwiseUnknown;
			break;
		case GroupWise::Offline:
			status = groupwiseOffline;
			break;
		case GroupWise::Available:
			status = groupwiseAvailable;
			break;
		case GroupWise::Busy:
			status = groupwiseBusy;
			break;
		case GroupWise::Away:
			status = groupwiseAway;
			break;
		case GroupWise::AwayIdle:
			status = groupwiseAwayIdle;
			break;
		case GroupWise::Invalid:
			status = groupwiseInvalid;
			break;
		default:
			status = groupwiseInvalid;
			kdWarning( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "Got unrecognised status value" << gwInternal << endl;
	}
	return status;
}

QString GroupWiseProtocol::rtfizeText( const QString & plain )
{
	// transcode a utf-8 encoded string into an rtf string
	// iterate through the input string converting each char into the equivalent rtf 
	// of single-byte characters with first byte =< 0x7f (127), { } \ are escaped. \n are converted into \par , the rest are appended verbatim
	// of multi-byte UTF-8 characters 2 to 6 bytes long (with first byte > 0x7f), these are recoded as 32 bit values, escaped as \u<val>? strings

	// vanilla RTF "envelope" that doesn't say much but causes other clients to accept the message
	QString rtfTemplate = QString::fromLatin1("{\\rtf1\\ansi\n"
						"{\\fonttbl{\\f0\\fnil Unknown;}}\n"
						"{\\colortbl ;\\red0\\green0\\blue0;}\n"
						"\\uc1\\cf1\\f0\\fs18 %1\\par\n}");
	QString outputText; // output text
	QCString plainUtf8 = plain.utf8(); // encoded as UTF8, because that's what this encoding algorithm, taken from Gaim's Novell plugin
	uint index = 0; // current char to transcode
	while ( index  < plainUtf8.length() )
	{
		Q_UINT8 current = plainUtf8.data()[ index ];
		if ( current <= 0x7F )
		{
			switch ( current )
			{
				case '{':
				case '}':
				case '\\':
					outputText.append( QString( "\\%1" ).arg( QChar( current ) ) );
					break;
				case '\n':
					outputText.append( "\\par " );
					break;
				default:
					outputText.append( QChar( current ) );
					break;
			}
			++index;
		}
		else
		{
			Q_UINT32 ucs4Char;
			int bytesEncoded;
			QString escapedUnicodeChar;
			if ( current <= 0xDF )
			{
				ucs4Char = (( plainUtf8.data()[ index ] & 0x001F) << 6) |
					( plainUtf8.data()[ index+1 ] & 0x003F);
				bytesEncoded = 2;
			}
			else if ( current <= 0xEF )
			{
				ucs4Char = (( plainUtf8.data()[ index ] & 0x000F) << 12) |
					(( plainUtf8.data()[ index+1 ] & 0x003F) << 6) |
					( plainUtf8.data()[ index+2 ] & 0x003F);
				bytesEncoded = 3;
			}
			else if ( current <= 0xF7 )
			{
				ucs4Char = (( plainUtf8.data()[ index ] & 0x0007) << 18) |
					(( plainUtf8.data()[ index+1 ] & 0x003F) << 12) |
					(( plainUtf8.data()[ index+2 ] & 0x003F) << 6) |
					( plainUtf8.data()[ index+3 ] & 0x003F);
				bytesEncoded = 4;
			}
			else if ( current <= 0xFB )
			{
				ucs4Char = (( plainUtf8.data()[ index ] & 0x0003) << 24 ) |
					(( plainUtf8.data()[ index+1 ] & 0x003F) << 18) |
					(( plainUtf8.data()[ index+2 ] & 0x003F) << 12) |
					(( plainUtf8.data()[ index+3 ] & 0x003F) << 6) |
					( plainUtf8.data()[ index+4 ] & 0x003F);
				bytesEncoded = 5;
			}
			else if ( current <= 0xFD )
			{
				ucs4Char = (( plainUtf8.data()[ index ] & 0x0001) << 30 ) |
					(( plainUtf8.data()[ index+1 ] & 0x003F) << 24) |
					(( plainUtf8.data()[ index+2 ] & 0x003F) << 18) |
					(( plainUtf8.data()[ index+3 ] & 0x003F) << 12) |
					(( plainUtf8.data()[ index+4 ] & 0x003F) << 6) |
					( plainUtf8.data()[ index+5 ] & 0x003F);
				bytesEncoded = 6;
			}
			else
			{
				kdDebug( GROUPWISE_DEBUG_GLOBAL ) << k_funcinfo << "bogus utf-8 lead byte: 0x" << QTextStream::hex << current << endl;
				ucs4Char = 0x003F;
				bytesEncoded = 1;
			}
			index += bytesEncoded;
			escapedUnicodeChar = QString("\\u%1?").arg( ucs4Char );
			kdDebug( GROUPWISE_DEBUG_GLOBAL ) << "unicode escaped char: " << escapedUnicodeChar << endl;
			outputText.append( escapedUnicodeChar );
		}
	}
	return rtfTemplate.arg( outputText );
}

QString GroupWiseProtocol::dnToDotted( const QString & dn )
{
	QRegExp rx("[a-zA-Z]*=(.*)$", false );
	if( !dn.find( '=' ) ) // if it's not a DN, return it unprocessed
		return dn;

	// split the dn into elements
	QStringList elements = QStringList::split( ',', dn );
	// remove the key, keep the value
	for ( QStringList::Iterator it = elements.begin(); it != elements.end(); ++it )
	{
		if ( rx.search( *it ) != -1 )
			*it = rx.cap( 1 );
	}
	QString dotted = elements.join( "." );
	// reassemble as dotted

	return dotted;
}
#include "gwprotocol.moc"
