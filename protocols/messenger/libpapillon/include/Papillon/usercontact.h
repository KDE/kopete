/*
   usercontact.h - Windows Live Messenger user contact

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#ifndef PAPILLONUSERCONTACT_H
#define PAPILLONUSERCONTACT_H

#include <Papillon/Contact> // Include Macros and Enums too

namespace Papillon
{

class Client;
class StatusMessage;
/**
 * @class UserContact usercontact.h <Papillon/UserContact>
 * @brief Manage the user contact information.
 *
 * UserContact represent the self contact. It is used to set the
 * login information, change presence, change nickname, change avatar,
 * change status message and personnal information.
 *
 * To connect to Windows Live Messenger service, you need to set
 * login information using setLoginInformation(). This is required to be able
 * to succesfully login. For more information, see Client.
 *
 * @author Michaël Larouche <larouche@kde.org>
 */
class PAPILLON_EXPORT UserContact : public Contact
{
	Q_OBJECT
public:
	/**
	 * @brief Create the UserContact.
	 * You must pass the instance of the Client.
	 * @param client Instance of the client
	 */
	UserContact(Client *client);
	/**
	 * Destructor
	 */
	~UserContact();

	/**
	 * @brief Set login information for the user contact
	 *
	 * This is used during login process to store login information.
	 * @param passportId Microsoft Passport identifier (ex: test@passport.com)
	 * @param password Password (TODO: Use QCA::SecureArray ?)
	 */
	void setLoginInformation(const QString &passportId, const QString &password);

	/**
	 * @brief Get the password of the user contact.
	 * @return password of the user contact.
	 */
	QString password() const;

	/**
	 * @brief Set the authentification cookie obtained during login process.
	 * @param cookie Authentification cookie
	 */
	void setLoginCookie(const QString &cookie);
	/**
	 * @brief Get the login cookie
	 * @return Authentification cookie
	 */
	QString loginCookie() const;

public slots:
	/**
	 * @brief Change online presence
	 * @param newPresence New presence
	 */
	void setPresence(Papillon::Presence::Status newPresence);

	/**
	 * @brief Set a new personal status message.
	 * @param statusMessage New personal status message
	 */
	void setPersonalStatusMessage(const Papillon::StatusMessage &statusMessage);
	
	/**
	 * @brief Set the personal information to be updated on server.
	 * @param type The type of the personal information it need to update on server.
	 * @param value New value for the given personal information. Set an empty string to reset the value.
	 */
	void setPersonalInformation(Papillon::ClientInfo::PersonalInformation type, const QString &value);
	
private:
	/**
	 * @brief Get current Client instance.
	 * @return Client instance.
	 */
	Client *client();

private:
	class Private;
	Private *d;
};

}

#endif
