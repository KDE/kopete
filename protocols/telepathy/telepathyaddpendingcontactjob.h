/*
   telepathyaddpendingcontactjob.h - Telepathy Add Pending Contact Job

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

   *************************************************************************
   *                                                                       *
   * This program is free software; you can redistribute it and/or modify  *
   * it under the terms of the GNU General Public License as published by  *
   * the Free Software Foundation; either version 2 of the License, or     *
   * (at your option) any later version.                                   *
   *                                                                       *
   *************************************************************************
*/
#ifndef TELEPATHY_ADDPENDINGCONTACTJOB_H
#define TELEPATHY_ADDPENDINGCONTACTJOB_H

#include <kjob.h>

namespace QtTapioca
{
	class Contact;
}

class TelepathyAccount;
/**
 * @brief Small job to add a pending Telepathy contact to Kopete contact list.
 * @author Michaël Larouche <larouche@kde.org>
 */
class TelepathyAddPendingContactJob : public KJob
{
	Q_OBJECT
public:
	TelepathyAddPendingContactJob(TelepathyAccount *parent);
	~TelepathyAddPendingContactJob();

	/**
	 * Set the internal pending contact that need to be added to
	 * Kopete contact list.
	 * @param contact Internal QtTapioca contact.
	 */
	void setPendingContact(QtTapioca::Contact *contact);

	/**
	 * Set the job to only ask for authorization.
	 * @param value true or false
	 */
	void setAuthorizeOnly(bool value);

	/**
	 * Start the processus
	 */
	virtual void start();

private slots:
	void slotAddedInfoEventActionActivated( uint actionId );
	void slotAddedInfoEventClosed();

private:
	class Private;
	Private *d;
};

#endif
