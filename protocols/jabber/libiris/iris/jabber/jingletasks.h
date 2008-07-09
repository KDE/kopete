#ifndef JINGLE_TASKS
#define JINGLE_TASKS

#include <QDomElement>

#include "im.h"
#include "xmpp_task.h"
#include "jinglesession.h"

namespace XMPP
{
	class JingleSession;
	class IRIS_EXPORT JingleContent/* : public QObject*/
	{
		/*Q_OBJECT*/
	public:
		JingleContent();
		~JingleContent();
		/*enum Type {
			Audio = 0,
			Video,
			FileTransfer
		};*/

		void addPayloadType(const QDomElement& pl);
		void addTransportNS(const QDomElement& t);
		//void setType(Type);
		void setCreator(const QString& c);
		void setName(const QString& n);
		void setDesriptionNS(const QString& desc);
		void setProfile(const QString& p);

		QList<QDomElement> payloadTypes() const;
		QList<QDomElement> transports() const;
		void fromElement(const QDomElement& e);
		QDomElement contentElement();
		QString name() const;
		QString descriptionNS() const;
		QString dataType();
		void addTransportInfo(const QDomElement& e);
		QString iceUdpPassword();
		QString iceUdpUFrag();

	private:
		class Private;
		Private *d;
	};
	
	// This class will be renamed to JT_PushJingleAction
	class IRIS_EXPORT JT_PushJingleSession : public Task
	{
		Q_OBJECT
	public:
		JT_PushJingleSession(Task*);
		~JT_PushJingleSession();

		void onGo();
		bool take(const QDomElement&);
		JingleSession *takeNextIncomingSession();
	signals:
		void newSessionIncoming();
		void removeContent(const QString&, const QStringList&);
		void transportInfo(const QDomElement&);
	
	private:
		class Private;
		Private *d;
		void ack();
		void jingleError(const QDomElement&);
	};

	// This class will be replaced by JT_JingleAction.
	class IRIS_EXPORT JT_JingleSession : public Task
	{
		Q_OBJECT
	public:
		JT_JingleSession(Task*);
		~JT_JingleSession();
		
		void start(JingleSession*);

		void onGo();
		bool take(const QDomElement&);
		enum Errors {
			ServiceUnavailable = 0,
			Redirect,
			BadRequest,
			ResourceConstraint
		};

	signals:
		void error(int);
	private:
		enum State {
			Initiation = 0,
			WaitContentAccept,
			StartNegotiation,
			Active
		};
		class Private;
		Private *d;
		QString redirectTo;
		/*JingleSession::JingleAction*/int jingleAction(const QDomElement&);
		void sendCandidate();
	};
	
	class IRIS_EXPORT JT_JingleAction : public Task
	{
		Q_OBJECT
	public:
		JT_JingleAction(Task*);
		~JT_JingleAction();
		
		void onGo();
		bool take(const QDomElement&);
		
		void setSession(JingleSession*);
		
		void terminate(int);
		void contentAccept();
		void removeContent(const QString&);
		void ringing();
		
	private :
		class Private;
		Private *d;
	signals :
		void finished();
	
	};
}

#endif
