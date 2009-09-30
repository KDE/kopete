/*
 * jinglecontent.cpp - Jingle content
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
#ifndef JINGLE_CONTENT_H
#define JINGLE_CONTENT_H

#include <QObject>

#include "im.h"
#include "jingleapplication.h"
#include "jingletransport.h"

class QHostAddress;
class QDomElement;
class QUdpSocket;
namespace XMPP
{
	/*
	 * This class manages a jingle content. It is an essential part of a jingle session.
	 * 
	 * FIXME: when a content is removed, the only way to now it is by
	 * connecting to the destroyed() signal because the JingleContent is deleted by the parent JingleSession.
	 */
	class Task;
	class JingleSession;
	class JingleContent : public QObject
	{
		Q_OBJECT
	public:
		enum ReasonType {
			UnsupportedApplications = 0,
			UnsupportedApplication = 0,
			UnsupportedTransports,
			UnsupportedTransport,
			NoReason
		};

		enum Senders {
			Both = 0,
			Initiator,
			Responder
		};

		JingleContent(JingleSession *parent = 0);
		virtual ~JingleContent();

		void setRootTask(Task *rt);

		void setParent(JingleSession *parent);

		/*
		 * Set all the supported application.
		 * When a session initiate comes in, if we don't find anything
		 * compatible in the list, it means that the application is
		 * not supported
		 */
		void setSupportedApplications(QList<JingleApplication*>);
		void setApplication(JingleApplication*);

		/*
		 * Returns the currently used application.
		 *
		 * When the session is in the Pending state, it is the local application.
		 * When the session is in the Active state, it is the accepted application.
		 */
		JingleApplication *application() const;

		void setTransport(JingleTransport*);
		JingleTransport *transport() const;

		/**
		 * Set the transport info in the transport.
		 */
		void addTransportInfo(const QDomElement& e);

		/*
		 * Set the creator of this content, the creator only accept 2 values :
		 * 	* initiator
		 * 	* responder
		 */
		void setCreator(const QString&);
		
		/*
		 * Set the direction for this jingle content.
		 * The direction is set by the peer(s) that send(s) data.
		 * Values can be "initiator", "responder" or "both".
		 */
		void setSenders(const Senders s);

		/*
		 * Set this content's name.
		 */
		void setName(const QString&);

		/*
		 * Fill this content from a QDomElement.
		 * Calling this method will automatically add the content(s)
		 * to the remote contents list.
		 */
		void fromElement(const QDomElement&);

		/*
		 * Return a QDomElement with the content element and all it's children
		 * so it's ready to be sent.
		 */
		QDomElement contentElement(JingleTransport::TransportType tType = JingleTransport::NoCandidate, JingleApplication::ApplicationType aType = JingleApplication::NoApplication);

		/*
		 * This is called to write data on the established stream.
		 * Data will be written on the channel channel (0 = RTP, 1 = RTCP)
		 */
		void writeDatagram(const QByteArray&, JingleTransport::Channel channel = JingleTransport::Rtp);

		/* 
		 * Get all data available on the socket.
		 * Data will be read from the channel channel (0 = RTP, 1 = RTCP)
		 */
		QByteArray readAll(JingleTransport::Channel channel = JingleTransport::Rtp);

		void activated();
		void muted();

		QString creator() const;
		QString name() const;
		Senders senders() const;

		bool isReady() const;

		JingleContent& operator=(const JingleContent&);

		JingleSession *parent() const;

		Task* rootTask() const;

		void sessionInfo(const QDomElement& info);

		ReasonType reason() const;

		static QString sendersToStr(const Senders s);
		static Senders strToSenders(const QString& s);
		
		void stopNegotiation();
		
	signals:
		/**
		 * Emitted when the transport has succeeded for this content.
		 * Data can be transferred with this content as soon as this
		 * signal is emitted.
		 */
		void established();

		/**
		 * Emitted when data is ready to be read.
		 * The given argument is the channel on which data is ready
		 * (e.g. : 0 = RTP, 1 = RTCP)
		 */
		void readyRead(int);

		/**
		 * Emitted when the direction of the content has changed.
		 * For example, when one of the peers wants to stop sending
		 * video but keeps receiving
		 * (both -> responder or both -> initiator, depending on
		 * who stops sending.)
		 */
		void sendersChanged();

	public slots:
		/*
		 * Starts the content. This is called when the JingleApplication,
		 * the JingleTransport and the parent session are set.
		 * It wil also start the transport.
		 */
		void start();
		
	private:
		class Private;
		Private *d;
		
	};
}

#endif
