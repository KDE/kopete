
#ifndef OSCARSOCKET_ICQ_H
#define OSCARSOCKET_ICQ_H

#include <qmap.h>
#include <qvaluelist.h>

const QString ICQ_SERVER 	= "login.icq.com";
const unsigned int ICQ_PORT 	= 5190;

// Internal status for the ICQ protocol
// some are dupliated because setting and getting the status
// do not use the same value
const unsigned long ICQ_STATUS_OFFLINE		= 0xFFFFFFFF;
const unsigned long ICQ_STATUS_ONLINE		= 0x00000000;

const unsigned long ICQ_STATUS_IS_INVIS		= 0x00000100;
const unsigned long ICQ_STATUS_IS_DND		= 0x00000002;
const unsigned long ICQ_STATUS_IS_OCC		= 0x00000010;
const unsigned long ICQ_STATUS_IS_NA		= 0x00000004;
const unsigned long ICQ_STATUS_IS_AWAY		= 0x00000001;
const unsigned long ICQ_STATUS_IS_FFC		= 0x00000020;

const unsigned long ICQ_STATUS_SET_INVIS	= 0x00000100;
const unsigned long ICQ_STATUS_SET_DND		= 0x00000013;
const unsigned long ICQ_STATUS_SET_OCC		= 0x00000011;
const unsigned long ICQ_STATUS_SET_NA		= 0x00000005;
const unsigned long ICQ_STATUS_SET_AWAY		= 0x00000001;
const unsigned long ICQ_STATUS_SET_FFC		= 0x00000020;

const unsigned long ICQ_STATUS_WEBAWARE		= 0x00010000;
const unsigned long ICQ_STATUS_SHOWIP		= 0x00020000;

const unsigned short ICQ_SEARCHSTATE_OFFLINE 	= 0;
const unsigned short ICQ_SEARCHSTATE_ONLINE 	= 1;
const unsigned short ICQ_SEARCHSTATE_DISABLED 	= 2;


const unsigned char ICQ_TCP_VERSION 	= 0x0008;
const char ICQ_CLIENTSTRING[] 		= "ICQ Inc. - Product of ICQ (TM).2003a.5.45.1.3777.85";
const WORD ICQ_CLIENTID 		= 0x010A;
const WORD ICQ_MAJOR 			= 0x0005;
const WORD ICQ_MINOR 			= 0x002D;
const WORD ICQ_POINT 			= 0x0001;
const WORD ICQ_BUILD 			= 0x0EC1;
const char ICQ_OTHER[] 			= { 0x00, 0x00, 0x00, 0x55 };
const char ICQ_COUNTRY[] 		= "us";
const char ICQ_LANG[] 			= "en";


// Taken from libicq, not sure if we ever support these requests
const unsigned char PHONEBOOK_SIGN[16] =
{
	0x90, 0x7C, 0x21, 0x2C, 0x91, 0x4D, 0xD3, 0x11,
	0xAD, 0xEB, 0x00, 0x04, 0xAC, 0x96, 0xAA, 0xB2
};

const unsigned char PLUGINS_SIGN[16] =
{
	0xF0, 0x02, 0xBF, 0x71, 0x43, 0x71, 0xD3, 0x11,
	0x8D, 0xD2, 0x00, 0x10, 0x4B, 0x06, 0x46, 0x2E
};

/*
const unsigned char SHARED_FILES_SIGN[16] =
{
	0xF0, 0x2D, 0x12, 0xD9, 0x30, 0x91, 0xD3, 0x11,
	0x8D, 0xD7, 0x00, 0x10, 0x4B, 0x06, 0x46, 0x2E
};
*/
// ==================================================================


class ICQSearchResult
{
	public:
		unsigned long uin;
		QString nickName;
		QString firstName;
		QString lastName;
		QString eMail;
		bool needAuth;
		unsigned int status; // 0=offline, 1=online, 2=not webaware
};

/**
 * Classes encapsulating user data retrieved from the server
 */


class ICQGeneralUserInfo
{
	public:
		unsigned long uin;
		QString nickName;
		QString firstName;
		QString lastName;
		QString eMail;
		QString city;
		QString state;
		QString phoneNumber;
		QString faxNumber;
		QString street;
		QString cellularNumber;
		QString zip;
		int countryCode;
		char timezoneCode;
		bool publishEmail;
		bool showOnWeb;
};

class ICQWorkUserInfo
{
	public:
		QString city;
		QString state;
		QString phone;
		QString fax;
		QString address;
		QString zip;
		int countryCode;
		QString company;
		QString department;
		QString position;
		int occupation;
		QString homepage;
};

class ICQMoreUserInfo
{
	public:
		int age;
		unsigned int gender;
		QString homepage;
		QDate birthday;
		unsigned int lang1;
		unsigned int lang2;
		unsigned int lang3;
};

class ICQInfoItem
{
	public:
		int category;
		QString description;
};

typedef QMap<QString, bool> ICQMailList;
typedef QValueList<ICQInfoItem> ICQInfoItemList;

#endif
// vim: set noet ts=4 sts=4 sw=4:
