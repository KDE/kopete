#ifndef JINGLE_TASKS
#define JINGLE_TASKS

#include <QDomElement>

#include "im.h"
#include "xmpp_task.h"


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
		void addTransportNS(const QString& t);
		//void setType(Type);
		void setCreator(const QString& c);
		void setName(const QString& n);
		void setDesriptionNS(const QString& desc);
		void setProfile(const QString& p);

		QList<QDomElement> payloadTypes() const;
		QStringList transportNS() const;
		QDomElement contentElement();
		//Type type();

	private:
		class Private;
		Private *d;
	};
	
	class IRIS_EXPORT JT_PushJingleSession : public Task
	{
		Q_OBJECT
	public:
		JT_PushJingleSession(Task*);
		~JT_PushJingleSession();

		void onGo();
		bool take(const QDomElement &);
	
	private:
		class Private;
		Private *d;
	};
	
	class IRIS_EXPORT JT_JingleSession : public Task
	{
		Q_OBJECT
	public:
		JT_JingleSession(Task*);
		~JT_JingleSession();
		
		void start(JingleSession*);

		void onGo();
		bool take(const QDomElement &);
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
			WaitContentAccept
		};
		class Private;
		Private *d;
		QDomElement iq;
		QString sid;
		QString redirectTo;
	};

}

#endif
