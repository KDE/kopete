#ifndef JINGLE_SESSION
#define JINGLE_SESSION

#include <QObject>
#include <QString>
#include <QDomElement>

#include "im.h"
//#include "xmpp_client.h"
//#include "xmpp_jid.h"
#include "jingletasks.h"

namespace XMPP
{
	class JingleContent;
	class JT_JingleSession;
	class JT_PushJingleSession;

	class IRIS_EXPORT JingleSession : public QObject
	{
		Q_OBJECT
	public:
		JingleSession();
		JingleSession(Task*, const Jid&);
		~JingleSession();

		typedef enum {
			Decline = 0
		} Reason;
		/**
		 * Adds a content to the session.
		 * Currently, the content is just added in the contents list.
		 * TODO: addContent should add a content even when the session
		 * is in ACTIVE state so the session is modified with a content-add action.
		 */
		void addContent(JingleContent*);
		void addContent(const QDomElement&);
		void addContents(const QList<JingleContent*>&);
		void addTransportInfo(const QDomElement&);

		void acceptContent();
		void acceptSession();
		void removeContent(const QString&);
		void removeContent(const QStringList&);
		void terminate(Reason);
		void ring();
		
		/*TODO: there should also be removeContent, modifyContent,...*/
		
		void sendIceUdpCandidates();
		void startRawUdpConnection(const JingleContent&);

		Jid to() const;
		//Jid from() const;
		QList<JingleContent*> contents() const;
		void start();
		void setSid(const QString&);
		//void setFrom(const QString&);
		void setTo(const Jid&);
		void setInitiator(const QString&); //Or const Jid& ??

		QString initiator() const;
		
		void startNegotiation();
		
		JingleContent *contentWithName(const QString& n);

		QString sid() const;

		typedef enum {
			SessionInitiate = 0,
			SessionTerminate,
			SessionAccept,
			SessionInfo,
			ContentAdd,
			ContentRemove,
			ContentModify,
			ContentReplace,
			ContentAccept,
			TransportInfo,
			NoAction
		} JingleAction;
		
		typedef enum {
			Pending = 0,
			Active,
			Ended
		} State;

	signals:
		void terminated();
	public slots:
		void slotRemoveAcked();
		void slotSessTerminated();

	private:
		class Private;
		Private *d;
	};
}

#endif
