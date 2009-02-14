/*
 * jingletasks.cpp - Tasks for the Jingle specification.
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
#ifndef JINGLE_TASKS
#define JINGLE_TASKS

#include <QDomElement>
#include <QUdpSocket>

#include "im.h"
#include "xmpp_task.h"
#include "jinglesession.h"
#include "jinglecontent.h"

namespace XMPP
{
	class JingleSession;
	class JingleReason;
	class JingleAction;
	
	/*
	 * This class is a Task that received all jingle actions and give them to the JingleSessionManager
	 */
	class /*IRIS_EXPORT*/ JT_PushJingleAction : public Task
	{
		Q_OBJECT
	public:
		JT_PushJingleAction(Task*);
		~JT_PushJingleAction();

		void onGo();
		bool take(const QDomElement&);
		
		bool hasPendingAction();
		JingleAction* takeNextPendingAction();
		
		/*
		 * Returns the next incoming session, this
		 * method should be called each time the newSessionIncoming()
		 * SIGNAL is emitted.
		 */
		//JingleSession *takeNextIncomingSession();
	signals:
		/*
		 * Emitted when a new jingle action is incoming. The JingleAction
		 * can be retrieved with getLastPendingAction()
		 */
		void jingleActionReady();

		/*
		 * Emitted when a peer wants to remove 1 or more content(s)
		 * from a session (content-remove action). It contains the
		 * session id and a list of the contents to remove.
		 */
		//void removeContent(const QString&, const QStringList&);

		/*
		 * Emitted when a peer sends a session information
		 * (session-info jingle action).
		 * In the case of RAW UDP transport, a session info can be an
		 * informational message like "trying" or "received".
		 * Argument is a QDomElement containing the jingle
		 * tag (and children).
		 */
		//void sessionInfo(const QDomElement&);

		/*
		 * Emitted when a peer sends a transport info.
		 * In most cases, a transport-info jingle action
		 * is used to transfer candidate(s).
		 * Argument is a QDomElement containing the jingle
		 * tag (and children).
		 */
		//void transportInfo(const QDomElement&);

		/*
		 * Emitted when a peer wants to terminate a session
		 * (session-terminate jingle action)
		 * Arguments are the session ID and the Reason of the termination.
		 */
		//void sessionTerminate(const QString&, const JingleReason&);

		/*
		 * Signal emitted when a session-accept jingle action has been received.
		 */
		//void sessionAccepted(const QDomElement&);
	
	private:
		class Private;
		Private *d;

		/* This method is called to acknowledge the sender it's stanza
		 * has been received. Before it is called, d->id must be set
		 * to the received stanza's id.
		 */
		void ack();

		/*
		 * Called when an error iq stanza is received.
		 * This method should do whatever it must be
		 * done in the case of an error.
		 * TODO:Implement me!
		 */
		void jingleError(const QDomElement&);
	};

	/*
	 * This class is a task which is used to send all
	 * possible jingle action to a contact, asked by a
	 * JingleAction.
	 */
	class /*IRIS_EXPORT*/ JT_JingleAction : public Task
	{
		Q_OBJECT
	public:
		JT_JingleAction(Task*);
		~JT_JingleAction();
		
		void onGo();
		bool take(const QDomElement&);
		
		/*
		 * Before doing anything, this method must
		 * be called to set the JingleSession pointer
		 * so the task has all necessary information.
		 */
		void setSession(JingleSession*);
		
		/*
		 * Send a session-initiate jingle action.
		 * There is no argument as the JingleSession set
		 * sooner must have all necessary information
		 * (to, contents and sid)
		 * In contents list, contents with raw-udp transport
		 * must have a candidate set.
		 */
		void initiate();

		/*
		 * Send a session-terminate jingle action.
		 * A reason is given as a parameter.
		 */
		void terminate(const JingleReason&);

		/*
		 * Send a content-accept jingle action.
		 * TODO:should take a list of contents to accept.
		 * 	Contents must be what we support, not the
		 * 	contents we received in the session-initiate
		 * 	jingle action.
		 * TODO:(Re)implement me!
		 */
		void contentAccept();

		/*
		 * Send a content-remove jingle action.
		 * The argument is a list containing the
		 * content names to remove.
		 */
		void removeContents(const QStringList&);

		/*
		 * Sends a "ringing" informational message.
		 * FIXME:Ringing is a session-info jingle action.
		 * 	 It should be sent via a sessionInfo()
		 * 	 method.
		 */
		void ringing();

		/*
		 * Sends a "trying" informational message.
		 * FIXME:Same as ringing();
		 */
		void trying(const JingleContent&);

		/*
		 * Sends a "received" informational message.
		 * FIXME:Same as ringing();
		 */
		void received();

		/*
		 * Sends a transport-info jingle action for the
		 * content contentName with the given transport.
		 */
		void transportInfo(const QString& contentName, const QDomElement& transport);

		/*
		 * Sends a session-accept jingle action.
		 * Once acked, this will mean the session is in the ACTIVE state
		 */
		void sessionAccept(const QList<JingleContent*>&);
		
	private :
		class Private;
		Private *d;
		/* 
		 * Sets a jingle iq with the action provided in d->iq.
		 */
		void createJingleIq(const QString& action);

	signals :
		/*
		 * This signal is emitted when the sent jingle
		 * action has been acknowledged
		 */
		void finished();
	
	};
}

#endif
