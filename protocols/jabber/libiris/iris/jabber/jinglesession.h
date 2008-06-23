#ifndef JINGLE_SESSION
#define JINGLE_SESSION

#include <QObject>
#include <QDomElement>

#include "xmpp_client.h"
#include "xmpp_jid.h"
#include "jingletasks.h"

namespace XMPP
{
	class IRIS_EXPORT JingleSession : public QObject
	{
	public:
		JingleSession() {}
		JingleSession(Task*, const Jid&);
		~JingleSession();

		/**
		 * Adds a content to the session.
		 * Currently, the content is just added in the contents list.
		 * TODO: addContent should add a content even when the session
		 * is in ACTIVE state so the session is modified with a content-add action.
		 */
		void addContent(const JingleContent&);
		/*TODO: there should also be removeContent, modifyContent,...*/

		Jid to() const;
		QList<JingleContent> contents() const;
		void start();

	private:
		class Private;
		Private *d;
	};
}

#endif
