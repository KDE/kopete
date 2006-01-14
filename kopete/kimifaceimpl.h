/*
    kimifaceimpl.cpp - Kopete DCOP Interface

    Copyright (c) 2004 by Will Stephenson     <lists@stevello.free-online.co.uk>

    Kopete    (c) 2002-2004      by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#ifndef KIMIFACEIMPL_H
#define KIMIFACEIMPL_H

#include <qobject.h>
#include "kimiface.h"

namespace Kopete
{
class MetaContact;
}

class KIMIfaceImpl : public QObject, public KIMIface
{
	Q_OBJECT
public:
	KIMIfaceImpl();
	~KIMIfaceImpl();

	QStringList allContacts();
	QStringList reachableContacts();
	QStringList onlineContacts();
	QStringList fileTransferContacts();
	
// individual
	bool isPresent( const QString &uid );
	QString displayName( const QString &uid );
	QString presenceString( const QString &uid );
	int presenceStatus( const QString &uid );
	bool canReceiveFiles( const QString &uid );
	bool canRespond( const QString &uid );
	QString locate( const QString &contactId, const QString &protocol );
// metadata
	QPixmap icon( const QString &uid );
	QString context( const QString &uid );
// App capabilities
	QStringList protocols();
	
// ACTORS
	/**
	 * Message a contact by their metaContactId, aka their uid in KABC.
	 */
	void messageContact( const QString &uid, const QString& message );
	
	/**
	 * Open a chat to a contact, and optionally set some initial text
	 */
	void messageNewContact(  const QString &contactId, const QString &protocolId );

	/**
	 * Message a contact by their metaContactId, aka their uid in KABC.
	 */
	void chatWithContact( const QString &uid );

	/**
	 * Send the file to the contact
	 */
	void sendFile(const QString &uid, const KURL &sourceURL,
		const QString &altFileName = QString::null, uint fileSize = 0);

// MUTATORS
// Contact list
	bool addContact( const QString &contactId, const QString &protocolId );
// SIGNALS
	/**
	 * DCOP Signal used to notify 
	 * external apps of status changes.
	 */
	void contactStatusChanged( const QString &uid);

protected:
	void unknown( const QString &uid );
protected slots:
	void slotMetaContactAdded( Kopete::MetaContact *mc );
	void slotContactStatusChanged( Kopete::MetaContact *mc );	
	
private:
	Kopete::MetaContact *locateProtocolContact( const QString & contactId, const QString & protocolId );
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

