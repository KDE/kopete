/*
    kopetedbusinterface.h - Kopete D-Bus interface

    Copyright (c) 2007      by Michaël Larouche      <larouche@kde.org>
    Copyright (c) 2007         Will Stephenson       <wstephenson@kde.org>

    Kopete    (c) 2002-2007 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#ifndef KOPETE_DBUSINTERFACE_H
#define KOPETE_DBUSINTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QVariantMap>

class KopeteDBusInterfacePrivate;

/**
 * @brief Public D-Bus interface for Kopete
 * @author Michaël Larouche <larouche@kde.org>
 */
class KopeteDBusInterface : public QObject
{
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "org.kde.Kopete")

public:
	/**
	 * @brief Constructor
	 * @param parent QObject parent
	 */
	KopeteDBusInterface(QObject *parent);
	/**
	 * @brief Destructor
	 */
	~KopeteDBusInterface();

public Q_SLOTS:
	/**
	 * @brief Connect all accounts
	 */
	void connectAll();

	/**
	 * @brief Disconnect all accounts
	 */
	void disconnectAll();

	/**
	 * @brief Saves the online status of all accounts and disconnects them
	 */
	void suspend();

	/**
	 * @brief Sets the online status of all accounts to the status they had when suspend was called
	 */
	void resume();

	/**
	 * @brief Get information if we are connected to a given account in the given protocol
	 * @param protocolName The protocol name (ex: Jabber, Yahoo, Messenger)
	 * @param accountId Account ID
	 * @return if the given protocolName and accountId are valid and we
	 * are connected then true is returned else false is returned.
	 */
	bool isConnected(const QString &protocolName, const QString &accountId);

	/**
	 * @brief Connect a given account in the given protocol
	 * @param protocolName The protocol name (ex: Jabber, Yahoo, Messenger)
	 * @param accountId Account ID
	 */
	void connect(const QString &protocolName, const QString &accountId);

	/**
	 * @brief Disconnect a given account in the given protocol
	 * @param protocolName The protocol name (ex: Jabber, Yahoo, Messenger)
	 * @param accountId Account ID
	 */
	void disconnect(const QString &protocolName, const QString &accountId);

	/**
	* @brief Get a list of all protocol names.
	* @return a list of all protocol names.
	*/
	QStringList protocols() const;

	/**
	 * @brief Get a list of all identities' ID
	 * @return a list of all identities' ID
	 */
	QStringList identities() const;

	/**
	 * @brief Get a list of all account's ID
	 * @return a list of all accounts' ID
	 */
	QStringList accounts() const;

	/**
	 * @brief Return all contacts
	 * @return a list of all contacts display name
	 */
	QStringList contacts() const;

	/**
	 * @brief Get UI labels for identities
     * @return a label for a identity.
	 */
	QString labelForIdentity(const QString & id) const;

	/**
	 * @brief Get a filered list of contacts based on the filter
	 *
	 * Accetable filters are: online, reachable, filecapable.
	 * If no accepted filters are provived, it will return an
	 * empty list.
	 *
	 * @return list of filtered contacts
	 */
	QStringList contactsByFilter(const QString &filter) const;
	
	/**
	 * @brief Set the nickname for the given identity
	 *
	 * To set the nickname of the default identity, use an empty identityId
	 *
	 * @param nickName New nickname of the identity
	 * @param identityId Identity ID to modify. If empty, will use default identity.
	 */
	void setIdentityNickName(const QString &nickName, const QString &identityId = QString());

	/**
	 * @brief Set the avatar for the given identity
	 *
	 * To set the avatar of the default identity, use an empty identityId
	 *
	 * @param avatarUrl Path to the avatar image
	 * @param identityId Identity ID to modify. If empty, will use default identity
	 */
	void setIdentityAvatar(const QString &avatarUrl, const QString &identityId = QString());
	
	/**
	 * @brief Set the online status for the given identity
	 *
	 * To set the online status of the default identity, use an empty identityId
	 *
	 * @param status The english word for the status you want.
	 * @param message Status message, can be an empty string
	 * @param identityId Identity ID to modify. If empty, will use default identity
	 */
	void setIdentityOnlineStatus(const QString &status, const QString &message, const QString &identityId = QString());

	/**
	 * @brief Change the online status for all accounts
	 *
	 * Accepted values for status are: Online, Busy, Away, Available 
	 * @param status The english word for the status you want.
	 * @param message Optional status message
	 */
	void setOnlineStatus(const QString &status, const QString &message = QString());

	/**
	 * @brief Change the status message for all accounts
	 *
 	 * Use an empty string to clear the status message
	 * @param message Status message
	 */
	void setStatusMessage(const QString &message);

	/**
	 * @brief Send a message to the given contact
     * @param contactId Metacontact ID or displayName to send a message to
	 * @param message The message to send
	 */
	void sendMessage(const QString &contactId, const QString &message);
	
    /**
     * Open a chat window for the given contact
     * @param contactId Metacontact ID or displayName
     */
    void openChat(const QString &contactId);

    /**
	 * @brief Adds a contact with the specified params.
	 *
	 * @param protocolName The name of the protocol this contact is for ("Jabber", etc)
	 * @param accountId The account ID to add the contact to
	 * @param contactId The unique ID for this protocol
	 * @param displayName The displayName of the contact (may equal userId for some protocols
	 * @param groupName The name of the group to add the contact to
	 * @return Weather or not the contact was added successfully
	 */
	bool addContact( const QString &protocolName, const QString &accountId, const QString &contactId, const QString &displayName, const QString &groupName = QString() );

	/**
	 * @brief Send a file to the given contact
     * @param contactId Metacontact ID or displayName to send a file to
	 * @param fileUrl Url of the file to send
	 */
	void sendFile( const QString &contactId, const QString &fileUrl );

	/**
	 * @brief Retrieve the Display Name from the given contact ID
	 * @param contactId Metacontact contactId
	 */
	QString getDisplayName(const QString &contactId);

	/**
	 * @brief Get the Online Status of the contact
     * @param contactId Metacontact ID or displayName
	 */
	bool isContactOnline(const QString &contactId);
	
	/**
	 * Look up details for a specific contact
	 * @param contactId Contact ID or display Name
	 * @return A QVariantMap containing contact properties like online status, avatar, ...
	 */
	QVariantMap contactProperties(const QString &contactId);
	
Q_SIGNALS:
    /**
     * Contact properties have changed: displayName, avatar, pending messages...
     * @param contactId ID of the contact whose properties have changed
     */
    void contactChanged(QString contactId);
    
private:
    KopeteDBusInterfacePrivate * const d;    
};

#endif
