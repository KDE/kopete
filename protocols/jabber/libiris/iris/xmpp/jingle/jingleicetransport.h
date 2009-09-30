/*
 * jingleicetransport.h
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
#ifndef JINGLE_ICE_TRANSPORT_H
#define JINGLE_ICE_TRANSPORT_H

#include <QObject>

#include "im.h"
#include "jinglecontent.h"
#include "jingletransport.h"
#include "ice176.h"

class QHostAddress;
class QDomElement;
class QUdpSocket;

namespace XMPP
{
	/*
	 * This class contains all information about a particular transport in a jingle content.
	 * This is the Ice-udp jingle transport.
	 */
	
	class JingleContent;
	class JingleIceTransport : public JingleTransport
	{
		Q_OBJECT
	public:
		JingleIceTransport(Mode mode, JingleContent *parent = 0, const QDomElement& elem = QDomElement());
		~JingleIceTransport();

		virtual void init();
		
		virtual void start();
		virtual void setComponentCount(int);
		
		/*
		 * Adds transport info (mostly a candidate). Doing so will try to
		 * connect to this candidate.
		 */
		virtual void addTransportInfo(const QDomElement&);

		/*
		 * Returns the transport type of this content.
		 */
		virtual QString transportNS() const;
		
		/*
		 * This is called to write RTP data on the established stream.
		 */
		virtual void writeDatagram(const QByteArray&, Channel channel = Rtp);

		/* 
		 * Get all data available on the socket.
		 * Usually, this will be an RTP packet.
		 */
		virtual QByteArray readAll(Channel channel = Rtp);

		/*
		 * Gets a list containing all local addresses.
		 */
		QList<Ice176::LocalAddress> getAddresses();
		
		QDomElement toXml(TransportType type);
		
	private slots:
		void slotIceStarted();
		void slotIceComponentReady(int);
		void slotIceLocalCandidatesReady(const QList<XMPP::Ice176::Candidate>&);
	
	signals:
		void started();

	private:
		class Private;
		Private *d;

		/* 
		 * Transforms a candidate xml element into a Ice176::Candidate.
		 */
		Ice176::Candidate xmlToCandidate(const QDomElement& c);

		/*
		 * Quite obvious.
		 */
		QDomElement candidateToXml(const Ice176::Candidate& candidate);

		void sendLocalCandidates(const QList<XMPP::Ice176::Candidate>& candidates);
	};
}

#endif
