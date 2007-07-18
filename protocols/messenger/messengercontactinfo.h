
#ifndef _MESSENGERCONTACTINFO_H_
#define _MESSENGERCONTACTINFO_H_

class MessengerContactInfo
{
public;
	MessengerContactInfo();
	~MessengerContactInfo();

	void loadProp();
	void storeProp();

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
	void loadProp();
	void storeProp();

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
	void loadProp();
	void storeProp();
	
	MessengerInfoValue<QString> JobTitle;
	MessengerInfoValue<QString> NickName;
	MessengerInfoValue<QString> Spouse;
}

class MessengerEmailUserInfo
{
public:
	MessengerEmailUserInfo();
	void loadProp();
	void storeProp();
	
	MessengerInfoValue<QString> ContactEmailBusiness;
	MessengerInfoValue<QString> ContactEmailMessenger;
	MessengerInfoValue<QString> ContactEmailOther;
	MessengerInfoValue<QString> ContactEmailPersonal;
}

class MessengerPhoneUserInfo
{
public:
	MessengerPhoneUserInfo();
	void loadProp();
	void storeProp();
	
	MessengerInfoValue<QString> ContactPhoneBusiness;
	MessengerInfoValue<QString> ContactPhoneFax;
	MessengerInfoValue<QString> ContactPhoneMobile;
	MessengerInfoValue<QString> ContactPhoneOther;	
	MessengerInfoValue<QString> ContactPhonePager;	
	MessengerInfoValue<QString> ContactPhonePersonal;	
}

class MessengerLocationUserInfo
{
public:
	MessengerLocationUserInfo()
	{
	};

	void loadProp();
	void storeProp();

	//Business Location
	MessengerInfoValue<QString> BusinessName;
	MessengerInfoValue<QString> BusinessStreet;
	MessengerInfoValue<QString> BusinessCity;
	MessengerInfoValue<QString> BusinessState;
	MessengerInfoValue<QString> BusinessCountry;
	MessengerInfoValue<QString> BusinessPostalCode;

	//Personal Location
	MessengerInfoValue<QString> PersonalStreet;
	MessengerInfoValue<QString> PersonalCity;
	MessengerInfoValue<QString> PersonalState;
	MessengerInfoValue<QString> PersonalCountry;
	MessengerInfoValue<QString> PersonalPostalCode;

}

class MessengerWebSiteUserInfo
{
public:
	MessengerWebSiteUserInfo();
	void loadProp();
	void storeProp();

	MessengerInfoValue<QString> ContactWebSiteBusiness;
	MessengerInfoValue<QString> ContactWebSitePersonal;
}

#endif

