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
	yahooId = e.attribute("yi");
	YABId = e.attribute("id", "-1").toInt();
	firstName = e.attribute("fn");
	secondName = e.attribute("mn");
	lastName = e.attribute("ln");
	nickName = e.attribute("nn");
	email = e.attribute("e0");
	privatePhone = e.attribute("hp");
	workPhone = e.attribute("wp");
	pager = e.attribute("pa");
	fax = e.attribute("fa");
	phoneMobile = e.attribute("mo");
	additionalNumber = e.attribute("ot");
	altEmail1 = e.attribute("e1");
	altEmail2 = e.attribute("e2");
	privateURL = e.attribute("pu");
	title = e.attribute("ti");
	corporation = e.attribute("co");
	workAdress = e.attribute("wa").replace( "&#xd;&#xa;", "\n" );
	workCity = e.attribute("wc");
	workState = e.attribute("ws");
	workZIP = e.attribute("wz");
	workCountry = e.attribute("wn");
	workURL = e.attribute("wu");
	privateAdress = e.attribute("ha").replace( "&#xd;&#xa;", "\n" );
	privateCity = e.attribute("hc");
	privateState = e.attribute("hs");
	privateZIP = e.attribute("hz");
	privateCountry = e.attribute("hn");
	QString birtday = e.attribute("bi");
	birthday = QDate( birtday.section('/',2,2).toInt(), birtday.section('/',1,1).toInt(), birtday.section('/',0,0).toInt() );
	QString an = e.attribute("an");
	anniversary = QDate( an.section('/',2,2).toInt(), an.section('/',1,1).toInt(), an.section('/',0,0).toInt() );
	additional1 = e.attribute("c1");
	additional2 = e.attribute("c2");
	additional3 = e.attribute("c3");
	additional4 = e.attribute("c4");
	notes = e.attribute("cm").replace( "&#xd;&#xa;", "\n" );
	imAIM = e.attribute("ima");
	imGoogleTalk = e.attribute("img");
	imICQ = e.attribute("imq");
	imIRC = e.attribute("imc");
	imMSN = e.attribute("imm");
	imQQ = e.attribute("imqq");
	imSkype = e.attribute("imk");
}

void YABEntry::fromQDomDocument( const QDomDocument &d )
{
	kDebug() << d.toString() << 
		d.elementsByTagName("yi").item(0).toElement().text();
	yahooId = d.elementsByTagName("yi").item(0).toElement().text();
	firstName = d.elementsByTagName("fn").item(0).toElement().text();
	secondName = d.elementsByTagName("mn").item(0).toElement().text();
	lastName = d.elementsByTagName("ln").item(0).toElement().text();
	nickName = d.elementsByTagName("nn").item(0).toElement().text();
	email = d.elementsByTagName("e0").item(0).toElement().text();
	privatePhone = d.elementsByTagName("hp").item(0).toElement().text();
	workPhone = d.elementsByTagName("wp").item(0).toElement().text();
	pager = d.elementsByTagName("pa").item(0).toElement().text();
	fax = d.elementsByTagName("fa").item(0).toElement().text();
	phoneMobile = d.elementsByTagName("mo").item(0).toElement().text();
	additionalNumber = d.elementsByTagName("ot").item(0).toElement().text();
	altEmail1 = d.elementsByTagName("e1").item(0).toElement().text();
	altEmail2 = d.elementsByTagName("e2").item(0).toElement().text();
	privateURL = d.elementsByTagName("pu").item(0).toElement().text();
	title = d.elementsByTagName("ti").item(0).toElement().text();
	corporation = d.elementsByTagName("co").item(0).toElement().text();
	workAdress = d.elementsByTagName("wa").item(0).toElement().text().replace( "&#xd;&#xa;", "\n" );
	workCity = d.elementsByTagName("wc").item(0).toElement().text();
	workState = d.elementsByTagName("ws").item(0).toElement().text();
	workZIP = d.elementsByTagName("wz").item(0).toElement().text();
	workCountry = d.elementsByTagName("wn").item(0).toElement().text();
	workURL = d.elementsByTagName("wu").item(0).toElement().text();
	privateAdress = d.elementsByTagName("ha").item(0).toElement().text().replace( "&#xd;&#xa;", "\n" );
	privateCity = d.elementsByTagName("hc").item(0).toElement().text();
	privateState = d.elementsByTagName("hs").item(0).toElement().text();
	privateZIP = d.elementsByTagName("hz").item(0).toElement().text();
	privateCountry = d.elementsByTagName("hn").item(0).toElement().text();
	QString birtday = d.elementsByTagName("bi").item(0).toElement().text();
	birthday = QDate( birtday.section('/',2,2).toInt(), birtday.section('/',1,1).toInt(), birtday.section('/',0,0).toInt() );
	QString an = d.elementsByTagName("an").item(0).toElement().text();
	anniversary = QDate( an.section('/',2,2).toInt(), an.section('/',1,1).toInt(), an.section('/',0,0).toInt() );
	additional1 = d.elementsByTagName("c1").item(0).toElement().text();
	additional2 = d.elementsByTagName("c2").item(0).toElement().text();
	additional3 = d.elementsByTagName("c3").item(0).toElement().text();
	additional4 = d.elementsByTagName("c4").item(0).toElement().text();
	notes = d.elementsByTagName("cm").item(0).toElement().text().replace( "&#xd;&#xa;", "\n" );
	imAIM = d.elementsByTagName("ima").item(0).toElement().text();
	imGoogleTalk = d.elementsByTagName("img").item(0).toElement().text();
	imICQ = d.elementsByTagName("imq").item(0).toElement().text();
	imIRC = d.elementsByTagName("imc").item(0).toElement().text();
	imMSN = d.elementsByTagName("imm").item(0).toElement().text();
	imQQ = d.elementsByTagName("imqq").item(0).toElement().text();
	imSkype = d.elementsByTagName("imk").item(0).toElement().text();
}

void YABEntry::fillQDomElement( QDomElement &e ) const
{
	e.setAttribute( "yi", yahooId );
	e.setAttribute( "id", YABId );
	e.setAttribute( "fn", firstName );
	e.setAttribute( "mn", secondName );
	e.setAttribute( "ln", lastName );
	e.setAttribute( "nn", nickName );
	e.setAttribute( "e0", email );
	e.setAttribute( "hp", privatePhone );
	e.setAttribute( "wp", workPhone );
	e.setAttribute( "pa", pager );
	e.setAttribute( "fa", fax );
	e.setAttribute( "mo", phoneMobile );
	e.setAttribute( "ot", additionalNumber );
	e.setAttribute( "e1", altEmail1 );
	e.setAttribute( "e2", altEmail2 );
	e.setAttribute( "pu", privateURL );
	e.setAttribute( "ti", title );
	e.setAttribute( "co", corporation );
	e.setAttribute( "wa", QString( workAdress ).replace( '\n', "&#xd;&#xa;" ) );
	e.setAttribute( "wc", workCity );
	e.setAttribute( "ws", workState );
	e.setAttribute( "wz", workZIP );
	e.setAttribute( "wn", workCountry );
	e.setAttribute( "wu", workURL );
	e.setAttribute( "ha", QString( privateAdress ).replace( '\n', "&#xd;&#xa;" ) );
	e.setAttribute( "hc", privateCity );
	e.setAttribute( "hs", privateState );
	e.setAttribute( "hz", privateZIP );
	e.setAttribute( "hn", privateCountry );
	e.setAttribute( "bi", QString("%1/%2/%3").arg( birthday.day() ).arg( birthday.month() ).arg( birthday.year() ) );
	e.setAttribute( "an", QString("%1/%2/%3").arg( anniversary.day() ).arg( anniversary.month() ).arg( anniversary.year() ) );
	e.setAttribute( "c1", additional1 );
	e.setAttribute( "c2", additional2 );
	e.setAttribute( "c3", additional3 );
	e.setAttribute( "c4", additional4 );
	e.setAttribute( "cm", QString( notes ).replace( '\n', "&#xd;&#xa;" ) );
	e.setAttribute( "ima", imAIM );
	e.setAttribute( "img", imGoogleTalk );
	e.setAttribute( "imq", imICQ );
	e.setAttribute( "imc", imIRC );
	e.setAttribute( "imm", imMSN );
	e.setAttribute( "imqq", imQQ );
	e.setAttribute( "imk", imSkype );
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
