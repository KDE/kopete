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
		void trying(const JingleContent&);
		void transportInfo(const JingleContent&);
		
	private :
		class Private;
		Private *d;
	signals :
		void finished();
	
	};
}

#endif
