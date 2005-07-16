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

#include <qdatastream.h>

#include <kdebug.h>
#include <kmdcodec.h>

MSNChallengeHandler::MSNChallengeHandler(const QString& productKey, const QString& productId)
{
	m_productKey = productKey;
	m_productId  = productId;
}


MSNChallengeHandler::~MSNChallengeHandler()
{
	kdDebug(14140) << k_funcinfo << endl;
}

QString MSNChallengeHandler::computeHash(const QString& challengeString)
{
  	// Step One: THe MD5 Hash.

  	// Combine the received challenge string with the product key.
 	KMD5 md5((challengeString + m_productKey).utf8());
 	QCString digest = md5.hexDigest();

 	kdDebug(14140) << k_funcinfo << "md5: " << digest << endl;

 	QValueVector<Q_INT32> md5Integers(4);
 	for(Q_UINT32 i=0; i < md5Integers.count(); i++)
 	{
 		md5Integers[i] = hexSwap(digest.mid(i*8, 8)).toUInt(0, 16) & 0x7FFFFFFF;
 		kdDebug(14140) << k_funcinfo << ("0x" + hexSwap(digest.mid(i*8, 8))) << " " << md5Integers[i] << endl;
 	}

	// Step Two: Create the challenge string key

	QString challengeKey = challengeString + m_productId;
	// Pad to multiple of 8.
	challengeKey = challengeKey.leftJustify(challengeKey.length() + (8 - challengeKey.length() % 8), '0');

	kdDebug(14140) << k_funcinfo << "challenge key: " << challengeKey << endl;

	QValueVector<Q_INT32> challengeIntegers(challengeKey.length() / 4);
	for(Q_UINT32 i=0; i < challengeIntegers.count(); i++)
	{
		QString sNum = challengeKey.mid(i*4, 4), sNumHex;

		// Go through the number string, determining the hex equivalent of each value
		// and add that to our new hex string for this number.
		for(uint j=0; j < sNum.length(); j++) {
			sNumHex += QString::number((int)sNum[j].latin1(), 16);
		}

		// swap because of the byte ordering issue.
		sNumHex = hexSwap(sNumHex);
		// Assign the converted number.
		challengeIntegers[i] = sNumHex.toInt(0, 16);
		kdDebug(14140) << k_funcinfo << sNum << (": 0x"+sNumHex) << " " << challengeIntegers[i] << endl;
	}

	// Step Three: Create the 64-bit hash key.

	// Get the hash key using the specified arrays.
	Q_INT64 key = createHashKey(md5Integers, challengeIntegers);
	kdDebug(14140) << k_funcinfo << "key: " << key << endl;

	// Step Four: Create the final hash key.

	QString upper = QString::number(QString(digest.mid(0, 16)).toULongLong(0, 16)^key, 16);
	if(upper.length() % 16 != 0)
		upper = upper.rightJustify(upper.length() + (16 - upper.length() % 16), '0');

	QString lower = QString::number(QString(digest.mid(16, 16)).toULongLong(0, 16)^key, 16);
	if(lower.length() % 16 != 0)
		lower = lower.rightJustify(lower.length() + (16 - lower.length() % 16), '0');

	return (upper + lower);
}

Q_INT64 MSNChallengeHandler::createHashKey(const QValueVector<Q_INT32>& md5Integers,
	const QValueVector<Q_INT32>& challengeIntegers)
{
	kdDebug(14140) << k_funcinfo << "Creating 64-bit key." << endl;

	Q_INT64 magicNumber = 0x0E79A9C1L, high = 0L, low = 0L;
		
	for(uint i=0; i < challengeIntegers.count(); i += 2)
	{
		Q_INT64 temp = ((challengeIntegers[i] * magicNumber) % 0x7FFFFFFF) + high;
		temp = ((temp * md5Integers[0]) + md5Integers[1]) % 0x7FFFFFFF;

		high = (challengeIntegers[i + 1] + temp) % 0x7FFFFFFF;
		high = ((high * md5Integers[2]) + md5Integers[3]) % 0x7FFFFFFF;

		low += high + temp;
	}

	high = (high + md5Integers[1]) % 0x7FFFFFFF;
	low  = (low  + md5Integers[3]) % 0x7FFFFFFF;

	QDataStream buffer(QByteArray(8), IO_ReadWrite);
	buffer.setByteOrder(QDataStream::LittleEndian);
	buffer << (Q_INT32)high;
	buffer << (Q_INT32)low;

	buffer.device()->reset();
	buffer.setByteOrder(QDataStream::BigEndian);
	Q_INT64 key;
	buffer >> key;
	
	return key;
}

QString MSNChallengeHandler::hexSwap(const QString& in)
{
	QString sHex = in, swapped;
	while(sHex.length() > 0)
	{
		swapped = swapped + sHex.mid(sHex.length() - 2, 2);
		sHex = sHex.remove(sHex.length() - 2, 2);
	}
	return swapped;
}

QString MSNChallengeHandler::productId()
{
	return m_productId;
}

#include "msnchallengehandler.moc"
