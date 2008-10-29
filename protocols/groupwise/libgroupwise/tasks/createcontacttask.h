/*
    Kopete Groupwise Protocol
    createcontacttask.cpp - high level task responsible for creating both a contact and any folders it belongs to locally, on the server

    Copyright (c) 2004      SUSE Linux AG	 	 http://www.suse.com
    
    Based on Iris, Copyright (C) 2003  Justin Karneges <justin@affinix.com>

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>
 
    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef CREATECONTACTTASK_H
#define CREATECONTACTTASK_H

#include <QList>

#include "gwerror.h"
#include "libgroupwise_export.h"

#include "task.h"

using namespace GroupWise;

/**
	Creates a contact on the server, as well as any folders that do not exist on the server, and add the contact to those folders.
	This is a meta-task to suit Kopete.  If you maintain your own copy of the server side contact list and follow the server's 
	contact semantics (contact instances rather than contacts in the contact list), you can just use CreateContactInstanceTask.
	This task causes the @ref Client to emit folderReceived() and contactReceived() as the task proceeds.  Kopete processes these 
	signals as usual, because it created the contact optimistically, before invoking this task.

	The finished() signal indicates the whole procedure has completed and the sender can be queried for success as usual
@author SUSE AG
*/
class LIBGROUPWISE_EXPORT CreateContactTask : public Task
{
Q_OBJECT
public:
	CreateContactTask(Task* parent);
	~CreateContactTask();
	/**
	 * Get the userId of the contact just created
	 */
	QString userId();
	/**
	 * Get the DN of the contact just created
	 */
	QString dn();
	QString displayName();

	/** 
	 * Sets up the task.
	 * @param userId the user Id of the contact to create
	 * @param displayName the display name we should give to this contact
	 * @param firstSeqNo Used to create the folders - the first unused folder sequence number we know of
	 * @param folders A list of folders that the contact should belong to - any folders that do not exist on the server should have a objectId of 0, and will be created
	 * @param topLevel is the folder also in the top level folder?
	 */
	void contactFromUserId( const QString & userId, const QString & displayName, const int firstSeqNo, const QList< FolderItem > folders, bool topLevel );
	//void contactFromDN( const QString & dn, const QString & displayName, const int parentFolder );
	/** 
	 * This task doesn't do any I/O itself, so this take prints an error and returns false;
	 */
	bool take( Transfer * );
	/** 
	 * Starts off the whole process
	 */
	void onGo();
protected slots:
	void slotContactAdded( const ContactItem & );
	void slotCheckContactInstanceCreated();
private:
	int m_firstSequenceNumber;
	QString m_userId;
	QString m_dn;
	QString m_displayName;
	QList< FolderItem > m_folders;
	bool m_topLevel;
};

#endif
