// -*- Mode: c++-mode; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
//
// Copyright (C) 	2004	 Grzegorz Jaskiewicz <gj at pointblue.com.pl>
//
// gaducontactlist.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//

#ifndef GADUCONTACTLIST_H
#define GADUCONTACTLIST_H

#include <qvaluelist.h>

class QString;

class GaduContactsList
{
public:
	struct ContactLine {

		QString displayname;
		QString group;
		QString uin;
		QString firstname;
		QString surname;
		QString nickname;
		QString phonenr;
		QString email;
		bool 	ignored;
		bool 	offlineTo;
		QString landline;
	};

	GaduContactsList();
	GaduContactsList( QString );
	~GaduContactsList();
	QString asString();
	void addContact( ContactLine &cl );
	void addContact(	QString& displayname, QString& group,
				QString& uin, QString& firstname,
				QString& surname, QString& nickname,
				QString& phonenr, QString& email,
				bool 	ignored, bool offlineTo,
				QString& landline
	);
	unsigned int size();
	const GaduContactsList::ContactLine& operator[]( unsigned int i );
private:
	typedef QValueList<ContactLine> CList;
	CList cList;
	CList::iterator	it;
};
#endif
