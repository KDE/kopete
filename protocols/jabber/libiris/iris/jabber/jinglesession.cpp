#include "jinglesession.h"

using namespace XMPP;

class JingleSession::Private
{
public:
	Jid to;
	QList<JingleContent> contents;
	Task *rootTask;
};

JingleSession::JingleSession(Task *t, const Jid &j)
: d(new Private)
{
	d->to = j;
	d->rootTask = t;
}

JingleSession::~JingleSession()
{
	delete d;
}

void JingleSession::addContent(const JingleContent& c)
{
	d->contents << c;
}

Jid JingleSession::to() const
{
	return d->to;
}

QList<JingleContent> JingleSession::contents() const
{
	return d->contents;
}

void JingleSession::start()
{
	JT_JingleSession *jt = new JT_JingleSession(d->rootTask);
	jt->start(this);
	jt->go();
}
