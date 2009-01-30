/*
 * jinglesession.cpp - Jingle session
 * 
 * This class defines a Jingle Session which contains all information about the session.
 * This is here that the state machine is and where almost everything is done for a session.
 * 
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
#ifndef JINGLE_SESSION
#define JINGLE_SESSION

#include <QObject>
#include <QString>
#include <QDomElement>

#define NS_JINGLE "urn:xmpp:tmp:jingle:0"
#define NS_JINGLE_TRANSPORTS_RAW "urn:xmpp:tmp:jingle:transports:raw-udp:1"
#define NS_JINGLE_TRANSPORTS_ICE "urn:xmpp:tmp:jingle:transports:ice-udp:0"
#define NS_JINGLE_APPS_RTP "urn:xmpp:tmp:jingle:apps:rtp:0"

#include "im.h"
//#include "xmpp_client.h"
//#include "xmpp_jid.h"
#include "jingletasks.h"

namespace XMPP
{
	/*
	 * This class defines a jingle reason used when sending
	 * a session-terminate jingle action.
	 */
	class IRIS_EXPORT JingleReason
	{
	public:
		/*
		 * Default constructor : create a No Reason reason with no text.
		 */
		JingleReason();
		enum Type {
			Decline = 0,
			Busy,
			UnsupportedApplications,
			NoReason
		};
		/*
		 * Creates a reason with a type and a text reason.
		 */
		JingleReason(JingleReason::Type, const QString& text = QString());
		~JingleReason();
		
		//static Type stringToType(const QString&);

		void setType(Type);
		void setText(const QString&);
		Type type() const;
		QString text() const;
	private:
		class Private;
		Private *d;
	};

	class JingleContent;
	class JT_JingleAction;
	class JT_PushJingleSession;

	class IRIS_EXPORT JingleSession : public QObject
	{
		Q_OBJECT
	public:
		JingleSession();
		JingleSession(Task*, const Jid&);
		~JingleSession();

		/*
		 * Adds a content to the session.
		 * Currently, the content is just added in the contents list.
		 * TODO: addContent should add a content even when the session
		 * is in ACTIVE state so the session is modified with a content-add action.
		 */
		void addContent(JingleContent*);

		/*
		 * Same as above but the content is in a QDomElement form.
		 * For convenience.
		 */
		void addContent(const QDomElement&);

		/*
		 * Adds multiple contents to the session. It is advised
		 * to use this method instead of addContent() even for
		 * one content.
		 */
		void addContents(const QList<JingleContent*>&);

		/* 
		 * Adds session information to the session
		 * (used to inform the session that a "received"
		 * informational message has been received for eg.)
		 * Argument is a QDomElement containing the child(s -- TODO)
		 * of the jingle tag in a jingle stanza.
		 */
		void addSessionInfo(const QDomElement&);

		/*
		 * Adds transport info to the session.
		 * Mostly, it adds a candidate to the session
		 * and the session starts to try to connect to it.
		 * Argument is a QDomElement containing the child(s -- TODO)
		 * of the jingle tag in a jingle stanza.
		 */
		void addTransportInfo(const QDomElement&);

		/*
		 * Sends a content-accept jingle action.
		 * Not used yet, may be removed.
		 */
		void acceptContent();

		/*
		 * Sends a session-accept jingle action.
		 * Not used yet, may be removed.
		 */
		void acceptSession();

		/*
		 * Sends a remove-content jingle action with the content
		 * name given as an argument.
		 */
		void removeContent(const QString&);

		/*
		 * Sends a remove-content jingle action with the contents
		 * name given as an argument.
		 * Prefer this method instead of removeContent(const QString&);
		 */
		void removeContent(const QStringList&);

		/*
		 * Sends a session-terminate jingle action with the reason r.
		 * Once the responder sends the acknowledgement stanza, the
		 * signal terminated() is emitted.
		 */
		void sessionTerminate(const JingleReason& r = JingleReason());

		/*
		 * Sends a ringing informational message.
		 * FIXME:Would be better to use the sessionInfo() method.
		 */
		void ring();
		
		/*
		 * Returns the Jid of the other peer with whom the session is established.
		 */
		Jid to() const;

		/*
		 * Returns the contents of this session.
		 * In Pending state, it should return contents sent by the other peer.
		 * In Active state, it should return contents being used.
		 * This is right as we know which contents we do support.
		 */
		QList<JingleContent*> contents() const;

		/*
		 * Starts the session by sending a session-initiate jingle action.
		 * if a SID has been set, it will be overwritten by a new generated one.
		 */
		void start();
		
		/* This method sets the SID.
		 * For an incoming session, the sid must be set and not
		 * generated randomly.
		 * Calling the start method after this one will change the SID
		 */
		void setSid(const QString&);

		/*
		 * Sets peer's Jid.
		 */
		void setTo(const Jid&);

		/*
		 * Sets the initiator Jid.
		 * This can be already set if a session is redirected.
		 * Session redirection is NOT supported yet.
		 */
		void setInitiator(const QString&); //Or const Jid& ??

		/*
		 * Return initiator Jid.
		 */
		QString initiator() const;
		
		/*
		 * Start negotiation.
		 * This function is called after receiving a session initiate.
		 * This will start negotiating a connection depending on the transport.
		 */
		void startNegotiation();
		
		/*
		 * Returns a pointer to the first JingleContent with the name n.
		 * Each content must have a unique name so returning the first
		 * one returns the only one.
		 */
		JingleContent *contentWithName(const QString& n);
		
		/*
		 * Returns the sid of this session.
		 */
		QString sid() const;

		/*
		 * Call this function when a session-accept jingle action has been received for it.
		 * Once the session is accepted, we will get the supported payloads of the initiator
		 * and switch to Active state, Media can begin to flow on each content's socket.
		 */
		void sessionAccepted(const QDomElement&);

		// Jingle actions
		enum JingleAction {
			SessionInitiate = 0,
			SessionTerminate,
			SessionAccept,
			SessionInfo,
			ContentAdd,
			ContentRemove,
			ContentModify,
			TransportReplace,
			TransportAccept,
			TransportInfo,
			NoAction
		};
		
		// Session states
		enum State {
			Pending = 0,
			Active,
			Ended
		};
		
		/*
		 * Returns the current state of the session.
		 */
		State state() const;

	signals:
		
		/*
		 * Emitted once a session-terminate has been acknowledged
		 */
		void terminated();
		
		/* 
		 * needData() is emitted once for each content.
		 * Once it has been emitted, streaming must start on this socket until stopSending is emitted.
		 * FIXME: Shouldn't pass by here, should stay in JingleContent.
		 */
		void needData(XMPP::JingleContent*);

		/**
		 * Emitted when the session state has changed (Pending --> Active)
		 */
		void stateChanged();
	public slots:
		
		/*
		 * This slot is called when a content-remove has been acked.
		 */
		void slotRemoveAcked();
		
		/*
		 * This slot is called when a session-terminate has been acked.
		 */
		void slotSessTerminated();

		/*
		 * This slot is called when data is received on the raw udp socket.
		 */
		void slotRawUdpDataReady();

		/*
		 * Called when a content has been established.
		 */
		void slotContentConnected();

		/*
		 * This slot is called when a JT_JingleAction has been acknowledged
		 * and we just have to delete it.
		 */
		void slotAcked();

		/*
		 * This slot is called when the session has been accepted by the responder.
		 */
		void slotSessionAcceptAcked();

		/*
		 * This slot is called when a "receive" informational message has been received.
		 * Currently, this slot simply calls setSending() on all contents.
		 */
		void slotReceivingData();

	private:
		class Private;
		Private *d;
		
		/*
		 * Sends ice udp cadidates
		 */
		void sendIceUdpCandidates();
		
		/*
		 * Starts a raw udp connection for this JingleContent.
		 * (Create socket, ask to start sending data on it)
		 */
		void startRawUdpConnection(JingleContent*);
		
		/*
		 * Deletes an action when it is not used anymore.
		 */
		void deleteAction(JT_JingleAction*);
	};
}

#endif
