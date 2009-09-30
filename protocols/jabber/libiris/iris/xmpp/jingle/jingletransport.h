/*
 * jingletransport.h - Jingle Transport
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

#ifndef JINGLETRANSPORT_H
#define JINGLETRANSPORT_H

#include <QObject>
#include <QDomElement>

namespace XMPP
{
	class JingleContent;
	class JingleSession;
	class Task;
	class JingleTransport : public QObject
	{
		Q_OBJECT
	public :
		
		/*
		 * Initiator means, as we created it, that we are the initiator.
		 * Responder means, as we created it, that we are the responder.
		 * Unknown is used when we don't know (but we always know).
		 */
		enum Mode {
			Initiator = 0,
			Responder,
			Unknown
		};
		
		enum Channel {
			Rtp = 0,
			Rtcp,
			Srtp,
			Srtcp,
			Socks5,
			IBB
		};
		
		JingleTransport(Mode mode, JingleContent *parent = 0);
		virtual ~JingleTransport();
		
		/*
		 * This enum is used to tell contentElement() what candidates
		 * to set in the returned QDomElement.
		 *	NoCandidate is for no candidates.
		 *	[Local|Remote]Candidates is all [Local|Remote] candidates
		 *	UsedCandidate is the candidate that works for this content.
		 */
		/*enum CandidateType {
			NoCandidate = 0,
			LocalCandidates,
			RemoteCandidates,
			UsedCandidate
		};*/
		
		enum TransportType {
			NoCandidate = 0,
			LocalCandidates, // Meaning candidates
			AcceptedCandidate // Meaning a form of the transport to accept the session.
			//UsedTransport
		};

		/*
		 * Set the Transport's parent. This will add the transport to the provided parent.
		 */
		void setParent(JingleContent *c);
		
		static JingleTransport* createFromXml(const QDomElement&, Mode mode, JingleContent *parent);

		JingleSession *parentSession() const;
		
		/*
		 * Called when the parent has been set so the transport can be initiated.
		 */
		virtual void init() = 0;
		
		/*
		 * Called when the transport must be started (when negotiation can begin)
		 */
		virtual void start() = 0;

		/*
		 * Adds transport info (mostly a candidate). Doing so will try to
		 * connect to this candidate.
		 */
		virtual void addTransportInfo(const QDomElement& e) = 0;

		/*
		 * Returns the transport namespace of this content.
		 */
		virtual QString transportNS() const = 0;
	
		/*
		 * Returns the transport in an Xml form so it cam be added in a stanza.
		 * The TransportType argument tells how the transport should be generated (with candidates, with other information)
		 * FIXME:drop the TransportType, this method should always return the XML element as sent in session-initiate.
		 */
		virtual QDomElement toXml(TransportType) = 0;

		/*
		 * Set the number of component that must be established by the transport. (e.g. For RTP, 2 components : RTP + RTCP)
		 */
		virtual void setComponentCount(int) = 0;

		/*
		 * When the failure() signal is emitted, this method can be called to retreive a fallback transport.
		 * Returns NULL if no fallback is possible/available.
		 *
		 * When subclassing JingleTransport, this method returns NULL if not reimplemented.
		 */
		virtual JingleTransport *fallbackTransport();
		
		/*
		 * This method writes data one the given channel (corresponding to the component)
		 * FIXME:review Channels/Components
		 */
		virtual void writeDatagram(const QByteArray& data, Channel c) = 0;
		
		/*
		 * Reads the available data on the given Channel.
		 * FIXME:review Channels/Components
		 */
		virtual QByteArray readAll(Channel c = Rtp) = 0;
		
		/*
		 * Returns the transport type of the content content.
		 */
		//FIXME:currently a QString, this could be an enum.
		static QString transportNS(const QDomElement& elem);

		Mode mode() const;
		JingleContent *parent() const;

		QDomElement transport() const;
		void setTransport(const QDomElement& t);

		bool isConnected() const;
		void setConnected(bool); /*Visibility to set on only subclasses*/

		Task* rootTask();

	signals:
		/**
		 * Emitted when the transport has succeeded in creating a connection.
		 */
		void success();

		/**
		 * Emitted when the transport was unable to create a connection.
		 */
		void failure();

		/**
		 * Emitted when data is ready to be read on the network.
		 * The argument is the channel on which data has arrived (0 = RTP, 1 = RTCP).
		 */
		void readyRead(int);

	private :
		class Private;
		Private *d;
	};
}
#endif //JINGLETRANSPORT_H
