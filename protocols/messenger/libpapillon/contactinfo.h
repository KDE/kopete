/*
   contactinfo.h - Information for a Windows Live Messenger contact.

   Copyright (c) 2006 by Michaël Larouche <michael.larouche@kdemail.net>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#ifndef PAPILLONCONTACTINFO_H
#define PAPILLONCONTACTINFO_H

#include <QtCore/QSharedDataPointer>
#include <papillon_macros.h>
#include <papillon_enums.h>

class QStringList;
class QDomElement;

namespace Papillon
{

/**
 * @brief A Contact and all its informations
 * Hold address book details, client feature, presence for a single contact.
 * This is managed by ContactListManager.
 *
 * This class is implicit shared.
 */
class PAPILLON_EXPORT ContactInfo
{
public:
	/**
	 * @brief Create an empty ContactInfo.
	 */
	ContactInfo();
	/**
	 * d-tor
	 */
	~ContactInfo();
	/**
	 * @brief Copy constructor
	 * Set a reference to the private data, do not create a deep copy.
	 * @param copy ContactInfo to "copy"
	 */
	ContactInfo(const ContactInfo &copy);
	/**
	 * @brief Copy-assignment operator
	 * Set a rerefrence to the private data, do not create a deep copy.
	 * @param other 
	 */
	ContactInfo &operator=(const ContactInfo &other);

	/**
	 * @brief Is this ContactInfo valid ?
	 * @return true if the ContactInfo is valid
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
	 * @brief Set clients feautres for this contact
	 */
	void setClientFeatures(const ClientInfo::Features &featuers);

	/**
	 * @brief Get the lists which the contact is member of.
	 * A contact can be in multiple lists.
	 * @return Contact List flags value.
	 *
	 * @see Papillon::ContactListType
	 */
	ContactList::ContactListFlags lists() const;
	/**
	 * @brief Replace contact list flags with the given flags.
	 * @param flags Flags to replace the current values.
	 * @see Papillon::ContactListType
	 */
	void setContactListFlags(const ContactList::ContactListFlags &flags);
	/**
	 * @brief Add given flags to the contact list flags.
	 * @see Papillon::ContactListType
	 */
	void addContactListFlags(const ContactList::ContactListFlags &flags);
	/**
	 * @brief Remove given flags from contact list flags.
	 * @see Papillon::ContactListType
	 */
	void removeContactListFlags(const ContactList::ContactListFlags &flags);
	
	/**
	 * @brief Get groups which the contacts is associated.
	 * @return the list of groups GUID.
	 */
	QStringList groups() const;
	/**
	 * @brief Set groups for this contact.
	 * @param groups Group GUID list.
	 */
	void setGroups(const QStringList &groups);

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

	/**
	 * @brief Get a XML representation of a contact.
	 * Used to save a cache of whole contactlist.
	 * @return XML representation of the contact.
	 */
	QDomElement toXml() const;
	/**
	 * @brief Set ContactInfo's value from a XML element.
	 * XML is retrieved from MSN Address book service.
	 * @param xml XML element with tagname "Contact"
	 */
	void fromXml(const QDomElement &xml);

private:
	class Private;
	QSharedDataPointer<Private> d;
};

}

#endif
