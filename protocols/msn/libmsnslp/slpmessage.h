/*
    slpmessage.h - Peer to Peer Session Layer Protocol Message class

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef CLASS_P2P__SLPMESSAGE_H
#define CLASS_P2P__SLPMESSAGE_H

#include "message.h"

namespace PeerToPeer
{

/**
 * @brief Represents the session layer protocol message base class.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class SlpMessage : public Message
{
	public :
		/** @brief Creates a new instance of the SlpMessage class. */
		SlpMessage(const QString& version);
		SlpMessage(const SlpMessage& other);
		virtual ~SlpMessage();

		virtual const QString from() const;
		virtual const QString startLine() const = 0;
		virtual const QString to() const;

}; // SlpMessage
}

#endif
