/*
    kopeteavatarmanager.h - Global avatar manager

    Copyright (c) 2007      by Michaël Larouche      <larouche@kde.org>

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
#ifndef KOPETE_AVATARMANAGER_H
#define KOPETE_AVATARMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtGui/QImage>
#include <kjob.h>
#include "kopete_export.h"

class QImage;

namespace Kopete
{

class Contact;
/**
 * @brief Manage the avatar storage
 *
 * AvatarManager is a class that manage avatar storage.
 * It can manage the user defined avatar and avatar received
 * from contacts.
 *
 * It can also list the available avatars and filter it
 * on the categery, like User or Contact.
 *
 * @section avatar_dirlayout Avatar Directory Layout
 * AvatarManager use Kopete user application directory
 * to actually store avatars on disk.
 *
 * The directory is in $KDEHOME/share/apps/kopete/avatars/
 *
 * Sub-directory User store user defined avatars.
 * It keep track of each avatar change so you can go back to a
 * previous avatar you had.
 *
 * Sub-directory Contacts store avatar received from contacts.
 * Avatars are managed by account to avoid conflict of avatars between
 * multiple contacts having the same ID.
 * For example, an user can have the same ID for MSN and Jabber but
 * use a different avatar for each account.
 *
 * @section avatar_management Avatar management
 *
 * @subsection avatar_management_list Listing avatar
 * If you want to list only an avatar category,
 * use Kopete::AvatarQueryJob
 *
 * @subsection avatar_management_add Adding an avatar
 * Use the add() method to add an new avatar. For an avatar received from a contact, you
 * should use the contactId for the name. You can either specify a image path or a QImage.
 * AvatarManager will save the QImage on disk. When adding an avatar for Contact category,
 * the "contact" entry need to be filled with a pointer to a Kopete::Contact instance.
 *
 * @code
Kopete::AvatarManager::AvatarEntry newEntry;
newEntry.name = "A new avatar"
newEntry.image = avatarImage;
newEntry.contact = this;
newEntry.category = Kopete::AvatarManager::Contact;

Kopete::AvatarManager::self()->add(newEntry);
 * @endcode
 *
 * If the operation has failed, the resulting AvatarEntry path will be empty.
 * The following code is a good way to test the success of add() method:
 * @code
if( !resultEntry.path.isEmpty() )
{
	// Set avatar on server
}
 * @endcode
 *
 * @subsection avatar_management_delete Removing an avatar
 * To remove an avatar, create a new AvatarEntry struct and pass the avatar path 
 * to the struct, it will be used to identify which avatar to remove.
 * 
 * Then just use the remove() method.
 * @code
Kopete::AvatarManager::AvatarEntry entryToRemove;
entryToRemove.path = imagePath;

Kopete::AvatarManager::self()->remove(entryToRemove);
 * @endcode
 *
 * @subsection avatar_management_update Updating an avatar
 * Adding an avatar with the same name will update the previous avatar.
 *
 * @author Michaël Larouche <larouche@kde.org>
 */
class KOPETE_EXPORT AvatarManager : public QObject
{
	Q_OBJECT
public:
	/**
	 * Available avatar category.
	 */
	enum AvatarCategoryType
	{
		User=1, ///< User defined avatar
		Contact, ///< Avatar from a contact.
		All = User | Contact ///< Only used to list all category.
	};
	Q_DECLARE_FLAGS(AvatarCategory, AvatarCategoryType)

	/**
	 * @brief A single entry in AvatarManager.
	 *
	 * @author Michaël Larouche <larouche@kde.org>
	 * @sa Kopete::AvatarManager
	*/
	typedef struct AvatarEntry
	{
		QString name; ///< name is a friendly name to identity the avatar
		QString path; ///< path is the full path to the image on disk
		QImage image; ///< image is used when adding a new avatar, AvatarManager will write the image on disk.
		QByteArray data; ///< original data used to construct the image
		QString dataPath; ///< path is the full path to the data on disk
		Kopete::Contact *contact; ///< contact is used when adding a new contact avatar. AvatarManager use it to create the final url.
		AvatarManager::AvatarCategory category; ///< category in which the avatar belong
	} AvatarEntry;

public:
	/**
	 * @internal
	 * Destructor
	 */
	~AvatarManager();

	/**
	 * Get the only instance of AvatarManager
	 * @return AvatarManager single instance
	 */
	static AvatarManager *self();

Q_SIGNALS:
	/**
	 * @brief An avatar was added into the storage
	 *
	 * Listen to this signal if you want to get notified of
	 * new avatar added to the storage. It is used to update
	 * the AvatarSelectorWidget lists.
	 *
	 * @param newEntry new avatar added into the storage.
	 */
	void avatarAdded(Kopete::AvatarManager::AvatarEntry newEntry);

	/**
	 * @brief An avatar was removed from storage
	 *
	 * Listen to this signal if you want to get notified of
	 * avatar being removed from storage. It is used to update
	 * the AvatarSelectorWidget lists.
	 *
	 * @param entryRemoved avatar being remove from storage.
	 */
	void avatarRemoved(Kopete::AvatarManager::AvatarEntry entryRemoved);

public Q_SLOTS:
	/**
	 * @brief Add an new avatar to the storage
	 *
	 * No need to scale the image, add() will do it for you.
	 *
	 * @param newEntry New avatar entry
	 * @return a new AvatarEntry struct. If the adding failed, the path is null.
	 */
	Kopete::AvatarManager::AvatarEntry add(Kopete::AvatarManager::AvatarEntry newEntry);

	/**
	 * @brief Remove an avatar from the storage
	 * @param entryToRemove Avatar entry to remove
	 */
	bool remove(Kopete::AvatarManager::AvatarEntry entryToRemove);

	/**
	 * @brief Check if an avatar exists
	 * @param entryToCheck Avatar entry to check
	 */
	bool exists(Kopete::AvatarManager::AvatarEntry avatarToCheck);

	/**
	 * @brief Check if an avatar exists by his name
	 * @param avatarName Avatar entry to check
	 */
	bool exists(const QString &avatarName);

private:
	/**
	 * @internal
	 * Constructor is private because the class is a singleton
	 * @param parent QObject parent, not really used
	 */
	AvatarManager(QObject *parent = 0);

private:
	static AvatarManager *s_self;

	class Private;
	Private * const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Kopete::AvatarManager::AvatarCategory)

/**
 * @brief Job to query avatar on disk.
 *
 * First create the job:
 * @code
Kopete::AvatarQueryJob *queryJob = new Kopete::AvatarQueryJob(parent);
queryJob->setQueryFilter(Kopete::AvatarManager::User);
connect(queryJob, SIGNAL(result(KJob*)), this, SLOT(queryResult(KJob*)));
queryJob->start();
 * @endcode
 *
 * Then get the avatar list in the resulting slot:
 * @code
void SwallowAndCoconut::queryResult(KJob* job)
{
	Kopete::AvatarQueryJob *queryJob = static_cast<Kopete::AvatarQueryJob*>(job);
	if(queryJob && !queryJob->error())
	{
		QList<Kopete::AvatarManager::AvatarManager> list = queryJob->avatarList();
		// Iterate over list
	}
}
 * @endcode
 *
 * @author Michaël Larouche <larouche@kde.org>
 * @sa Kopete::AvatarManager
 */
// TODO: Use new Kopete::Task or KCompositeJob
class KOPETE_EXPORT AvatarQueryJob : public KJob
{
	Q_OBJECT
public:
	AvatarQueryJob(QObject *parent = 0);
	~AvatarQueryJob();

	/**
	 * @brief Set the filter for the avatar job.
	 *
	 * This is used to only list the user defined avatars or contact avatar.
	 */
	void setQueryFilter(Kopete::AvatarManager::AvatarCategory category);

	/**
	 * @copydoc KJob::start()
	 */
	virtual void start();

	/**
	 * @brief Get the avatar list based on the query
	 * @return List of AvatarManager::AvatarEntry
	 */
	QList<Kopete::AvatarManager::AvatarEntry> avatarList() const;

private:
	class Private;
	Private * const d;
};

}

#endif
