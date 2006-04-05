/*
   contactlistmanager.h - Windows Live Messenger Contact List

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
#ifndef PAPILLONCONTACTLISTMANAGER_H
#define PAPILLONCONTACTLISTMANAGER_H

#include <QtCore/QObject>
#include <papillon_macros.h>

class QDomDocument;

namespace Papillon 
{

/**
 * @brief Manage contact list and create contact list operations tasks.
 *
 * @author Michaël Larouche <michael.larouche@kdemail.net>
*/
class PAPILLON_EXPORT ContactListManager : public QObject
{
	Q_OBJECT
public:
	/**
	 * @brief Create a new ContactListManager.
	 */
	ContactListManager(QObject *parent = 0);
	/**
	 * d-tor
	 */
	~ContactListManager();

	/**
	 * @brief Restore Contact List from a XML cache file.
	 * @param contatListCache Contact List cache as QDomDocument.
	 */
	void fromXml(const QDomDocument &contactListCache);

	/**
	 * @brief XML represention of the contact list ready to be saved on disk.
	 * Use same format received from Contacts MSN web service.
	 * @return XML represention of the contact list.
	 */
	QDomDocument toXml();

	/**
	 * @brief Create the payload data for ADL command.
	 * Generate in the string the whole contact list(just contacts) in XML
	 * format ready from ADL command.
	 * @return the ADL payload data for the current contact list.
	 */
	QString createAddListPayload();

private:
	class Private;
	Private *d;
};

}

#endif
