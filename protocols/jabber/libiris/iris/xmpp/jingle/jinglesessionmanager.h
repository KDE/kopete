/*
 * jinglesessionmanager.h - Manager for Jingle sessions
 *
 * Manages all Jingle sessions.
 * This class receives all incoming jingle actions and perform these
 * actions on the right jingle session.
 * It also keeps information about protocols supported by the application (transports, payloads, profiles)
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
#ifndef JINGLE_SESSION_MANAGER
#define JINGLE_SESSION_MANAGER

//#include <QObject>

#include "im.h"
//#include "xmpp_client.h"
//#include "xmpp_jid.h"

namespace XMPP
{
	class JingleSession;
	class JingleContent;
	class JingleApplication;
	//class JingleReason;
	class /*IRIS_EXPORT*/ JingleSessionManager : public QObject
	{
		Q_OBJECT
	public:
		JingleSessionManager(Client*);
		~JingleSessionManager();

		static JingleSessionManager *manager();
		
		/*
		 * Create a new jingle session to a Jid and with a list of contents,
		 * starts it and returns it.
		 */
		XMPP::JingleSession *createNewSession(const Jid&);
		
		/*
		 * Set supported transports for jingle sessions.
		 */
		void setSupportedTransports(const QStringList&);
		
		void setSupportedApplications(QList<JingleApplication*>&);

		QList<JingleApplication*> supportedApplications() const;
		
		/*
		 * Provides the next available UDP port.
		 */
		int nextUdpPort(int);
		void setBasePort(int);

		void setStunServiceAddress(const QHostAddress& addr, const int port);
		int stunPort() const;
		QHostAddress stunAddress() const;

		void setSelfAddress(const QHostAddress&);
		QHostAddress selfAddr() const;

		/*
		 * If set to true, contacts trying to start multiple sessions at the same time
		 * will be asked to use the already existing session.
		 * If set to false, multiple sessions with the same contact will be allowed.
		 *
		 * Default set to false.
		 */
		void setUniqueSession(bool u);

	signals:
		/*
		 * Emitted when a new jingle session comes.
		 */
		void newJingleSession(XMPP::JingleSession*);

		/*
		 * Emitted when a session-terminate is received.
		 */
		void sessionTerminate(XMPP::JingleSession*);
	
	private slots:
		/*
		 * Removes the session emitting the signal to which this slot is connected from the list.
		 */
		void slotRemoveSession();
		
		/*
		 * Slot executed each time a Jingle action arrives.
		 */
		void slotJingleActionReady();

		void cleanup();
		
	private:
		class Private;
		Private *d;
		/*
		 * Returns the session with the SID sid.
		 */
		JingleSession *session(const QString& sid);

		static JingleSessionManager *self;
	};
}

#endif
