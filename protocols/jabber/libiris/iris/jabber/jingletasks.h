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
		void setTransport(const QDomElement& t);
		//void setType(Type);
		void setCreator(const QString& c);
		void setName(const QString& n);
		void setDesriptionNS(const QString& desc);
		void setProfile(const QString& p);

		QList<QDomElement> payloadTypes() const;
		QDomElement transport() const;
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
		void transportInfo(const QDomElement&);
	
	private:
		class Private;
		Private *d;
		void ack();
		void jingleError(const QDomElement&);
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
		
		void initiate();
		void terminate(int);
		void contentAccept();
		void removeContents(const QStringList&);
		void ringing();
		
	private :
		class Private;
		Private *d;
	signals :
		void finished();
	
	};
}

#endif
