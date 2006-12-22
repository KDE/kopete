/*
   contactlist.h - Windows Live Messenger Contact List

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
#ifndef PAPILLONCONTACTLIST_H
#define PAPILLONCONTACTLIST_H

#include <QtCore/QObject>
#include <Papillon/Macros>

class QDomDocument;

namespace Papillon 
{

class Client;
/**
 * @brief Manage contact list.
 *
 * @author Michaël Larouche <larouche@kde.org>
*/
class PAPILLON_EXPORT ContactList : public QObject
{
	Q_OBJECT
public:
	/**
	 * @brief Create a new ContactList.
	 */
	ContactList(Client *client);
	/**
	 * d-tor
	 */
	~ContactList();

private:
	/**
	 * Get the current instance of Client.
	 * @return the current Client pointer.
	 */
	Client *client();

private:
	class Private;
	Private *d;
};

}

#endif
