/*
    kopeteiface.h - Kopete DCOP Interface

    Copyright (c) 2002 by Hendrik vom Lehn       <hennevl@hennevl.de>

    Kopete    (c) 2002-2003      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KopeteIface_h
#define KopeteIface_h

#include <dcopobject.h>
#include <qstringlist.h>
#include <kurl.h>

#include "kopeteonlinestatus.h"

/**
 * DCOP interface for kopete
 */
class KopeteIface : virtual public DCOPObject
{
	K_DCOP

public:
	KopeteIface();

k_dcop:
	QStringList contacts();
	QStringList reachableContacts();
	QStringList onlineContacts();
	QStringList fileTransferContacts();
	QStringList contactFileProtocols(const QString &displayName);

	/*void sendFile(const QString &displayName, const KURL &sourceURL,
		const QString &altFileName = QString::null, uint fileSize = 0);*/

	// FIXME: Do we *need* this one? Sounds error prone to me, because
	// nicknames can contain parentheses too.
	// Better add a contactStatus( const QString id ) I'd say - Martijn
	QStringList contactsStatus();

	/**
	 * Open a chat to a contact, and optionally set some initial text
	 */
	QString messageContact( const QString &contactId, const QString &messageText = QString::null );

	/**
	 * Describe the status of a contact by their metaContactId,
	 * aka their uid in KABC.
	 */
	QString onlineStatus( const QString &metaContactId );

	/**
	 * Message a contact by their metaContactId, aka their uid in KABC.
	 */
	void messageContactById( const QString &metaContactId );

	/**
	 * Adds a contact with the specified params.
	 *
	 * @param protocolName The name of the protocol this contact is for ("ICQ", etc)
	 * @param accountId The account ID to add the contact to
	 * @param contactId The unique ID for this protocol
	 * @param displayName The displayName of the contact (may equal userId for some protocols
	 * @param groupName The name of the group to add the contact to
	 * @return Weather or not the contact was added successfully
	 */
	bool addContact( const QString &protocolName, const QString &accountId, const QString &contactId,
		const QString &displayName, const QString &groupName = QString::null );

	/**
	 * return a list of alls accounts.
	 * form: XXXProtocol||AccountId
	 */
	QStringList accounts();

	/**
	 * connect a given account in the given protocol
	 */
	void connect(const QString &protocolName, const QString &accountId);
	/**
	 * disconnect a given account in the given protocol
	 */
	void disconnect(const QString &protocolName, const QString &accountId);

	/**
	 * Ask all accounts to connect
	 */
	void connectAll();

	/**
	 * Ask all accounts to disconnect
	 */
	void disconnectAll();

	/**
	 * load a plugin
	 * the name is the name of the library: example: kopete_msn
	 * but you can ommit the kopete_ prefix
	 */
	bool loadPlugin( const QString& name );
	/**
	 * unload a plugin
	 * the name is the name of the library: example: kopete_msn
	 * but you can ommit the kopete_ prefix
	 */
	bool unloadPlugin( const QString& name );

	/**
	 * set all account away using the global away function
	 */
	void setAway();

	/**
	 * set all account away using the global away function
	 * and set an away message
	 */
	void setAway( const QString &msg ) { setAway( msg, true ); }

	/**
	 * set all account away using the global away function
	 * and set an away message.
	 * @param away decides if the message is away/non-away
	 */
	void setAway( const QString &msg, bool away );

	/**
	 * set Available all accountes
	 */
	void setAvailable();
	/**
	 * set all account away using the auto away funciton.
	 * accounts will return online if activity is detected again
	 */
	void setAutoAway();

	/**
	 * set the global nickname if global identity is enabled.
	 * @param nickname the new global nickname
	 */
	void setGlobalNickname( const QString &nickname );

	/**
	 * set the global photo if global identity is enabled.
	 * @param photoUrl URL to the photo
	 */
	void setGlobalPhoto( const KURL &photoUrl );
    
    /**
     * get the contactIds for a given display name
     * @param displayName
     */
    QStringList contactsForDisplayName( const QString & displayName );

    /**
     * get the metacontactIds that have the given contactId
     * @param contactId
     */
    QStringList metacontactsForContactId( const QString & contactId );
};

#endif



/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

