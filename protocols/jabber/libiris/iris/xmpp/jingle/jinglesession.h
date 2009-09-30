/*
 * jinglesession.cpp - Jingle session
 * 
 * This class defines a Jingle Session which contains all information about the session.
 * This is here that the state machine is and where almost everything is done for a session.
 * 
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
#ifndef JINGLE_SESSION
#define JINGLE_SESSION

#include <QObject>
#include <QString>
#include <QDomElement>

#define NS_JINGLE			"urn:xmpp:jingle:1"
#define NS_JINGLE_ERROR			"urn:xmpp:jingle:errors:0"
#define NS_JINGLE_APPS_RTP		"urn:xmpp:jingle:apps:rtp:1"
#define NS_JINGLE_APPS_RTP_INFO		"urn:xmpp:jingle:apps:rtp:info:1"
#define NS_JINGLE_TRANSPORTS_S5B	"urn:xmpp:jingle:transports:s5b:1"
#define NS_JINGLE_TRANSPORTS_RAW 	"urn:xmpp:jingle:transports:raw-udp:1"
#define NS_JINGLE_TRANSPORTS_ICE 	"urn:xmpp:jingle:transports:ice-udp:1"

#include "im.h"
//#include "xmpp_client.h"
//#include "xmpp_jid.h"
#include "jingletasks.h"
#include "jinglesessionmanager.h"
#include "jinglecontent.h"

namespace XMPP
{
	/*
	 * This class defines a jingle reason used when sending
	 * a session-terminate jingle action.
	 */
	class /*IRIS_EXPORT*/ JingleReason
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

	class JingleAction;
	class JT_JingleAction;
	class JT_PushJingleSession;

	class /*IRIS_EXPORT*/ JingleSession : public QObject
	{
		Q_OBJECT
	public:
		JingleSession();
		JingleSession(Task*, const Jid&, JingleSessionManager *parent = 0);
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
		 * Adds transport info to the session.
		 * Mostly, it adds a candidate to the session
		 * and the session starts to try to connect to it.
		 * Argument is a QDomElement containing
		 * the jingle tag in a jingle stanza.
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
		void removeContent(JingleContent* c);

		/*
		 * Sends a remove-content jingle action with the contents
		 * name given as an argument.
		 * Prefer this method instead of removeContent(JingleContent*);
		 */
		void removeContent(QList<JingleContent*>&);

		/*
		 * Sends a session-terminate jingle action with the reason r.
		 * Once the responder sends the acknowledgement stanza, the
		 * signal terminated() is emitted.
		 *
		 * If the application wants to terminate the session,
		 * this is the method to use. There are multiple reasons
		 * for terminating a session :
		 * 	* Declined
		 * 	* Normal termination
		 * 	* Unable to start device (Webcam or audio)
		 */
		void sessionTerminate(const QDomElement& r = QDomElement());

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
		 * Returns true if the session is started.
		 * The session is started when the session-initiate has been sent.
		 */
		bool isStarted() const;

		/*
		 * Starts the session by sending a session-initiate jingle action.
		 * if a SID has been set, it will be overwritten by a new generated one.
		 */
		void start();
		
		/* 
		 * This method sets the SID.
		 * For an incoming session, the sid must be set and not
		 * randomly generated.
		 * Calling the start() method after this one will reset the SID.
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
		void setInitiator(const QString&, bool isInit = false);
		void setResponder(const QString&, bool isResp = false);

		/*
		 * Return initiator Jid.
		 */
		QString initiator() const;

		QString responder() const;
		
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

		// Session states
		enum State {
			Pending = 0,
			Ringing,
			Hold,
			Mute,
			Active,
			Ended
		};
		
		/*
		 * Returns the current state of the session.
		 */
		State state() const;

		/*
		 * Add an action to the session so it is properly processed.
		 */
		void appendAction(JingleAction *action);	

		/*
		 * Return true if this session has been initiated by us.
		 */
		bool isInitiator() const;

		Task *rootTask() const;

	signals:
		/*
		 * Emitted when the session has been started (when session-initiate has been acked with no error)
		 */
		void started();
		
		/*
		 * Emitted once a session-terminate has been acknowledged or received.
		 *
		 * Once this signal has been sent, Iris will forget about it.
		 * It's the job of the application to delete it.
		 */
		void terminated();
		
		/**
		 * Emitted when the session state has changed (Pending --> Active)
		 *
		 * If the state switched from Pending to Active, the
		 * JingleApplication, that can be retreived with application()
		 * on each content, will have changed if we are the initiator.
		 */
		void stateChanged();
		
		/**
		 * Emitted when an error occured.
		 * This way, the application is notified and can delete the session.
		 * 
		 * The int argument is the received error code and the QString is
		 * the error status.
		 */
		void error(int, const QString&);

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
		 * Called when a content has been established.
		 */
		void slotContentConnected();

		/*
		 * This slot is called when the session has been accepted by the responder.
		 */
		void slotSessionAcceptAcked();

		/*
		 * This slot is called when the session initiate has been acked.
		 */
		void slotInitiateAcked();

	private:
		class Private;
		Private *d;
		
		/*
		 * Sends ice udp cadidates
		 */
		//void sendIceUdpCandidates();
		
		/*
		 * Starts a raw udp connection for this JingleContent.
		 * (Create socket, ask to start sending data on it)
		 */
		//void startRawUdpConnection(JingleContent*);
	};
}

#endif
