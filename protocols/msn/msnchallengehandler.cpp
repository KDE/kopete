/*
    msnchallengehandler.h - Computes a msn challenge response hash key.

    Copyright (c) 2005 by Gregg Edghill       <gregg.edghill@gmail.com>
    Kopete    (c) 2003-2005 by The Kopete developers <kopete-devel@kde.org>

    Portions taken from
    	http://msnpiki.msnfanatic.com/index.php/MSNP11:Challenges

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "msnchallengehandler.h"

#include <kdebug.h>
#include <kmdcodec.h>

MsnChallengeHandler::MsnChallengeHandler(const QString& productKey, const QString& productId)
{
	m_productKey = productKey;
	m_productId  = productId;
}


MsnChallengeHandler::~MsnChallengeHandler()
{
	kdDebug(14140) << k_funcinfo << endl;
}

QString MsnChallengeHandler::computeHash(const QString& challenge)
{
  	// Step One: THe MD5 Hash.

  	// Combine the received challenge string with the product key.
 	KMD5 md5((challenge + m_productKey).utf8());
 	QCString md5Hash = md5.hexDigest();

 	QString initHash(md5Hash);
 	kdDebug(14140) << k_funcinfo << "md5: " << initHash << endl;

 	int md5HashArray[4];
 	for(int i=0; i < 4; i++)
 	{
 		md5HashArray[i] = hexSwap(initHash.mid(0, 8)).toUInt(0, 16) & 0x7FFFFFFF;
 		kdDebug(14140) << k_funcinfo << ("0x" + hexSwap(initHash.mid(0, 8))) << " " << md5HashArray[i] << endl;
 		initHash = initHash.remove(0, 8);
 	}

	// Step Two: Create the challenge string key
	QString chlString = challenge + m_productId;
	// Pad to multiple of 8.
	chlString = chlString.leftJustify(chlString.length() + (8 - chlString.length() % 8), '0');

	kdDebug(14140) << k_funcinfo << "challenge key: " << chlString << endl;
	
	int chlArray[chlString.length() / 4];
	for(uint i=0; i < chlString.length() / 4; i++)
	{
		QString sNum = chlString.mid(i*4, 4);
		QString sNumHex = "";

		// Go through the number string, determining the hex equivalent of each value
		// and add that to our new hex string for this number.
		for(uint j=0; j < sNum.length(); j++) {
			sNumHex += QString("%1").arg((int)sNum[j].latin1(), 0, 16);
		}

		// swap because of the byte ordering issue.
		sNumHex = hexSwap(sNumHex);
		// Assign the converted number.
		chlArray[i] = sNumHex.toInt(0, 16);
		kdDebug(14140) << k_funcinfo << sNum << (": 0x"+sNumHex) << " " << chlArray[i] << endl;
	}

	// Step Three: Create the 64-bit key.
	long long high = 0L;
	long long low  = 0L;
	
	kdDebug(14140) << k_funcinfo << "Creating 64-bit key.." << endl;

	for(uint i=0; i < chlString.length() / 4; i += 2)
	{
		long long temp = chlArray[i];
		temp = (temp * 242854337) % 0x7FFFFFFF;
		temp += high;
		temp = md5HashArray[0] * temp + md5HashArray[1];
		temp = temp % 0x7FFFFFFF;

		// Now the high part of the key.
		high = chlArray[i + 1];
		high = (high + temp) % 0x7FFFFFFF;
		high = md5HashArray[2] * high + md5HashArray[3];
		high = high % 0x7FFFFFFF;

		// Now the low part of the key.
		low = low + high + temp;
	}

	bool OK;
	
	// Get the final low and high values.
	high = (high + md5HashArray[1]) % 0x7FFFFFFF;
	
	QString sHigh;
	sHigh.setNum(high, 16);
	
	if(sHigh.length() % 2)
		sHigh = hexSwap(sHigh.rightJustify(sHigh.length() + (2 - sHigh.length() % 2), '0'));
	else
		sHigh = hexSwap(sHigh);

	high = sHigh.toLongLong(&OK, 16);
	if(!OK)
	{
		kdDebug(14140) << k_funcinfo << "high value conversion error" << endl;
		return "0";
	}

	low = (low + md5HashArray[3]) % 0x7FFFFFFF;
	
	QString sLow;
	sLow.setNum(low, 16);
	
	if(sLow.length() % 2)
		sLow = hexSwap(sLow.rightJustify(sLow.length() + (2 - sLow.length() % 2), '0'));
	else
		sLow = hexSwap(sLow);
		
	low = sLow.toLongLong(&OK, 16);
	if(!OK)
	{
		kdDebug(14140) << k_funcinfo << "low value conversion error" << endl;
		return "0";
	}
 	
	// bit shift high to the top end of a QWORD, add low on the end to form a full QWORD.
	long long key = (high << 32) + low;

	kdDebug(14140) << k_funcinfo << "key: " << key << endl;
	
	// Step Four: Create the final hash key.
		
	QString finalHash = QString("%1").arg(QString(md5Hash.mid(0, 16)).toULongLong(0, 16)^key, 0, 16) + QString("%1").arg(QString(md5Hash.mid(16, 16)).toULongLong(0, 16)^key, 0, 16);

	return finalHash;
}

QString MsnChallengeHandler::hexSwap(const QString& in)
{
	QString sHex = in, swapped;
	while(sHex.length() > 0)
	{
		swapped = swapped + sHex.mid(sHex.length() - 2, 2);
		sHex = sHex.remove(sHex.length() - 2, 2);
	}
	return swapped;
}

QString MsnChallengeHandler::productId()
{
	return m_productId;
}

#include "msnchallengehandler.moc"
