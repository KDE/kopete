/*
    irccontact.h - IRC Contact

    Copyright (c) 2003-2004 by Michel Hermier <michel.hermier@wanadoo.fr>
    Copyright (c) 2003      by Jason Keirstead <jason@keirstead.org>
    Copyright (c) 2002      by Nick Betcher <nbetcher@kde.org>

    Kopete    (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef IRCCONTACT_H
#define IRCCONTACT_H

#include "ircconst.h"

#include "kopetecontact.h"
#include "kopetemessage.h"

#include <QMap>

class IRCAccount;
class IRCProtocol;

namespace KIRC
{
class Client;
class Entity;
}

namespace Kopete
{
class ChatSession;
class MetaContact;
}

class KopeteView;

class QTextCodec;

/**
 * @author Jason Keirstead <jason@keirstead.org>
 * @author Michel Hermier <michel.hermier@wanadoo.fr>
 */
class IRCContact
	: public Kopete::Contact
{
	Q_OBJECT

public:
	IRCContact(IRCAccount *account, KIRC::Entity *entity,
		Kopete::MetaContact *metac = 0, const QString& icon = QString::null);
	~IRCContact();

	IRCAccount *ircAccount() const;
	KIRC::Client *kircClient() const;

	QString caption() const;

	/**
	 * This function attempts to find the nickname specified within the current chat
	 * session. Returns a pointer to that IRCUserContact, or 0L if the user does not
	 * exist in this session. More useful for channels. Calling IRCChannelContact::locateUser()
	 * for example tells you if a user is in a certain channel.
	 */
	Kopete::Contact *locateUser(const QString &nickName);

	bool isReachable();

	/**
	 * return true if the contact is in a chat. false if the contact is in no chats
	 * that loop over all manager, and checks the presence of the user
	 */
	bool isChatting( Kopete::ChatSession *avoid = 0L ) const;

	Kopete::ChatSession *manager(CanCreateFlags cancreate = CannotCreate);
	Kopete::ChatSession *chatSessionCreate(IRC::ChatSessionType type = IRC::SERVER);

	void appendMessage(Kopete::Message &);

	QTextCodec *codec();

	/**
	 * We serialise the contactId and the server group in 'contactId'
	 * so that other IRC programs reading this from KAddressBook have a chance of figuring
	 * which server the contact relates to
	 */
	void serialize( QMap<QString, QString> &serializedData, QMap<QString, QString> &addressBookData );

signals:
	void destoyed(IRCContact *self);

public slots:
	void updateStatus();

	void setCodec(QTextCodec *codec);

private slots:
	void entityUpdated();

	void slotSendMsg(Kopete::Message &message, Kopete::ChatSession *chatSession);
	QString sendMessage( const QString &msg );

	void chatSessionDestroyed(Kopete::ChatSession *chatSession);

	void deleteContact();

private:
	Q_DISABLE_COPY(IRCContact)

	class Private;
	Private * const d;
};

#endif
