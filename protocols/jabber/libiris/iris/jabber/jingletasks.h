#ifndef JINGLE_TASKS
#define JINGLE_TASKS

#include <QDomElement>
#include <QUdpSocket>

#include "im.h"
#include "xmpp_task.h"
#include "jinglesession.h"
#include "jinglecontent.h"

namespace XMPP
{
	class JingleSession;
	
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
