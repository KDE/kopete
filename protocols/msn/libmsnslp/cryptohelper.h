/*
   cryptohelper.h - PeerToPeer Cryptography Helper class..

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

#ifndef CLASS_P2P__CRYPTOHELPER_H
#define CLASS_P2P__CRYPTOHELPER_H

#include <quuid.h>

namespace PeerToPeer
{

/**
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class CryptoHelper
{
	public:
		static QUuid hashNonce(const QUuid& nonce);

	private:
		CryptoHelper();

}; //CryptoHelper
}

#endif
