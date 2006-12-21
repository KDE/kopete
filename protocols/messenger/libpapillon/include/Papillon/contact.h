/*
   contact.h - Information for a Windows Live Messenger contact.

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
#ifndef PAPILLONCONTACT_H
#define PAPILLONCONTACT_H

#include <Papillon/Macros>
#include <Papillon/Enums>

class QStringList;
class QDomElement;

namespace Papillon
{

/**
 * @brief A Contact and all its information
 * Hold client feature, presence for a single contact.
 * This is managed by ContactListManager.
 *
 */
class PAPILLON_EXPORT Contact
{
public:
	/**
	 * @brief Create an empty Contact.
	 */
	Contact();
	/**
	 * d-tor
	 */
	~Contact();

	/**
	 * @brief Is this Contact valid ?
	 * @return true if the Contact is valid
	 */
	bool isValid() const;

	/**
	 * @brief Get contact ID.
	 * Contact ID is a unique GUID used to identity the contact.
	 * @return Contact ID as a GUID.
	 */
	QString contactId() const;
	/**
	 * @brief Set Contact ID (this is a GUID)
	 * @param contactId Contact GUID.
	 */
	void setContactId(const QString &contactId);

	/**
	 * @brief Get Passport ID for this contact.
	 * Passport ID is in this form: example@passport.com
	 * @return Passport ID
	 */
	QString passportId() const;
	/**
	 * @brief Set Passport ID for this contact
	 * @param passportId Passport ID
	 */
	void setPassportId(const QString &passportId);

	/**
	 * @brief Get client features for this contact.
	 */
	ClientInfo::Features clientFeatures() const;
	/**
	 * @brief Set clients features for this contact
	 * @param features New client features
	 */
	void setClientFeatures(const ClientInfo::Features &features);

//BEGIN AddressBook data
	/**
	 * @brief Get display name for this contact.
	 * @return Display name for this contact.
	 */
	QString displayName() const;
	/**
	 * @brief Set display name for this contact.
	 * @param displayName Display name to set 
	 */
	void setDisplayName(const QString &displayName);
//END AddressBook data

private:
	/**
	 * @internal
	 * Disable copy constructor
	 */
	Contact(const Contact &copy);
	/**
	 * @internal
	 * Disable copy-assignment operator
	 */
	Contact &operator=(const Contact &other);

	class Private;
	Private *d;
};

}

#endif
