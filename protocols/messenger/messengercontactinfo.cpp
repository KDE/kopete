/*
 * messengercontactinfo.cpp - Windows Live Messenger Messenger Contact Info
 *
 * Copyright (c) 2007 by Zhang Panyong <pyzhang@gmail.com>
 * Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>
 *
 *************************************************************************
 *                                                                       *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 *************************************************************************
 */

MessengerContactInfo::MessengerContactInfo()
{
	m_generalUserInfo = new MessengerGeneralUserInfo();
	m_annotationUserInfo = new MessengerAnnotationUserInfo();
	m_emailUserInfo = new MessengerEmailUserInfo();
	m_phoneUserInfo = new MessengerPhoneUserInfo();
	m_locationUserInfo = new MessengerLocationUserInfo();
	m_websiteUserInfo = new MessengerWebSiteUserInfo();
}

MessengerContactInfo::~MessengerContactInfo()
{
	delete m_generalUserInfo;
	delete m_annotationUserInfo;
	delete m_emailUserInfo;
	delete m_phoneUserInfo;
	delete m_locationUserInfo;
	delete m_websiteUserInfo;
}

MessengerContactInfo::fromQDomElement(const QDomElement &e)
{

}

void MessengerContactInfo::fillQDomElement(const QDomElement &e)
{

}

MessengerContactInfo::loadProp(MessengerContactInfo *m_contact)
{
	m_contact->m_generalUserInfo->loadProp();
	m_contact->m_annotationUserInfo->loadProp();
	m_contact->m_emailUserInfo->loadProp();
	m_contact->m_phoneUserInfo->loadProp();
	m_contact->m_locationUserInfo->loadProp();
	m_contact->m_websiteUserInfo->loadProp();
}

MessengerContactInfo::storeProp(MessengerContactInfo *m_contact)
{
	m_contact->m_generalUserInfo->storeProp();
	m_contact->m_annotationUserInfo->storeProp();
	m_contact->m_emailUserInfo->storeProp();
	m_contact->m_phoneUserInfo->storeProp();
	m_contact->m_locationUserInfo->storeProp();
	m_contact->m_websiteUserInfo->storeProp();
}

const MessengerContactInfo::dump() const
{
	KDebug()<<"Dumping Contact Info";
}

MessengerGeneralUserInfo::loadProp()
{
	email		= property(MessengerProtocol::protocol()->propEmail).value().toString();
	ContactType = property(MessengerProtocol::protocol()->propContactType).value().toString();
	firstName	= property(MessengerProtocol::protocol()->propFirstName).value().toString();
	lastName	= property(MessengerProtocol::protocol()->propLastName).value().toString();
	comment		= property(MessengerProtocol->protocol()->propComment).value().toString();
	Anniversary	= QDate::fromString( property(MessengerProtocol::protocol()->propAnniversary).value().toString() , Qt::ISODate );
	birthdate	= QDate::fromString( property(MessengerProtocol::protocol()->propBirthday).value().toString(), Qt::ISODate );
}

MessengerGeneralUserInfo::storeProp()
{
	//general User info
	setProperty(MessengerProtocol::protocol()->propEmail , email);
	setProperty(MessengerProtocol::protocol()->propContactType , ContactType );
	setProperty(MessengerProtocol::protocol()->propFirstName , firstName );
	setProperty(MessengerProtocol::protocol()->propLastName , lastName );
	setProperty(MessengerProtocol::protocol()->propComment , comment );
	setProperty(MessengerProtocol::protocol()->propAnniversary , Anniversary.toString(Qt::ISODate) );
	setProperty(MessengerProtocol::protocol()->propBirthday , birthdate.toString(Qt::ISODate) );
}

MessengerGeneralUserInfo::fromQDomElement(const QDomElement &e)
{

}

MessengerGeneralUserInfo::fillDomElement(const QDomElement &e)
{

}

MessengerAnnotationUserInfo::loadProp()
{
	JobTitle	= property(MessengerProtocol::protocol()->propABJobTitle).value().toString();
	NickName	= property(MessengerProtocol::protocol()->propABNickName).value().toString();
	Spouse		= property(MessengerProtocol::protocol()->propABJobSpouse).value().toString();
}

MessengerAnnotationUserInfo::storeProp()
{
	//Annotation
	setProperty(MessengerProtocol::protocol()->propABJobTitle, JobTitle) ;
	setProperty(MessengerProtocol::protocol()->propABNickName, NickName) ;
	setProperty(MessengerProtocol::protocol()->propABJobSpouse , Spouse);
}

MessengerEmailUserInfo::loadProp()
{
	ContactEmailBusiness	= property(MessengerProtocol::protocol()->propContactEmailBusiness).value().toString();
	ContactEmailMessenger	= property(MessengerProtocol::protocol()->propContactEmailMessenger).value().toString();
	ContactEmailOther		= property(MessengerProtocol::protocol()->propContactEmailOther).value().toString();
	ContactEmailPersonal	= property(MessengerProtocol::protocol()->propContactEmailPersonal).value().toString();
}

MessengerEmailUserInfo::storeProp()
{
	setProperty(MessengerProtocol::protocol()->propContactEmailBusiness, ContactEmailBusiness );
	setProperty(MessengerProtocol::protocol()->propContactEmailMessenger , ContactEmailMessenger );
	setProperty(MessengerProtocol::protocol()->propContactEmailOther , ContactEmailOther );
	setProperty(MessengerProtocol::protocol()->propContactEmailPersonal , ContactEmailPersonal );
}

MessengerPhoneUserInfo::loadProp()
{
	ContactPhoneBusiness	= property(MessengerProtocol::protocol()->propContactPhoneBusiness).value().toString();
	ContactPhoneFax			= property(MessengerProtocol::protocol()->propContactPhoneFax).value().toString();
	ContactPhoneMobile		= property(MessengerProtocol::protocol()->propContactPhoneMobile).value().toString();
	ContactPhoneOther		= property(MessengerProtocol::protocol()->propContactPhoneOther).value().toString();
	ContactPhonePager		= property(MessengerProtocol::protocol()->propContactPhonePager).value().toString();
	ContactPhonePer			= property(MessengerProtocol::protocol()->propContactPhonePersonal).value().toString();
}

MessengerPhoneUserInfo::storeProp()
{
	setProperty(MessengerProtocol::protocol()->propContactPhoneBusiness , ContactPhoneBusiness );
	setProperty(MessengerProtocol::protocol()->propContactPhoneFax , ContactPhoneFax );
	setProperty(MessengerProtocol::protocol()->propContactPhoneMobile , ContactPhoneMobile);
	setProperty(MessengerProtocol::protocol()->propContactPhoneOther , ContactPhoneOther );	
	setProperty(MessengerProtocol::protocol()->propContactPhonePager , ContactPhonePager);	
	setProperty(MessengerProtocol::protocol()->propContactPhonePersonal , ContactPhonePersonal);	
}

MessengerLocationUserInfo::loadProp()
{
	//Business Location
	BusinessName	= property(MessengerProtocol::protocol()->propBusinessName).value().toString();
	BusinessStreet	= property(MessengerProtocol::protocol()->propBusinessStreet).value().toString();
	BusinessCity	= property(MessengerProtocol::protocol()->propBusinessCity).value().toString();
	BusinessState	= property(MessengerProtocol::protocol()->propBusinessState).value().toString();
	BusinessCountry	= property(MessengerProtocol::protocol()->propBusinessCountry).value().toString();
	BusinessPostalCode	= property(MessengerProtocol::protocol()->propBusinessPostalCode).value().toString();

	//Personal Location
	PersonalName	= property(MessengerProtocol::protocol()->propPersonalName).value().toString();
	PersonalStreet	= property(MessengerProtocol::protocol()->propPersonalStreet).value().toString();
	PersonalCity	= property(MessengerProtocol::protocol()->propPersonalCity).value().toString();
	PersonalState	= property(MessengerProtocol::protocol()->propPersonalState).value().toString();
	PersonalCountry	= property(MessengerProtocol::protocol()->propPersonalCountry).value().toString();
	PersonalPostalCode = property(MessengerProtocol::protocol()->propPersonalPostalCode).value().toString();
}

MessengerLocationUserInfo::storeProp()
{
	//Business Location
	setProperty(MessengerProtocol::protocol()->propBusinessName , BusinessName);
	setProperty(MessengerProtocol::protocol()->propBusinessStreet , BusinessStreet );
	setProperty(MessengerProtocol::protocol()->propBusinessCity ,BusinessCity );
	setProperty(MessengerProtocol::protocol()->propBusinessState , BusinessState );
	setProperty(MessengerProtocol::protocol()->propBusinessCountry , BusinessCountry );
	setProperty(MessengerProtocol::protocol()->propBusinessPostalCode, BusinessPostalCode);

	//Personal Location
	setProperty(MessengerProtocol::protocol()->propPersonalName, PersonalName);
	setProperty(MessengerProtocol::protocol()->propPersonalStreet , PersonalStreet);
	setProperty(MessengerProtocol::protocol()->propPersonalCity , PersonalCity);
	setProperty(MessengerProtocol::protocol()->propPersonalState , PersonalState);
	setProperty(MessengerProtocol::protocol()->propPersonalCountry, PersonalCountry);
	setProperty(MessengerProtocol::protocol()->propPersonalPostalCode , PersonalPostalCode);
}

MessengerWebSiteUserInfo::loadProp()
{
	ContactWebSiteBusiness	= property(MessengerProtocol::protocol()->propContactWebSiteBusiness).value().toString();
	ContactWebSitePersonal	= property(MessengerProtocol::protocol()->propContactWebSitePersonal).value().toString();
}

MessengerWebSiteUserInfo::storeProp()
{
	setProperty(MessengerProtocol::protocol()->propContactWebSiteBusiness, ContactWebSiteBusiness);
	setProperty(MessengerProtocol::protocol()->propContactWebSitePersonal, ContactWebSitePersonal);
}

#include "messengercontactinfo.moc"
