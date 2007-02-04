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
 * Sub-directory Contact store avatar received from contacts.
 * Avatar are managed by protocol to avoid conflit of avatar between
 * multiple contact with the same ID.
 * For example, an user can have the same ID for MSN and Jabber but
 * use a different avatar for each account.
 *
 * @section avatar_management Avatar management
 *
 * @subsection avatar_management_list Listing avatar
 * If you want to list only an avatar category,
 * pass a category to query() method.
 * @code
QList<Kopete::AvatarManager::AvatarEntry> avatarList = Kopete::AvatarManager::self()->query();
foreach(Kopete::AvatarManager::AvatarEntry entry, avatarList)
{
  QString example = QString("Name: %1, Path: %2, Category: %3").arg(entry.name).arg(entry.path).arg(entry.category);
  qDebug() << "Listing avatar" << example;
}
 * @endcode
 *
 * @subsection avatar_management_add Adding an avatar
 * Use the add() method to add an new avatar. For an avatar received from a contact, you
 * should use the contactId for the name. You can either specify a image path or a QImage.
 * AvatarManager will save the QImage on disk.
 * @code
Kopete::AvatarManager::AvatarEntry newEntry;
newEntry.name = "A new avatar"
newEntry.path = avatarPath;
newEntry.category = Kopete::AvatarManager::User;

Kopete::AvatarManager::self()->add(newEntry);
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
 * @author Michaël Larouche <larouche@kde.org>
 */
class KOPETE_EXPORT AvatarManager : public QObject
{
	Q_OBJECT
public:
	/**
	 * Available avatar category.
	 */
	enum AvatarCategory
	{
		User, ///< User defined avatar
		Contact, ///< Avatar from a contact.
		All ///< Only used to list all category.
	};

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

	/**
	 * @brief Query the avatar storage
	 * @param category Pass a AvatarCategory value if you want to filter. List all category by default.
	 * @return List of AvatarEntry
	 */
	QList<Kopete::AvatarManager::AvatarEntry> query(Kopete::AvatarManager::AvatarCategory category = All);

public Q_SLOTS:
	/**
	 * @brief Add an new avatar to the storage
	 * @param newEntry New avatar entry
	 */
	void add(Kopete::AvatarManager::AvatarEntry newEntry);
	/**
	 * @brief Remove an avatar from the storage
	 * @param entryToRemove Avatar entry to remove
	 */
	void remove(Kopete::AvatarManager::AvatarEntry entryToRemove);

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
	Private *d;
};

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
	Private *d;
};

}

#endif
