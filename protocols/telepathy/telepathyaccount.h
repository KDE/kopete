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

#include <QtTapioca/ConnectionManager>
#include <QtTapioca/Connection>

class KActionMenu;

namespace Kopete 
{ 
	class MetaContact;
	class StatusMessage;
}

class TelepathyProtocol;
class TelepathyContactManager;

using namespace QtTapioca;
/**
 * @author Michaël Larouche <larouche@kde.org>
 */
class TelepathyAccount : public Kopete::Account
{
	Q_OBJECT
public:
	TelepathyAccount(TelepathyProtocol *parent, const QString &accountId);
	~TelepathyAccount();

	virtual KActionMenu *actionMenu();

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
	QList<QtTapioca::ConnectionManager::Parameter> connectionParameters() const;

	/**
	 * @brief Get all connection parameters merged with values from the config
	 *
	 * @return all connection parameters.
	 */
	QList<QtTapioca::ConnectionManager::Parameter> allConnectionParameters();

	/**
	 * @brief Return the contact manager
	 * @return the Telepathy contact manager.
	 */
	TelepathyContactManager *contactManager();

signals:
	/**
	 * Emitted when we are connected to a Telepathy connection manager.
	 */
	void telepathyConnected();

public slots:
	virtual void connect(const Kopete::OnlineStatus& initialStatus = Kopete::OnlineStatus());
	virtual void disconnect();
	
	virtual void setOnlineStatus(const Kopete::OnlineStatus& status, const Kopete::StatusMessage &reason = Kopete::StatusMessage());
	virtual void setStatusMessage(const Kopete::StatusMessage &statusMessage);
	
protected:
	virtual bool createContact(const QString &contactId, Kopete::MetaContact *parentMetaContact);

private slots:
	void telepathyStatusChanged(Connection *connection, Connection::Status status, Connection::Reason reason);
	/**
	 * @brief Do all the initialition stuff after being connection
	 */
	void slotTelepathyConnected();
	/**
	 * @brief Fetch the contact list.
	 */
	void fetchContactList();

private:
	class Private;
	Private *d;
};
#endif
