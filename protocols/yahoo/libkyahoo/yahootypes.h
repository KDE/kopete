/*
    yahootypes.h - Kopete Yahoo Protocol definitions

    Copyright (c) 2004 Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete (c) 2002-2004 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef YAHOOTYPESH
#define YAHOOTYPESH

#include <qglobal.h>

namespace Yahoo
{
	enum Service 
	{ 
		/* these are easier to see in hex */
		ServiceLogon = 1,
		ServiceLogoff,
		ServiceIsAway,
		ServiceIsBack,
		ServiceIdle, /* 5 (placemarker) */
		ServiceMessage,
		/* TODO switch all the rest to CamelCase */
		ServiceIDACT,
		ServiceIDDEACT,
		ServiceMailStat,
		ServiceUserStat, /* 0xa */
		ServiceNEWMAIL,
		ServiceCHATINVITE,
		ServiceCALENDAR,
		ServiceNEWPERSONALMAIL,
		ServiceNEWCONTACT,
		ServiceADDIDENT, /* 0x10 */
		ServiceADDIGNORE,
		ServicePING,
		ServiceGOTGROUPRENAME, /* < 1, 36(old), 37(new) */
		ServiceSYSMESSAGE = 0x14,
		ServicePASSTHROUGH2 = 0x16,
		ServiceCONFINVITE = 0x18,
		ServiceCONFLOGON,
		ServiceCONFDECLINE,
		ServiceCONFLOGOFF,
		ServiceCONFADDINVITE,
		ServiceCONFMSG,
		ServiceCHATLOGON,
		ServiceCHATLOGOFF,
		ServiceCHATMSG = 0x20,
		ServiceGAMELOGON = 0x28,
		ServiceGAMELOGOFF,
		ServiceGAMEMSG = 0x2a,
		ServiceFILETRANSFER = 0x46,
		ServiceVOICECHAT = 0x4A,
		ServiceNOTIFY,
		ServiceVERIFY,
		ServiceP2PFILEXFER,
		ServicePEERTOPEER = 0x4F,	/* Checks if P2P possible */
		ServiceWEBCAM,
		ServiceAUTHRESP = 0x54,
		ServiceLIST,
		ServiceAUTH = 0x57,
		ServiceADDBUDDY = 0x83,
		ServiceREMBUDDY,
		ServiceIGNORECONTACT,	/* > 1, 7, 13 < 1, 66, 13, 0*/
		ServiceREJECTCONTACT,
		ServiceGROUPRENAME = 0x89, /* > 1, 65(new), 66(0), 67(old) */
		ServiceCHATONLINE = 0x96, /* > 109(id), 1, 6(abcde) < 0,1*/
		ServiceCHATGOTO,
		ServiceCHATJOIN,	/* > 1 104-room 129-1600326591 62-2 */
		ServiceCHATLEAVE,
		ServiceCHATEXIT = 0x9b,
		ServiceCHATLOGOUT = 0xa0,
		ServiceCHATPING,
		ServiceCOMMENT = 0xa8
	};
	
	enum Status 
	{
		StatusAvailable = 0,
		StatusBRB,
		StatusBUSY,
		StatusNOTATHOME,
		StatusNOTATDESK,
		StatusNOTINOFFICE,
		StatusONPHONE,
		StatusONVACATION,
		StatusOUTTOLUNCH,
		StatusSTEPPEDOUT,
		StatusINVISIBLE = 12,
		StatusCUSTOM = 99,
		StatusIDLE = 999,
		StatusOffline = 0x5a55aa56, /* don't ask */
		StatusNotify = 0x16
	};

	typedef Q_UINT8 BYTE;
	typedef Q_UINT16 WORD;
	typedef Q_UINT32 DWORD;
}

#endif
