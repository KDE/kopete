/*
    gwprotocol.cpp - Kopete GroupWise Protocol

    Copyright (c) 2006,2007 Novell, Inc	 	 http://www.opensuse.org
    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com

    Based on Testbed
    Copyright (c) 2003,2007 by Will Stephenson		 <wstephenson@kde.org>
    rtfizeTest from nm_rtfize_text, from Gaim src/protocols/novell/nmuser.c
    Copyright (c) 2004 Novell, Inc. All Rights Reserved

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU General Public                   *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#include "gwprotocol.h"
#include <qregexp.h>
#include <qstringlist.h>
#include <QTextStream>
#include <QByteArray>

#include <kgenericfactory.h>
#include <kdebug.h>

#include "kopeteaccountmanager.h"
#include "kopeteonlinestatusmanager.h"
#include "kopeteglobal.h"

#include "gwaccount.h"
#include "gwerror.h"
#include "gwcontact.h"
#include "ui/gwaddcontactpage.h"
#include "ui/gweditaccountwidget.h"

K_PLUGIN_FACTORY( GroupWiseProtocolFactory, registerPlugin<GroupWiseProtocol>(); )
K_EXPORT_PLUGIN( GroupWiseProtocolFactory( "kopete_groupwise" ) )

GroupWiseProtocol *GroupWiseProtocol::s_protocol = 0L;

GroupWiseProtocol::GroupWiseProtocol( QObject* parent, const QVariantList &/*args*/ )
	: Kopete::Protocol( GroupWiseProtocolFactory::componentData(), parent ),
/* initialise Kopete::OnlineStatus that should be user selectable in the user interface */
	  groupwiseOffline ( Kopete::OnlineStatus::Offline,    0,  this, GroupWise::Offline, QStringList(),
			i18n( "Offline" ), i18n( "O&ffline" ), Kopete::OnlineStatusManager::Offline ),
	  groupwiseAvailable  ( Kopete::OnlineStatus::Online,  25, this, GroupWise::Available, QStringList(), 
			i18n( "Online" ), i18n( "A&vailable" ), Kopete::OnlineStatusManager::Online ),
	  groupwiseBusy       ( Kopete::OnlineStatus::Busy,    18, this, GroupWise::Busy, QStringList( "contact_busy_overlay" ),
			i18n( "Busy" ), i18n( "&Busy" ), Kopete::OnlineStatusManager::Busy, Kopete::OnlineStatusManager::HasStatusMessage ),
	  groupwiseAway       ( Kopete::OnlineStatus::Away,    20, this, GroupWise::Away, QStringList( "contact_away_overlay" ),
			i18n( "Away" ), i18n( "&Away" ), Kopete::OnlineStatusManager::Away, Kopete::OnlineStatusManager::HasStatusMessage ),
	  groupwiseAwayIdle   ( Kopete::OnlineStatus::Away,    15, this, GroupWise::AwayIdle, QStringList( "contact_away_overlay" ),
			i18n( "Idle" ), "FIXME: Make groupwiseAwayIdle unselectable", Kopete::OnlineStatusManager::Idle,
			Kopete::OnlineStatusManager::HideFromMenu ),
	  groupwiseAppearOffline( Kopete::OnlineStatus::Invisible, 2, this, 98, QStringList( "contact_invisible_overlay" ),
	  		i18n( "Appear Offline" ), i18n( "A&ppear Offline" ), Kopete::OnlineStatusManager::Invisible ),
/* initialise Kopete::OnlineStatus used by the protocol, but that are not user selectable */
	  groupwiseUnknown    ( Kopete::OnlineStatus::Unknown, 25, this, GroupWise::Unknown, QStringList( "status_unknown" ),
			i18n( "Unknown" ) ),
	  groupwiseInvalid    ( Kopete::OnlineStatus::Unknown, 25, this, GroupWise::Invalid, QStringList( "status_unknown" ),
			i18n( "Invalid Status" ) ),
	  groupwiseConnecting ( Kopete::OnlineStatus::Connecting, 25, this, 99, QStringList( "groupwise_connecting" ),
			i18n( "Connecting" ) ),
	  propGivenName( Kopete::Global::Properties::self()->firstName() ),
	  propLastName( Kopete::Global::Properties::self()->lastName() ),
	  propFullName( Kopete::Global::Properties::self()->fullName() ),
	  propAutoReply( "groupwiseAutoReply", i18n( "Auto Reply Message" ), QString() ),
	  propCN( "groupwiseCommonName", i18n( "Common Name" ), QString(), Kopete::PropertyTmpl::PersistentProperty ),
	  propPhoneWork( Kopete::Global::Properties::self()->workPhone() ),
	  propPhoneMobile( Kopete::Global::Properties::self()->privateMobilePhone() ),
	  propEmail( Kopete::Global::Properties::self()->emailAddress() )
{
	// ^^ That is all member initialiser syntax, not broken indentation!
	kDebug() ;

	s_protocol = this;

  addAddressBookField( "messaging/groupwise", Kopete::Plugin::MakeIndexField );
}

GroupWiseProtocol::~GroupWiseProtocol()
{
}

Kopete::Contact *GroupWiseProtocol::deserializeContact(
	Kopete::MetaContact *metaContact, const QMap<QString, QString> &serializedData,
	const QMap<QString, QString> &/* addressBookData */)
{
	QString dn = serializedData[ "DN" ];
	QString accountId = serializedData[ "accountId" ];
	Kopete::Contact::NameType nameType = Kopete::Contact::nameTypeFromString(serializedData[ "preferredNameType" ]);
	int objectId = serializedData[ "objectId" ].toInt();
	int parentId = serializedData[ "parentId" ].toInt();
	int sequence = serializedData[ "sequenceNumber" ].toInt();
	
	QList<Kopete::Account*> accounts = Kopete::AccountManager::self()->accounts( this );

	Kopete::Account *account = (Kopete::AccountManager::self()->findAccount(pluginId(), accountId));

	if ( !account )
	{
		kDebug() << "Account doesn't exist, skipping";
		return 0;
	}

	// FIXME: creating a contact with a userId here
	GroupWiseContact *contact = new GroupWiseContact(account, dn, metaContact, objectId, parentId, sequence );
	contact->setPreferredNameType(nameType);
	return contact;
}

AddContactPage * GroupWiseProtocol::createAddContactWidget( QWidget *parent, Kopete::Account *  account )
{
	kDebug() << "Creating Add Contact Page";
	return new GroupWiseAddContactPage( account, parent );
}

KopeteEditAccountWidget * GroupWiseProtocol::createEditAccountWidget( Kopete::Account *account, QWidget *parent )
{
	kDebug() << "Creating Edit Account Page";
	return new GroupWiseEditAccountWidget( parent, account );
}

Kopete::Account *GroupWiseProtocol::createNewAccount( const QString &accountId )
{
	return new GroupWiseAccount( this, accountId );
}

GroupWiseProtocol *GroupWiseProtocol::protocol()
{
	return s_protocol;
}

Kopete::OnlineStatus GroupWiseProtocol::gwStatusToKOS( const int gwInternal )
{
	Kopete::OnlineStatus status;
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
			kWarning() << "Got unrecognised status value" << gwInternal;
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
	QByteArray plainUtf8 = plain.toUtf8(); // encoded as UTF8, because that's what this encoding algorithm, taken from Gaim's Novell plugin
	int index = 0; // current char to transcode
	while ( index  < plainUtf8.length() )
	{
		quint8 current = plainUtf8.data()[ index ];
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
			quint32 ucs4Char;
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
				kDebug() << "bogus utf-8 lead byte: 0x" << QTextStream::hex << current;
				ucs4Char = 0x003F;
				bytesEncoded = 1;
			}
			index += bytesEncoded;
			escapedUnicodeChar = QString("\\u%1?").arg( ucs4Char );
			kDebug() << "unicode escaped char: " << escapedUnicodeChar;
			outputText.append( escapedUnicodeChar );
		}
	}
	return rtfTemplate.arg( outputText );
}

QString GroupWiseProtocol::dnToDotted( const QString & dn )
{
	QRegExp rx("[a-zA-Z]*=(.*)$", Qt::CaseInsensitive );
	if( dn.indexOf( '=' ) == -1 ) // if it's not a DN, return it unprocessed
		return dn;

	// split the dn into elements
	QStringList elements = dn.split( ',' );
	// remove the key, keep the value
	for ( QStringList::Iterator it = elements.begin(); it != elements.end(); ++it )
	{
		if ( rx.indexIn( *it ) != -1 )
			*it = rx.cap( 1 );
	}
	QString dotted = elements.join( "." );
	// reassemble as dotted

	return dotted;
}
#include "gwprotocol.moc"
