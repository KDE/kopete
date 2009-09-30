/*
 * jingletasks.cpp - Tasks for the Jingle specification.
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
		
		/**
		 * This method is called to acknowledge the sender (to) it's stanza
		 * has been received.
		 *
		 */
		void ack(const QString& id, const Jid& to);
		
		/*
		 * This method respond with an unknown session error stanza.
		 *
		 * This is the job of the JingleManager to do that if a
		 * jingle action tries to modify an unexisting session.
		 */
		void unknownSession(const QString& id, const Jid& to);

	signals:
		/*
		 * Emitted when a new jingle action is incoming. The JingleAction
		 * can be retrieved with getLastPendingAction()
		 */
		void jingleActionReady();

	private:
		class Private;
		Private *d;
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
		void terminate(const QDomElement&);

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
		void removeContents(const QList<JingleContent*>&);

		/*
		 * Sends a transport-info jingle action for the
		 * content contentName with the given transport.
		 */
		void transportInfo(const QString& contentName, const QDomElement& transport);

		/**
		 * Sends a session-info jingle action with the inforation contented in e.
		 */
		void sessionInfo(const QDomElement& e);

		/*
		 * Sends a session-accept jingle action.
		 * Once acked, this will mean the session is in the ACTIVE state
		 */
		void sessionAccept(const QList<JingleContent*>&);

		/*
		 * Sends an iq error saying that there is no such session.
		 */
		void noSessionError(const QDomElement&);
		
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
