/*
 * jinglecontent.cpp - Jingle content
 * Copyright (C) 2008 - Detlev Casanova <detlev.casanova@gmail.com>
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

class QHostAddress;
class QDomElement;
class QUdpSocket;
namespace XMPP
{
	/*
	 * This class contains all information about a particular content in a jingle session.
	 * It also has the socket that will be used for streaming.
	 * Keep in mind that this class must be used to represent received contents from the remote peer with candidates but also, contents which will be sent.
	 * Might be usefull, add an argument to JingleContent() which could be 'Local' or 'Remote'.
	 */
	//This is an abstract content, all different contents must inherit from this class.
	//TODO:It would be good if the content had also a pointer to the session.
	//	Informations like are we the initiator or the responder could be easily retrieved.
	class Task;
	class JingleSession;
	class IRIS_EXPORT JingleContent : public QObject
	{
		Q_OBJECT
	public:
		//FIXME: may not feel good being here.
		enum Channel {
			Rtp = 0,
			Rtcp
		};

		enum Mode {
			Initiator = 0, // Means, as we created it, that we are the initiator.
			Responder, // Means, as we created it, that we are the responder.
			Unknown // When we don't know (but we always know).
		};

		/*
		 * This enum is used to tell contentElement() what candidates
		 * to set in the returned QDomElement.
		 *	None is no candidates.
		 *	[Local|Remote]Candidate is the [Local|Remote] candidate that works for this content.
		 *	[Local|Remote]Candidates is all [Local|Remote] candidates
		 */
		enum CandidateType {
			NoCandidate = 0,
			LocalCandidate,
			RemoteCandidate,
			LocalCandidates,
			RemoteCandidates
		};

		enum PayloadType {
			NoPayload = 0,
			LocalPayloads,
			RemotePayloads,
			UsedPayload
		};


		JingleContent(Mode mode, JingleSession *parent);
		virtual ~JingleContent();

		/**
		 * Defines the content type, this represesent the media attribute.
		 */
		enum Type {
			Audio = 0,
			Video,
			FileTransfer,
			NoType
		};//Not used anymore, subclassing is used.

		/*
		 * Adds a payload type to this content.
		 */
		void addLocalPayload(const QDomElement&);
		
		/*
		 * Adds a payload type list to this content.
		 */
		void addLocalPayloads(const QList<QDomElement>&);

		/*
		 * Overwrite the current payload types list with this one.
		 */
		void setLocalPayloads(const QList<QDomElement>&);
		
		/*
		 * Returns the payload type list. those payloads are
		 * our payloads if in Pending state or the content
		 * used payloads if in Active state. (TODO)
		 */
		QList<QDomElement> localPayloads() const;

		void addRemotePayload(const QDomElement&);
		void addRemotePayloads(const QList<QDomElement>&);
		void setRemotePayloads(const QList<QDomElement>&);
		QList<QDomElement> remotePayloads() const;

		/*
		 * Sets the transport for this content.
		 * Most likely, this QDomElement will contain the transport and one candidate.
		 */
		void setTransport(const QDomElement&); //FIXME:How is that usefull, used classes are reimplementations of this one and the choice of reimplementation is based on this.

		/*
		 * Set the content type, this will set the "media" attribute of
		 * the content tag in the stanza.
		 */
		void setType(Type);

		/*
		 * Gets the type of this content.
		 */
		Type type() const;

		/*
		 * Returns the transport type of the content content.
		 */
		//FIXME:currently a QString, this could be an enum.
		static QString transportNS(const QDomElement& elem);

		/*
		 * Set the creator of this content, the creator only accept 2 values :
		 * 	* initiator
		 * 	* responder
		 * TODO:An enum should be created to avoid confusion
		 */
		void setCreator(const QString&);
		
		/*
		 * Set this content's name
		 */
		void setName(const QString&);

		/*
		 * Set this content description namespace.
		 * The only one supported currently is
		 * 	NS
		 */
		void setDescriptionNS(const QString&);

		/*
		 * Returns the transport XML element for this content.
		 */
		QDomElement transport() const;

		/*
		 * Fill this content from a QDomElement.
		 * The payloads in this QDomElement will be considered as the responder's
		 * TODO:add an argument to tell the method if those payloads are our's or
		 * responder's payloads.
		 */
		void fromElement(const QDomElement&);

		/*
		 * Return a QDomElement with the content element and all it's children
		 * so it's ready to be sent.
		 */
		QDomElement contentElement(CandidateType cType = NoCandidate, PayloadType pType = NoPayload);

		/*
		 * Returns a list with the available candidates for this content.
		 * TODO:should return the used candidate when in Active state.
		 */
		QList<QDomElement> candidates() const;

		/*
		 * Adds a candidate to this content. Doing so will add this content(s)
		 * to the transport when calling contentElement()
		 */
		virtual void addCandidate(const QDomElement&);

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
		 * This is called to write data on the established stream.
		 * Data will be written on the channel channel (0 = RTP, 1 = RTCP)
		 */
		virtual void writeDatagram(const QByteArray&, int channel = 0);

		/* 
		 * Get all data available on the socket.
		 * Data will be read from the channel channel (0 = RTP, 1 = RTCP)
		 */
		virtual QByteArray readAll(int channel = 0);
		
		void createUdpInSocket();
		
		QString creator() const;
		QString name() const;
		QString descriptionNS() const;
		bool sending();
		bool receiving();

		JingleContent& operator=(const JingleContent&);
		
		QString typeToString(Type);
		Type stringToType(const QString& s);

		QDomElement bestPayload();
		bool isReady() const;
		
		QList<QDomElement> localCandidates() const;
		QList<QDomElement> remoteCandidates() const;

		void setRootTask(Task *rt);

	signals:
		/**
		 * Emitted when the content is set up and session-initiate Jingle action cann be sent.
		 */
		void started();

		/**
		 * Emitted when sending and receiving streams have been established for this content 
		 */
		void established();

		/**
		 * emitted when data is ready to be read.
		 * The given argument is the channel on which data is ready (0 = RTP, 1 = RTCP)
		 */
		void readyRead(int);

	protected:
		/*
		 * Those 2 methods are protected because the content must find its local candidates by itself
		 * and remote candidates are added with Transport-info jingle action.
		 * That means that subclasses must be able to access thos methods but it must not be used by other classes.
		 */
		void addLocalCandidate(const QDomElement&); //FIXME:Could be a JingleCandidate which would be subclassed in JingleRawCandidate.

		virtual void addRemoteCandidate(const QDomElement&);

		Task *rootTask() const;

		QDomElement bestPayload(const QList<QDomElement>&, const QList<QDomElement>&);
		bool samePayload(const QDomElement&, const QDomElement&);
		void setSending(bool);
		void setReceiving(bool);
		virtual void sendCandidates();
		JingleSession *parentSession() const;
		Mode mode() const;
	private:
		class Private;
		Private *d;
		
	};
}

#endif
