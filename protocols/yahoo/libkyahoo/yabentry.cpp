/*
    yabcpp - Encapsulate Yahoo Adressbook information

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

#include "yabentry.h"

void YABEntry::fromQDomElement( const QDomElement &e )
{
	yahooId = e.attribute(QStringLiteral("yi"));
	YABId = e.attribute(QStringLiteral("id"), QStringLiteral("-1")).toInt();
	firstName = e.attribute(QStringLiteral("fn"));
	secondName = e.attribute(QStringLiteral("mn"));
	lastName = e.attribute(QStringLiteral("ln"));
	nickName = e.attribute(QStringLiteral("nn"));
	email = e.attribute(QStringLiteral("e0"));
	privatePhone = e.attribute(QStringLiteral("hp"));
	workPhone = e.attribute(QStringLiteral("wp"));
	pager = e.attribute(QStringLiteral("pa"));
	fax = e.attribute(QStringLiteral("fa"));
	phoneMobile = e.attribute(QStringLiteral("mo"));
	additionalNumber = e.attribute(QStringLiteral("ot"));
	altEmail1 = e.attribute(QStringLiteral("e1"));
	altEmail2 = e.attribute(QStringLiteral("e2"));
	privateURL = e.attribute(QStringLiteral("pu"));
	title = e.attribute(QStringLiteral("ti"));
	corporation = e.attribute(QStringLiteral("co"));
	workAdress = e.attribute(QStringLiteral("wa")).replace( QLatin1String("&#xd;&#xa;"), QLatin1String("\n") );
	workCity = e.attribute(QStringLiteral("wc"));
	workState = e.attribute(QStringLiteral("ws"));
	workZIP = e.attribute(QStringLiteral("wz"));
	workCountry = e.attribute(QStringLiteral("wn"));
	workURL = e.attribute(QStringLiteral("wu"));
	privateAdress = e.attribute(QStringLiteral("ha")).replace( QLatin1String("&#xd;&#xa;"), QLatin1String("\n") );
	privateCity = e.attribute(QStringLiteral("hc"));
	privateState = e.attribute(QStringLiteral("hs"));
	privateZIP = e.attribute(QStringLiteral("hz"));
	privateCountry = e.attribute(QStringLiteral("hn"));
	QString birtday = e.attribute(QStringLiteral("bi"));
	birthday = QDate( birtday.section('/',2,2).toInt(), birtday.section('/',1,1).toInt(), birtday.section('/',0,0).toInt() );
	QString an = e.attribute(QStringLiteral("an"));
	anniversary = QDate( an.section('/',2,2).toInt(), an.section('/',1,1).toInt(), an.section('/',0,0).toInt() );
	additional1 = e.attribute(QStringLiteral("c1"));
	additional2 = e.attribute(QStringLiteral("c2"));
	additional3 = e.attribute(QStringLiteral("c3"));
	additional4 = e.attribute(QStringLiteral("c4"));
	notes = e.attribute(QStringLiteral("cm")).replace( QLatin1String("&#xd;&#xa;"), QLatin1String("\n") );
	imAIM = e.attribute(QStringLiteral("ima"));
	imGoogleTalk = e.attribute(QStringLiteral("img"));
	imICQ = e.attribute(QStringLiteral("imq"));
	imIRC = e.attribute(QStringLiteral("imc"));
	imMSN = e.attribute(QStringLiteral("imm"));
	imQQ = e.attribute(QStringLiteral("imqq"));
	imSkype = e.attribute(QStringLiteral("imk"));
}

void YABEntry::fromQDomDocument( const QDomDocument &d )
{
	kDebug() << d.toString() << 
		d.elementsByTagName(QStringLiteral("yi")).item(0).toElement().text();
	yahooId = d.elementsByTagName(QStringLiteral("yi")).item(0).toElement().text();
	firstName = d.elementsByTagName(QStringLiteral("fn")).item(0).toElement().text();
	secondName = d.elementsByTagName(QStringLiteral("mn")).item(0).toElement().text();
	lastName = d.elementsByTagName(QStringLiteral("ln")).item(0).toElement().text();
	nickName = d.elementsByTagName(QStringLiteral("nn")).item(0).toElement().text();
	email = d.elementsByTagName(QStringLiteral("e0")).item(0).toElement().text();
	privatePhone = d.elementsByTagName(QStringLiteral("hp")).item(0).toElement().text();
	workPhone = d.elementsByTagName(QStringLiteral("wp")).item(0).toElement().text();
	pager = d.elementsByTagName(QStringLiteral("pa")).item(0).toElement().text();
	fax = d.elementsByTagName(QStringLiteral("fa")).item(0).toElement().text();
	phoneMobile = d.elementsByTagName(QStringLiteral("mo")).item(0).toElement().text();
	additionalNumber = d.elementsByTagName(QStringLiteral("ot")).item(0).toElement().text();
	altEmail1 = d.elementsByTagName(QStringLiteral("e1")).item(0).toElement().text();
	altEmail2 = d.elementsByTagName(QStringLiteral("e2")).item(0).toElement().text();
	privateURL = d.elementsByTagName(QStringLiteral("pu")).item(0).toElement().text();
	title = d.elementsByTagName(QStringLiteral("ti")).item(0).toElement().text();
	corporation = d.elementsByTagName(QStringLiteral("co")).item(0).toElement().text();
	workAdress = d.elementsByTagName(QStringLiteral("wa")).item(0).toElement().text().replace( QLatin1String("&#xd;&#xa;"), QLatin1String("\n") );
	workCity = d.elementsByTagName(QStringLiteral("wc")).item(0).toElement().text();
	workState = d.elementsByTagName(QStringLiteral("ws")).item(0).toElement().text();
	workZIP = d.elementsByTagName(QStringLiteral("wz")).item(0).toElement().text();
	workCountry = d.elementsByTagName(QStringLiteral("wn")).item(0).toElement().text();
	workURL = d.elementsByTagName(QStringLiteral("wu")).item(0).toElement().text();
	privateAdress = d.elementsByTagName(QStringLiteral("ha")).item(0).toElement().text().replace( QLatin1String("&#xd;&#xa;"), QLatin1String("\n") );
	privateCity = d.elementsByTagName(QStringLiteral("hc")).item(0).toElement().text();
	privateState = d.elementsByTagName(QStringLiteral("hs")).item(0).toElement().text();
	privateZIP = d.elementsByTagName(QStringLiteral("hz")).item(0).toElement().text();
	privateCountry = d.elementsByTagName(QStringLiteral("hn")).item(0).toElement().text();
	QString birtday = d.elementsByTagName(QStringLiteral("bi")).item(0).toElement().text();
	birthday = QDate( birtday.section('/',2,2).toInt(), birtday.section('/',1,1).toInt(), birtday.section('/',0,0).toInt() );
	QString an = d.elementsByTagName(QStringLiteral("an")).item(0).toElement().text();
	anniversary = QDate( an.section('/',2,2).toInt(), an.section('/',1,1).toInt(), an.section('/',0,0).toInt() );
	additional1 = d.elementsByTagName(QStringLiteral("c1")).item(0).toElement().text();
	additional2 = d.elementsByTagName(QStringLiteral("c2")).item(0).toElement().text();
	additional3 = d.elementsByTagName(QStringLiteral("c3")).item(0).toElement().text();
	additional4 = d.elementsByTagName(QStringLiteral("c4")).item(0).toElement().text();
	notes = d.elementsByTagName(QStringLiteral("cm")).item(0).toElement().text().replace( QLatin1String("&#xd;&#xa;"), QLatin1String("\n") );
	imAIM = d.elementsByTagName(QStringLiteral("ima")).item(0).toElement().text();
	imGoogleTalk = d.elementsByTagName(QStringLiteral("img")).item(0).toElement().text();
	imICQ = d.elementsByTagName(QStringLiteral("imq")).item(0).toElement().text();
	imIRC = d.elementsByTagName(QStringLiteral("imc")).item(0).toElement().text();
	imMSN = d.elementsByTagName(QStringLiteral("imm")).item(0).toElement().text();
	imQQ = d.elementsByTagName(QStringLiteral("imqq")).item(0).toElement().text();
	imSkype = d.elementsByTagName(QStringLiteral("imk")).item(0).toElement().text();
}

void YABEntry::fillQDomElement( QDomElement &e ) const
{
	e.setAttribute( QStringLiteral("yi"), yahooId );
	e.setAttribute( QStringLiteral("id"), YABId );
	e.setAttribute( QStringLiteral("fn"), firstName );
	e.setAttribute( QStringLiteral("mn"), secondName );
	e.setAttribute( QStringLiteral("ln"), lastName );
	e.setAttribute( QStringLiteral("nn"), nickName );
	e.setAttribute( QStringLiteral("e0"), email );
	e.setAttribute( QStringLiteral("hp"), privatePhone );
	e.setAttribute( QStringLiteral("wp"), workPhone );
	e.setAttribute( QStringLiteral("pa"), pager );
	e.setAttribute( QStringLiteral("fa"), fax );
	e.setAttribute( QStringLiteral("mo"), phoneMobile );
	e.setAttribute( QStringLiteral("ot"), additionalNumber );
	e.setAttribute( QStringLiteral("e1"), altEmail1 );
	e.setAttribute( QStringLiteral("e2"), altEmail2 );
	e.setAttribute( QStringLiteral("pu"), privateURL );
	e.setAttribute( QStringLiteral("ti"), title );
	e.setAttribute( QStringLiteral("co"), corporation );
	e.setAttribute( QStringLiteral("wa"), QString( workAdress ).replace( '\n', QLatin1String("&#xd;&#xa;") ) );
	e.setAttribute( QStringLiteral("wc"), workCity );
	e.setAttribute( QStringLiteral("ws"), workState );
	e.setAttribute( QStringLiteral("wz"), workZIP );
	e.setAttribute( QStringLiteral("wn"), workCountry );
	e.setAttribute( QStringLiteral("wu"), workURL );
	e.setAttribute( QStringLiteral("ha"), QString( privateAdress ).replace( '\n', QLatin1String("&#xd;&#xa;") ) );
	e.setAttribute( QStringLiteral("hc"), privateCity );
	e.setAttribute( QStringLiteral("hs"), privateState );
	e.setAttribute( QStringLiteral("hz"), privateZIP );
	e.setAttribute( QStringLiteral("hn"), privateCountry );
	e.setAttribute( QStringLiteral("bi"), QStringLiteral("%1/%2/%3").arg( birthday.day() ).arg( birthday.month() ).arg( birthday.year() ) );
	e.setAttribute( QStringLiteral("an"), QStringLiteral("%1/%2/%3").arg( anniversary.day() ).arg( anniversary.month() ).arg( anniversary.year() ) );
	e.setAttribute( QStringLiteral("c1"), additional1 );
	e.setAttribute( QStringLiteral("c2"), additional2 );
	e.setAttribute( QStringLiteral("c3"), additional3 );
	e.setAttribute( QStringLiteral("c4"), additional4 );
	e.setAttribute( QStringLiteral("cm"), QString( notes ).replace( '\n', QLatin1String("&#xd;&#xa;") ) );
	e.setAttribute( QStringLiteral("ima"), imAIM );
	e.setAttribute( QStringLiteral("img"), imGoogleTalk );
	e.setAttribute( QStringLiteral("imq"), imICQ );
	e.setAttribute( QStringLiteral("imc"), imIRC );
	e.setAttribute( QStringLiteral("imm"), imMSN );
	e.setAttribute( QStringLiteral("imqq"), imQQ );
	e.setAttribute( QStringLiteral("imk"), imSkype );
}

void YABEntry::dump() const
{
	kDebug() << "firstName: " << firstName << endl << 
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
