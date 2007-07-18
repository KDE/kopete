
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

const MessengerContactInfo::dump() const
{
	KDebug()<<
}

MessengerGeneralUserInfo::fromQDomElement(const QDomElement &e)
{

}

MessengerGeneralUserInfo::fillDomElement(const QDomElement &e)
{
	e
}

