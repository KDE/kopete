#ifndef JINGLEFOOSESSION_H_
#define JINGLEFOOSESSION_H_

class JabberAccount;
class JingleSession;
class QDomDocument;
enum JingleStateEnum;

/**
 * Session which sends ASCII text "foo"
 *
*/
class JingleFooSession : public JingleSession
{
	Q_OBJECT
public:
	typedef Q3ValueList<XMPP::Jid> JidList;

	JingleFooSession(JabberAccount *account, const JidList &peers);
	virtual ~JingleFooSession();

	virtual QString sessionType();

public slots:
	virtual void accept();
	virtual void decline();
	virtual void start();
	virtual void terminate();

protected slots:
	void receiveStanza(const QString &stanza);

private:
	class JingleIQResponder;

	/**
	* Process the XMPP stanza, and take appropriate action.
	* Some parts of this function will die if the stanza is malformed,
	* checks need to be added
	*/
	void processStanza(QDomDocument doc);

	JingleStateEnum state;
	QList<JingleContentType> types;
	QList<JingleFooConnectionCandidate> remoteCandidates;
	QString initiator;
	QString responder;
	QString sid;
	QDomElement checkPayload(QDomElement stanza);

	/**
	* Removes designated content type.  If there are none left, closes the session.
	*/
	void removeContent(QDomElement stanza);

	JingleFooTransport fooTransport;
	JingleFooConnectionCandidate connection;
	
};
 
#endif