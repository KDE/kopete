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

#ifndef MSNCHALLENGEHANDLER_H
#define MSNCHALLENGEHANDLER_H

#include <qobject.h>
#include <qvaluevector.h>

/**
 * Provides a simple way to compute a msn challenge response hash key.
 *
 * @author Gregg Edghill
 */
class MSNChallengeHandler : public QObject
{
Q_OBJECT
public:
	MSNChallengeHandler(const QString& productKey, const QString& productId);
    ~MSNChallengeHandler();

	/**
	 * Computes the response hash string for the specified challenge string.
	 */
    QString computeHash(const QString& challengeString);

	/**
	 * Returns the product id used by the challenge handler.
	 */
    QString productId();
    
private:

	/**
	 * Creates a 64-bit hash key.
	 */
	 Q_INT64 createHashKey(const QValueVector<Q_INT32>& md5Integers, const QValueVector<Q_INT32>& challengeIntegers);
	 
	/**
	 * Swaps the bytes in a hex string.
	 */
	QString hexSwap(const QString& in);
	
	QString m_productKey;
	QString m_productId;
};

#endif
