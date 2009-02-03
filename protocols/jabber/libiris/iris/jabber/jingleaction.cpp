#include "jingleaction.h"

using namespace XMPP;

class JingleAction::Private {
public:
	Action action;
	Jid from;
	QString id; // may not be needed;
	QString sid;
	QString initiator;
	QString type;
	QDomElement data;
	bool valid; // True if the parsed stanza is a valid Jingle stanza, false if not.
};

JingleAction::JingleAction()
 : d (new Private())
{
	d->valid = false;
}

JingleAction::JingleAction(const QDomElement& stanza)
 : d (new Private())
{
	d->valid = setStanza(stanza) == EXIT_SUCCESS ? true : false;
}

JingleAction::~JingleAction()
{
	delete d;
}

bool JingleAction::isValid() const
{
	return d->valid;
}

void JingleAction::setData(const QDomElement& data)
{
	d->data = data; // FIXME:Is deep copy needed here ?
}

int JingleAction::setStanza(const QDomElement& stanza)
{
	/* Fill Private here. */
	if (stanza.tagName() != "iq" || stanza.firstChildElement().tagName() != "jingle")
		return EXIT_FAILURE;
	
	d->type = stanza.attribute("type");
	// FIXME:Mmmm, what am I gonna do if it is an error ?
	d->action = jingleAction(stanza.firstChildElement().attribute("action"));
	d->sid = stanza.firstChildElement().attribute("sid");
	d->initiator = stanza.firstChildElement().attribute("initiator");
	d->id = stanza.attribute("id");
	d->from = Jid(stanza.attribute("from"));
	d->data = stanza.firstChildElement().firstChildElement(); // FIXME:Is deep copy needed here ?

	return EXIT_SUCCESS;
}

QString JingleAction::sid() const
{
	return d->sid;
}

JingleAction::Action JingleAction::action() const
{
	return d->action;
}

Jid JingleAction::from() const
{
	return d->from;
}

QString JingleAction::initiator() const
{
	return d->initiator;
}

QDomElement JingleAction::data() const
{
	return d->data;
}

JingleAction::Action JingleAction::jingleAction(const QString& action)
{
	if (action == "session-initiate")
		return SessionInitiate;
	else if (action == "session-terminate")
		return SessionTerminate;
	else if (action == "session-accept")
		return SessionAccept;
	else if (action == "session-info")
		return SessionInfo;
	else if (action == "content-add")
		return ContentAdd;
	else if (action == "content-remove")
		return ContentRemove;
	else if (action == "content-modify")
		return ContentModify;
	else if (action == "transport-replace")
		return TransportReplace;
	else if (action == "transport-accept")
		return TransportAccept;
	else if (action == "transport-info")
		return TransportInfo;
	else
		return NoAction;
}

