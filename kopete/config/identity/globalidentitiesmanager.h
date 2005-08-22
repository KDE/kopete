/*
    globalidentitiesmanager.h  -  Kopete Global identities manager.

    Copyright (c) 2005      by Michaël Larouche       <michael.larouche@kdemail.net>

    Kopete    (c) 2003-2005 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef GLOBALIDENTITIESMANAGER_H
#define GLOBALIDENTITIESMANAGER_H

#include <qobject.h>
#include <qmap.h>

namespace Kopete
{
	class MetaContact;
}
class QDomDocument;

/**
 * This singleton class handle the loading, saving and manipulating of all the global identities from a XML file. 
 * It also hold the pointer list of metacontacts.
 * Use this class with GlobalIdentitiesManager::self()
 *
 * @author Michaël Larouche <michael.larouche@kdemail.net>
*/
class GlobalIdentitiesManager : public QObject
{
    Q_OBJECT
public:
	/**
	 * @brief Return the single instance of GlobalIdentitiesManager class	
	 *
	 * The global identities manager is a singleton class of which only
	 * a single instance will exist. If no manager exists yet, this method
	 * create one for you.
	 *
	 * @return The single instance of GlobalIdentitiesManager class.
	 * FIXME: Should I remove the singleton pattern ?
	 */
	static GlobalIdentitiesManager* self();
	~GlobalIdentitiesManager();

	/**
	 * @brief Create a new identity and add it to the internal list.
	 */	
	void createNewIdentity(const QString &identityName);

	/**
	 * @brief Copy a identity
	 *
	 * @param copyIdentityName Name for the copy identity.
	 * @param sourceIdentity Name of the source identity.
	 */
	void copyIdentity(const QString &copyIdentityName, const QString &sourceIdentity);

	/**
	 * @brief Rename a identity
	 *
	 * @param oldName Identity to rename.
	 * @param newName New identity name.
	 */
	void renameIdentity(const QString &oldName, const QString &newName);

	/**
	 * @brief Delete identity
	 *
	 * @param removedIdentity Identity name to remove.
	 */
	void removeIdentity(const QString &removedIdentity);

	/**
	 * @brief Update the specified identity using the source MetaContact
	 *
	 * @param updatedIdentity Identity to update.
	 * @param sourceMetaContact Source of data.
	 */
	void updateIdentity(const QString &updatedIdentity, Kopete::MetaContact *sourceMetaContact);

	/**
	 * @brief Check if the specified identityName exists.
	 * 
	 * This is a helper method to avoid duplicated entries.
	 * @return if the identityName is in the internal list.
	 */
	bool isIdentityPresent(const QString &identityName);

	/**
	 * @brief Return the specified identity.
	 *
	 * @param identityName Identity to retrive.
	 * @return Identity data as Kopete::MetaContact.
	 */
	Kopete::MetaContact *getIdentity(const QString &identityName);

	/**
	 * @brief Load the XML file where global identities metacontacts are stored.
	 */
	void loadXML();

	/**
	 * @brief Save the global identities metacontacts to XML file.
	 */
	void saveXML();

	/**
	 * @brief Return the list of global identities metacontact.
	 * @return The pointer list of metacontact as QValueList
	 */
	QMap<QString, Kopete::MetaContact*> getGlobalIdentitiesList();

private:
	GlobalIdentitiesManager(QObject *parent = 0, const char *name = 0);

	/**
	 * @brief Return a XML representation of the global identities list.
	 * 
	 * @return the XML represention as QDomDocument.
	 */
	const QDomDocument toXML();

	Kopete::MetaContact *createNewMetaContact();
	Kopete::MetaContact *createCopyMetaContact(Kopete::MetaContact *source);
	void copyMetaContact(Kopete::MetaContact *destination, Kopete::MetaContact *source);

private:
	static GlobalIdentitiesManager *s_self;
	class Private;
	Private *d;

};

#endif
