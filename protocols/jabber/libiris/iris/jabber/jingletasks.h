#ifndef JINGLE_TASKS
#define JINGLE_TASKS

#include <QDomElement>
#include <QUdpSocket>

#include "im.h"
#include "xmpp_task.h"
#include "jinglesession.h"

namespace XMPP
{
	class JingleSession;
	/*
	 * This class contains all informations about a particular content in a jingle session.
	 * It also has the socket that will be used for streaming.
	 */
	class IRIS_EXPORT JingleContent : public QObject
	{
		Q_OBJECT
	public:
		JingleContent();
		~JingleContent();
		/*enum Type {
			Audio = 0,
			Video,
			FileTransfer
		};*/

		void addPayloadType(const QDomElement&);
		void addPayloadTypes(const QList<QDomElement>&);
		void setTransport(const QDomElement&);
		//void setType(Type);
		void setCreator(const QString&);
		void setName(const QString&);
		void setDescriptionNS(const QString&);
		void setProfile(const QString&);

		QList<QDomElement> payloadTypes() const;
		QDomElement transport() const;
		void fromElement(const QDomElement&);
		QDomElement contentElement();
		QString name() const;
		QString descriptionNS() const;
		QString dataType();
		void addTransportInfo(const QDomElement&);
		QString iceUdpPassword();
		QString iceUdpUFrag();
		void createUdpInSocket();
		QUdpSocket *socket(); // FIXME:Is it socket for data IN or for data OUT ?
				      //       Currently, it's data IN.
		bool sending();
		void setSending(bool);
		bool receiving();
		void setReceiving(bool);
	signals:
		void rawUdpDataReady();

	private:
		class Private;
		Private *d;
	};
	
	/*
	 * This class is a Task that received all jingle actions and give them to the JingleSessionManager
	 */
	class IRIS_EXPORT JT_PushJingleAction : public Task
	{
		Q_OBJECT
	public:
		JT_PushJingleAction(Task*);
		~JT_PushJingleAction();

		void onGo();
		bool take(const QDomElement&);
		JingleSession *takeNextIncomingSession();
	signals:
		void newSessionIncoming();
		void removeContent(const QString&, const QStringList&);
		void sessionInfo(const QDomElement&);
		void transportInfo(const QDomElement&);
	
	private:
		class Private;
		Private *d;
		void ack();
		void jingleError(const QDomElement&);
	};

	/*
	 * This class is a task which is used to send all possible jingle action to a contact, asked by a JingleAction
	 */
	class IRIS_EXPORT JT_JingleAction : public Task
	{
		Q_OBJECT
	public:
		JT_JingleAction(Task*);
		~JT_JingleAction();
		
		void onGo();
		bool take(const QDomElement&);
		
		void setSession(JingleSession*);
		
		void initiate();
		void terminate(int);
		void contentAccept();
		void removeContents(const QStringList&);
		void ringing();
		void trying(const JingleContent&);
		void received();
		void transportInfo(const JingleContent&);
		
	private :
		class Private;
		Private *d;
	signals :
		void finished();
	
	};
}

#endif
