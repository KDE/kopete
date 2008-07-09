/*
 * Manages a Jingle session using iris library jingle implementation
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
	class IRIS_EXPORT JingleSessionManager : public QObject
	{
		Q_OBJECT
	public:
		JingleSessionManager(Client*);
		~JingleSessionManager();
		void startNewSession(const Jid&);
		void setSupportedTransports(const QStringList&);
		void setSupportedAudioPayloads(const QList<QDomElement>&);
		void setSupportedVideoPayloads(const QList<QDomElement>&); // FIXME:a class name QNodeList does exists in Qt.
		void setSupportedProfiles(const QStringList&);

	signals:
		void newJingleSession(XMPP::JingleSession*);
	
	public slots:
		void slotSessionIncoming();
		void slotRemoveContent(const QString&, const QStringList&);
		void slotTransportInfo(const QDomElement&);
		void slotDeleteSession();

	private: // FIXME: must go in JingleSessionManager::Private
		class Private;
		Private *d;
	};
}

#endif
