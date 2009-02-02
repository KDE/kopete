/*
 * telepathyaccount.h - Telepathy Kopete Account.
 *
 * Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>
 *
 * Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>
 *
 *************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
 */
#ifndef TELEPATHYACCOUNT_H
#define TELEPATHYACCOUNT_H

#include <kopeteaccount.h>
#include <QtCore/QList>

#include <TelepathyQt4/Client/ConnectionManager>

#include <QtTapioca/ConnectionManager>
#include <QtTapioca/Connection>

class KActionMenu;

namespace QtTapioca
{
	class Channel;
	class TextChannel;
	class Contact;
}

namespace Kopete
{
	class MetaContact;
	class StatusMessage;
}

class TelepathyProtocol;
class TelepathyContactManager;
class TelepathyContact;

namespace Telepathy
{
    namespace Client
    {
        class ContactManager;
    }
}

/**
 * @author Michaël Larouche <larouche@kde.org>
 */
class TelepathyAccount : public Kopete::Account
{
	Q_OBJECT
public:
	TelepathyAccount(TelepathyProtocol *parent, const QString &accountId);
	~TelepathyAccount();

	virtual void fillActionMenu( KActionMenu *actionMenu );

	/**
	 * @brief Get the casted instance of myself contact
	 * @return Myself contact as TelepathyContact.
	 */
	TelepathyContact *myself();

	/**
	 * @brief Read the configuration for the current account.
	 * @return true if reading went well.
	 */
	bool readConfig();

	/**
	 * @brief Get the name of the connection manager for this account.
	 *
	 * You must call readConfig() before.
	 * @return name of the connection manager used by this account.
	 */
	QString connectionManager() const;
	/**
	 * @brief Get the current protocol used by this account.
	 *
	 * You must call readConfig() before.
	 * @return name of the protocol used by this account.
	 */
	QString connectionProtocol() const;
	/**
	 * @brief Get the connection parameters read from the config.
	 * Only needed parameters are included in the list.
	 *
	 * You must call readConfig() before.
	 *
	 * @return saved connection parameters.
	 */
	Telepathy::Client::ProtocolParameterList connectionParameters() const;

	/**
	 * @brief Get all connection parameters merged with values from the config
	 *
	 * @return all connection parameters.
	 */
	Telepathy::Client::ProtocolParameterList allConnectionParameters();

	/**
	 * @brief Return the contact manager
	 * @return the Telepathy contact manager.
	 */
	Telepathy::Client::ContactManager *contactManager();

	/**
	 * @brief Create a new chat session using the given text channel
	 * @param newChannel Instance of TextChannel.
	 */
	void createTextChatSession(QtTapioca::TextChannel *newChannel);

	/**
	 * @brief Create a new text channel to the given contacté
	 * @param internalContact QtTapioca contact instance.
	 */
	QtTapioca::TextChannel *createTextChannel(QtTapioca::Contact *internalContact);

	/**
	 * @brief Change the alias of the contact if the connection manager support it
	 *
	 * @param newAlias New alias to give to the contact
	 * @return true if alias was changed.
	 */
	bool changeAlias(const QString &newAlias);

signals:
	/**
	 * Emitted when we are connected to a Telepathy connection manager.
	 */
	void telepathyConnected();

public slots:
	virtual void connect(const Kopete::OnlineStatus& initialStatus = Kopete::OnlineStatus());
	virtual void disconnect();

	virtual void setOnlineStatus(const Kopete::OnlineStatus& status, const Kopete::StatusMessage &reason = Kopete::StatusMessage(),
	                             const OnlineStatusOptions& options = None);
	virtual void setStatusMessage(const Kopete::StatusMessage &statusMessage);

	/**
	 * @brief Called from the menu to change the myself contact alias
	 */
	void slotSetAlias();

protected:
	virtual bool createContact(const QString &contactId, Kopete::MetaContact *parentMetaContact);

private slots:
	/**
	 * @brief State of Telepathy connection changed.
	 */
	void telepathyStatusChanged(QtTapioca::Connection *connection, QtTapioca::Connection::Status status, QtTapioca::Connection::Reason reason);

	/**
	 * @brief Dispatch incoming channel request to the Kopete equivalent.
	 *
	 * @param connection Connection where the channel request come from
	 * @param channel Incoming channel.
	 */
	void telepathyChannelCreated(QtTapioca::Connection *connection, QtTapioca::Channel *channel);

	/**
	 * @brief Do all the initialition stuff after being connection
	 */
	void slotTelepathyConnected();

	/**
	 * @brief Fetch the contact list.
	 */
	void fetchContactList();

	/**
	 * @brief Change the current avatar
	 */
	void slotChangeAvatar();

private:
	class Private;
	Private *d;
};
#endif
