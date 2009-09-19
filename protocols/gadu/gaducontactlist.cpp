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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.
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

	QStringList ln  = sList.split( QChar( '\n' ), QString::KeepEmptyParts );
	QStringList::iterator lni = ln.begin( );

	while( lni != ln.end() ) {

		QString cline = (*lni);
		if ( cline.isNull() ) {
			break;
		}

		strList  = cline.split( QChar( ';' ), QString::KeepEmptyParts );

		stringIterator = strList.begin();

		if ((*stringIterator) == "GG70ExportString,") {
			// since it doesn't matter for neither kopete nor ggserver
			// its just list of current groups but may spoil contact list
			kDebug( 14100 ) << "Ignoring GG70ExportSting contact list member";
			++lni;
			continue;
		} else if ( strList.size() < 7 ) {
			kDebug( 14100 ) << "Malformed entry, too short! Ignoring entry: " << strList;
			++lni;
			continue;
		}


		if ( strList.count() >= 12 ) {
			email = true;
		}
		else {
			email = false;
		}


//each line ((firstname);(secondname);(nickname);(altnick);(tel);(group);(uin);
// new stuff attached at the end:
// email;aliveSoundfile;notifyType;msgSoundType;messageSound;offlineTo;homePhone
		stringIterator = strList.begin();

		cl.firstname		= (*stringIterator);

		if ( cl.firstname == QString( 'i' ) ) {
			kDebug(14100) << cline << " ignored";
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

			if ( strList.count() >= 13 )
				cl.offlineTo = (*++stringIterator) == QString('0') ? false : true;
			if ( strList.count() >= 14 )
				cl.landline  = (*++stringIterator);	
		}
		else {
			 cl.email	= empty;
		}

		++lni;

		if(cl.uin.isEmpty()) {
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
			contacts += "i;;;;;;" + (*it).uin + "\r\n";
		}
		else {
//	name;surname;nick;displayname;telephone;group(s);uin;email;0;;0;;offlineTo;homePhone
			contacts +=
				(*it).firstname + ';'+
				(*it).surname + ';'+
				(*it).nickname + ';'+
				(*it).displayname + ';'+
				(*it).phonenr + ';'+
				(*it).group + ';'+
				(*it).uin + ';'+
				(*it).email +
				";0;;0;;" +
				((*it).offlineTo == true ? '1' : '0')
				+ ';' +
				(*it).landline +
				"\r\n";
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
