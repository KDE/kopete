/*
   challengetask.cpp - Answer to challenge string given by Notification Server.

   Copyright (c) 2006 by Michaël Larouche <larouche@kde.org>

   Portions used from Kopete with Gregg's approval on LGPL license:
   Copyright (c) 2005 by Gregg Edghill       <gregg.edghill@gmail.com>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/
#include "Papillon/Tasks/ChallengeTask"

// Qt includes
#include <QtCore/QLatin1String>
#include <QtCore/QByteArray>
#include <QtCore/QDataStream>
#include <QtCore/QVector>
#include <QtDebug>

// QCA includes
#include <QtCrypto>

// Papillon includes
#include "Papillon/Transfer"
#include "Papillon/Connection"

namespace Papillon
{

const QString challengeProductKey = QString("O4BG@C7BWLYQX?5G");
const QString challengeProductId = QString("PROD01065C%ZFN6F");

class ChallengeTask::Private
{
public:
	Private()
	{}

	/**
	 * Creates a 64-bit hash key.
	 */
	 qint64 createHashKey(const QVector<qint32>& md5Integers, const QVector<qint32>& challengeIntegers);
	/**
	 * Swaps the bytes in a hex string.
	 */
	QString hexSwap(const QString& in);
};

ChallengeTask::ChallengeTask(Papillon::Task *parent)
 : Papillon::Task(parent), d(new Private)
{}

ChallengeTask::~ChallengeTask()
{
	delete d;
}

bool ChallengeTask::take(Transfer *transfer)
{
	if( transfer->command() == QLatin1String("CHL") )
	{
		QString challenge = transfer->arguments()[0];
		
		QString challengeHash = createChallengeHash(challenge);

		Transfer *challengeResult = new Transfer(Transfer::PayloadTransfer | Transfer::TransactionTransfer);
		challengeResult->setCommand( QLatin1String("QRY") );
		challengeResult->setTransactionId( QString::number( connection()->transactionId() ) );
		challengeResult->setPayloadData( challengeHash.toUtf8() );

		challengeResult->setArguments( challengeProductId );

		send(challengeResult);

		return true;
	}

	return false;
}


QString ChallengeTask::createChallengeHash(const QString &challengeString)
{
	// Step One: THe MD5 Hash.

	if( !QCA::isSupported("md5") )
	{
		qDebug() << PAPILLON_FUNCINFO << "ERROR: MD5 not supported !";
	}
	Q_ASSERT( QCA::isSupported("md5") );

  	// Combine the received challenge string with the product key.
 	QByteArray digest = QCA::arrayToHex( QCA::Hash("md5").hash( (challengeString + challengeProductKey).toUtf8() ) ).toUtf8();

 	qDebug() << PAPILLON_FUNCINFO << "md5: " << digest;

 	QVector<qint32> md5Integers(4);
 	for(qint32 i=0; i < md5Integers.count(); i++)
 	{
 		md5Integers[i] = d->hexSwap(digest.mid(i*8, 8)).toUInt(0, 16) & 0x7FFFFFFF;
 		qDebug() << PAPILLON_FUNCINFO << ("0x" + d->hexSwap(digest.mid(i*8, 8))) << " " << md5Integers[i];
 	}

	// Step Two: Create the challenge string key

	QString challengeKey = challengeString + challengeProductId;
	// Pad to multiple of 8.
	challengeKey = challengeKey.leftJustified(challengeKey.length() + (8 - challengeKey.length() % 8), '0');

	qDebug() << PAPILLON_FUNCINFO << "challenge key: " << challengeKey;

	QVector<qint32> challengeIntegers(challengeKey.length() / 4);
	for(qint32 i=0; i < challengeIntegers.count(); i++)
	{
		QString sNum = challengeKey.mid(i*4, 4), sNumHex;

		// Go through the number string, determining the hex equivalent of each value
		// and add that to our new hex string for this number.
		for(int j=0; j < sNum.length(); j++) {
			sNumHex += QString::number((int)sNum[j].toLatin1(), 16);
		}

		// swap because of the byte ordering issue.
		sNumHex = d->hexSwap(sNumHex);
		// Assign the converted number.
		challengeIntegers[i] = sNumHex.toInt(0, 16);
		qDebug() << PAPILLON_FUNCINFO << sNum << (": 0x"+sNumHex) << " " << challengeIntegers[i];
	}

	// Step Three: Create the 64-bit hash key.

	// Get the hash key using the specified arrays.
	qint64 key = d->createHashKey(md5Integers, challengeIntegers);
	qDebug() << PAPILLON_FUNCINFO << "key: " << key;

	// Step Four: Create the final hash key.

	QString upper = QString::number(QString(digest.mid(0, 16)).toULongLong(0, 16)^key, 16);
	if(upper.length() % 16 != 0)
		upper = upper.rightJustified(upper.length() + (16 - upper.length() % 16), '0');

	QString lower = QString::number(QString(digest.mid(16, 16)).toULongLong(0, 16)^key, 16);
	if(lower.length() % 16 != 0)
		lower = lower.rightJustified(lower.length() + (16 - lower.length() % 16), '0');

	return (upper + lower);
}

qint64 ChallengeTask::Private::createHashKey(const QVector<qint32>& md5Integers, const QVector<qint32>& challengeIntegers)
{
	qDebug() << PAPILLON_FUNCINFO << "Creating 64-bit key.";

	qint64 magicNumber = 0x0E79A9C1L, high = 0L, low = 0L;
		
	for(int i=0; i < challengeIntegers.count(); i += 2)
	{
		qint64 temp = ((challengeIntegers[i] * magicNumber) % 0x7FFFFFFF) + high;
		temp = ((temp * md5Integers[0]) + md5Integers[1]) % 0x7FFFFFFF;

		high = (challengeIntegers[i + 1] + temp) % 0x7FFFFFFF;
		high = ((high * md5Integers[2]) + md5Integers[3]) % 0x7FFFFFFF;

		low += high + temp;
	}

	high = (high + md5Integers[1]) % 0x7FFFFFFF;
	low  = (low  + md5Integers[3]) % 0x7FFFFFFF;

	QByteArray tempArray;
	tempArray.reserve(8);

	QDataStream buffer(&tempArray,QIODevice::ReadWrite);
	buffer.setByteOrder(QDataStream::LittleEndian);
	buffer << (qint32)high;
	buffer << (qint32)low;

	buffer.device()->reset();
	buffer.setByteOrder(QDataStream::BigEndian);
	qint64 key;
	buffer >> key;
	
	return key;
}

QString ChallengeTask::Private::hexSwap(const QString& in)
{
	QString sHex = in, swapped;
	while(sHex.length() > 0)
	{
		swapped = swapped + sHex.mid(sHex.length() - 2, 2);
		sHex = sHex.remove(sHex.length() - 2, 2);
	}
	return swapped;
}

}

#include "challengetask.moc"
