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


#include "gaducontactlist.h"
#include "qstringlist.h"
#include "kdebug.h"

GaduContactsList::GaduContactsList()
{
}

GaduContactsList::~GaduContactsList()
{
}

GaduContactsList::GaduContactsList( QString sList )
{
	QStringList::iterator stringIterator;
	QStringList strList;
	QString empty;
	ContactLine cl;
	bool email;

	if ( sList.isEmpty() || sList.isNull() ) {
		return;
	}

	if ( ( !sList.contains( '\n' ) && sList.contains( ';' ) )  || !sList.contains( ';' ) ) {
		return;
	}

	QStringList ln  = QStringList::split( QChar( '\n' ),  sList, true );
	QStringList::iterator lni = ln.begin( );

	while( lni != ln.end() ) {

		QString cline = (*lni);
		if ( cline.isNull() ) {
			break;
		}

		strList  = QStringList::split( QChar( ';' ), cline, true );

		stringIterator = strList.begin();
		
		if ( strList.count() >= 12 ) {
			email = true;
		}
		else {
			email = false;
		}


//each line ((firstname);(secondname);(nickname);(altnick);(tel);(group);(uin);
// new stuff attached at the end:
// email;aliveSoundfile;notifyType;msgSoundType;messageSound;offlineTo;homePhone;
		stringIterator = strList.begin();

		cl.firstname		= (*stringIterator);

		if ( cl.firstname == QString( "i" ) ) {
			kdDebug(14100) << cline << " ignored" << endl;
			cl.ignored	= true;
			cl.uin		= strList[6];
			++lni;
			cList.append( cl );
			continue;
		}
		else {
			cl.ignored = false;
		}

		cl.surname		= (*++stringIterator);
		cl.nickname		= (*++stringIterator);
		cl.displayname		= (*++stringIterator);
		cl.phonenr		= (*++stringIterator);
		cl.group		= (*++stringIterator);
		cl.uin			= (*++stringIterator);
		if ( email ) {
			cl.email	= (*++stringIterator);
			// no use for custom sounds, at least now
			++stringIterator;
			++stringIterator;
			++stringIterator;
			++stringIterator;

			if ( stringIterator != strList.end() ) {
				cl.offlineTo = (*++stringIterator) == QString("0") ? false : true;
				cl.landline  = (*++stringIterator);
			}
        	}
		else {
			 cl.email	= empty;
		}

		++lni;

		if ( cl.uin.isNull() ) {
			continue;
		}

		cList.append( cl );
	}

	return;
}

void
GaduContactsList::addContact( ContactLine& cl )
{
	cList.append( cl );
}

void
GaduContactsList::addContact(
		QString& displayname,
		QString& group,
		QString& uin,
		QString& firstname,
		QString& surname,
		QString& nickname,
		QString& phonenr,
		QString& email,
		bool ignored,
		bool offlineTo,
		QString& landline
)
{
	ContactLine cl;

	cl.displayname	= displayname;
	cl.group	= group;
	cl.uin		= uin;
	cl.firstname	= firstname;
	cl.surname	= surname;
	cl.nickname	= nickname;
	cl.phonenr	= phonenr;
	cl.email	= email;
	cl.ignored	= ignored;
	cl.offlineTo	= offlineTo;
	cl.landline	= landline;

	cList.append( cl );

}

QString
GaduContactsList::asString()
{
	QString contacts;

	for (  it = cList.begin(); it != cList.end(); ++it ) {
		if ( (*it).ignored ) {
			contacts += "i;;;;;;" + (*it).uin + "\n";
		}
		else {
//	name;surname;nick;displayname;telephone;group(s);uin;email;;0;0;;offlineTo;homePhone;
			contacts +=
				(*it).firstname + ";"+
				(*it).surname + ";"+
				(*it).nickname + ";"+
				(*it).displayname + ";"+
				(*it).phonenr + ";"+
				(*it).group + ";"+
				(*it).uin + ";"+
				(*it).email +
				";;0;0;;" +
				((*it).offlineTo == true ? QString("1") : QString("0"))
				+ ";" +
				(*it).landline +
				";\r\n";
		}
	}
	return contacts;
}

unsigned int
GaduContactsList::size()
{
	return cList.size();
}

const GaduContactsList::ContactLine&
GaduContactsList::operator[]( unsigned int i )
{
	return cList[i];
}
