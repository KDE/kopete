/*
 * tasks.cpp - basic tasks
 * Copyright (C) 2001, 2002  Justin Karneges
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

#include <QRegExp>
#include <QList>
#include <QTimer>

#include "xmpp_tasks.h"
#include "xmpp_xmlcommon.h"
#include "xmpp_vcard.h"
#include "xmpp_bitsofbinary.h"
#include "xmpp_captcha.h"
#include "xmpp/base/timezone.h"
#include "xmpp_caps.h"

using namespace XMPP;

static QString lineEncode(QString str)
{
	str.replace(QRegExp("\\\\"), QStringLiteral("\\\\"));   // backslash to double-backslash
	str.replace(QRegExp("\\|"), QStringLiteral("\\p"));     // pipe to \p
	str.replace(QRegExp("\n"), QStringLiteral("\\n"));      // newline to \n
	return str;
}

static QString lineDecode(const QString &str)
{
	QString ret;

	for(int n = 0; n < str.length(); ++n) {
		if(str.at(n) == '\\') {
			++n;
			if(n >= str.length())
				break;

			if(str.at(n) == 'n')
				ret.append('\n');
			if(str.at(n) == 'p')
				ret.append('|');
			if(str.at(n) == '\\')
				ret.append('\\');
		}
		else {
			ret.append(str.at(n));
		}
	}

	return ret;
}

static Roster xmlReadRoster(const QDomElement &q, bool push)
{
	Roster r;

	for(QDomNode n = q.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement i = n.toElement();
		if(i.isNull())
			continue;

		if(i.tagName() == QLatin1String("item")) {
			RosterItem item;
			item.fromXml(i);

			if(push)
				item.setIsPush(true);

			r += item;
		}
	}

	return r;
}

//----------------------------------------------------------------------------
// JT_Session
//----------------------------------------------------------------------------
//
#include "protocol.h"

JT_Session::JT_Session(Task *parent) : Task(parent)
{
}

void JT_Session::onGo()
{
	QDomElement iq = createIQ(doc(), QStringLiteral("set"), QLatin1String(""), id());
	QDomElement session = doc()->createElement(QStringLiteral("session"));
	session.setAttribute(QStringLiteral("xmlns"),NS_SESSION);
	iq.appendChild(session);
	send(iq);
}

bool JT_Session::take(const QDomElement& x)
{
	QString from = x.attribute(QStringLiteral("from"));
	if (!from.endsWith(QLatin1String("chat.facebook.com"))) {
		// remove this code when chat.facebook.com is disabled completely
		from.clear();
	}
	if(!iqVerify(x, from, id()))
		return false;

	if(x.attribute(QStringLiteral("type")) == QLatin1String("result")) {
		setSuccess();
	}
	else {
		setError(x);
	}
	return true;
}

//----------------------------------------------------------------------------
// JT_Register
//----------------------------------------------------------------------------
class JT_Register::Private
{
public:
	Private() {}

	Form form;
	XData xdata;
	bool hasXData;
	Jid jid;
	int type;
};

JT_Register::JT_Register(Task *parent)
:Task(parent)
{
	d = new Private;
	d->type = -1;
	d->hasXData = false;
}

JT_Register::~JT_Register()
{
	delete d;
}

void JT_Register::reg(const QString &user, const QString &pass)
{
	d->type = 0;
	to = client()->host();
	iq = createIQ(doc(), QStringLiteral("set"), to.full(), id());
	QDomElement query = doc()->createElement(QStringLiteral("query"));
	query.setAttribute(QStringLiteral("xmlns"), QStringLiteral("jabber:iq:register"));
	iq.appendChild(query);
	query.appendChild(textTag(doc(), QStringLiteral("username"), user));
	query.appendChild(textTag(doc(), QStringLiteral("password"), pass));
}

void JT_Register::changepw(const QString &pass)
{
	d->type = 1;
	to = client()->host();
	iq = createIQ(doc(), QStringLiteral("set"), to.full(), id());
	QDomElement query = doc()->createElement(QStringLiteral("query"));
	query.setAttribute(QStringLiteral("xmlns"), QStringLiteral("jabber:iq:register"));
	iq.appendChild(query);
	query.appendChild(textTag(doc(), QStringLiteral("username"), client()->user()));
	query.appendChild(textTag(doc(), QStringLiteral("password"), pass));
}

void JT_Register::unreg(const Jid &j)
{
	d->type = 2;
	to = j.isEmpty() ? client()->host() : j.full();
	iq = createIQ(doc(), QStringLiteral("set"), to.full(), id());
	QDomElement query = doc()->createElement(QStringLiteral("query"));
	query.setAttribute(QStringLiteral("xmlns"), QStringLiteral("jabber:iq:register"));
	iq.appendChild(query);

	// this may be useful
	if(!d->form.key().isEmpty())
		query.appendChild(textTag(doc(), QStringLiteral("key"), d->form.key()));

	query.appendChild(doc()->createElement(QStringLiteral("remove")));
}

void JT_Register::getForm(const Jid &j)
{
	d->type = 3;
	to = j;
	iq = createIQ(doc(), QStringLiteral("get"), to.full(), id());
	QDomElement query = doc()->createElement(QStringLiteral("query"));
	query.setAttribute(QStringLiteral("xmlns"), QStringLiteral("jabber:iq:register"));
	iq.appendChild(query);
}

void JT_Register::setForm(const Form &form)
{
	d->type = 4;
	to = form.jid();
	iq = createIQ(doc(), QStringLiteral("set"), to.full(), id());
	QDomElement query = doc()->createElement(QStringLiteral("query"));
	query.setAttribute(QStringLiteral("xmlns"), QStringLiteral("jabber:iq:register"));
	iq.appendChild(query);

	// key?
	if(!form.key().isEmpty())
		query.appendChild(textTag(doc(), QStringLiteral("key"), form.key()));

	// fields
	for(Form::ConstIterator it = form.begin(); it != form.end(); ++it) {
		const FormField &f = *it;
		query.appendChild(textTag(doc(), f.realName(), f.value()));
	}
}

void JT_Register::setForm(const Jid& to, const XData& xdata)
{
	d->type = 4;
	iq = createIQ(doc(), QStringLiteral("set"), to.full(), id());
	QDomElement query = doc()->createElement(QStringLiteral("query"));
	query.setAttribute(QStringLiteral("xmlns"), QStringLiteral("jabber:iq:register"));
	iq.appendChild(query);
	query.appendChild(xdata.toXml(doc(), true));
}

const Form & JT_Register::form() const
{
	return d->form;
}

bool JT_Register::hasXData() const
{
	return d->hasXData;
}

const XData& JT_Register::xdata() const
{
	return d->xdata;
}

void JT_Register::onGo()
{
	send(iq);
}

bool JT_Register::take(const QDomElement &x)
{
	if(!iqVerify(x, to, id()))
		return false;

	Jid from(x.attribute(QStringLiteral("from")));
	if(x.attribute(QStringLiteral("type")) == QLatin1String("result")) {
		if(d->type == 3) {
			d->form.clear();
			d->form.setJid(from);

			QDomElement q = queryTag(x);
			for(QDomNode n = q.firstChild(); !n.isNull(); n = n.nextSibling()) {
				QDomElement i = n.toElement();
				if(i.isNull())
					continue;

				if(i.tagName() == QLatin1String("instructions"))
					d->form.setInstructions(tagContent(i));
				else if(i.tagName() == QLatin1String("key"))
					d->form.setKey(tagContent(i));
				else if(i.tagName() == QLatin1String("x") && i.attribute(QStringLiteral("xmlns")) == QLatin1String("jabber:x:data")) {
					d->xdata.fromXml(i);
					d->hasXData = true;
				}
				else if(i.tagName() == QLatin1String("data") && i.attribute(QStringLiteral("xmlns")) == QLatin1String("urn:xmpp:bob")) {
					client()->bobManager()->append(BoBData(i)); // xep-0231
				}
				else {
					FormField f;
					if(f.setType(i.tagName())) {
						f.setValue(tagContent(i));
						d->form += f;
					}
				}
			}
		}

		setSuccess();
	}
	else
		setError(x);

	return true;
}

//----------------------------------------------------------------------------
// JT_UnRegister
//----------------------------------------------------------------------------
class JT_UnRegister::Private
{
public:
	Private() { }

	Jid j;
	JT_Register *jt_reg;
};

JT_UnRegister::JT_UnRegister(Task *parent)
: Task(parent)
{
	d = new Private;
	d->jt_reg = 0;
}

JT_UnRegister::~JT_UnRegister()
{
	delete d->jt_reg;
	delete d;
}

void JT_UnRegister::unreg(const Jid &j)
{
	d->j = j;
}

void JT_UnRegister::onGo()
{
	delete d->jt_reg;

	d->jt_reg = new JT_Register(this);
	d->jt_reg->getForm(d->j);
	connect(d->jt_reg, SIGNAL(finished()), SLOT(getFormFinished()));
	d->jt_reg->go(false);
}

void JT_UnRegister::getFormFinished()
{
	disconnect(d->jt_reg, 0, this, 0);

	d->jt_reg->unreg(d->j);
	connect(d->jt_reg, SIGNAL(finished()), SLOT(unregFinished()));
	d->jt_reg->go(false);
}

void JT_UnRegister::unregFinished()
{
	if ( d->jt_reg->success() )
		setSuccess();
	else
		setError(d->jt_reg->statusCode(), d->jt_reg->statusString());

	delete d->jt_reg;
	d->jt_reg = 0;
}

//----------------------------------------------------------------------------
// JT_Roster
//----------------------------------------------------------------------------
class JT_Roster::Private
{
public:
	Private() {}

	Roster roster;
	QList<QDomElement> itemList;
};

JT_Roster::JT_Roster(Task *parent)
:Task(parent)
{
	type = -1;
	d = new Private;
}

JT_Roster::~JT_Roster()
{
	delete d;
}

void JT_Roster::get()
{
	type = 0;
	//to = client()->host();
	iq = createIQ(doc(), QStringLiteral("get"), to.full(), id());
	QDomElement query = doc()->createElement(QStringLiteral("query"));
	query.setAttribute(QStringLiteral("xmlns"), QStringLiteral("jabber:iq:roster"));
	iq.appendChild(query);
}

void JT_Roster::set(const Jid &jid, const QString &name, const QStringList &groups)
{
	type = 1;
	//to = client()->host();
	QDomElement item = doc()->createElement(QStringLiteral("item"));
	item.setAttribute(QStringLiteral("jid"), jid.full());
	if(!name.isEmpty())
		item.setAttribute(QStringLiteral("name"), name);
	for(QStringList::ConstIterator it = groups.begin(); it != groups.end(); ++it)
		item.appendChild(textTag(doc(), QStringLiteral("group"), *it));
	d->itemList += item;
}

void JT_Roster::remove(const Jid &jid)
{
	type = 1;
	//to = client()->host();
	QDomElement item = doc()->createElement(QStringLiteral("item"));
	item.setAttribute(QStringLiteral("jid"), jid.full());
	item.setAttribute(QStringLiteral("subscription"), QStringLiteral("remove"));
	d->itemList += item;
}

void JT_Roster::onGo()
{
	if(type == 0)
		send(iq);
	else if(type == 1) {
		//to = client()->host();
		iq = createIQ(doc(), QStringLiteral("set"), to.full(), id());
		QDomElement query = doc()->createElement(QStringLiteral("query"));
		query.setAttribute(QStringLiteral("xmlns"), QStringLiteral("jabber:iq:roster"));
		iq.appendChild(query);
		foreach (const QDomElement& it, d->itemList)
			query.appendChild(it);
		send(iq);
	}
}

const Roster & JT_Roster::roster() const
{
	return d->roster;
}

QString JT_Roster::toString() const
{
	if(type != 1)
		return QLatin1String("");

	QDomElement i = doc()->createElement(QStringLiteral("request"));
	i.setAttribute(QStringLiteral("type"), QStringLiteral("JT_Roster"));
	foreach (const QDomElement& it, d->itemList)
		i.appendChild(it);
	return lineEncode(Stream::xmlToString(i));
	return QLatin1String("");
}

bool JT_Roster::fromString(const QString &str)
{
	QDomDocument *dd = new QDomDocument;
	if(!dd->setContent(lineDecode(str).toUtf8()))
		return false;
	QDomElement e = doc()->importNode(dd->documentElement(), true).toElement();
	delete dd;

	if(e.tagName() != QLatin1String("request") || e.attribute(QStringLiteral("type")) != QLatin1String("JT_Roster"))
		return false;

	type = 1;
	d->itemList.clear();
	for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement i = n.toElement();
		if(i.isNull())
			continue;
		d->itemList += i;
	}

	return true;
}

bool JT_Roster::take(const QDomElement &x)
{
	if(!iqVerify(x, client()->host(), id()))
		return false;

	// get
	if(type == 0) {
		if(x.attribute(QStringLiteral("type")) == QLatin1String("result")) {
			QDomElement q = queryTag(x);
			d->roster = xmlReadRoster(q, false);
			setSuccess();
		}
		else {
			setError(x);
		}

		return true;
	}
	// set
	else if(type == 1) {
		if(x.attribute(QStringLiteral("type")) == QLatin1String("result"))
			setSuccess();
		else
			setError(x);

		return true;
	}
	// remove
	else if(type == 2) {
		setSuccess();
		return true;
	}

	return false;
}

//----------------------------------------------------------------------------
// JT_PushRoster
//----------------------------------------------------------------------------
JT_PushRoster::JT_PushRoster(Task *parent)
:Task(parent)
{
}

JT_PushRoster::~JT_PushRoster()
{
}

bool JT_PushRoster::take(const QDomElement &e)
{
	// must be an iq-set tag
	if(e.tagName() != QLatin1String("iq") || e.attribute(QStringLiteral("type")) != QLatin1String("set"))
		return false;

	if(!iqVerify(e, client()->host(), QLatin1String(""), QStringLiteral("jabber:iq:roster")))
		return false;

	roster(xmlReadRoster(queryTag(e), true));
	send(createIQ(doc(), QStringLiteral("result"), e.attribute(QStringLiteral("from")), e.attribute(QStringLiteral("id"))));

	return true;
}

//----------------------------------------------------------------------------
// JT_Presence
//----------------------------------------------------------------------------
JT_Presence::JT_Presence(Task *parent)
:Task(parent)
{
	type = -1;
}

JT_Presence::~JT_Presence()
{
}

void JT_Presence::pres(const Status &s)
{
	type = 0;

	tag = doc()->createElement(QStringLiteral("presence"));
	if(!s.isAvailable()) {
		tag.setAttribute(QStringLiteral("type"), QStringLiteral("unavailable"));
		if(!s.status().isEmpty())
			tag.appendChild(textTag(doc(), QStringLiteral("status"), s.status()));
	}
	else {
		if(s.isInvisible())
			tag.setAttribute(QStringLiteral("type"), QStringLiteral("invisible"));

		if(!s.show().isEmpty())
			tag.appendChild(textTag(doc(), QStringLiteral("show"), s.show()));
		if(!s.status().isEmpty())
			tag.appendChild(textTag(doc(), QStringLiteral("status"), s.status()));

		tag.appendChild( textTag(doc(), QStringLiteral("priority"), QStringLiteral("%1").arg(s.priority()) ) );

		if(!s.keyID().isEmpty()) {
			QDomElement x = textTag(doc(), QStringLiteral("x"), s.keyID());
			x.setAttribute(QStringLiteral("xmlns"), QStringLiteral("http://jabber.org/protocol/e2e"));
			tag.appendChild(x);
		}
		if(!s.xsigned().isEmpty()) {
			QDomElement x = textTag(doc(), QStringLiteral("x"), s.xsigned());
			x.setAttribute(QStringLiteral("xmlns"), QStringLiteral("jabber:x:signed"));
			tag.appendChild(x);
		}

		if (client()->capsManager()->isEnabled()) {
			CapsSpec cs = client()->caps();
			if (cs.isValid()) {
				tag.appendChild(cs.toXml(doc()));
			}
		}

		if(s.isMUC()) {
			QDomElement m = doc()->createElement(QStringLiteral("x"));
			m.setAttribute(QStringLiteral("xmlns"),QStringLiteral("http://jabber.org/protocol/muc"));
			if (!s.mucPassword().isEmpty()) {
				m.appendChild(textTag(doc(),QStringLiteral("password"),s.mucPassword()));
			}
			if (s.hasMUCHistory()) {
				QDomElement h = doc()->createElement(QStringLiteral("history"));
				if (s.mucHistoryMaxChars() >= 0)
					h.setAttribute(QStringLiteral("maxchars"),s.mucHistoryMaxChars());
				if (s.mucHistoryMaxStanzas() >= 0)
					h.setAttribute(QStringLiteral("maxstanzas"),s.mucHistoryMaxStanzas());
				if (s.mucHistorySeconds() >= 0)
					h.setAttribute(QStringLiteral("seconds"),s.mucHistorySeconds());
				if (!s.mucHistorySince().isNull())
					h.setAttribute(QStringLiteral("since"), s.mucHistorySince().toUTC().addSecs(1).toString(Qt::ISODate));
				m.appendChild(h);
			}
			tag.appendChild(m);
		}

		if(s.hasPhotoHash()) {
			QDomElement m = doc()->createElement(QStringLiteral("x"));
			m.setAttribute(QStringLiteral("xmlns"), QStringLiteral("vcard-temp:x:update"));
			m.appendChild(textTag(doc(), QStringLiteral("photo"), s.photoHash()));
			tag.appendChild(m);
		}

		// bits of binary
		foreach(const BoBData &bd, s.bobDataList()) {
			tag.appendChild(bd.toXml(doc()));
		}
	}
}

void JT_Presence::pres(const Jid &to, const Status &s)
{
	pres(s);
	tag.setAttribute(QStringLiteral("to"), to.full());
}

void JT_Presence::sub(const Jid &to, const QString &subType, const QString& nick)
{
	type = 1;

	tag = doc()->createElement(QStringLiteral("presence"));
	tag.setAttribute(QStringLiteral("to"), to.full());
	tag.setAttribute(QStringLiteral("type"), subType);
	if (!nick.isEmpty()) {
		QDomElement nick_tag = textTag(doc(),QStringLiteral("nick"),nick);
		nick_tag.setAttribute(QStringLiteral("xmlns"),QStringLiteral("http://jabber.org/protocol/nick"));
		tag.appendChild(nick_tag);
	}
}

void JT_Presence::probe(const Jid &to)
{
	type = 2;

	tag = doc()->createElement(QStringLiteral("presence"));
	tag.setAttribute(QStringLiteral("to"), to.full());
	tag.setAttribute(QStringLiteral("type"), QStringLiteral("probe"));
}

void JT_Presence::onGo()
{
	send(tag);
	setSuccess();
}

//----------------------------------------------------------------------------
// JT_PushPresence
//----------------------------------------------------------------------------
JT_PushPresence::JT_PushPresence(Task *parent)
:Task(parent)
{
}

JT_PushPresence::~JT_PushPresence()
{
}

bool JT_PushPresence::take(const QDomElement &e)
{
	if(e.tagName() != QLatin1String("presence"))
		return false;

	Jid j(e.attribute(QStringLiteral("from")));
	Status p;

	if(e.hasAttribute(QStringLiteral("type"))) {
		QString type = e.attribute(QStringLiteral("type"));
		if(type == QLatin1String("unavailable")) {
			p.setIsAvailable(false);
		}
		else if(type == QLatin1String("error")) {
			QString str = QLatin1String("");
			int code = 0;
			getErrorFromElement(e, client()->stream().baseNS(), &code, &str);
			p.setError(code, str);
		}
		else if(type == QLatin1String("subscribe") || type == QLatin1String("subscribed") || type == QLatin1String("unsubscribe") || type == QLatin1String("unsubscribed")) {
			QString nick;
			QDomElement tag = e.firstChildElement(QStringLiteral("nick"));
			if (!tag.isNull() && tag.attribute(QStringLiteral("xmlns")) == QLatin1String("http://jabber.org/protocol/nick")) {
				nick = tagContent(tag);
			}
			subscription(j, type, nick);
			return true;
		}
	}

	QDomElement tag;

	tag = e.firstChildElement(QStringLiteral("status"));
	if(!tag.isNull())
		p.setStatus(tagContent(tag));
	tag = e.firstChildElement(QStringLiteral("show"));
	if(!tag.isNull())
		p.setShow(tagContent(tag));
	tag = e.firstChildElement(QStringLiteral("priority"));
	if(!tag.isNull())
		p.setPriority(tagContent(tag).toInt());

	QDateTime stamp;

	for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement i = n.toElement();
		if(i.isNull())
			continue;

		if(i.tagName() == QLatin1String("x") && i.attribute(QStringLiteral("xmlns")) == QLatin1String("jabber:x:delay")) {
			if(i.hasAttribute(QStringLiteral("stamp")) && !stamp.isValid()) {
				stamp = stamp2TS(i.attribute(QStringLiteral("stamp")));
			}
		}
		else if(i.tagName() == QLatin1String("delay") && i.attribute(QStringLiteral("xmlns")) == QLatin1String("urn:xmpp:delay")) {
			if(i.hasAttribute(QStringLiteral("stamp")) && !stamp.isValid()) {
				stamp = QDateTime::fromString(i.attribute(QStringLiteral("stamp")).left(19), Qt::ISODate);
			}
		}
		else if(i.tagName() == QLatin1String("x") && i.attribute(QStringLiteral("xmlns")) == QLatin1String("gabber:x:music:info")) {
			QDomElement t;
			QString title, state;

			t = i.firstChildElement(QStringLiteral("title"));
			if(!t.isNull())
				title = tagContent(t);
			t = i.firstChildElement(QStringLiteral("state"));
			if(!t.isNull())
				state = tagContent(t);

			if(!title.isEmpty() && state == QLatin1String("playing"))
				p.setSongTitle(title);
		}
		else if(i.tagName() == QLatin1String("x") && i.attribute(QStringLiteral("xmlns")) == QLatin1String("jabber:x:signed")) {
			p.setXSigned(tagContent(i));
		}
		else if(i.tagName() == QLatin1String("x") && i.attribute(QStringLiteral("xmlns")) == QLatin1String("http://jabber.org/protocol/e2e")) {
			p.setKeyID(tagContent(i));
		}
 		else if(i.tagName() == QLatin1String("c") && i.attribute(QStringLiteral("xmlns")) == NS_CAPS) {
			p.setCaps(CapsSpec::fromXml(i));
			if(!e.hasAttribute(QStringLiteral("type")) && p.caps().isValid()) {
				client()->capsManager()->updateCaps(j, p.caps());
			}
  		}
		else if(i.tagName() == QLatin1String("x") && i.attribute(QStringLiteral("xmlns")) == QLatin1String("vcard-temp:x:update")) {
			QDomElement t;
			t = i.firstChildElement(QStringLiteral("photo"));
			if (!t.isNull())
				p.setPhotoHash(tagContent(t));
			else
				p.setPhotoHash(QLatin1String(""));
		}
		else if(i.tagName() == QLatin1String("x") && i.attribute(QStringLiteral("xmlns")) == QLatin1String("http://jabber.org/protocol/muc#user")) {
			for(QDomNode muc_n = i.firstChild(); !muc_n.isNull(); muc_n = muc_n.nextSibling()) {
				QDomElement muc_e = muc_n.toElement();
				if(muc_e.isNull())
					continue;

				if (muc_e.tagName() == QLatin1String("item"))
					p.setMUCItem(MUCItem(muc_e));
				else if (muc_e.tagName() == QLatin1String("status"))
					p.addMUCStatus(muc_e.attribute(QStringLiteral("code")).toInt());
				else if (muc_e.tagName() == QLatin1String("destroy"))
					p.setMUCDestroy(MUCDestroy(muc_e));
			}
		}
		else if (i.tagName() == QLatin1String("data") && i.attribute(QStringLiteral("xmlns")) == QLatin1String("urn:xmpp:bob")) {
			BoBData bd(i);
			client()->bobManager()->append(bd);
			p.addBoBData(bd);
		}
	}

	if (stamp.isValid()) {
		if (client()->manualTimeZoneOffset()) {
			stamp = stamp.addSecs(client()->timeZoneOffset() * 3600);
		} else {
			stamp.setTimeSpec(Qt::UTC);
			stamp = stamp.toLocalTime();
		}
		p.setTimeStamp(stamp);
	}

	presence(j, p);

	return true;
}

//----------------------------------------------------------------------------
// JT_Message
//----------------------------------------------------------------------------
static QDomElement oldStyleNS(const QDomElement &e)
{
	// find closest parent with a namespace
	QDomNode par = e.parentNode();
	while(!par.isNull() && par.namespaceURI().isNull())
		par = par.parentNode();
	bool noShowNS = false;
	if(!par.isNull() && par.namespaceURI() == e.namespaceURI())
		noShowNS = true;

	QDomElement i;
	int x;
	//if(noShowNS)
		i = e.ownerDocument().createElement(e.tagName());
	//else
	//	i = e.ownerDocument().createElementNS(e.namespaceURI(), e.tagName());

	// copy attributes
	QDomNamedNodeMap al = e.attributes();
	for(x = 0; x < al.count(); ++x)
		i.setAttributeNode(al.item(x).cloneNode().toAttr());

	if(!noShowNS)
		i.setAttribute(QStringLiteral("xmlns"), e.namespaceURI());

	// copy children
	QDomNodeList nl = e.childNodes();
	for(x = 0; x < nl.count(); ++x) {
		QDomNode n = nl.item(x);
		if(n.isElement())
			i.appendChild(oldStyleNS(n.toElement()));
		else
			i.appendChild(n.cloneNode());
	}
	return i;
}

JT_Message::JT_Message(Task *parent, const Message &msg)
:Task(parent)
{
	m = msg;
	if (m.id().isEmpty())
		m.setId(id());
}

JT_Message::~JT_Message()
{
}

void JT_Message::onGo()
{
	Stanza s = m.toStanza(&(client()->stream()));
	QDomElement e = oldStyleNS(s.element());
	send(e);
	setSuccess();
}

//----------------------------------------------------------------------------
// JT_PushMessage
//----------------------------------------------------------------------------
JT_PushMessage::JT_PushMessage(Task *parent)
:Task(parent)
{
}

JT_PushMessage::~JT_PushMessage()
{
}

bool JT_PushMessage::take(const QDomElement &e)
{
	if(e.tagName() != QLatin1String("message"))
		return false;

	QDomElement e1 = e;
	QDomElement forward;
	Message::CarbonDir cd = Message::NoCarbon;

	Jid fromJid = Jid(e1.attribute(QLatin1String("from")));
	// Check for Carbon
	QDomNodeList list = e1.childNodes();
	for (int i = 0; i < list.size(); ++i) {
		QDomElement el = list.at(i).toElement();

		if (el.attribute("xmlns") == QLatin1String("urn:xmpp:carbons:2")
		    && (el.tagName() == QLatin1String("received") || el.tagName() == QLatin1String("sent"))
		    && fromJid.compare(Jid(e1.attribute(QLatin1String("to"))), false)) {
			QDomElement el1 = el.firstChildElement();
			if (el1.tagName() == QLatin1String("forwarded")
			    && el1.attribute(QLatin1String("xmlns")) == QLatin1String("urn:xmpp:forward:0")) {
				QDomElement el2 = el1.firstChildElement(QLatin1String("message"));
				if (!el2.isNull()) {
					forward = el2;
					cd = el.tagName() == QLatin1String("received")? Message::Received : Message::Sent;
					break;
				}
			}
		}
		else if (el.tagName() == QLatin1String("forwarded")
			 && el.attribute(QLatin1String("xmlns")) == QLatin1String("urn:xmpp:forward:0")) {
			forward = el.firstChildElement(QLatin1String("message")); // currently only messages are supportted
			// TODO <delay> element support
			if (!forward.isNull()) {
				break;
			}
		}
	}

	Stanza s = client()->stream().createStanza(addCorrectNS(forward.isNull()? e1 : forward));
	if(s.isNull()) {
		//printf("take: bad stanza??\n");
		return false;
	}

	Message m;
	if(!m.fromStanza(s, client()->manualTimeZoneOffset(), client()->timeZoneOffset())) {
		//printf("bad message\n");
		return false;
	}
	if (!forward.isNull()) {
		m.setForwardedFrom(fromJid);
		m.setCarbonDirection(cd);
	}

	emit message(m);
	return true;
}

//----------------------------------------------------------------------------
// JT_GetServices
//----------------------------------------------------------------------------
JT_GetServices::JT_GetServices(Task *parent)
:Task(parent)
{
}

void JT_GetServices::get(const Jid &j)
{
	agentList.clear();

	jid = j;
	iq = createIQ(doc(), QStringLiteral("get"), jid.full(), id());
	QDomElement query = doc()->createElement(QStringLiteral("query"));
	query.setAttribute(QStringLiteral("xmlns"), QStringLiteral("jabber:iq:agents"));
	iq.appendChild(query);
}

const AgentList & JT_GetServices::agents() const
{
	return agentList;
}

void JT_GetServices::onGo()
{
	send(iq);
}

bool JT_GetServices::take(const QDomElement &x)
{
	if(!iqVerify(x, jid, id()))
		return false;

	if(x.attribute(QStringLiteral("type")) == QLatin1String("result")) {
		QDomElement q = queryTag(x);

		// agents
		for(QDomNode n = q.firstChild(); !n.isNull(); n = n.nextSibling()) {
			QDomElement i = n.toElement();
			if(i.isNull())
				continue;

			if(i.tagName() == QLatin1String("agent")) {
				AgentItem a;

				a.setJid(Jid(i.attribute(QStringLiteral("jid"))));

				QDomElement tag;

				tag = i.firstChildElement(QStringLiteral("name"));
				if(!tag.isNull())
					a.setName(tagContent(tag));

				// determine which namespaces does item support
				QStringList ns;

				tag = i.firstChildElement(QStringLiteral("register"));
				if(!tag.isNull())
					ns << QStringLiteral("jabber:iq:register");
				tag = i.firstChildElement(QStringLiteral("search"));
				if(!tag.isNull())
					ns << QStringLiteral("jabber:iq:search");
				tag = i.firstChildElement(QStringLiteral("groupchat"));
				if(!tag.isNull())
					ns << QStringLiteral("jabber:iq:conference");
				tag = i.firstChildElement(QStringLiteral("transport"));
				if(!tag.isNull())
					ns << QStringLiteral("jabber:iq:gateway");

				a.setFeatures(ns);

				agentList += a;
			}
		}

		setSuccess(true);
	}
	else {
		setError(x);
	}

	return true;
}

//----------------------------------------------------------------------------
// JT_VCard
//----------------------------------------------------------------------------
class JT_VCard::Private
{
public:
	Private() {}

	QDomElement iq;
	Jid jid;
	VCard vcard;
};

JT_VCard::JT_VCard(Task *parent)
:Task(parent)
{
	type = -1;
	d = new Private;
}

JT_VCard::~JT_VCard()
{
	delete d;
}

void JT_VCard::get(const Jid &_jid)
{
	type = 0;
	d->jid = _jid;
	d->iq = createIQ(doc(), QStringLiteral("get"), type == 1 ? Jid().full() : d->jid.full(), id());
	QDomElement v = doc()->createElement(QStringLiteral("vCard"));
	v.setAttribute(QStringLiteral("xmlns"), QStringLiteral("vcard-temp"));
	d->iq.appendChild(v);
}

const Jid & JT_VCard::jid() const
{
	return d->jid;
}

const VCard & JT_VCard::vcard() const
{
	return d->vcard;
}

void JT_VCard::set(const VCard &card)
{
	type = 1;
	d->vcard = card;
	d->jid = "";
	d->iq = createIQ(doc(), QStringLiteral("set"), d->jid.full(), id());
	d->iq.appendChild(card.toXml(doc()) );
}

// isTarget is when we setting target's vcard. for example in case of muc own vcard
void JT_VCard::set(const Jid &j, const VCard &card, bool isTarget)
{
	type = 1;
	d->vcard = card;
	d->jid = j;
	d->iq = createIQ(doc(), QStringLiteral("set"), isTarget? j.full() : QLatin1String(""), id());
	d->iq.appendChild(card.toXml(doc()) );
}

void JT_VCard::onGo()
{
	send(d->iq);
}

bool JT_VCard::take(const QDomElement &x)
{
	Jid to = d->jid;
	if (to.bare() == client()->jid().bare())
		to = client()->host();
	if(!iqVerify(x, to, id()))
		return false;

	if(x.attribute(QStringLiteral("type")) == QLatin1String("result")) {
		if(type == 0) {
			for(QDomNode n = x.firstChild(); !n.isNull(); n = n.nextSibling()) {
				QDomElement q = n.toElement();
				if(q.isNull())
					continue;

				if(q.tagName().toUpper() == QLatin1String("VCARD")) {
					d->vcard = VCard::fromXml(q);
					if(d->vcard) {
						setSuccess();
						return true;
					}
				}
			}

			setError(ErrDisc + 1, tr("No VCard available"));
			return true;
		}
		else {
			setSuccess();
			return true;
		}
	}
	else {
		setError(x);
	}

	return true;
}

//----------------------------------------------------------------------------
// JT_Search
//----------------------------------------------------------------------------
class JT_Search::Private
{
public:
	Private() {}

	Jid jid;
	Form form;
	bool hasXData;
	XData xdata;
	QList<SearchResult> resultList;
};

JT_Search::JT_Search(Task *parent)
:Task(parent)
{
	d = new Private;
	type = -1;
}

JT_Search::~JT_Search()
{
	delete d;
}

void JT_Search::get(const Jid &jid)
{
	type = 0;
	d->jid = jid;
	d->hasXData = false;
	d->xdata = XData();
	iq = createIQ(doc(), QStringLiteral("get"), d->jid.full(), id());
	QDomElement query = doc()->createElement(QStringLiteral("query"));
	query.setAttribute(QStringLiteral("xmlns"), QStringLiteral("jabber:iq:search"));
	iq.appendChild(query);
}

void JT_Search::set(const Form &form)
{
	type = 1;
	d->jid = form.jid();
	d->hasXData = false;
	d->xdata = XData();
	iq = createIQ(doc(), QStringLiteral("set"), d->jid.full(), id());
	QDomElement query = doc()->createElement(QStringLiteral("query"));
	query.setAttribute(QStringLiteral("xmlns"), QStringLiteral("jabber:iq:search"));
	iq.appendChild(query);

	// key?
	if(!form.key().isEmpty())
		query.appendChild(textTag(doc(), QStringLiteral("key"), form.key()));

	// fields
	for(Form::ConstIterator it = form.begin(); it != form.end(); ++it) {
		const FormField &f = *it;
		query.appendChild(textTag(doc(), f.realName(), f.value()));
	}
}

void JT_Search::set(const Jid &jid, const XData &form)
{
	type = 1;
	d->jid = jid;
	d->hasXData = false;
	d->xdata = XData();
	iq = createIQ(doc(), QStringLiteral("set"), d->jid.full(), id());
	QDomElement query = doc()->createElement(QStringLiteral("query"));
	query.setAttribute(QStringLiteral("xmlns"), QStringLiteral("jabber:iq:search"));
	iq.appendChild(query);
	query.appendChild(form.toXml(doc(), true));
}

const Form & JT_Search::form() const
{
	return d->form;
}

const QList<SearchResult> & JT_Search::results() const
{
	return d->resultList;
}

bool JT_Search::hasXData() const
{
	return d->hasXData;
}

const XData & JT_Search::xdata() const
{
	return d->xdata;
}

void JT_Search::onGo()
{
	send(iq);
}

bool JT_Search::take(const QDomElement &x)
{
	if(!iqVerify(x, d->jid, id()))
		return false;

	Jid from(x.attribute(QStringLiteral("from")));
	if(x.attribute(QStringLiteral("type")) == QLatin1String("result")) {
		if(type == 0) {
			d->form.clear();
			d->form.setJid(from);

			QDomElement q = queryTag(x);
			for(QDomNode n = q.firstChild(); !n.isNull(); n = n.nextSibling()) {
				QDomElement i = n.toElement();
				if(i.isNull())
					continue;

				if(i.tagName() == QLatin1String("instructions"))
					d->form.setInstructions(tagContent(i));
				else if(i.tagName() == QLatin1String("key"))
					d->form.setKey(tagContent(i));
				else if(i.tagName() == QLatin1String("x") && i.attribute(QStringLiteral("xmlns")) == QLatin1String("jabber:x:data")) {
					d->xdata.fromXml(i);
					d->hasXData = true;
				}
				else {
					FormField f;
					if(f.setType(i.tagName())) {
						f.setValue(tagContent(i));
						d->form += f;
					}
				}
			}
		}
		else {
			d->resultList.clear();

			QDomElement q = queryTag(x);
			for(QDomNode n = q.firstChild(); !n.isNull(); n = n.nextSibling()) {
				QDomElement i = n.toElement();
				if(i.isNull())
					continue;

				if(i.tagName() == QLatin1String("item")) {
					SearchResult r(Jid(i.attribute(QStringLiteral("jid"))));

					QDomElement tag;

					tag = i.firstChildElement(QStringLiteral("nick"));
					if(!tag.isNull())
						r.setNick(tagContent(tag));
					tag = i.firstChildElement(QStringLiteral("first"));
					if(!tag.isNull())
						r.setFirst(tagContent(tag));
					tag = i.firstChildElement(QStringLiteral("last"));
					if(!tag.isNull())
						r.setLast(tagContent(tag));
					tag = i.firstChildElement(QStringLiteral("email"));
					if(!tag.isNull())
						r.setEmail(tagContent(tag));

					d->resultList += r;
				}
				else if(i.tagName() == QLatin1String("x") && i.attribute(QStringLiteral("xmlns")) == QLatin1String("jabber:x:data")) {
					d->xdata.fromXml(i);
					d->hasXData = true;
				}
			}
		}
		setSuccess();
	}
	else {
		setError(x);
	}

	return true;
}

//----------------------------------------------------------------------------
// JT_ClientVersion
//----------------------------------------------------------------------------
JT_ClientVersion::JT_ClientVersion(Task *parent)
:Task(parent)
{
}

void JT_ClientVersion::get(const Jid &jid)
{
	j = jid;
	iq = createIQ(doc(), QStringLiteral("get"), j.full(), id());
	QDomElement query = doc()->createElement(QStringLiteral("query"));
	query.setAttribute(QStringLiteral("xmlns"), QStringLiteral("jabber:iq:version"));
	iq.appendChild(query);
}

void JT_ClientVersion::onGo()
{
	send(iq);
}

bool JT_ClientVersion::take(const QDomElement &x)
{
	if(!iqVerify(x, j, id()))
		return false;

	if(x.attribute(QStringLiteral("type")) == QLatin1String("result")) {
		QDomElement q = queryTag(x);
		QDomElement tag;
		tag = q.firstChildElement(QStringLiteral("name"));
		if(!tag.isNull())
			v_name = tagContent(tag);
		tag = q.firstChildElement(QStringLiteral("version"));
		if(!tag.isNull())
			v_ver = tagContent(tag);
		tag = q.firstChildElement(QStringLiteral("os"));
		if(!tag.isNull())
			v_os = tagContent(tag);

		setSuccess();
	}
	else {
		setError(x);
	}

	return true;
}

const Jid & JT_ClientVersion::jid() const
{
	return j;
}

const QString & JT_ClientVersion::name() const
{
	return v_name;
}

const QString & JT_ClientVersion::version() const
{
	return v_ver;
}

const QString & JT_ClientVersion::os() const
{
	return v_os;
}

//----------------------------------------------------------------------------
// JT_EntityTime
//----------------------------------------------------------------------------
JT_EntityTime::JT_EntityTime(Task* parent) : Task(parent)
{
}

/**
 * \brief Queried entity's JID.
 */
const Jid & JT_EntityTime::jid() const
{
	return j;
}

/**
 * \brief Prepares the task to get information from JID.
 */
void JT_EntityTime::get(const Jid &jid)
{
	j = jid;
	iq = createIQ(doc(), QStringLiteral("get"), jid.full(), id());
	QDomElement time = doc()->createElement(QStringLiteral("time"));
	time.setAttribute(QStringLiteral("xmlns"), QStringLiteral("urn:xmpp:time"));
	iq.appendChild(time);
}

void JT_EntityTime::onGo()
{
	send(iq);
}

bool JT_EntityTime::take(const QDomElement &x)
{
	if (!iqVerify(x, j, id()))
		return false;

	if (x.attribute(QStringLiteral("type")) == QLatin1String("result")) {
		QDomElement q = x.firstChildElement(QStringLiteral("time"));
		QDomElement tag;
		tag = q.firstChildElement(QStringLiteral("utc"));
		do {
			if (tag.isNull()) {
				break;
			}
			utc = QDateTime::fromString(tagContent(tag), Qt::ISODate);
			tag = q.firstChildElement(QStringLiteral("tzo"));
			if (!utc.isValid() || tag.isNull()) {
				break;
			}
			tzo = TimeZone::tzdToInt(tagContent(tag));
			if (tzo == -1) {
				break;
			}
			setSuccess();
			return true;
		}
		while (false);
		setError(406);
	}
	else {
		setError(x);
	}

	return true;
}

const QDateTime & JT_EntityTime::dateTime() const
{
	return utc;
}

int JT_EntityTime::timezoneOffset() const
{
	return tzo;
}

//----------------------------------------------------------------------------
// JT_ServInfo
//----------------------------------------------------------------------------
JT_ServInfo::JT_ServInfo(Task *parent)
:Task(parent)
{
}

JT_ServInfo::~JT_ServInfo()
{
}

bool JT_ServInfo::take(const QDomElement &e)
{
	if(e.tagName() != QLatin1String("iq") || e.attribute(QStringLiteral("type")) != QLatin1String("get"))
		return false;

	QString ns = queryNS(e);
	if(ns == QLatin1String("jabber:iq:version")) {
		QDomElement iq = createIQ(doc(), QStringLiteral("result"), e.attribute(QStringLiteral("from")), e.attribute(QStringLiteral("id")));
		QDomElement query = doc()->createElement(QStringLiteral("query"));
		query.setAttribute(QStringLiteral("xmlns"), QStringLiteral("jabber:iq:version"));
		iq.appendChild(query);
		query.appendChild(textTag(doc(), QStringLiteral("name"), client()->clientName()));
		query.appendChild(textTag(doc(), QStringLiteral("version"), client()->clientVersion()));
		query.appendChild(textTag(doc(), QStringLiteral("os"), client()->OSName() + ' ' + client()->OSVersion()));
		send(iq);
		return true;
	}
	else if(ns == QLatin1String("http://jabber.org/protocol/disco#info")) {
		// Find out the node
		QString node;
		QDomElement q = e.firstChildElement(QStringLiteral("query"));
		if(!q.isNull()) // NOTE: Should always be true, since a NS was found above
			node = q.attribute(QStringLiteral("node"));

		if (node.isEmpty() || node == client()->caps().flatten()) {

			QDomElement iq = createIQ(doc(), QStringLiteral("result"), e.attribute(QStringLiteral("from")), e.attribute(QStringLiteral("id")));
			DiscoItem item = client()->makeDiscoResult(node);
			iq.appendChild(item.toDiscoInfoResult(doc()));
			send(iq);
		}
		else {
			// Create error reply
			QDomElement error_reply = createIQ(doc(), QStringLiteral("result"), e.attribute(QStringLiteral("from")), e.attribute(QStringLiteral("id")));

			// Copy children
			for (QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
				error_reply.appendChild(n.cloneNode());
			}

			// Add error
			QDomElement error = doc()->createElement(QStringLiteral("error"));
			error.setAttribute(QStringLiteral("type"),QStringLiteral("cancel"));
			error_reply.appendChild(error);
			QDomElement error_type = doc()->createElement(QStringLiteral("item-not-found"));
			error_type.setAttribute(QStringLiteral("xmlns"),QStringLiteral("urn:ietf:params:xml:ns:xmpp-stanzas"));
			error.appendChild(error_type);
			send(error_reply);
		}
		return true;
	}
	if (!ns.isEmpty()) {
		return false;
	}

	ns = e.firstChildElement(QStringLiteral("time")).attribute(QStringLiteral("xmlns"));
	if (ns == QLatin1String("urn:xmpp:time")) {
		QDomElement iq = createIQ(doc(), QStringLiteral("result"), e.attribute(QStringLiteral("from")), e.attribute(QStringLiteral("id")));
		QDomElement time = doc()->createElement(QStringLiteral("time"));
		time.setAttribute(QStringLiteral("xmlns"), ns);
		iq.appendChild(time);

		QDateTime local = QDateTime::currentDateTime();

		int off = TimeZone::offsetFromUtc();
		QTime t = QTime(0, 0).addSecs(qAbs(off)*60);
		QString tzo = (off < 0 ? "-" : "+") + t.toString(QStringLiteral("HH:mm"));
		time.appendChild(textTag(doc(), QStringLiteral("tzo"), tzo));
		QString localTimeStr = local.toUTC().toString(Qt::ISODate);
		if (!localTimeStr.endsWith(QLatin1String("Z")))
			localTimeStr.append("Z");
		time.appendChild(textTag(doc(), QStringLiteral("utc"), localTimeStr));

		send(iq);
		return true;
	}

	return false;
}

//----------------------------------------------------------------------------
// JT_Gateway
//----------------------------------------------------------------------------
JT_Gateway::JT_Gateway(Task *parent)
:Task(parent)
{
	type = -1;
}

void JT_Gateway::get(const Jid &jid)
{
	type = 0;
	v_jid = jid;
	iq = createIQ(doc(), QStringLiteral("get"), v_jid.full(), id());
	QDomElement query = doc()->createElement(QStringLiteral("query"));
	query.setAttribute(QStringLiteral("xmlns"), QStringLiteral("jabber:iq:gateway"));
	iq.appendChild(query);
}

void JT_Gateway::set(const Jid &jid, const QString &prompt)
{
	type = 1;
	v_jid = jid;
	v_prompt = prompt;
	iq = createIQ(doc(), QStringLiteral("set"), v_jid.full(), id());
	QDomElement query = doc()->createElement(QStringLiteral("query"));
	query.setAttribute(QStringLiteral("xmlns"), QStringLiteral("jabber:iq:gateway"));
	iq.appendChild(query);
	query.appendChild(textTag(doc(), QStringLiteral("prompt"), v_prompt));
}

void JT_Gateway::onGo()
{
	send(iq);
}

Jid JT_Gateway::jid() const
{
	return v_jid;
}

QString JT_Gateway::desc() const
{
	return v_desc;
}

QString JT_Gateway::prompt() const
{
	return v_prompt;
}

Jid JT_Gateway::translatedJid() const
{
	return v_translatedJid;
}

bool JT_Gateway::take(const QDomElement &x)
{
	if(!iqVerify(x, v_jid, id()))
		return false;

	if(x.attribute(QStringLiteral("type")) == QLatin1String("result")) {
		if(type == 0) {
			QDomElement query = queryTag(x);
			QDomElement tag;
			tag = query.firstChildElement(QStringLiteral("desc"));
			if (!tag.isNull()) {
				v_desc = tagContent(tag);
			}
			tag = query.firstChildElement(QStringLiteral("prompt"));
			if (!tag.isNull()) {
				v_prompt = tagContent(tag);
			}
		}
		else {
			QDomElement query = queryTag(x);
			QDomElement tag;
			tag = query.firstChildElement(QStringLiteral("jid"));
			if (!tag.isNull()) {
				v_translatedJid = tagContent(tag);
			}
			// we used to read 'prompt' in the past
			// and some gateways still send it
			tag = query.firstChildElement(QStringLiteral("prompt"));
			if (!tag.isNull()) {
				v_prompt = tagContent(tag);
			}
		}

		setSuccess();
	}
	else {
		setError(x);
	}

	return true;
}

//----------------------------------------------------------------------------
// JT_Browse
//----------------------------------------------------------------------------
class JT_Browse::Private
{
public:
	QDomElement iq;
	Jid jid;
	AgentList agentList;
	AgentItem root;
};

JT_Browse::JT_Browse (Task *parent)
:Task (parent)
{
	d = new Private;
}

JT_Browse::~JT_Browse ()
{
	delete d;
}

void JT_Browse::get (const Jid &j)
{
	d->agentList.clear();

	d->jid = j;
	d->iq = createIQ(doc(), QStringLiteral("get"), d->jid.full(), id());
	QDomElement query = doc()->createElement(QStringLiteral("item"));
	query.setAttribute(QStringLiteral("xmlns"), QStringLiteral("jabber:iq:browse"));
	d->iq.appendChild(query);
}

const AgentList & JT_Browse::agents() const
{
	return d->agentList;
}

const AgentItem & JT_Browse::root() const
{
	return d->root;
}

void JT_Browse::onGo ()
{
	send(d->iq);
}

AgentItem JT_Browse::browseHelper (const QDomElement &i)
{
	AgentItem a;

	if ( i.tagName() == QLatin1String("ns") )
		return a;

	a.setName ( i.attribute(QStringLiteral("name")) );
	a.setJid  ( i.attribute(QStringLiteral("jid")) );

	// there are two types of category/type specification:
	//
	//   1. <item category="category_name" type="type_name" />
	//   2. <category_name type="type_name" />

	if ( i.tagName() == QLatin1String("item") || i.tagName() == QLatin1String("query") )
		a.setCategory ( i.attribute(QStringLiteral("category")) );
	else
		a.setCategory ( i.tagName() );

	a.setType ( i.attribute(QStringLiteral("type")) );

	QStringList ns;
	for(QDomNode n = i.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement i = n.toElement();
		if(i.isNull())
			continue;

		if ( i.tagName() == QLatin1String("ns") )
			ns << i.text();
	}

	// For now, conference.jabber.org returns proper namespace only
	// when browsing individual rooms. So it's a quick client-side fix.
	if ( !a.features().canGroupchat() && a.category() == QLatin1String("conference") )
		ns << QStringLiteral("jabber:iq:conference");

	a.setFeatures (ns);

	return a;
}

bool JT_Browse::take(const QDomElement &x)
{
	if(!iqVerify(x, d->jid, id()))
		return false;

	if(x.attribute(QStringLiteral("type")) == QLatin1String("result")) {
		for(QDomNode n = x.firstChild(); !n.isNull(); n = n.nextSibling()) {
			QDomElement i = n.toElement();
			if(i.isNull())
				continue;

			d->root = browseHelper (i);

			for(QDomNode nn = i.firstChild(); !nn.isNull(); nn = nn.nextSibling()) {
				QDomElement e = nn.toElement();
				if ( e.isNull() )
					continue;
				if ( e.tagName() == QLatin1String("ns") )
					continue;

				d->agentList += browseHelper (e);
			}
		}

		setSuccess(true);
	}
	else {
		setError(x);
	}

	return true;
}

//----------------------------------------------------------------------------
// JT_DiscoItems
//----------------------------------------------------------------------------
class JT_DiscoItems::Private
{
public:
	Private() { }

	QDomElement iq;
	Jid jid;
	DiscoList items;
};

JT_DiscoItems::JT_DiscoItems(Task *parent)
: Task(parent)
{
	d = new Private;
}

JT_DiscoItems::~JT_DiscoItems()
{
	delete d;
}

void JT_DiscoItems::get(const DiscoItem &item)
{
	get(item.jid(), item.node());
}

void JT_DiscoItems::get (const Jid &j, const QString &node)
{
	d->items.clear();

	d->jid = j;
	d->iq = createIQ(doc(), QStringLiteral("get"), d->jid.full(), id());
	QDomElement query = doc()->createElement(QStringLiteral("query"));
	query.setAttribute(QStringLiteral("xmlns"), QStringLiteral("http://jabber.org/protocol/disco#items"));

	if ( !node.isEmpty() )
		query.setAttribute(QStringLiteral("node"), node);

	d->iq.appendChild(query);
}

const DiscoList &JT_DiscoItems::items() const
{
	return d->items;
}

void JT_DiscoItems::onGo ()
{
	send(d->iq);
}

bool JT_DiscoItems::take(const QDomElement &x)
{
	if(!iqVerify(x, d->jid, id()))
		return false;

	if(x.attribute(QStringLiteral("type")) == QLatin1String("result")) {
		QDomElement q = queryTag(x);

		for(QDomNode n = q.firstChild(); !n.isNull(); n = n.nextSibling()) {
			QDomElement e = n.toElement();
			if( e.isNull() )
				continue;

			if ( e.tagName() == QLatin1String("item") ) {
				DiscoItem item;

				item.setJid ( e.attribute(QStringLiteral("jid"))  );
				item.setName( e.attribute(QStringLiteral("name")) );
				item.setNode( e.attribute(QStringLiteral("node")) );
				item.setAction( DiscoItem::string2action(e.attribute(QStringLiteral("action"))) );

				d->items.append( item );
			}
		}

		setSuccess(true);
	}
	else {
		setError(x);
	}

	return true;
}

//----------------------------------------------------------------------------
// JT_DiscoPublish
//----------------------------------------------------------------------------
class JT_DiscoPublish::Private
{
public:
	Private() { }

	QDomElement iq;
	Jid jid;
	DiscoList list;
};

JT_DiscoPublish::JT_DiscoPublish(Task *parent)
: Task(parent)
{
	d = new Private;
}

JT_DiscoPublish::~JT_DiscoPublish()
{
	delete d;
}

void JT_DiscoPublish::set(const Jid &j, const DiscoList &list)
{
	d->list = list;
	d->jid = j;

	d->iq = createIQ(doc(), QStringLiteral("set"), d->jid.full(), id());
	QDomElement query = doc()->createElement(QStringLiteral("query"));
	query.setAttribute(QStringLiteral("xmlns"), QStringLiteral("http://jabber.org/protocol/disco#items"));

	// FIXME: unsure about this
	//if ( !node.isEmpty() )
	//	query.setAttribute("node", node);

	DiscoList::ConstIterator it = list.begin();
	for ( ; it != list.end(); ++it) {
		QDomElement w = doc()->createElement(QStringLiteral("item"));

		w.setAttribute(QStringLiteral("jid"), (*it).jid().full());
		if ( !(*it).name().isEmpty() )
			w.setAttribute(QStringLiteral("name"), (*it).name());
		if ( !(*it).node().isEmpty() )
		w.setAttribute(QStringLiteral("node"), (*it).node());
		w.setAttribute(QStringLiteral("action"), DiscoItem::action2string((*it).action()));

		query.appendChild( w );
	}

	d->iq.appendChild(query);
}

void JT_DiscoPublish::onGo ()
{
	send(d->iq);
}

bool JT_DiscoPublish::take(const QDomElement &x)
{
	if(!iqVerify(x, d->jid, id()))
		return false;

	if(x.attribute(QStringLiteral("type")) == QLatin1String("result")) {
		setSuccess(true);
	}
	else {
		setError(x);
	}

	return true;
}

// ---------------------------------------------------------
// JT_BoBServer
// ---------------------------------------------------------
JT_BoBServer::JT_BoBServer(Task *parent)
	: Task(parent)
{

}

bool JT_BoBServer::take(const QDomElement &e)
{
	if (e.tagName() != QLatin1String("iq") || e.attribute(QStringLiteral("type")) != QLatin1String("get"))
		return false;

	QDomElement data = e.firstChildElement(QStringLiteral("data"));
	if (data.attribute(QStringLiteral("xmlns")) == QLatin1String("urn:xmpp:bob")) {
		QDomElement iq;
		BoBData bd = client()->bobManager()->bobData(data.attribute(QStringLiteral("cid")));
		if (bd.isNull()) {
			iq = createIQ(client()->doc(), QStringLiteral("error"),
						  e.attribute(QStringLiteral("from")), e.attribute(QStringLiteral("id")));
			Stanza::Error error(Stanza::Error::Cancel,
								Stanza::Error::ItemNotFound);
			iq.appendChild(error.toXml(*doc(), client()->stream().baseNS()));
		}
		else {
			iq = createIQ(doc(), QStringLiteral("result"), e.attribute(QStringLiteral("from")), e.attribute(QStringLiteral("id")));
			iq.appendChild(bd.toXml(doc()));
		}
		send(iq);
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------
// JT_BitsOfBinary
//----------------------------------------------------------------------------
class JT_BitsOfBinary::Private
{
public:
	Private() { }

	QDomElement iq;
	Jid jid;
	QString cid;
	BoBData data;
};

JT_BitsOfBinary::JT_BitsOfBinary(Task *parent)
: Task(parent)
{
	d = new Private;
}

JT_BitsOfBinary::~JT_BitsOfBinary()
{
	delete d;
}

void JT_BitsOfBinary::get(const Jid &j, const QString &cid)
{
	d->jid = j;
	d->cid = cid;

	d->data = client()->bobManager()->bobData(cid);
	if (d->data.isNull()) {
		d->iq = createIQ(doc(), QStringLiteral("get"), d->jid.full(), id());
		QDomElement data = doc()->createElement(QStringLiteral("data"));
		data.setAttribute(QStringLiteral("xmlns"), QStringLiteral("urn:xmpp:bob"));
		data.setAttribute(QStringLiteral("cid"), cid);
		d->iq.appendChild(data);
	}
}

void JT_BitsOfBinary::onGo()
{
	if (d->data.isNull()) {
		send(d->iq);
	}
	else {
		setSuccess();
	}
}

bool JT_BitsOfBinary::take(const QDomElement &x)
{
	if (!iqVerify(x, d->jid, id())) {
		return false;
	}

	if (x.attribute(QStringLiteral("type")) == QLatin1String("result")) {
		QDomElement data = x.firstChildElement(QStringLiteral("data"));

		if (!data.isNull() && data.attribute(QStringLiteral("cid")) == d->cid) { // check xmlns?
			d->data.fromXml(data);
			client()->bobManager()->append(d->data);
		}

		setSuccess();
	}
	else {
		setError(x);
	}

	return true;
}

BoBData &JT_BitsOfBinary::data()
{
	return d->data;
}

//----------------------------------------------------------------------------
// JT_PongServer
//----------------------------------------------------------------------------
/**
 * \class JT_PongServer
 * \brief Answers XMPP Pings
 */

JT_PongServer::JT_PongServer(Task *parent)
:Task(parent)
{

}

bool JT_PongServer::take(const QDomElement &e)
{
	if (e.tagName() != QLatin1String("iq") || e.attribute(QStringLiteral("type")) != QLatin1String("get"))
		return false;

	QDomElement ping = e.firstChildElement(QStringLiteral("ping"));
	if (!e.isNull() && ping.attribute(QStringLiteral("xmlns")) == QLatin1String("urn:xmpp:ping")) {
		QDomElement iq = createIQ(doc(), QStringLiteral("result"), e.attribute(QStringLiteral("from")), e.attribute(QStringLiteral("id")));
		send(iq);
		return true;
	}
	return false;
}

//---------------------------------------------------------------------------
// JT_CaptchaChallenger
//---------------------------------------------------------------------------
class JT_CaptchaChallenger::Private
{
public:
	Jid j;
	CaptchaChallenge challenge;
};

JT_CaptchaChallenger::JT_CaptchaChallenger(Task *parent) :
    Task(parent),
    d(new Private)
{
}

JT_CaptchaChallenger::~JT_CaptchaChallenger()
{
	delete d;
}

void JT_CaptchaChallenger::set(const Jid &j, const CaptchaChallenge &c)
{
	d->j = j;
	d->challenge = c;
}

void JT_CaptchaChallenger::onGo()
{
	setTimeout(CaptchaValidTimeout);

	Message m;
	m.setId(id());
	m.setBody(d->challenge.explanation());
	m.setUrlList(d->challenge.urls());

	XData form = d->challenge.form();
	XData::FieldList fl = form.fields();
	XData::FieldList::Iterator it;
	for (it = fl.begin(); it < fl.end(); ++it) {
		if (it->var() == QLatin1String("challenge") && it->type() == XData::Field::Field_Hidden) {
			it->setValue(QStringList() << id());
		}
	}
	if (it == fl.end()) {
		XData::Field f;
		f.setType(XData::Field::Field_Hidden);
		f.setVar(QStringLiteral("challenge"));
		f.setValue(QStringList() << id());
		fl.append(f);
	}
	form.setFields(fl);

	m.setForm(form);
	m.setTo(d->j);
	client()->sendMessage(m);
}

bool JT_CaptchaChallenger::take(const QDomElement &x)
{
	if(x.tagName() == QLatin1String("message") && x.attribute(QStringLiteral("id")) == id() &&
	        Jid(x.attribute(QStringLiteral("from"))) == d->j && !x.firstChildElement(QStringLiteral("error")).isNull())
	{
		setError(x);
		return true;
	}

	XDomNodeList nl;
	XData xd;
	QString rid = x.attribute(QStringLiteral("id"));
	if (rid.isEmpty() || x.tagName() != QLatin1String("iq") ||
	        Jid(x.attribute(QStringLiteral("from"))) != d->j || x.attribute(QStringLiteral("type")) != QLatin1String("set") ||
	        (nl = childElementsByTagNameNS(x, QStringLiteral("urn:xmpp:captcha"), QStringLiteral("captcha"))).isEmpty() ||
	        (nl = childElementsByTagNameNS(nl.item(0).toElement(), QStringLiteral("jabber:x:data"), QStringLiteral("x"))).isEmpty() ||
	        (xd.fromXml(nl.item(0).toElement()), xd.getField(QStringLiteral("challenge")).value().value(0) != id()))
	{
		return false;
	}

	CaptchaChallenge::Result r = d->challenge.validateResponse(xd);
	QDomElement iq;
	if (r == CaptchaChallenge::Passed) {
		iq = createIQ(doc(), QStringLiteral("result"), d->j.full(), rid);
	} else {
		Stanza::Error::ErrorCond ec;
		if (r == CaptchaChallenge::Unavailable) {
			ec = Stanza::Error::ServiceUnavailable;
		} else {
			ec = Stanza::Error::NotAcceptable;
		}
		iq = createIQ(doc(), QStringLiteral("error"), d->j.full(), rid);
		Stanza::Error error(Stanza::Error::Cancel, ec);
		iq.appendChild(error.toXml(*doc(), client()->stream().baseNS()));
	}
	send(iq);

	setSuccess();

	return true;
}

//---------------------------------------------------------------------------
// JT_CaptchaSender
//---------------------------------------------------------------------------
JT_CaptchaSender::JT_CaptchaSender(Task *parent) :
    Task(parent)
{}

void JT_CaptchaSender::set(const Jid &j, const XData &xd)
{
	to = j;

	iq = createIQ(doc(), QStringLiteral("set"), to.full(), id());
	iq.appendChild(doc()->createElementNS(QStringLiteral("urn:xmpp:captcha"), QStringLiteral("captcha")))
	        .appendChild(xd.toXml(doc(), true));
}

void JT_CaptchaSender::onGo()
{
	send(iq);
}

bool JT_CaptchaSender::take(const QDomElement &x)
{
	if (!iqVerify(x, to, id())) {
		return false;
	}

	if (x.attribute(QStringLiteral("type")) == QLatin1String("result")) {
		setSuccess();
	}
	else {
		setError(x);
	}

	return true;
}

//----------------------------------------------------------------------------
// JT_MessageCarbons
//----------------------------------------------------------------------------
JT_MessageCarbons::JT_MessageCarbons(Task *parent)
	: Task(parent)
{

}

void JT_MessageCarbons::enable()
{
	_iq = createIQ(doc(), QStringLiteral("set"), QLatin1String(""), id());

	QDomElement enable = doc()->createElement(QStringLiteral("enable"));
	enable.setAttribute(QStringLiteral("xmlns"), QStringLiteral("urn:xmpp:carbons:2"));

	_iq.appendChild(enable);
}

void JT_MessageCarbons::disable()
{
	_iq = createIQ(doc(), QStringLiteral("set"), QLatin1String(""), id());

	QDomElement disable = doc()->createElement(QStringLiteral("disable"));
	disable.setAttribute(QStringLiteral("xmlns"), QStringLiteral("urn:xmpp:carbons:2"));

	_iq.appendChild(disable);
}

void JT_MessageCarbons::onGo()
{
	send(_iq);
	setSuccess();
}

bool JT_MessageCarbons::take(const QDomElement &e)
{
	if (e.tagName() != QLatin1String("iq") || e.attribute(QStringLiteral("type")) != QLatin1String("result"))
		return false;

	bool res = iqVerify(e, Jid(), id());
	return res;
}
