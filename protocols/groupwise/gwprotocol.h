/*
    gwprotocol.h - Kopete GroupWise Protocol

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Testbed   
    Copyright (c) 2003      by Will Stephenson		 <will@stevello.free-online.co.uk>
    
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

#ifndef TESTBEDPROTOCOL_H
#define TESTBEDPROTOCOL_H

#include <kopeteprotocol.h>
#include "kopetecontactproperty.h"

/**
 * Encapsulates the generic actions associated with this protocol
 * @author Will Stephenson
 */
class GroupWiseProtocol : public KopeteProtocol
{
	Q_OBJECT
public:
	GroupWiseProtocol(QObject *parent, const char *name, const QStringList &args);
    ~GroupWiseProtocol();
	/**
	 * Convert the serialised data back into a GroupWiseContact and add this
	 * to its KopeteMetaContact
	 */
	virtual KopeteContact *deserializeContact(
			KopeteMetaContact *metaContact,
			const QMap< QString, QString > & serializedData,
			const QMap< QString, QString > & addressBookData
		);
	/**
	 * Generate the widget needed to add GroupWiseContacts
	 */
	virtual AddContactPage * createAddContactWidget( QWidget *parent, KopeteAccount *account );
	/**
	 * Generate the widget needed to add/edit accounts for this protocol
	 */
	virtual KopeteEditAccountWidget * createEditAccountWidget( KopeteAccount *account, QWidget *parent );
	/**
	 * Generate a GroupWiseAccount
	 */
	virtual KopeteAccount * createNewAccount( const QString &accountId );
	/**
	 * Access the instance of this protocol
	 */
	static GroupWiseProtocol *protocol();
	/**
	 * Transform a GroupWise internal status into a KopeteOnlineStatus
	 */
	KopeteOnlineStatus gwStatusToKOS( const int gwInternal );
	/**
	 * Online statuses used for contacts' presence
	 */
	const KopeteOnlineStatus groupwiseUnknown;
	const KopeteOnlineStatus groupwiseOffline;
	const KopeteOnlineStatus groupwiseAvailable;
	const KopeteOnlineStatus groupwiseBusy;
	const KopeteOnlineStatus groupwiseAway;
	const KopeteOnlineStatus groupwiseAwayIdle;
	const KopeteOnlineStatus groupwiseInvalid;
	const KopeteOnlineStatus groupwiseConnecting;
	
	/**
	 * Represents contacts that are Away
	 */
	const Kopete::ContactPropertyTmpl propGivenName;
	const Kopete::ContactPropertyTmpl propLastName;
	const Kopete::ContactPropertyTmpl propFullName;
	const Kopete::ContactPropertyTmpl propAwayMessage;
	const Kopete::ContactPropertyTmpl propAutoReply;
	const Kopete::ContactPropertyTmpl propCN;
	
	
protected:
	static GroupWiseProtocol *s_protocol;
};

#endif
