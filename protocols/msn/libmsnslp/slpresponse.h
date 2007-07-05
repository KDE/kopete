/*
    slpresponse.h - Peer to Peer Session Layer Protocol Response class

    Copyright (c) 2006 by Gregg Edghill     <gregg.edghill@gmail.com>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; version 2 of the License.               *
    *                                                                       *
    *************************************************************************
*/

#ifndef CLASS_P2P__SLPRESPONSE_H
#define CLASS_P2P__SLPRESPONSE_H

#include "slpmessage.h"

namespace PeerToPeer
{

/**
 * @brief Represents a session layer protocol response message.
 *
 * @author Gregg Edghill <gregg.edghill@gmail.com>
 */
class SlpResponse : public SlpMessage
{
	public :
		/** @brief Defines the status codes returned for a given request. */
		enum StatusCode {OK=200, NotFound=404, NoSuchCall=481, InternalError=500, VersionNotSupported=505, Decline=603};

	public :
		/** @brief Creates a new instance of the SlpResponse class. */
		SlpResponse(const QString& version="1.0");
		SlpResponse(Q_INT32 statusCode, const QString& statusDescription, const QString& version="1.0");
		SlpResponse(const SlpResponse& other);
		SlpResponse & operator=(const SlpResponse& other);
		virtual ~SlpResponse();

		/** @brief Get the status code returned with the response. */
		const Q_INT32 statusCode() const;
		void setStatusCode(Q_INT32 statusCode);
		/** @brief Get the text that describes the status code returned with the response. */
		const QString statusDescription() const;
		void setStatusDescription(const QString& statusDescription);
		virtual const QString startLine() const;

	private:
		class SlpResponsePrivate;
		SlpResponsePrivate *d;

}; // SlpResponse
}

#endif
