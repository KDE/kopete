/*
    ircaccount.h - IRC Account

    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>

    Kopete    (c) 2002      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef IRCACCOUNT_H
#define IRCACCOUNT_H

#include <qstring.h>
#include <qstringlist.h>

#include "kopeteaccount.h"

class KAction;
class KActionMenu;

class KopeteMetaContact;
class KopeteMessage;
class KopeteContact;
class KopeteMessageManager;

class KIRC;

class IRCProtocol;
class IRCContactManager;
class IRCChannelContact;
class IRCServerContact;
class IRCUserContact;

class IRCAccount
	: public KopeteAccount
{
	Q_OBJECT

public:
	IRCAccount(IRCProtocol *p, const QString &accountid);
	~IRCAccount();

	// Load the user preferences.
	virtual void loaded();

	const QString userName() const;
	
	const QStringList connectCommands() const;
	
	void setConnectCommands( const QStringList & ) const;
	
	void setDefaultPart( const QString & );
	
	void setDefaultQuit( const QString & );
	
	void setUserName( const QString & );
	
	void setAltNick( const QString & );
	
	const QString defaultPart() const;
	
	const QString defaultQuit() const;
	
	const QString altNick() const;
	
	QMap< QString, QString > customCtcp() const;
	
	void setCustomCtcpReplies( const QMap< QString, QString > &replys ) const;
	
	const QMap<QString, QString> customCtcpReplies() const;
	
public slots:

	void unregister(KopeteContact *);

	IRCServerContact *findServer(const QString &name, KopeteMetaContact *m = 0L);

	/**
	 * Attempts to find a IRCChannelContact with the specified name in
	 * our registered channels list. If one exists, it is returned. If not, a new contact
	 * is created for it, and either assigned to the passed in @ref KopeteMestaContact
	 * if it exists, or assigned to a temporary @ref KopeteMetaContact. If a temporary
	 * @ref KopeteMetaContact is created, it is *not* added to the contact list by this
	 * function.
	 */
	IRCChannelContact *findChannel(const QString &name, KopeteMetaContact *m = 0L);

	/**
	 * Attempts to find a IRCUserContact with the specified name in
	 * our registered users list. If one exists, it is returned. If not, a new contact
	 * is created for it, and either assigned to the passed in @ref KopeteMestaContact
	 * if it exists, or assigned to a temporary @ref KopeteMetaContact. If a temporary
	 * @ref KopeteMetaContact is created, it is *not* added to the contact list by this
	 * function.
	 */
	IRCUserContact *findUser(const QString &name, KopeteMetaContact *m = 0L);

	void unregisterServer(const QString &name);

	/**
	 * Unregisters a channel contact. This function checks the channels conversation
	 * count (the number of conversations it is taking part in), and if it is 0, it deletes the
	 * contact and removes it from the registered channels list. Channels in the contact
	 * list are never deleted.
	 */
	void unregisterChannel(const QString &name);

	/**
	 * Unregisters a user contact. This function checks the users conversation
	 * count (the number of conversations it is taking part in), and if it is 0, it deletes the
	 * contact and removes it from the registered channels list. Users in the contact
	 * list are never deleted.
	 */
	void unregisterUser(const QString &name);

	virtual KActionMenu *actionMenu();

	virtual void setAway( bool isAway, const QString &awayMessage = QString::null );

	virtual bool isConnected();
	
	// Returns the KIRC engine instance
	KIRC *engine() const
		{ return m_engine; }

	// Returns the IRCProtocol instance for contacts
	IRCProtocol *protocol() const
		{ return m_protocol; }

	IRCContactManager *contactManager() const
		{ return m_contactManager; }

	// Returns the KopeteContact of the user
	IRCUserContact *mySelf() const;

	// Returns the KopeteContact of the server of the user
	IRCServerContact *myServer() const;

	void successfullyChangedNick(const QString &, const QString &);
	virtual void connect();
	virtual void disconnect();
	
	void quit( const QString &quitMessage = QString::null );
	
protected:
	virtual bool addContactToMetaContact( const QString &contactId, const QString &displayName, KopeteMetaContact *parentContact ) ;


private slots:
	void slotFailedServerPassword();
	void slotGoAway( const QString &reason );
	void slotJoinChannel();
	void slotShowServerWindow();
	void slotNickInUse( const QString &nick );
	void slotNickInUseAlert( const QString &nick );
	void slotConnectedToServer();

private:
	IRCProtocol *m_protocol;
	KopeteMessageManager *m_manager;
	QString mNickName;
	
	QString m_server;
	uint m_port;
	bool triedAltNick;

	KIRC *m_engine;

	IRCContactManager *m_contactManager;
	IRCServerContact *m_myServer;
	
	QMap< QString, QString > m_customCtcp;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

