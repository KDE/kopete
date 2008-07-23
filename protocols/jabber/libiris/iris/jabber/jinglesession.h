/*
 * This class defines a Jingle Session which contains all informations about the session.
 * This is here that the state machine is and where almost everything is done for a session.
 */
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
	class IRIS_EXPORT JingleReason
	{
	public:
		JingleReason();
		enum Type {
			Decline = 0,
			Busy,
			NoReason
		};
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
	class JT_JingleSession;
	class JT_PushJingleSession;

	class IRIS_EXPORT JingleSession : public QObject
	{
		Q_OBJECT
	public:
		JingleSession();
		JingleSession(Task*, const Jid&);
		~JingleSession();

		/**
		 * Adds a content to the session.
		 * Currently, the content is just added in the contents list.
		 * TODO: addContent should add a content even when the session
		 * is in ACTIVE state so the session is modified with a content-add action.
		 */
		void addContent(JingleContent*);
		void addContent(const QDomElement&);
		void addContents(const QList<JingleContent*>&);
		void addSessionInfo(const QDomElement&);
		void addTransportInfo(const QDomElement&);

		void acceptContent();
		void acceptSession();
		void removeContent(const QString&);
		void removeContent(const QStringList&);
		void terminate(const JingleReason& r = JingleReason());
		void ring();
		
		/*TODO: there should also be removeContent, modifyContent,...*/
		
		void sendIceUdpCandidates();
		void startRawUdpConnection(JingleContent*);

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

		enum JingleAction {
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
		};
		
		enum State {
			Pending = 0,
			Active,
			Ended
		};

		State state() const;

	signals:
		void terminated();
		// needData() is emitted once.
		// Once it has been emitted, streaming must start on this socket until stopSending is emitted.
		// FIXME:It would be best to give a JingleContent as an argument so we know for which content it is (the socket is in that content btw)
		void needData(XMPP::JingleContent*); //QDomElement is a payload-type element, could be a QString containing a SDP (Session Description Protocol) string.
	public slots:
		void slotRemoveAcked();
		void slotSessTerminated();
		void slotRawUdpDataReady();

	private:
		class Private;
		Private *d;
	};
}

#endif
