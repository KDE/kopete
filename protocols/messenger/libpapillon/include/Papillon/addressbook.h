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
#ifndef PAPILLONADDRESSBOOK_H
#define PAPILLONADDRESSBOOK_H
namespace Papillon
{
class PAPILLON_EXPORT AddressBook : public QObject
{
	Q_OBJECT
public:
	AddressBook(Client *client);
	~AddressBook();

	Client *client();

	void load();
private:
	class Private;
	Private *d;
}
#endif
