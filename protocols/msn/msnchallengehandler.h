/*
    msnchallengehandler.h - Computes a msn challenge response hash key.

    Copyright (c) 2005 by Gregg Edghill       <edghill@kde.org>
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

/**
@author Gregg Edghill
*/

/**
 * Provides a simple way to compute a msn challenge response hash key.
 */
class MsnChallengeHandler : public QObject
{
Q_OBJECT
public:
	MsnChallengeHandler(const QString& productKey, const QString& productId);
    ~MsnChallengeHandler();

	/**
	 *Computes the response hash value for the specified challenge string.
	 */
    QString computeHash(const QString& challenge);

	/**
	 * Returns the product id used by the challenge handler.
	 */
    QString productId();
    
private:
	/**
	 * Swaps the bytes in a hex string.
	 */
	QString hexSwap(QString in);
	
	QString m_productKey;
	QString m_productId;
};

#endif
