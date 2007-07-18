
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
	email		= property(MessengerProtocol->protocol()->propEmail).value().toString();
	ContactType = property(MessengerProtocol->protocol()->propContactType).value().toString();
	firstName	= property(MessengerProtocol->protocol()->propFirstName).value().toString();
	lastName	= property(MessengerProtocol->protocol()->propLastName).value().toString();
	comment		= property(MessengerProtocol->protocol()->propComment).value().toString();
	Anniversary	= QDate::fromString( property(MessengerProtocol->protocol()->propAnniversary).value().toString() , Qt::ISODate );
	birthdate	= QDate::fromString( property(MessengerProtocol->protocol()->propBirthday).value().toString(), Qt::ISODate );
}

MessengerGeneralUserInfo::storeProp()
{
	//general User info
	setProperty(MessnegerProtocol->protocol()->propEmail , email);
	setProperty(MessnegerProtocol->protocol()->propContactType , ContactType );
	setProperty(MessnegerProtocol->protocol()->propFirstName , firstName );
	setProperty(MessnegerProtocol->protocol()->propLastName , lastName );
	setProperty(MessnegerProtocol->protocol()->propComment , comment );
	setProperty(MessnegerProtocol->protocol()->propAnniversary , Anniversary.toString(Qt::ISODate) );
	setProperty(MessnegerProtocol->protocol()->propBirthday , birthdate.toString(Qt::ISODate) );
}

MessengerGeneralUserInfo::fromQDomElement(const QDomElement &e)
{

}

MessengerGeneralUserInfo::fillDomElement(const QDomElement &e)
{

}

MessengerAnnotationUserInfo::loadProp()
{
	JobTitle	= property(MessengerProtocol->protocol()->propABJobTitle).value().toString();
	NickName	= property(MessengerProtocol->protocol()->propABNickName).value().toString();
	Spouse		= property(MessengerProtocol->protocol()->propABJobSpouse).value().toString();
}

MessengerAnnotationUserInfo::storeProp()
{
	//Annotation
	setProperty(MessnegerProtocol->protocol()->propABJobTitle, JobTitle) ;
	setProperty(MessnegerProtocol->protocol()->propABNickName, NickName) ;
	setProperty(MessnegerProtocol->protocol()->propABJobSpouse , Spouse);
}

MessengerEmailUserInfo::loadProp()
{
	ContactEmailBusiness	= property(MessengerProtocol->protocol()->propContactEmailBusiness).value().toString();
	ContactEmailMessenger	= property(MessengerProtocol->protocol()->propContactEmailMessenger).value().toString();
	ContactEmailOther		= property(MessengerProtocol->protocol()->propContactEmailOther).value().toString();
	ContactEmailPersonal	= property(MessengerProtocol->protocol()->propContactEmailPersonal).value().toString();
}

MessengerEmailUserInfo::storeProp()
{
	setProperty(MessnegerProtocol->protocol()->propContactEmailBusiness, ContactEmailBusiness );
	setProperty(MessnegerProtocol->protocol()->propContactEmailMessenger , ContactEmailMessenger );
	setProperty(MessnegerProtocol->protocol()->propContactEmailOther , ContactEmailOther );
	setProperty(MessnegerProtocol->protocol()->propContactEmailPersonal , ContactEmailPersonal );
}

MessengerPhoneUserInfo::loadProp()
{
	ContactPhoneBusiness	= property(MessengerProtocol->protocol()->propContactPhoneBusiness).value().toString();
	ContactPhoneFax			= property(MessengerProtocol->protocol()->propContactPhoneFax).value().toString();
	ContactPhoneMobile		= property(MessengerProtocol->protocol()->propContactPhoneMobile).value().toString();
	ContactPhoneOther		= property(MessengerProtocol->protocol()->propContactPhoneOther).value().toString();
	ContactPhonePager		= property(MessengerProtocol->protocol()->propContactPhonePager).value().toString();
	ContactPhonePer			= property(MessengerProtocol->protocol()->propContactPhonePersonal).value().toString();
}

MessengerPhoneUserInfo::storeProp()
{
	setProperty(MessnegerProtocol->protocol()->propContactPhoneBusiness , ContactPhoneBusiness );
	setProperty(MessnegerProtocol->protocol()->propContactPhoneFax , ContactPhoneFax );
	setProperty(MessnegerProtocol->protocol()->propContactPhoneMobile , ContactPhoneMobile);
	setProperty(MessnegerProtocol->protocol()->propContactPhoneOther , ContactPhoneOther );	
	setProperty(MessnegerProtocol->protocol()->propContactPhonePager , ContactPhonePager);	
	setProperty(MessnegerProtocol->protocol()->propContactPhonePersonal , ContactPhonePersonal);	
}

MessengerLocationUserInfo::loadProp()
{
	//Business Location
	BusinessName	= property(MessengerProtocol->protocol()->propBusinessName).value().toString();
	BusinessStreet	= property(MessengerProtocol->protocol()->propBusinessStreet).value().toString();
	BusinessCity	= property(MessengerProtocol->protocol()->propBusinessCity).value().toString();
	BusinessState	= property(MessengerProtocol->protocol()->propBusinessState).value().toString();
	BusinessCountry	= property(MessengerProtocol->protocol()->propBusinessCountry).value().toString();
	BusinessPostalCode	= property(MessengerProtocol->protocol()->propBusinessPostalCode).value().toString();

	//Personal Location
	PersonalName	= property(MessengerProtocol->protocol()->propPersonalName).value().toString();
	PersonalStreet	= property(MessengerProtocol->protocol()->propPersonalStreet).value().toString();
	PersonalCity	= property(MessengerProtocol->protocol()->propPersonalCity).value().toString();
	PersonalState	= property(MessengerProtocol->protocol()->propPersonalState).value().toString();
	PersonalCountry	= property(MessengerProtocol->protocol()->propPersonalCountry).value().toString();
	PersonalPostalCode = property(MessengerProtocol->protocol()->propPersonalPostalCode).value().toString();
}

MessengerLocationUserInfo::storeProp()
{
	//Business Location
	setProperty(MessnegerProtocol->protocol()->propBusinessName , BusinessName);
	setProperty(MessnegerProtocol->protocol()->propBusinessStreet , BusinessStreet );
	setProperty(MessnegerProtocol->protocol()->propBusinessCity ,BusinessCity );
	setProperty(MessnegerProtocol->protocol()->propBusinessState , BusinessState );
	setProperty(MessnegerProtocol->protocol()->propBusinessCountry , BusinessCountry );
	setProperty(MessnegerProtocol->protocol()->propBusinessPostalCode, BusinessPostalCode);

	//Personal Location
	setProperty(MessnegerProtocol->protocol()->propPersonalName, PersonalName);
	setProperty(MessnegerProtocol->protocol()->propPersonalStreet , PersonalStreet);
	setProperty(MessnegerProtocol->protocol()->propPersonalCity , PersonalCity);
	setProperty(MessnegerProtocol->protocol()->propPersonalState , PersonalState);
	setProperty(MessnegerProtocol->protocol()->propPersonalCountry, PersonalCountry);
	setProperty(MessnegerProtocol->protocol()->propPersonalPostalCode , PersonalPostalCode);
}

MessengerWebSiteUserInfo::loadProp()
{
	ContactWebSiteBusiness	= property(MessengerProtocol->protocol()->propContactWebSiteBusiness).value().toString();
	ContactWebSitePersonal	= property(MessengerProtocol->protocol()->propContactWebSitePersonal).value().toString();
}

MessengerWebSiteUserInfo::storeProp()
{
	setProperty(MessnegerProtocol->protocol()->propContactWebSiteBusiness, ContactWebSiteBusiness);
	setProperty(MessnegerProtocol->protocol()->propContactWebSitePersonal, ContactWebSitePersonal);
}

