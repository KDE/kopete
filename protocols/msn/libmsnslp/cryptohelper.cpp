/*
   cryptohelper.cpp - PeerToPeer Cryptography Helper class.

   Copyright (c) 2006 by Gregg Edghill <gregg.edghill@gmail.com>

   *************************************************************************
   *                                                                       *
   * This library is free software; you can redistribute it and/or         *
   * modify it under the terms of the GNU Lesser General Public            *
   * License as published by the Free Software Foundation; either          *
   * version 2 of the License, or (at your option) any later version.      *
   *                                                                       *
   *************************************************************************
*/

#include "cryptohelper.h"
#include "sha1.h"
#include <qglobal.h>
#include <qregexp.h>

namespace PeerToPeer
{

// FIXME There seems to be an inconsistency in the SHA1 hash on windows
// and that on linux. e.g. SHA1 hash of nonce {7356BA62-7DE3-4687-A132DC315E8E50F5}
// on windows is {A1984D80-C440-9353-AF81-F8B7EAB3862B}
// on linux is   {38596524-C854-4846-106D-29641A90485B}
QUuid CryptoHelper::hashNonce(const QUuid& nonce)
{
	// Convert the Uuid to a byte array and SHA1 hash the byte array.
	QByteArray hash = SHA1::hashString(nonce.toString().remove(QRegExp("[\\{\\}-]")).utf8());
	// Get the Uuid byte parameters from the byte array.
	const Q_UINT32 l = (Q_UINT32(hash[3]) << 24) + (Q_UINT32(hash[2]) << 16) +
		(Q_UINT32(hash[1]) << 8) + Q_UINT32(hash[0]);
	const Q_UINT16 w1 = (Q_UINT16(hash[5]) << 8) + (Q_UINT16(hash[4]));
	const Q_UINT16 w2 = (Q_UINT16(hash[7]) << 8) + (Q_UINT16(hash[6]));
	const Q_UINT8 b1= hash[8], b2= hash[9], b3= hash[10], b4= hash[11];
	const Q_UINT8 b5= hash[12], b6= hash[13], b7= hash[14], b8= hash[15];

	return QUuid(l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8);
}

}
