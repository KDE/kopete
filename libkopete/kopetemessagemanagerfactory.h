/*
    kopetemessagemanagerfactory.h - Creates chat sessions

    Copyright   : (c) 2002 by Duncan Mac-Vicar Prett
    Email       : duncan@kde.org

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KOPETEMESSAGEMANAGERFACTORY_H
#define KOPETEMESSAGEMANAGERFACTORY_H

#include <qobject.h>
#include <qptrlist.h>
#include <qintdict.h>
#include <qvaluelist.h>

#include "kopetemessagemanager.h"

/**
 * @author Duncan Mac-Vicar Prett
 */
class KopeteMessageManager;
class KopeteMessage;
class KopeteContact;
class KopeteProtocol;

typedef QPtrList<KopeteContact>        KopeteContactPtrList;
typedef QValueList<KopeteMessage>      KopeteMessageList;
typedef QIntDict<KopeteMessageManager> KopeteMessageManagerDict;

class KopeteMessageManagerFactory : public QObject
{
	Q_OBJECT

public:
	static KopeteMessageManagerFactory* factory();

	~KopeteMessageManagerFactory();

	/**
	 * Create a new chat session. Provided is the initial list of contacts in
	 * the session. If a session with exactly these contacts already exists,
	 * it will be reused. Otherwise a new session is created.
	 */
	KopeteMessageManager* create( const KopeteContact *user,
		KopeteContactPtrList chatContacts, KopeteProtocol *protocol,
		KopeteMessageManager::WidgetType widget = KopeteMessageManager::ChatWindow);

	KopeteMessageManager* findKopeteMessageManager( const KopeteContact *user,
		KopeteContactPtrList chatContacts, KopeteProtocol *protocol,
		KopeteMessageManager::WidgetType widget = KopeteMessageManager::ChatWindow);

	void addKopeteMessageManager(KopeteMessageManager *);

	KopeteMessageManager *findKopeteMessageManager( int );

	/**
	 * Get a list of all open sessions
	 */
	const KopeteMessageManagerDict& sessions();
	/**
	 * Get a list of all open sessions  for a protocol
	 */
	KopeteMessageManagerDict protocolSessions( KopeteProtocol *);

	/**
	 * Clean sessions for a protocol
	 */
	void cleanSessions( KopeteProtocol *);

signals:
	void messageReceived( KopeteMessage& message );
	void messageQueued( KopeteMessage& message );

protected slots:
	void slotRemoveSession( KopeteMessageManager *session );

private:
	KopeteMessageManagerFactory( QObject* parent = 0, const char* name = 0 );

	int mId;
	KopeteMessageManagerDict mSessionDict;

	static KopeteMessageManagerFactory *s_factory;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

