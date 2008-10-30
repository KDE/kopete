/*
 * jinglesessionmanager.cpp - Manager for Jingle sessions
 *
 * Manages all Jingle sessions.
 * This class receives all incoming jingle actions and perform these
 * actions on the right jingle session.
 * It also keeps informations about protocols supported by the application (transports, payloads, profiles)
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
	class JingleReason;
	class IRIS_EXPORT JingleSessionManager : public QObject
	{
		Q_OBJECT
	public:
		JingleSessionManager(Client*);
		~JingleSessionManager();
		
		/*
		 * Create a new jingle session to a Jid and with a list of contents,
		 * starts it and returns it.
		 */
		XMPP::JingleSession *startNewSession(const Jid&, const QList<JingleContent*>&);
		
		/*
		 * Set supported transports for jingle sessions.
		 */
		void setSupportedTransports(const QStringList&);
		
		/*
		 * Set supported audio payloads for jingle sessions.
		 */
		void setSupportedAudioPayloads(const QList<QDomElement>&);

		/*
		 * Returns the supported audio payloads.
		 */
		QList<QDomElement> supportedAudioPayloads() const;
		
		/*
		 * Set supported video payloads for jingle sessions.
		 */
		void setSupportedVideoPayloads(const QList<QDomElement>&); // FIXME:a class name QNodeList does exists in Qt.

		/*
		 * Returns the supported video payloads.
		 */
		QList<QDomElement> supportedVideoPayloads() const;
		
		/*
		 * Set supported profiles for jingle sessions.
		 */
		void setSupportedProfiles(const QStringList&);

		/*
		 * Provides the next usable port for a raw-udp session.
		 * As the application should create a rtcp port with the
		 * provided rtp socket port + 1, this method will always
		 * give a port incremented by 2.
		 * The first port will be 9000 by default but it can be modified
		 * with setFirstPort().
		 * Also, this method will share a list of used ports with the
		 * iceUdpPort method.
		 * It would be nice to be informed of the ports which are freed
		 * when a session is terminated so we can reuse them.
		 */
		int nextRawUdpPort();
		void setFirstPort(int);

		QString externalIP() const;
		void setExternalIP(const QString& eip);
	signals:
		
		/*
		 * Emitted when a new jingle session comes.
		 */
		void newJingleSession(XMPP::JingleSession*);

		/*
		 * Emitted when a session-terminate is received.
		 */
		void sessionTerminate(XMPP::JingleSession*);
	
	public slots:
		/* 
		 * Slots for each jingle action
		 */
		void slotSessionIncoming();
		void slotRemoveContent(const QString&, const QStringList&);
		void slotSessionInfo(const QDomElement&);
		void slotTransportInfo(const QDomElement&);
		void slotSessionTerminate(const QString&, const JingleReason&);
		void slotSessionAccepted(const QDomElement&);

		/*
		 * This slot is called when a session has been
		 * terminated and should be removed from the
		 * sessions list.
		 */
		void slotSessionTerminated();

		/*
		 * This slot is called when the external IP has been retrieved by http
		 */
		void slotExternalIPDone(bool);

	private:
		class Private;
		Private *d;
		/*
		 * Returns the session with the SID sid.
		 */
		JingleSession *session(const QString& sid);

		/*
		 * Check if this content has supported contents.
		 * If yes, returns true, returns false if not.
		 */
		bool checkSupportedPayloads(JingleContent *c);
		
		/*
		 * Check if this content has a supported transport.
		 * If yes, returns true, returns false if not.
		 */
		bool checkSupportedTransport(JingleContent *c);

	};
}

#endif
