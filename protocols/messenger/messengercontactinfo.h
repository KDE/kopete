#ifndef _MESSENGERCONTACTINFO_H_
#define _MESSENGERCONTACTINFO_H_

class MessengerContactInfo
{
public;
	MessengerContactInfo();
	~MessengerContactInfo();

public:
	MessengerGeneralUserInfo* m_generalUserInfo;
	MessengerAnnotationUserInfo* m_annotationUserInfo;
	MessengerEmailUserInfo* m_emailUserInfo;
	MessengerPhoneUserInfo* m_phoneUserInfo;
	MessengerLocationUserInfo* m_locationUserInfo;
	MessengerWebSiteUserInfo* m_websiteUserInfo;
}

class MessengerGeneralUserInfo
{
public:
	MessengerGeneralUserInfo();
	
	MessengerInfoValue<QString> email; 
	
	MessengerInfoValue<QString> ContactType;
	MessengerInfoValue<QString> firstName; 
	MessengerInfoValue<QString> lastName; 
	MessengerInfoValue<QString> comment; 
	MessengerInfoValue<QDate> Anniversary;
	MessengerInfoValue<QDate> birthdate;
}

class MessengerAnnotationUserInfo
{
public:
	MessengerAnnotationUserInfo();
	
	MessengerInfoValue<QString> JobTitle;
	MessengerInfoValue<QString> NickName;
	MessengerInfoValue<QString> Spouse;
}

class MessengerEmailUserInfo
{
public:
	MessengerEmailUserInfo();
	
	MessengerInfoValue<QString> ContactEmailBusiness;
	MessengerInfoValue<QString> ContactEmailMessenger;
	MessengerInfoValue<QString> ContactEmailOther;
	MessengerInfoValue<QString> ContactEmailPersonal;
}

class MessengerPhoneUserInfo
{
public:
	MessengerPhoneUserInfo();
	
	MessengerInfoValue<QString> ContactPhoneBusiness;
	MessengerInfoValue<QString> ContactPhoneFax;
	MessengerInfoValue<QString> ContactPhoneMobile;
	MessengerInfoValue<QString> ContactPhoneOther;	
	MessengerInfoValue<QString> ContactPhonePager;	
	MessengerInfoValue<QString> ContactPhonePersonal;	
}

class MessengerLocation
{
public:
	messengerLocation();
	
	MessengerInfoValue<QString> name;
	MessengerInfoValue<QString> street;
	MessengerInfoValue<QString> city;
	MessengerInfoValue<QString> state;
	MessengerInfoValue<QString> country;
	MessengerInfoValue<QString> postalCode;
}

class MessengerLocationUserInfo
{
public:
	MessengerLocationUserInfo()
	{
		ContactLocationBusiness = new MessengerLocation();
		ContactLocationPersonal = new MessengerLocation();
	};

	~MessengerLocation()
	{
		delete ContactLocationBusiness;
		delete ContactLocationPersonal;
	};
	
	MessengerLocation* ContactLocationBusiness;
	MessengerLocation* ContactLocationPersonal;
}

class MessengerWebSiteUserInfo
{
public:
	MessengerWebSiteUserInfo();
	
	MessengerInfoValue<QString> ContactWebSiteBusiness;
	MessengerInfoValue<QString> ContactWebSitePersonal;
}

#endif

