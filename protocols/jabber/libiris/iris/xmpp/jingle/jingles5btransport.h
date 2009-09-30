/*
 * jingles5btransport.h - Jingle SOCKS5 Transport
 * Copyright (C) 2009 - Detlev Casanova <detlev.casanova@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef JINGLE_S5B_TRANSPORT_H
#define JINGLE_S5B_TRANSPORT_H

#include "jingletransport.h"

namespace XMPP
{
	class JingleContent;
	class JingleS5BTransport : public JingleTransport
	{ 
	public:
		JingleS5BTransport(Mode mode, JingleContent *parent, const QDomElement& elem = QDomElement());
		~JingleS5BTransport();

		/*
		 * Called when the parent has been set so the transport can be initiated.
		 */
		virtual void init();
		
		/*
		 * Called when the transport must be started (when negotiation can begin)
		 */
		virtual void start();

		/*
		 * Adds transport info (mostly a candidate). Doing so will try to
		 * connect to this candidate.
		 */
		virtual void addTransportInfo(const QDomElement& e);

		/*
		 * Returns the transport namespace of this content.
		 */
		virtual QString transportNS() const;
	
		/*
		 * Returns the transport in an Xml form so it cam be added in a stanza.
		 * The TransportType argument tells how the transport should be generated (with candidates, with other information)
		 * FIXME:drop the TransportType, this method should always return the XML element as sent in session-initiate.
		 */
		virtual QDomElement toXml(TransportType);

		/*
		 * Set the number of component that must be established by the transport. (e.g. For RTP, 2 components : RTP + RTCP)
		 */
		virtual void setComponentCount(int);
		
		/*
		 * This method writes data one the given channel (corresponding to the component)
		 * FIXME:review Channels/Components
		 */
		virtual void writeDatagram(const QByteArray& data, Channel c);
		
		/*
		 * Reads the available data on the given Channel.
		 * FIXME:review Channels/Components
		 */
		virtual QByteArray readAll(Channel c = Rtp);
	
	private:
		class Private;
		Private *d;
	};
}

#endif //JINGLE_S5B_TRANSPORT_H
