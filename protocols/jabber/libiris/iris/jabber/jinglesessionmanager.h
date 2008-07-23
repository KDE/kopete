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
		XMPP::JingleSession *startNewSession(const Jid&, const QList<JingleContent*>&);
		void setSupportedTransports(const QStringList&);
		void setSupportedAudioPayloads(const QList<QDomElement>&);
		void setSupportedVideoPayloads(const QList<QDomElement>&); // FIXME:a class name QNodeList does exists in Qt.
		void setSupportedProfiles(const QStringList&);

	signals:
		void newJingleSession(XMPP::JingleSession*);
		void sessionTerminate(XMPP::JingleSession*);
	
	public slots:
		void slotSessionIncoming();
		void slotRemoveContent(const QString&, const QStringList&);
		void slotSessionInfo(const QDomElement&);
		void slotTransportInfo(const QDomElement&);
		//void slotSessionTerminated();
		void slotSessionTerminate(const QString&, const JingleReason&);

	private:
		JingleSession *session(const QString& sid);
		class Private;
		Private *d;
	};
}

#endif
