/*
    ircprotocol.h - IRC Protocol

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

#ifndef IRCPROTOCOL_H
#define IRCPROTOCOL_H

#include "kopeteonlinestatus.h"
#include "kopeteprotocol.h"

class KopeteMetaContact;
class AddContactPage;
class KIRC;

class EditIdentityWidget;
class KopeteIdentity;
class IRCIdentity;

class QStringList;
class QWidget;
class KSParser;

/**
 * @author Nick Betcher <nbetcher@kde.org>
 */
class IRCProtocol : public KopeteProtocol
{
	Q_OBJECT

public:
	IRCProtocol( QObject *parent, const char *name, const QStringList &args );
	~IRCProtocol();

	/** KopeteProtocol reimplementation */
	virtual AddContactPage *createAddContactWidget(QWidget *parent);
	virtual KActionMenu* protocolActions();

	/**
	 * Deserialize contact data
	 */
	virtual void deserializeContact( KopeteMetaContact *metaContact,
		const QMap<QString, QString> &serializedData, const QMap<QString, QString> &addressBookData );

	virtual EditIdentityWidget* createEditIdentityWidget(KopeteIdentity *identity, QWidget *parent);

	virtual KopeteIdentity* createNewIdentity(const QString &identityId);

	/*
	 * Returns a pointer to the KSParser. The KSParser changes IRC color codes into HTML
	 * for use in KopeteMessage.
	 */
	KSParser *parser() const { return mParser; };

	static IRCProtocol *protocol();

	static KopeteOnlineStatus IRCChannelOnline() { return m_ChannelOnline; };
	static KopeteOnlineStatus IRCUserOnline() { return m_UserOnline; };
	static KopeteOnlineStatus IRCChannelOffline() { return m_ChannelOffline; };
	static KopeteOnlineStatus IRCUserOffline() { return m_UserOffline; };


private:
	/** FIXME: Do something with this when Identity support is added!!!!!!!! */
	IRCIdentity *identity;

	QMap<QString,IRCIdentity*> mIdentityMap;

	static IRCProtocol *s_protocol;

	static KopeteOnlineStatus m_ChannelOnline;
	static KopeteOnlineStatus m_ChannelOffline;
	static KopeteOnlineStatus m_UserOnline;
	static KopeteOnlineStatus m_UserOffline;

	KSParser *mParser;

};

#endif

// vim: set noet ts=4 sts=4 sw=4:

