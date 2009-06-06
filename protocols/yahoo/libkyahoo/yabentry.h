/*
    yabentry.h - Encapsulate Yahoo Adressbook information

    Copyright (c) 2005 André Duffeck <duffeck@kde.org>
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
#include <qdom.h>

struct YABEntry
{
	enum Source { SourceYAB, SourceContact };

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
	int		YABId;
	Source		source;

	// Additional Information
	QString		pager;
	QString		fax;
	QString		additionalNumber;
	QString		altEmail1;
	QString		altEmail2;
	QString		imAIM;
	QString		imICQ;
	QString		imMSN;
	QString		imGoogleTalk;
	QString		imSkype;
	QString		imIRC;
	QString		imQQ;

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

	// Miscellaneous
	QDate		birthday;
	QDate		anniversary;
	QString		notes;
	QString		additional1;
	QString		additional2;
	QString		additional3;
	QString		additional4;

	
	void fromQDomElement( const QDomElement &e );
	void fromQDomDocument( const QDomDocument &e );
	void fillQDomElement( QDomElement &e ) const;

	void dump() const;
};

#endif
