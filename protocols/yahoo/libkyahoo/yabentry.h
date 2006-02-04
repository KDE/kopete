/*
    yabentry.h - Encapsulate Yahoo Adressbook information

    Copyright (c) 2005 André Duffeck <andre.duffeck@kdemail.net>
    Kopete (c) 2002-2006 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/
#ifndef YABEntry_H
#define YABEntry_H

#include <kdebug.h>
#include <qdatetime.h>

struct YABEntry
{
	// Personal
	QString		firstName;
	QString		secondName;
	QString		lastName;
	QString		nickName;
	QString		title;

	// Primary Information	
	QString		phoneMobile;
	QString		email;
	QString		yahooId;

	// Additional Information
	QString		pager;
	QString		fax;
	QString		additionalNumber;
	QString		altEmail1;
	QString		altEmail2;

	// Private Information
	QString		privateAdress;
	QString		privateCity;
	QString		privateState;
	QString		privateZIP;
	QString		privateCountry;
	QString		privatePhone;
	QString		privateURL;
		
	// Work Information
	QString		corporation;
	QString		workAdress;
	QString		workCity;
	QString		workState;
	QString		workZIP;
	QString		workCountry;
	QString		workPhone;
	QString		workURL;

	// Miscellanous
	QDate		birthday;
	QDate		anniversary;
	QString		notes;
	QString		additional1;
	QString		additional2;
	QString		additional3;
	QString		additional4;

	void dump()	{
	kdDebug() << "firstName: " << firstName << endl << 
			"secondName: " << secondName << endl << 
			"lastName: " << lastName << endl << 
			"nickName: " << nickName << endl << 
			"title: " << title << endl << 
			"phoneMobile: " << phoneMobile << endl << 
			"email: " << email << endl << 
			"yahooId: " << yahooId << endl << 
			"pager: " << pager << endl << 
			"fax: " << fax << endl << 
			"additionalNumber: " << additionalNumber << endl << 
			"altEmail1: " << altEmail1 << endl << 
			"altEmail2: " << altEmail2 << endl << 
			"privateAdress: " << privateAdress << endl << 
			"privateCity: " << privateCity << endl << 
			"privateState: " << privateState << endl << 
			"privateZIP: " << privateZIP << endl << 
			"privateCountry: " << privateCountry << endl << 
			"privatePhone: " << privatePhone << endl << 
			"privateURL: " << privateURL << endl << 
			"corporation: " << corporation << endl << 
			"workAdress: " << workAdress << endl << 
			"workCity: " << workCity << endl << 
			"workState: " << workState << endl << 
			"workZIP: " << workZIP << endl << 
			"workCountry: " << workCountry << endl << 
			"workURL: " << workURL << endl << 
			"birthday: " << birthday.toString() << endl << 
			"anniversary: " << anniversary.toString() << endl << 
			"notes: " << notes << endl << 
			"additional1: " << additional1 << endl << 
			"additional2: " << additional2 << endl << 
			"additional3: " << additional3 << endl << 
			"additional4: " << additional4 << endl;
	}
};

#endif
