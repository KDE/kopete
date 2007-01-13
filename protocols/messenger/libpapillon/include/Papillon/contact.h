/*
   contact.h - Information for a Windows Live Messenger contact.

   Copyright (c) 2006-2007 by Michaël Larouche <larouche@kde.org>

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

#include <QtCore/QObject>

class QStringList;
class QDomElement;

namespace Papillon
{

/**
 * @class Contact contact.h <Papillon/Contact>
 * @brief A Contact and all its information
 * Hold client feature, presence for a single contact.
 * This is managed by ContactList.
 * @author Michaël Larouche <larouche@kde.org>
 */
class PAPILLON_EXPORT Contact : public QObject
{
	Q_OBJECT
public:
	/**
	 * @brief Create an empty Contact.
	 */
	Contact(QObject *parent = 0);
	/**
	 * d-tor
	 */
	~Contact();

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

	/**
	 * @brief Get the lists on which the contact is subscribed.
	 * @return The list flags.
	 */
	Papillon::ContactListEnums::ListFlags lists() const;

	/**
	 * @brief Add the contact to the given list.
	 *
	 * You can pass multiple list flag to this method.
	 * @param list Lists on which the contact will be added.
	 */
	void addToList(const Papillon::ContactListEnums::ListFlags &list);
	
	/**
	 * @brief Remove the contact from the given list.
	 *
	 * You can pass multiple list flag to this method.
	 * @param list Lists on which the contact will be removed.
	 */
	void removeFromList(const Papillon::ContactListEnums::ListFlags &list);

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
