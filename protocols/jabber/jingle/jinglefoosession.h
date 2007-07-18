#ifdef JINGLEFOOSESSION_H_
#define JINGLEFOOSESSION_H_


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
	void processStanza(QDomDocument doc);
	JingleStateEnum state;
	QList<JingleContentType> types;
	QString initiator;
	QString responder;
	QString sid;
	
};
 
#endif