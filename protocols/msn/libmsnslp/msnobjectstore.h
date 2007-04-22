/*
    msnobjectstore.h - Peer to Peer Msn Object Store class

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef CLASS_P2P__MSNOBJECTSTORE_H
#define CLASS_P2P__MSNOBJECTSTORE_H

namespace PeerToPeer
{

/**
 * @brief Represent a data store used to manage msn objects.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class MsnObjectStore
{
	public :
		~MsnObjectStore();

	private:
		MsnObjectStore();

	private:
		class MsnObjectStorePrivate;
		MsnObjectStorePrivate *d;

}; // MsnObjectStore

}

#endif
