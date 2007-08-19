/*
    slprequest.h - Peer to Peer Session Layer Protocol Request class

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef CLASS_P2P__SLPREQUEST_H
#define CLASS_P2P__SLPREQUEST_H

#include "slpmessage.h"

namespace PeerToPeer
{

/**
 * @brief Represents a session layer protocol request.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class SlpRequest : public SlpMessage
{
	public:
		/** @brief Creates a new instance of the SlpRequest class. */
		SlpRequest(const QString& version="1.0");
		SlpRequest(const QString& method, const QString& requestUri, const QString& version="1.0");
		SlpRequest(const SlpRequest& other);
		SlpRequest & operator=(const SlpRequest& other);
		virtual ~SlpRequest();

		const QString method() const;
		void setMethod(const QString& method);
		const QString requestUri() const;
		void setRequestUri(const QString& requestUri);
		virtual const QString startLine() const;

	private:
		class SlpRequestPrivate;
		SlpRequestPrivate *d;

}; // SlpRequest
}

#endif
