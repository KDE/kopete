/*
    gwprotocol.h - Kopete GroupWise Protocol

    Copyright (c) 2006,2007 Novell, Inc	 	 http://www.opensuse.org
    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com

    Based on Testbed
    Copyright (c) 2003-2007 by Will Stephenson		 <wstephenson@kde.org>
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

#ifndef GWPROTOCOL_H
#define GWPROTOCOL_H

#include <kopeteprotocol.h>
#include <kopeteproperty.h>
#include <kopeteonlinestatus.h>

/**
 * Encapsulates the generic actions associated with this protocol
 * @author Will Stephenson
 */
class GroupWiseProtocol : public Kopete::Protocol
{
	Q_OBJECT
public:
	GroupWiseProtocol(QObject *parent, const QVariantList &args);
    ~GroupWiseProtocol();
	/**
	 * Convert the serialised data back into a GroupWiseContact and add this
	 * to its Kopete::MetaContact
	 */
	virtual Kopete::Contact *deserializeContact(
			Kopete::MetaContact *metaContact,
			const QMap< QString, QString > & serializedData,
			const QMap< QString, QString > & addressBookData
		);
	/**
	 * Generate the widget needed to add GroupWiseContacts
	 */
	virtual AddContactPage * createAddContactWidget( QWidget *parent, Kopete::Account *account );
	/**
	 * Generate the widget needed to add/edit accounts for this protocol
	 */
	virtual KopeteEditAccountWidget * createEditAccountWidget( Kopete::Account *account, QWidget *parent );
	/**
	 * Generate a GroupWiseAccount
	 */
	virtual Kopete::Account * createNewAccount( const QString &accountId );
	/**
	 * Access the instance of this protocol
	 */
	static GroupWiseProtocol *protocol();
	/**
	 * Transform a GroupWise internal status into a Kopete::OnlineStatus
	 */
	Kopete::OnlineStatus gwStatusToKOS( const int gwInternal );
	/**
	 * Wrap unformatted text in RTF formatting so that other GroupWise clients will display it
	 * @param plain unformatted text
	 * @return RTF text (in UCS-4 encoding)
	 */
	QString rtfizeText( const QString & plain );
	/**
	 * Convert full DNs to dotted-untyped format
	 * Assumes the DN is normalised - comma separated, no spaces between elements
	 * eg cn=wstephenson,o=suse becomes wstephenson.suse
	 */
	static QString dnToDotted( const QString & dn );
	/**
	 * Online statuses used for contacts' presence
	 */
	const Kopete::OnlineStatus groupwiseOffline;
	const Kopete::OnlineStatus groupwiseAvailable;
	const Kopete::OnlineStatus groupwiseBusy;
	const Kopete::OnlineStatus groupwiseAway;
	const Kopete::OnlineStatus groupwiseAwayIdle;
	const Kopete::OnlineStatus groupwiseAppearOffline;
	const Kopete::OnlineStatus groupwiseUnknown;
	const Kopete::OnlineStatus groupwiseInvalid;
	const Kopete::OnlineStatus groupwiseConnecting;

	/**
	 * Contact properties
	 */
	const Kopete::PropertyTmpl propGivenName;
	const Kopete::PropertyTmpl propLastName;
	const Kopete::PropertyTmpl propFullName;
	const Kopete::PropertyTmpl propAutoReply;
	const Kopete::PropertyTmpl propCN;
	const Kopete::PropertyTmpl propPhoneWork;
	const Kopete::PropertyTmpl propPhoneMobile;
	const Kopete::PropertyTmpl propEmail;
	
	
protected:
	static GroupWiseProtocol *s_protocol;
};

#endif // GWPROTOCOL_H
