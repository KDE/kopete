/*
 * Manages all Jingle sessions.
 * This class receives all incoming jingle actions and perform these
 * actions on the right jingle session.
 * It also keeps informations about protocols supported by the application (transports, payloads, profiles)
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
		 * Set supported video payloads for jingle sessions.
		 */
		void setSupportedVideoPayloads(const QList<QDomElement>&); // FIXME:a class name QNodeList does exists in Qt.
		
		/*
		 * Set supported profiles for jingle sessions.
		 */
		void setSupportedProfiles(const QStringList&);
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

	private:
		class Private;
		Private *d;
		/*
		 * Returns the session with the SID sid.
		 */
		JingleSession *session(const QString& sid);
	};
}

#endif
