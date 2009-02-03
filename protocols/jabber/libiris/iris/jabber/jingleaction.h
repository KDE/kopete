#ifndef JINGLE_ACTION_H
#define JINGLE_ACTION_H

#include <QDomElement>
#include <QString>

#include "im.h"

namespace XMPP
{
	class JingleAction /*: public QObject*/
	{
	//	Q_OBJECT
	public:
		enum Action {
			SessionInitiate = 0,
			SessionTerminate,
			SessionAccept,
			SessionInfo,
			ContentAdd,
			ContentRemove,
			ContentModify,
			TransportReplace,
			TransportAccept,
			TransportInfo,
			NoAction
		};

		/*
		 * Construct an empty JingleAction.
		 * Can be used to create a Jingle action.
		 * It can also be used to create an instance of the class to
		 * be filled with setStanza() or setData()
		 */
		JingleAction();

		/*
		 * Construct a JingleAction with the provided stanza.
		 * If an error occurs, isValud will return false.
		 */
		JingleAction(const QDomElement& stanza);
		~JingleAction();

		/*
		 * Return true if the action is valid (not a jingle action was given)
		 */
		bool isValid() const;
		
		/*
		 * Convert an action attribute in an Action enumerated type.
		 */
		Action jingleAction(const QString& action);

		/* Set the Xml stanza.
		 *
		 * The data must contain the whole stanza.
		 * Returns EXIT_SUCCESS on success and EXIT_FAILURE on failure.
		 */
		int setStanza(const QDomElement& data);

		/* Returns a stanza ready to be sent*/
		QDomElement stanza();
		
		Jid from() const;
		Jid to() const;
		QString initiator() const;
		QString sid() const;
		Action action() const;

		/* Set the Xml data.
		 *
		 * The data must contain what is to go between <jingle> and </jingle>.
		 * All other informations (contained in <iq> and <jingle>) are
		 * set with appropriate methods (TODO).
		 */
		void setData(const QDomElement& data);

		/* Returns the data contained between <jingle> and </jingle>.*/
		QDomElement data() const;

	private:
		class Private;
		Private *d;
	};
}

#endif //JINGLE_ACTION_H
