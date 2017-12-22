/*
 * types.cpp - IM data types
 * Copyright (C) 2003  Justin Karneges
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

#include <QMap>
#include <QList>

#include "im.h"
#include "xmpp_features.h"
#include "xmpp_xmlcommon.h"
#include "xmpp_bitsofbinary.h"
#include "xmpp_ibb.h"
#include "xmpp_captcha.h"
#include "protocol.h"
#define NS_XML     "http://www.w3.org/XML/1998/namespace"

namespace XMPP
{

//----------------------------------------------------------------------------
// Url
//----------------------------------------------------------------------------
class Url::Private
{
public:
	QString url;
	QString desc;
};

//! \brief Construct Url object with a given URL and Description.
//!
//! This function will construct a Url object.
//! \param QString - url (default: empty string)
//! \param QString - description of url (default: empty string)
//! \sa setUrl() setDesc()
Url::Url(const QString &url, const QString &desc)
{
	d = new Private;
	d->url = url;
	d->desc = desc;
}

//! \brief Construct Url object.
//!
//! Overloaded constructor which will constructs a exact copy of the Url object that was passed to the constructor.
//! \param Url - Url Object
Url::Url(const Url &from)
{
	d = new Private;
	*this = from;
}

//! \brief operator overloader needed for d pointer (Internel).
Url & Url::operator=(const Url &from)
{
	*d = *from.d;
	return *this;
}

//! \brief destroy Url object.
Url::~Url()
{
	delete d;
}

//! \brief Get url information.
//!
//! Returns url information.
QString Url::url() const
{
	return d->url;
}

//! \brief Get Description information.
//!
//! Returns desction of the URL.
QString Url::desc() const
{
	return d->desc;
}

//! \brief Set Url information.
//!
//! Set url information.
//! \param url - url string (eg: http://psi.affinix.com/)
void Url::setUrl(const QString &url)
{
	d->url = url;
}

//! \brief Set Description information.
//!
//! Set description of the url.
//! \param desc - description of url
void Url::setDesc(const QString &desc)
{
	d->desc = desc;
}

 //----------------------------------------------------------------------------
// Address
//----------------------------------------------------------------------------

//! \brief Construct Address object with a given Type and Jid.
//!
//! This function will construct a Address object.
//! \param Type - type (default: Unknown)
//! \param Jid - specify address (default: empty string)
//! \sa setType() setJid()
Address::Address(Type type, const Jid & jid)
: v_delivered(false)
{
	v_type = type;
	v_jid = jid;
	v_delivered = false;
}

Address::Address(const QDomElement& e)
: v_delivered(false)
{
	fromXml(e);
}

void Address::fromXml(const QDomElement& t)
{
	setJid(t.attribute(QStringLiteral("jid")));
	setUri(t.attribute(QStringLiteral("uri")));
	setNode(t.attribute(QStringLiteral("node")));
	setDesc(t.attribute(QStringLiteral("desc")));
	setDelivered(t.attribute(QStringLiteral("delivered")) == QLatin1String("true"));
	QString type = t.attribute(QStringLiteral("type"));
	if (type == QLatin1String("to"))
		setType(To);
	else if (type == QLatin1String("cc"))
		setType(Cc);
	else if (type == QLatin1String("bcc"))
		setType(Bcc);
	else if (type == QLatin1String("replyto"))
		setType(ReplyTo);
	else if (type == QLatin1String("replyroom"))
		setType(ReplyRoom);
	else if (type == QLatin1String("noreply"))
		setType(NoReply);
	else if (type == QLatin1String("ofrom"))
		setType(OriginalFrom);
	else if (type == QLatin1String("oto"))
		setType(OriginalTo);
}

QDomElement Address::toXml(Stanza& s) const
{
	QDomElement e = s.createElement(QStringLiteral("http://jabber.org/protocol/address"), QStringLiteral("address"));
	if(!jid().isEmpty())
		e.setAttribute(QStringLiteral("jid"), jid().full());
	if(!uri().isEmpty())
		e.setAttribute(QStringLiteral("uri"), uri());
	if(!node().isEmpty())
		e.setAttribute(QStringLiteral("node"), node());
	if(!desc().isEmpty())
		e.setAttribute(QStringLiteral("desc"), desc());
	if(delivered())
		e.setAttribute(QStringLiteral("delivered"), QStringLiteral("true"));
	switch (type()) {
		case To:
			e.setAttribute(QStringLiteral("type"), QStringLiteral("to"));
			break;
		case Cc:
			e.setAttribute(QStringLiteral("type"), QStringLiteral("cc"));
			break;
		case Bcc:
			e.setAttribute(QStringLiteral("type"), QStringLiteral("bcc"));
			break;
		case ReplyTo:
			e.setAttribute(QStringLiteral("type"), QStringLiteral("replyto"));
			break;
		case ReplyRoom:
			e.setAttribute(QStringLiteral("type"), QStringLiteral("replyroom"));
			break;
		case NoReply:
			e.setAttribute(QStringLiteral("type"), QStringLiteral("noreply"));
			break;
		case OriginalFrom:
			e.setAttribute(QStringLiteral("type"), QStringLiteral("ofrom"));
			break;
		case OriginalTo:
			e.setAttribute(QStringLiteral("type"), QStringLiteral("oto"));
			break;
		case Unknown:
			// Add nothing
			break;
	}
	return e;
}

//! \brief Get Jid information.
//!
//! Returns jid information.
const Jid& Address::jid() const
{
	return v_jid;
}

//! \brief Get Uri information.
//!
//! Returns desction of the Address.
const QString& Address::uri() const
{
	return v_uri;
}

//! \brief Get Node information.
//!
//! Returns node of the Address.
const QString& Address::node() const
{
	return v_node;
}

//! \brief Get Description information.
//!
//! Returns desction of the Address.
const QString& Address::desc() const
{
	return v_desc;
}

//! \brief Get Delivered information.
//!
//! Returns delivered of the Address.
bool Address::delivered() const
{
	return v_delivered;
}

//! \brief Get Type information.
//!
//! Returns type of the Address.
Address::Type Address::type() const
{
	return v_type;
}

//! \brief Set Address information.
//!
//! Set jid information.
//! \param jid - jid
void Address::setJid(const Jid &jid)
{
	v_jid = jid;
}

//! \brief Set Address information.
//!
//! Set uri information.
//! \param uri - url string (eg: http://psi.affinix.com/)
void Address::setUri(const QString &uri)
{
	v_uri = uri;
}

//! \brief Set Node information.
//!
//! Set node information.
//! \param node - node string
void Address::setNode(const QString &node)
{
	v_node = node;
}

//! \brief Set Description information.
//!
//! Set description of the url.
//! \param desc - description of url
void Address::setDesc(const QString &desc)
{
	v_desc = desc;
}

//! \brief Set delivered information.
//!
//! Set delivered information.
//! \param delivered - delivered flag
void Address::setDelivered(bool delivered)
{
	v_delivered = delivered;
}

//! \brief Set Type information.
//!
//! Set type information.
//! \param type - type
void Address::setType(Type type)
{
	v_type = type;
}

//----------------------------------------------------------------------------
// RosterExchangeItem
//----------------------------------------------------------------------------

RosterExchangeItem::RosterExchangeItem(const Jid& jid, const QString& name, const QStringList& groups, Action action) : jid_(jid), name_(name), groups_(groups), action_(action)
{
}

RosterExchangeItem::RosterExchangeItem(const QDomElement& el) : action_(Add)
{
	fromXml(el);
}

const Jid& RosterExchangeItem::jid() const
{
	return jid_;
}

RosterExchangeItem::Action RosterExchangeItem::action() const
{
	return action_;
}

const QString& RosterExchangeItem::name() const
{
	return name_;
}

const QStringList& RosterExchangeItem::groups() const
{
	return groups_;
}

bool RosterExchangeItem::isNull() const
{
	return jid_.isEmpty();
}

void RosterExchangeItem::setJid(const Jid& jid)
{
	jid_ = jid;
}

void RosterExchangeItem::setAction(Action action)
{
	action_ = action;
}

void RosterExchangeItem::setName(const QString& name)
{
	name_ = name;
}

void RosterExchangeItem::setGroups(const QStringList& groups)
{
	groups_ = groups;
}

QDomElement RosterExchangeItem::toXml(Stanza& s) const
{
	QDomElement e = s.createElement(QStringLiteral("http://jabber.org/protocol/rosterx"), QStringLiteral("item"));
	e.setAttribute(QStringLiteral("jid"), jid().full());
	if (!name().isEmpty())
		e.setAttribute(QStringLiteral("name"), name());
	switch(action()) {
		case Add:
			e.setAttribute(QStringLiteral("action"),QStringLiteral("add"));
			break;
		case Delete:
			e.setAttribute(QStringLiteral("action"),QStringLiteral("delete"));
			break;
		case Modify:
			e.setAttribute(QStringLiteral("action"),QStringLiteral("modify"));
			break;
	}
	foreach(QString group, groups_) {
		e.appendChild(s.createTextElement(QStringLiteral("http://jabber.org/protocol/rosterx"), QStringLiteral("group"),group));
	}
	return e;
}

void RosterExchangeItem::fromXml(const QDomElement& e)
{
	setJid(e.attribute(QStringLiteral("jid")));
	setName(e.attribute(QStringLiteral("name")));
	if (e.attribute(QStringLiteral("action")) == QLatin1String("delete")) {
		setAction(Delete);
	}
	else if (e.attribute(QStringLiteral("action")) == QLatin1String("modify")) {
		setAction(Modify);
	}
	else {
		setAction(Add);
	}
	QDomNodeList nl = e.childNodes();
	for(int n = 0; n < nl.count(); ++n) {
		QDomElement g = nl.item(n).toElement();
		if (!g.isNull() && g.tagName() == QLatin1String("group")) {
			groups_ += g.text();
		}
	}
}


//----------------------------------------------------------------------------
// MUCItem
//----------------------------------------------------------------------------
MUCItem::MUCItem(Role r, Affiliation a) : affiliation_(a), role_(r)
{
}

MUCItem::MUCItem(const QDomElement& el) : affiliation_(UnknownAffiliation), role_(UnknownRole)
{
	fromXml(el);
}

void MUCItem::setNick(const QString& n)
{
	nick_ = n;
}

void MUCItem::setJid(const Jid& j)
{
	jid_ = j;
}

void MUCItem::setAffiliation(Affiliation a)
{
	affiliation_ = a;
}

void MUCItem::setRole(Role r)
{
	role_ = r;
}

void MUCItem::setActor(const Jid& a)
{
	actor_ = a;
}

void MUCItem::setReason(const QString& r)
{
	reason_ = r;
}

const QString& MUCItem::nick() const
{
	return nick_;
}

const Jid& MUCItem::jid() const
{
	return jid_;
}

MUCItem::Affiliation MUCItem::affiliation() const
{
	return affiliation_;
}

MUCItem::Role MUCItem::role() const
{
	return role_;
}

const Jid& MUCItem::actor() const
{
	return actor_;
}

const QString& MUCItem::reason() const
{
	return reason_;
}

void MUCItem::fromXml(const QDomElement& e)
{
	if (e.tagName() != QLatin1String("item"))
		return;

	jid_ = Jid(e.attribute(QStringLiteral("jid")));
	nick_ = e.attribute(QStringLiteral("nick"));

	// Affiliation
	if (e.attribute(QStringLiteral("affiliation")) == QLatin1String("owner")) {
		affiliation_ = Owner;
	}
	else if (e.attribute(QStringLiteral("affiliation")) == QLatin1String("admin")) {
		affiliation_ = Admin;
	}
	else if (e.attribute(QStringLiteral("affiliation")) == QLatin1String("member")) {
		affiliation_ = Member;
	}
	else if (e.attribute(QStringLiteral("affiliation")) == QLatin1String("outcast")) {
		affiliation_ = Outcast;
	}
	else if (e.attribute(QStringLiteral("affiliation")) == QLatin1String("none")) {
		affiliation_ = NoAffiliation;
	}

	// Role
	if (e.attribute(QStringLiteral("role")) == QLatin1String("moderator")) {
		role_ = Moderator;
	}
	else if (e.attribute(QStringLiteral("role")) == QLatin1String("participant")) {
		role_ = Participant;
	}
	else if (e.attribute(QStringLiteral("role")) == QLatin1String("visitor")) {
		role_ = Visitor;
	}
	else if (e.attribute(QStringLiteral("role")) == QLatin1String("none")) {
		role_ = NoRole;
	}

	for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement i = n.toElement();
		if(i.isNull())
			continue;

		if (i.tagName() == QLatin1String("actor"))
			actor_ = Jid(i.attribute(QStringLiteral("jid")));
		else if (i.tagName() == QLatin1String("reason"))
			reason_ = i.text();
	}
}

QDomElement MUCItem::toXml(QDomDocument& d)
{
	QDomElement e = d.createElement(QStringLiteral("item"));

	if (!nick_.isEmpty())
		e.setAttribute(QStringLiteral("nick"),nick_);

	if (!jid_.isEmpty())
		e.setAttribute(QStringLiteral("jid"),jid_.full());

	if (!reason_.isEmpty())
		e.appendChild(textTag(&d,QStringLiteral("reason"),reason_));

	switch (affiliation_) {
		case NoAffiliation:
			e.setAttribute(QStringLiteral("affiliation"),QStringLiteral("none"));
			break;
		case Owner:
			e.setAttribute(QStringLiteral("affiliation"),QStringLiteral("owner"));
			break;
		case Admin:
			e.setAttribute(QStringLiteral("affiliation"),QStringLiteral("admin"));
			break;
		case Member:
			e.setAttribute(QStringLiteral("affiliation"),QStringLiteral("member"));
			break;
		case Outcast:
			e.setAttribute(QStringLiteral("affiliation"),QStringLiteral("outcast"));
			break;
		default:
			break;
	}
	switch (role_) {
		case NoRole:
			e.setAttribute(QStringLiteral("role"),QStringLiteral("none"));
			break;
		case Moderator:
			e.setAttribute(QStringLiteral("role"),QStringLiteral("moderator"));
			break;
		case Participant:
			e.setAttribute(QStringLiteral("role"),QStringLiteral("participant"));
			break;
		case Visitor:
			e.setAttribute(QStringLiteral("role"),QStringLiteral("visitor"));
			break;
		default:
			break;
	}

	return e;
}

bool MUCItem::operator==(const MUCItem& o)
{
	return !nick_.compare(o.nick_) && ((!jid_.isValid() && !o.jid_.isValid()) || jid_.compare(o.jid_,true)) && ((!actor_.isValid() && !o.actor_.isValid()) || actor_.compare(o.actor_,true)) && affiliation_ == o.affiliation_ && role_ == o.role_ && !reason_.compare(o.reason_);
}

//----------------------------------------------------------------------------
// MUCInvite
//----------------------------------------------------------------------------

MUCInvite::MUCInvite() : cont_(false)
{
}

MUCInvite::MUCInvite(const Jid& to, const QString& reason) : to_(to), reason_(reason), cont_(false)
{
}

MUCInvite::MUCInvite(const QDomElement& e) : cont_(false)
{
	fromXml(e);
}

const Jid& MUCInvite::from() const
{
	return from_;
}

void MUCInvite::setFrom(const Jid& j)
{
	from_ = j;
}

const Jid& MUCInvite::to() const
{
	return to_;
}

void MUCInvite::setTo(const Jid& j)
{
	to_ = j;
}

const QString& MUCInvite::reason() const
{
	return reason_;
}

void MUCInvite::setReason(const QString& r)
{
	reason_ = r;
}

bool MUCInvite::cont() const
{
	return cont_;
}

void MUCInvite::setCont(bool b)
{
	cont_ = b;
}

void MUCInvite::fromXml(const QDomElement& e)
{
	if (e.tagName() != QLatin1String("invite"))
		return;

	from_ = e.attribute(QStringLiteral("from"));
	to_ = e.attribute(QStringLiteral("to"));
	for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement i = n.toElement();
		if(i.isNull())
			continue;

		if (i.tagName() == QLatin1String("continue"))
		   cont_ = true;
		else if (i.tagName() == QLatin1String("reason"))
			reason_ = i.text();
	}
}

QDomElement MUCInvite::toXml(QDomDocument& d) const
{
	QDomElement invite = d.createElement(QStringLiteral("invite"));
	if (!to_.isEmpty()) {
		invite.setAttribute(QStringLiteral("to"),to_.full());
	}
	if (!from_.isEmpty()) {
		invite.setAttribute(QStringLiteral("from"),from_.full());
	}
	if (!reason_.isEmpty()) {
		invite.appendChild(textTag(&d, QStringLiteral("reason"), reason_));
	}
	if (cont_) {
		invite.appendChild(d.createElement(QStringLiteral("continue")));
	}
	return invite;
}

bool MUCInvite::isNull() const
{
	return to_.isEmpty() && from_.isEmpty();
}

//----------------------------------------------------------------------------
// MUCDecline
//----------------------------------------------------------------------------

MUCDecline::MUCDecline()
{
}

MUCDecline::MUCDecline(const QDomElement& e)
{
	fromXml(e);
}

const Jid& MUCDecline::from() const
{
	return from_;
}

void MUCDecline::setFrom(const Jid& j)
{
	from_ = j;
}

const Jid& MUCDecline::to() const
{
	return to_;
}

void MUCDecline::setTo(const Jid& j)
{
	to_ = j;
}

const QString& MUCDecline::reason() const
{
	return reason_;
}

void MUCDecline::setReason(const QString& r)
{
	reason_ = r;
}

void MUCDecline::fromXml(const QDomElement& e)
{
	if (e.tagName() != QLatin1String("decline"))
		return;

	from_ = e.attribute(QStringLiteral("from"));
	to_ = e.attribute(QStringLiteral("to"));
	for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement i = n.toElement();
		if(i.isNull())
			continue;

		if (i.tagName() == QLatin1String("reason"))
			reason_ = i.text();
	}
}

QDomElement MUCDecline::toXml(QDomDocument& d) const
{
	QDomElement decline = d.createElement(QStringLiteral("decline"));
	if (!to_.isEmpty()) {
		decline.setAttribute(QStringLiteral("to"),to_.full());
	}
	if (!from_.isEmpty()) {
		decline.setAttribute(QStringLiteral("from"),from_.full());
	}
	if (!reason_.isEmpty()) {
		decline.appendChild(textTag(&d, QStringLiteral("reason"), reason_));
	}
	return decline;
}

bool MUCDecline::isNull() const
{
	return to_.isEmpty() && from_.isEmpty();
}

//----------------------------------------------------------------------------
// MUCDestroy
//----------------------------------------------------------------------------

MUCDestroy::MUCDestroy()
{
}

MUCDestroy::MUCDestroy(const QDomElement& e)
{
	fromXml(e);
}

const Jid& MUCDestroy::jid() const
{
	return jid_;
}

void MUCDestroy::setJid(const Jid& j)
{
	jid_ = j;
}

const QString& MUCDestroy::reason() const
{
	return reason_;
}

void MUCDestroy::setReason(const QString& r)
{
	reason_ = r;
}

void MUCDestroy::fromXml(const QDomElement& e)
{
	if (e.tagName() != QLatin1String("destroy"))
		return;

	jid_ = e.attribute(QStringLiteral("jid"));
	for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement i = n.toElement();
		if(i.isNull())
			continue;

		if (i.tagName() == QLatin1String("reason"))
			reason_ = i.text();
	}
}

QDomElement MUCDestroy::toXml(QDomDocument& d) const
{
	QDomElement destroy = d.createElement(QStringLiteral("destroy"));
	if (!jid_.isEmpty()) {
		destroy.setAttribute(QStringLiteral("jid"),jid_.full());
	}
	if (!reason_.isEmpty()) {
		destroy.appendChild(textTag(&d, QStringLiteral("reason"), reason_));
	}
	return destroy;
}

//----------------------------------------------------------------------------
// HTMLElement
//----------------------------------------------------------------------------
HTMLElement::HTMLElement()
{
}

HTMLElement::HTMLElement(const QDomElement &body) { setBody(body); }

void HTMLElement::setBody(const QDomElement &body)
{
	body_ = doc_.importNode(body, true).toElement();
}

const QDomElement& HTMLElement::body() const
{
	return body_;
}

/**
 * Returns the string reperesentation of the HTML element.
 * By default, this is of the form <body ...>...</body>, but the
 * root tag can be modified using a parameter.
 *
 * \param rootTagName the tagname of the root element to use.
 */
QString HTMLElement::toString(const QString &rootTagName) const
{
	// create a copy of the body_ node,
	// get rid of the xmlns attribute and
	// change the root node name
	QDomElement e = body_.cloneNode().toElement();
	e.setTagName(rootTagName);

	// instead of using:
	//	QDomDocument msg;
	//	msg.appendChild(e);
	//	return msg.toString();
	// call Stream::xmlToString, to remove unwanted namespace attributes
	return(Stream::xmlToString(e));
}

QString HTMLElement::text() const
{
	return body_.text();
}

void HTMLElement::filterOutUnwanted(bool strict)
{
	Q_UNUSED(strict) //TODO filter out not xhtml-im elements
	filterOutUnwantedRecursive(body_, strict);
}

void HTMLElement::filterOutUnwantedRecursive(QDomElement &el, bool strict)
{
	Q_UNUSED(strict) //TODO filter out not xhtml-im elements

	static QSet<QString> unwanted = QSet<QString>()<<QStringLiteral("script")<<QStringLiteral("iframe");
	QDomNode child = el.firstChild();
	while (!child.isNull()) {
		QDomNode sibling = child.nextSibling();
		if (child.isElement()) {
			QDomElement childEl = child.toElement();
			if (unwanted.contains(childEl.tagName())) {
				child.parentNode().removeChild(child);
			}
			else {
				QDomNamedNodeMap domAttrs = childEl.attributes();
				int acnt = domAttrs.count();
				QStringList attrs; //attributes for removing
				for (int i=0; i<acnt; i++) {
					QString name = domAttrs.item(i).toAttr().name();
					if (name.startsWith(QLatin1String("on"))) {
						attrs.append(name);
					}
				}
				foreach(const QString &name, attrs) {
					domAttrs.removeNamedItem(name);
				}
				filterOutUnwantedRecursive(childEl, strict);
			}
		}
		child = sibling;
	}
}


//----------------------------------------------------------------------------
// Message
//----------------------------------------------------------------------------
class Message::Private
{
public:
	Jid to, from;
	QString id, type, lang;

	StringMap subject, body;
	QString thread;
	bool threadSend;
	Stanza::Error error;

	// extensions
	QDateTime timeStamp; // local time
	bool timeStampSend;
	UrlList urlList;
	AddressList addressList;
	RosterExchangeItems rosterExchangeItems;
	QList<MsgEvent> eventList;
	QString pubsubNode;
	QList<PubSubItem> pubsubItems;
	QList<PubSubRetraction> pubsubRetractions;
	QString eventId;
	QString xsigned, xencrypted, invite;
	ChatState chatState;
	MessageReceipt messageReceipt;
	QString messageReceiptId;
	QString nick;
	HttpAuthRequest httpAuthRequest;
	XData xdata;
	IBBData ibbData;
	QMap<QString,HTMLElement> htmlElements;
 	QDomElement sxe;
	QList<BoBData> bobDataList;
	Jid forwardedFrom;

	QList<int> mucStatuses;
	QList<MUCInvite> mucInvites;
	MUCDecline mucDecline;
	QString mucPassword;
	bool hasMUCUser;

	bool spooled, wasEncrypted;

	//XEP-0280 Message Carbons
	Message::CarbonDir carbonDir; // it's a forwarded message
	bool isDisabledCarbons;
	QString replaceId;
};

//! \brief Constructs Message with given Jid information.
//!
//! This function will construct a Message container.
//! \param to - specify receiver (default: empty string)
Message::Message(const Jid &to)
{
	d = new Private;
	d->to = to;
	d->spooled = false;
	d->threadSend = false;
	d->timeStampSend = false;
	d->wasEncrypted = false;
	/*d->flag = false;
	d->spooled = false;
	d->wasEncrypted = false;
	d->errorCode = -1;*/
	d->chatState = StateNone;
	d->messageReceipt = ReceiptNone;
	d->carbonDir = Message::NoCarbon;
	d->isDisabledCarbons = false;
	d->hasMUCUser = false;
}

//! \brief Constructs a copy of Message object
//!
//! Overloaded constructor which will constructs a exact copy of the Message
//! object that was passed to the constructor.
//! \param from - Message object you want to copy
Message::Message(const Message &from)
{
	d = new Private;
	*this = from;
}

//! \brief Required for internel use.
Message & Message::operator=(const Message &from)
{
	*d = *from.d;
	return *this;
}

//! \brief Destroy Message object.
Message::~Message()
{
	delete d;
}

//! \brief Return receiver's Jid information.
Jid Message::to() const
{
	return d->to;
}

//! \brief Return sender's Jid information.
Jid Message::from() const
{
	return d->from;
}

QString Message::id() const
{
	return d->id;
}

//! \brief Return type information
QString Message::type() const
{
	return d->type;
}

QString Message::lang() const
{
	return d->lang;
}

//! \brief Return subject information.
QString Message::subject(const QString &lang) const
{
	return d->subject.value(lang);
}

//! \brief Return body information.
//!
//! This function will return a plain text or the Richtext version if it
//! it exists.
//! \param rich - Returns richtext if true and plain text if false. (default: false)
//! \note Richtext is in Qt's richtext format and not in xhtml.
QString Message::body(const QString &lang) const
{
	if (d->body.empty())
		return QLatin1String("");
	else if (d->body.contains(lang))
		return d->body[lang];
	else
		return d->body.begin().value();
}

//! \brief Return xhtml body.
//!
//! This function will return the richtext version of the body, if
//! available.
//! \param lang - body language
//! \note The return string is in xhtml
HTMLElement Message::html(const QString &lang) const
{
	if(containsHTML()) {
		if (d->htmlElements.contains(lang))
			return d->htmlElements[lang];
		else
			return d->htmlElements.begin().value();
	}
	else
		return HTMLElement();
}

//! \brief Tells if message has xhtml-im items.
//!
//! Returns true if there is at least one xhtml-im body
//! in the message.
bool Message::containsHTML() const
{
	return !(d->htmlElements.isEmpty());
}

QString Message::thread() const
{
	return d->thread;
}

Stanza::Error Message::error() const
{
	return d->error;
}

//! \brief Set receivers information
//!
//! \param to - Receivers Jabber id
void Message::setTo(const Jid &j)
{
	d->to = j;
	//d->flag = false;
}

void Message::setFrom(const Jid &j)
{
	d->from = j;
	//d->flag = false;
}

void Message::setId(const QString &s)
{
	d->id = s;
}

//! \brief Set Type of message
//!
//! \param type - type of message your going to send
void Message::setType(const QString &s)
{
	d->type = s;
	//d->flag = false;
}

void Message::setLang(const QString &s)
{
	d->lang = s;
}

//! \brief Set subject
//!
//! \param subject - Subject information
void Message::setSubject(const QString &s, const QString &lang)
{
	d->subject[lang] = s;
	//d->flag = false;
}

//! \brief Set body
//!
//! \param body - body information
//! \param rich - set richtext if true and set plaintext if false.
//! \note Richtext support will be implemented in the future... Sorry.
void Message::setBody(const QString &s, const QString &lang)
{
	d->body[lang] = s;
	//d->flag = false;
}

//! \brief Set xhtml body
//!
//! \param s - body node
//! \param lang - body language
//! \note The body should be in xhtml.
void Message::setHTML(const HTMLElement &e, const QString &lang)
{
	d->htmlElements[lang] = e;
}

void Message::setThread(const QString &s, bool send)
{
	d->threadSend = send;
	d->thread = s;
}

void Message::setError(const Stanza::Error &err)
{
	d->error = err;
}

const QString& Message::pubsubNode() const
{
	return d->pubsubNode;
}

const QList<PubSubItem>& Message::pubsubItems() const
{
	return d->pubsubItems;
}

const QList<PubSubRetraction>& Message::pubsubRetractions() const
{
	return d->pubsubRetractions;
}

QDateTime Message::timeStamp() const
{
	return d->timeStamp;
}

void Message::setTimeStamp(const QDateTime &ts, bool send)
{
	d->timeStampSend = send;
	d->timeStamp = ts;
}

//! \brief Return list of urls attached to message.
UrlList Message::urlList() const
{
	return d->urlList;
}

//! \brief Add Url to the url list.
//!
//! \param url - url to append
void Message::urlAdd(const Url &u)
{
	d->urlList += u;
}

//! \brief clear out the url list.
void Message::urlsClear()
{
	d->urlList.clear();
}

//! \brief Set urls to send
//!
//! \param urlList - list of urls to send
void Message::setUrlList(const UrlList &list)
{
	d->urlList = list;
}

//! \brief Return list of addresses attached to message.
AddressList Message::addresses() const
{
	return d->addressList;
}

//! \brief Add Address to the address list.
//!
//! \param address - address to append
void Message::addAddress(const Address &a)
{
	d->addressList += a;
}

//! \brief clear out the address list.
void Message::clearAddresses()
{
	d->addressList.clear();
}

AddressList Message::findAddresses(Address::Type t) const
{
	AddressList matches;
	foreach(Address a, d->addressList) {
		if (a.type() == t)
			matches.append(a);
	}
	return matches;
}

//! \brief Set addresses to send
//!
//! \param list - list of addresses to send
void Message::setAddresses(const AddressList &list)
{
	d->addressList = list;
}

const RosterExchangeItems& Message::rosterExchangeItems() const
{
	return d->rosterExchangeItems;
}

void Message::setRosterExchangeItems(const RosterExchangeItems& items)
{
	d->rosterExchangeItems = items;
}

QString Message::eventId() const
{
	return d->eventId;
}

void Message::setEventId(const QString& id)
{
	d->eventId = id;
}

bool Message::containsEvents() const
{
	return !d->eventList.isEmpty();
}

bool Message::containsEvent(MsgEvent e) const
{
	return d->eventList.contains(e);
}

void Message::addEvent(MsgEvent e)
{
	if (!d->eventList.contains(e)) {
		if (e == CancelEvent || containsEvent(CancelEvent))
			d->eventList.clear(); // Reset list
		d->eventList += e;
	}
}

ChatState Message::chatState() const
{
	return d->chatState;
}

void Message::setChatState(ChatState state)
{
	d->chatState = state;
}

MessageReceipt Message::messageReceipt() const
{
	return d->messageReceipt;
}

void Message::setMessageReceipt(MessageReceipt messageReceipt)
{
	d->messageReceipt = messageReceipt;
}

QString Message::messageReceiptId() const
{
	return d->messageReceiptId;
}

void Message::setMessageReceiptId(const QString &s)
{
	d->messageReceiptId = s;
}

QString Message::xsigned() const
{
	return d->xsigned;
}

void Message::setXSigned(const QString &s)
{
	d->xsigned = s;
}

QString Message::xencrypted() const
{
	return d->xencrypted;
}

void Message::setXEncrypted(const QString &s)
{
	d->xencrypted = s;
}

const QList<int>& Message::getMUCStatuses() const
{
	return d->mucStatuses;
}

void Message::addMUCStatus(int i)
{
	d->mucStatuses += i;
}

void Message::addMUCInvite(const MUCInvite& i)
{
	d->mucInvites += i;
}

const QList<MUCInvite>& Message::mucInvites() const
{
	return d->mucInvites;
}

void Message::setMUCDecline(const MUCDecline& de)
{
	d->mucDecline = de;
}

const MUCDecline& Message::mucDecline() const
{
	return d->mucDecline;
}

const QString& Message::mucPassword() const
{
	return d->mucPassword;
}

void Message::setMUCPassword(const QString& p)
{
	d->mucPassword = p;
}

bool Message::hasMUCUser() const
{
	return d->hasMUCUser;
}

QString Message::invite() const
{
	return d->invite;
}

void Message::setInvite(const QString &s)
{
	d->invite = s;
}

const QString& Message::nick() const
{
	return d->nick;
}

void Message::setNick(const QString& n)
{
	d->nick = n;
}

void Message::setHttpAuthRequest(const HttpAuthRequest &req)
{
	d->httpAuthRequest = req;
}

HttpAuthRequest Message::httpAuthRequest() const
{
	return d->httpAuthRequest;
}

void Message::setForm(const XData &form)
{
	d->xdata = form;
}

const XData& Message::getForm() const
{
	return d->xdata;
}

const QDomElement& Message::sxe() const
{
	return d->sxe;
}

void Message::setSxe(const QDomElement& e)
{
	d->sxe = e;
}

void Message::addBoBData(const BoBData &bob)
{
	d->bobDataList.append(bob);
}

QList<BoBData> Message::bobDataList() const
{
	return d->bobDataList;
}

const IBBData& Message::ibbData() const
{
	return d->ibbData;
}

void Message::setDisabledCarbons(bool disabled)
{
	d->isDisabledCarbons = disabled;
}

bool Message::isDisabledCarbons() const
{
	return d->isDisabledCarbons;
}

void Message::setCarbonDirection(Message::CarbonDir cd)
{
	d->carbonDir = cd;
}

Message::CarbonDir Message::carbonDirection() const
{
	return d->carbonDir;
}

void Message::setForwardedFrom(const Jid &jid)
{
	d->forwardedFrom = jid;
}

const Jid &Message::forwardedFrom() const
{
	return d->forwardedFrom;
}

bool Message::spooled() const
{
	return d->spooled;
}

void Message::setSpooled(bool b)
{
	d->spooled = b;
}

bool Message::wasEncrypted() const
{
	return d->wasEncrypted;
}

void Message::setWasEncrypted(bool b)
{
	d->wasEncrypted = b;
}

QString Message::replaceId() const {
	return d->replaceId;
}

void Message::setReplaceId(const QString& id) {
	d->replaceId = id;
}

Stanza Message::toStanza(Stream *stream) const
{
	Stanza s = stream->createStanza(Stanza::Message, d->to, d->type);
	if(!d->from.isEmpty())
		s.setFrom(d->from);
	if(!d->id.isEmpty())
		s.setId(d->id);
	if(!d->lang.isEmpty())
		s.setLang(d->lang);

	StringMap::ConstIterator it;
	for (it = d->subject.constBegin(); it != d->subject.constEnd(); ++it) {
		const QString &str = (*it);
		if(!str.isNull()) {
			QDomElement e = s.createTextElement(s.baseNS(), QStringLiteral("subject"), str);
			if(!it.key().isEmpty())
				e.setAttributeNS(NS_XML, QStringLiteral("xml:lang"), it.key());
			s.appendChild(e);
		}
	}
	for (it = d->body.constBegin(); it != d->body.constEnd(); ++it) {
		const QString &str = (*it);
		if(!str.isEmpty()) {
			QDomElement e = s.createTextElement(s.baseNS(), QStringLiteral("body"), str);
			if(!it.key().isEmpty())
				e.setAttributeNS(NS_XML, QStringLiteral("xml:lang"), it.key());
			s.appendChild(e);
		}
	}

	if (containsHTML()) {
		QDomElement html = s.createElement(QStringLiteral("http://jabber.org/protocol/xhtml-im"), QStringLiteral("html"));
		s.appendChild(html);
		foreach (HTMLElement el, d->htmlElements) {
			html.appendChild(s.doc().importNode(el.body(), true).toElement());
		}
	}

	if(d->type == QLatin1String("error"))
		s.setError(d->error);

	// thread
	if(d->threadSend && !d->thread.isEmpty()) {
		QDomElement e = s.createTextElement(s.baseNS(), QStringLiteral("thread"), d->thread);
		s.appendChild(e);
	}

	// timestamp
	if(d->timeStampSend && !d->timeStamp.isNull()) {
		QDomElement e = s.createElement(QStringLiteral("urn:xmpp:delay"), QStringLiteral("delay"));
		e.setAttribute(QStringLiteral("stamp"), d->timeStamp.toUTC().toString(Qt::ISODate) + "Z");
		s.appendChild(e);

		e = s.createElement(QStringLiteral("jabber:x:delay"), QStringLiteral("x"));
		e.setAttribute(QStringLiteral("stamp"), TS2stamp(d->timeStamp.toUTC()));
		s.appendChild(e);
	}

	// urls
	foreach (const Url& uit, d->urlList) {
		QDomElement x = s.createElement(QStringLiteral("jabber:x:oob"), QStringLiteral("x"));
		x.appendChild(s.createTextElement(QStringLiteral("jabber:x:oob"), QStringLiteral("url"), uit.url()));
		if (!uit.desc().isEmpty())
			x.appendChild(s.createTextElement(QStringLiteral("jabber:x:oob"), QStringLiteral("desc"), uit.desc()));
		s.appendChild(x);
	}

	// events
	if (!d->eventList.isEmpty()) {
		QDomElement x = s.createElement(QStringLiteral("jabber:x:event"), QStringLiteral("x"));

		if (d->body.isEmpty()) {
			if (d->eventId.isEmpty())
				x.appendChild(s.createElement(QStringLiteral("jabber:x:event"),QStringLiteral("id")));
			else
				x.appendChild(s.createTextElement(QStringLiteral("jabber:x:event"),QStringLiteral("id"),d->eventId));
		}

		foreach (const MsgEvent& ev, d->eventList) {
			switch (ev) {
				case OfflineEvent:
					x.appendChild(s.createElement(QStringLiteral("jabber:x:event"), QStringLiteral("offline")));
					break;
				case DeliveredEvent:
					x.appendChild(s.createElement(QStringLiteral("jabber:x:event"), QStringLiteral("delivered")));
					break;
				case DisplayedEvent:
					x.appendChild(s.createElement(QStringLiteral("jabber:x:event"), QStringLiteral("displayed")));
					break;
				case ComposingEvent:
					x.appendChild(s.createElement(QStringLiteral("jabber:x:event"), QStringLiteral("composing")));
					break;
				case CancelEvent:
					// Add nothing
					break;
			}
		}
		s.appendChild(x);
	}

	// chat state
	QString chatStateNS = QStringLiteral("http://jabber.org/protocol/chatstates");
	if (d->chatState != StateNone) {
		switch(d->chatState) {
			case StateActive:
				s.appendChild(s.createElement(chatStateNS, QStringLiteral("active")));
				break;
			case StateComposing:
				s.appendChild(s.createElement(chatStateNS, QStringLiteral("composing")));
				break;
			case StatePaused:
				s.appendChild(s.createElement(chatStateNS, QStringLiteral("paused")));
				break;
			case StateInactive:
				s.appendChild(s.createElement(chatStateNS, QStringLiteral("inactive")));
				break;
			case StateGone:
				s.appendChild(s.createElement(chatStateNS, QStringLiteral("gone")));
				break;
			default:
				break;
		}
	}

	// message receipt
	QString messageReceiptNS = QStringLiteral("urn:xmpp:receipts");
	if (d->messageReceipt != ReceiptNone) {
		switch(d->messageReceipt) {
			case ReceiptRequest:
				s.appendChild(s.createElement(messageReceiptNS, QStringLiteral("request")));
				break;
			case ReceiptReceived:
				{
					QDomElement elem = s.createElement(messageReceiptNS, QStringLiteral("received"));
					if (!d->messageReceiptId.isEmpty()) {
						elem.setAttribute(QStringLiteral("id"), d->messageReceiptId);
					}
					s.appendChild(elem);
				}
				break;
			default:
				break;
		}
	}

	// xsigned
	if(!d->xsigned.isEmpty())
		s.appendChild(s.createTextElement(QStringLiteral("jabber:x:signed"), QStringLiteral("x"), d->xsigned));

	// xencrypted
	if(!d->xencrypted.isEmpty())
		s.appendChild(s.createTextElement(QStringLiteral("jabber:x:encrypted"), QStringLiteral("x"), d->xencrypted));

	// addresses
	if (!d->addressList.isEmpty()) {
		QDomElement as = s.createElement(QStringLiteral("http://jabber.org/protocol/address"),QStringLiteral("addresses"));
		foreach(Address a, d->addressList) {
			as.appendChild(a.toXml(s));
		}
		s.appendChild(as);
	}

	// roster item exchange
	if (!d->rosterExchangeItems.isEmpty()) {
		QDomElement rx = s.createElement(QStringLiteral("http://jabber.org/protocol/rosterx"),QStringLiteral("x"));
		foreach(RosterExchangeItem r, d->rosterExchangeItems) {
			rx.appendChild(r.toXml(s));
		}
		s.appendChild(rx);
	}

	// invite
	if(!d->invite.isEmpty()) {
		QDomElement e = s.createElement(QStringLiteral("jabber:x:conference"), QStringLiteral("x"));
		e.setAttribute(QStringLiteral("jid"), d->invite);
		s.appendChild(e);
	}

	// nick
	if(!d->nick.isEmpty()) {
		s.appendChild(s.createTextElement(QStringLiteral("http://jabber.org/protocol/nick"), QStringLiteral("nick"), d->nick));
	}

	// sxe
	if(!d->sxe.isNull()) {
		s.appendChild(d->sxe);
	}

	// muc
	if(!d->mucInvites.isEmpty()) {
		QDomElement e = s.createElement(QStringLiteral("http://jabber.org/protocol/muc#user"),QStringLiteral("x"));
		foreach(MUCInvite i, d->mucInvites) {
			e.appendChild(i.toXml(s.doc()));
		}
		if (!d->mucPassword.isEmpty()) {
			e.appendChild(s.createTextElement(QStringLiteral("http://jabber.org/protocol/muc#user"),QStringLiteral("password"),d->mucPassword));
		}
		s.appendChild(e);
	}
	else if(!d->mucDecline.isNull()) {
		QDomElement e = s.createElement(QStringLiteral("http://jabber.org/protocol/muc#user"),QStringLiteral("x"));
		e.appendChild(d->mucDecline.toXml(s.doc()));
		s.appendChild(e);
	}

	// http auth
	if(!d->httpAuthRequest.isEmpty()) {
		s.appendChild(d->httpAuthRequest.toXml(s.doc()));
	}

	// data form
	if(!d->xdata.fields().empty() || (d->xdata.type() == XData::Data_Cancel)) {
		bool submit = (d->xdata.type() == XData::Data_Submit) || (d->xdata.type() == XData::Data_Cancel);
		QDomElement dr = s.element();
		if (d->xdata.registrarType() == QLatin1String("urn:xmpp:captcha")) {
			dr = dr.appendChild(s.createElement(QStringLiteral("urn:xmpp:captcha"), QStringLiteral("captcha"))).toElement();
		}
		dr.appendChild(d->xdata.toXml(&s.doc(), submit));
	}

	// bits of binary
	foreach(const BoBData &bd, d->bobDataList) {
		s.appendChild(bd.toXml(&s.doc()));
	}

	// Avoiding Carbons
	if (isDisabledCarbons() || wasEncrypted()) {
		QDomElement e = s.createElement(QStringLiteral("urn:xmpp:carbons:2"),QStringLiteral("private"));
		s.appendChild(e);
	}
	if (!d->replaceId.isEmpty()) {
		QDomElement e = s.createElement(QStringLiteral("urn:xmpp:message-correct:0"), QStringLiteral("replace"));
		e.setAttribute(QStringLiteral("id"), d->replaceId);
		s.appendChild(e);
	}
	return s;
}

/**
  \brief Create Message from Stanza \a s, using given \a timeZoneOffset (old style)
  */
bool Message::fromStanza(const Stanza &s, int timeZoneOffset)
{
	return fromStanza(s, true, timeZoneOffset);
}

/**
  \brief Create Message from Stanza \a s
  */
bool Message::fromStanza(const Stanza &s)
{
	return fromStanza(s, false, 0);
}

/**
  \brief Create Message from Stanza \a s

  If \a useTimeZoneOffset is true, \a timeZoneOffset is used when converting between UTC and local time (old style).
  Else, \a timeZoneOffset is ignored and Qt is used to do the conversion (new style).

  This function exists to make transition between old and new style easier.
  */
bool Message::fromStanza(const Stanza &s, bool useTimeZoneOffset, int timeZoneOffset)
{
	if(s.kind() != Stanza::Message)
		return false;

	setTo(s.to());
	setFrom(s.from());
	setId(s.id());
	setType(s.type());
	setLang(s.lang());

	d->subject.clear();
	d->body.clear();
	d->htmlElements.clear();
	d->thread = QString();

	QDomElement root = s.element();

	XDomNodeList nl = root.childNodes();
	int n;
	for(n = 0; n < nl.count(); ++n) {
		QDomNode i = nl.item(n);
		if(i.isElement()) {
			QDomElement e = i.toElement();
			if(e.namespaceURI() == s.baseNS()) {
				if(e.tagName() == QLatin1String("subject")) {
					QString lang = e.attributeNS(NS_XML, QStringLiteral("lang"), QLatin1String(""));
					d->subject[lang] = e.text();
				}
				else if(e.tagName() == QLatin1String("body")) {
					QString lang = e.attributeNS(NS_XML, QStringLiteral("lang"), QLatin1String(""));
					d->body[lang] = e.text();
				}
				else if(e.tagName() == QLatin1String("thread"))
					d->thread = e.text();
			}
			else if(e.tagName() == QLatin1String("event") && e.namespaceURI() == QLatin1String("http://jabber.org/protocol/pubsub#event")) {
				for(QDomNode enode = e.firstChild(); !enode.isNull(); enode = enode.nextSibling()) {
					QDomElement eel = enode.toElement();
					if (eel.tagName() == QLatin1String("items")) {
						d->pubsubNode = eel.attribute(QStringLiteral("node"));
						for(QDomNode inode = eel.firstChild(); !inode.isNull(); inode = inode.nextSibling()) {
							QDomElement o = inode.toElement();
							if (o.tagName() == QLatin1String("item")) {
								for(QDomNode j = o.firstChild(); !j.isNull(); j = j.nextSibling()) {
									QDomElement item = j.toElement();
									if (!item.isNull()) {
										d->pubsubItems += PubSubItem(o.attribute(QStringLiteral("id")),item);
									}
								}
							}
							if (o.tagName() == QLatin1String("retract")) {
								d->pubsubRetractions += PubSubRetraction(o.attribute(QStringLiteral("id")));
							}
						}
					}
				}
			}
			else {
				//printf("extension element: [%s]\n", e.tagName().latin1());
			}
		}
	}

	if(s.type() == QLatin1String("error"))
		d->error = s.error();

	// Bits of Binary XEP-0231
	nl = childElementsByTagNameNS(root, QStringLiteral("urn:xmpp:bob"), QStringLiteral("data"));
	for(n = 0; n < nl.count(); ++n) {
		addBoBData(BoBData(nl.item(n).toElement()));
	}

	// xhtml-im
	nl = childElementsByTagNameNS(root, QStringLiteral("http://jabber.org/protocol/xhtml-im"), QStringLiteral("html"));
	if (nl.count()) {
		nl = nl.item(0).childNodes();
		for(n = 0; n < nl.count(); ++n) {
			QDomElement e = nl.item(n).toElement();
			if (e.tagName() == QLatin1String("body") && e.namespaceURI() == QLatin1String("http://www.w3.org/1999/xhtml")) {
				QString lang = e.attributeNS(NS_XML, QStringLiteral("lang"), QLatin1String(""));
				d->htmlElements[lang] = e;
				d->htmlElements[lang].filterOutUnwanted(false); // just clear iframes and javascript event handlers
			}
		}
	}

	// timestamp
	QDomElement t = childElementsByTagNameNS(root, QStringLiteral("urn:xmpp:delay"), QStringLiteral("delay")).item(0).toElement();
	QDateTime stamp;
	if (!t.isNull()) {
		stamp = QDateTime::fromString(t.attribute(QStringLiteral("stamp")).left(19), Qt::ISODate);
	} else {
		t = childElementsByTagNameNS(root, QStringLiteral("jabber:x:delay"), QStringLiteral("x")).item(0).toElement();
		if (!t.isNull()) {
			stamp = stamp2TS(t.attribute(QStringLiteral("stamp")));
		}
	}
	if (!stamp.isNull()) {
		if (useTimeZoneOffset) {
			d->timeStamp = stamp.addSecs(timeZoneOffset * 3600);
		} else {
			stamp.setTimeSpec(Qt::UTC);
			d->timeStamp = stamp.toLocalTime();
		}
		d->timeStampSend = true;
		d->spooled = true;
	}
	else {
		d->timeStamp = QDateTime::currentDateTime();
		d->timeStampSend = false;
		d->spooled = false;
	}

	// urls
	d->urlList.clear();
	nl = childElementsByTagNameNS(root, QStringLiteral("jabber:x:oob"), QStringLiteral("x"));
	for(n = 0; n < nl.count(); ++n) {
		QDomElement t = nl.item(n).toElement();
		Url u;
		u.setUrl(t.elementsByTagName(QStringLiteral("url")).item(0).toElement().text());
		u.setDesc(t.elementsByTagName(QStringLiteral("desc")).item(0).toElement().text());
		d->urlList += u;
	}

    // events
	d->eventList.clear();
	nl = childElementsByTagNameNS(root, QStringLiteral("jabber:x:event"), QStringLiteral("x"));
	if (nl.count()) {
		nl = nl.item(0).childNodes();
		for(n = 0; n < nl.count(); ++n) {
			QString evtag = nl.item(n).toElement().tagName();
			if (evtag == QLatin1String("id")) {
				d->eventId =  nl.item(n).toElement().text();
			}
			else if (evtag == QLatin1String("displayed"))
				d->eventList += DisplayedEvent;
			else if (evtag == QLatin1String("composing"))
				d->eventList += ComposingEvent;
			else if (evtag == QLatin1String("delivered"))
				d->eventList += DeliveredEvent;
		}
		if (d->eventList.isEmpty())
			d->eventList += CancelEvent;
	}

	// Chat states
	QString chatStateNS = QStringLiteral("http://jabber.org/protocol/chatstates");
	t = childElementsByTagNameNS(root, chatStateNS, QStringLiteral("active")).item(0).toElement();
	if(!t.isNull())
		d->chatState = StateActive;
	t = childElementsByTagNameNS(root, chatStateNS, QStringLiteral("composing")).item(0).toElement();
	if(!t.isNull())
		d->chatState = StateComposing;
	t = childElementsByTagNameNS(root, chatStateNS, QStringLiteral("paused")).item(0).toElement();
	if(!t.isNull())
		d->chatState = StatePaused;
	t = childElementsByTagNameNS(root, chatStateNS, QStringLiteral("inactive")).item(0).toElement();
	if(!t.isNull())
		d->chatState = StateInactive;
	t = childElementsByTagNameNS(root, chatStateNS, QStringLiteral("gone")).item(0).toElement();
	if(!t.isNull())
		d->chatState = StateGone;

	// message receipts
	QString messageReceiptNS = QStringLiteral("urn:xmpp:receipts");
	t = childElementsByTagNameNS(root, messageReceiptNS, QStringLiteral("request")).item(0).toElement();
	if(!t.isNull()) {
		d->messageReceipt = ReceiptRequest;
		d->messageReceiptId.clear();
	}
	t = childElementsByTagNameNS(root, messageReceiptNS, QStringLiteral("received")).item(0).toElement();
	if(!t.isNull()) {
		d->messageReceipt = ReceiptReceived;
		d->messageReceiptId = t.attribute(QStringLiteral("id"));
		if (d->messageReceiptId.isEmpty())
			d->messageReceiptId = id();
	}

	// xsigned
	t = childElementsByTagNameNS(root, QStringLiteral("jabber:x:signed"), QStringLiteral("x")).item(0).toElement();
	if(!t.isNull())
		d->xsigned = t.text();
	else
		d->xsigned = QString();

	// xencrypted
	t = childElementsByTagNameNS(root, QStringLiteral("jabber:x:encrypted"), QStringLiteral("x")).item(0).toElement();
	if(!t.isNull())
		d->xencrypted = t.text();
	else
		d->xencrypted = QString();

	// addresses
	d->addressList.clear();
	nl = childElementsByTagNameNS(root, QStringLiteral("http://jabber.org/protocol/address"), QStringLiteral("addresses"));
	if (nl.count()) {
		QDomElement t = nl.item(0).toElement();
		nl = t.elementsByTagName(QStringLiteral("address"));
		for(n = 0; n < nl.count(); ++n) {
			d->addressList += Address(nl.item(n).toElement());
		}
	}

	// roster item exchange
	d->rosterExchangeItems.clear();
	nl = childElementsByTagNameNS(root, QStringLiteral("http://jabber.org/protocol/rosterx"), QStringLiteral("x"));
	if (nl.count()) {
		QDomElement t = nl.item(0).toElement();
		nl = t.elementsByTagName(QStringLiteral("item"));
		for(n = 0; n < nl.count(); ++n) {
			RosterExchangeItem it = RosterExchangeItem(nl.item(n).toElement());
			if (!it.isNull())
				d->rosterExchangeItems += it;
		}
	}

	// invite
	t = childElementsByTagNameNS(root, QStringLiteral("jabber:x:conference"), QStringLiteral("x")).item(0).toElement();
	if(!t.isNull())
		d->invite = t.attribute(QStringLiteral("jid"));
	else
		d->invite = QString();

	// nick
	t = childElementsByTagNameNS(root, QStringLiteral("http://jabber.org/protocol/nick"), QStringLiteral("nick")).item(0).toElement();
	if(!t.isNull())
		d->nick = t.text();
	else
		d->nick = QString();

	// sxe
	t = childElementsByTagNameNS(root, QStringLiteral("http://jabber.org/protocol/sxe"), QStringLiteral("sxe")).item(0).toElement();
	if(!t.isNull())
		d->sxe = t;
	else
		d->sxe = QDomElement();

	t = childElementsByTagNameNS(root, QStringLiteral("http://jabber.org/protocol/muc#user"), QStringLiteral("x")).item(0).toElement();
	if(!t.isNull()) {
		d->hasMUCUser = true;
		for(QDomNode muc_n = t.firstChild(); !muc_n.isNull(); muc_n = muc_n.nextSibling()) {
			QDomElement muc_e = muc_n.toElement();
			if(muc_e.isNull())
				continue;
			if (muc_e.tagName() == QLatin1String("status")) {
				addMUCStatus(muc_e.attribute(QStringLiteral("code")).toInt());
			}
			else if (muc_e.tagName() == QLatin1String("invite")) {
				MUCInvite inv(muc_e);
				if (!inv.isNull())
					addMUCInvite(inv);
			}
			else if (muc_e.tagName() == QLatin1String("decline")) {
				setMUCDecline(MUCDecline(muc_e));
			}
			else if (muc_e.tagName() == QLatin1String("password")) {
				setMUCPassword(muc_e.text());
			}
		}
	}

	// http auth
	t = childElementsByTagNameNS(root, QStringLiteral("http://jabber.org/protocol/http-auth"), QStringLiteral("confirm")).item(0).toElement();
	if(!t.isNull()){
		d->httpAuthRequest = HttpAuthRequest(t);
	}
	else {
		d->httpAuthRequest = HttpAuthRequest();
	}

	QDomElement captcha = childElementsByTagNameNS(root, QStringLiteral("urn:xmpp:captcha"),
												 QStringLiteral("captcha")).item(0).toElement();
	QDomElement xdataRoot = root;
	if (!captcha.isNull()) {
		xdataRoot = captcha;
	}

	// data form
	t = childElementsByTagNameNS(xdataRoot, QStringLiteral("jabber:x:data"), QStringLiteral("x")).item(0).toElement();
	if (!t.isNull()) {
		d->xdata.fromXml(t);
	}

	t = childElementsByTagNameNS(root, IBBManager::ns(), QStringLiteral("data")).item(0).toElement();
	if (!t.isNull()) {
		d->ibbData.fromXml(t);
	}
	t = childElementsByTagNameNS(root, QStringLiteral("urn:xmpp:message-correct:0"), QStringLiteral("replace")).item(0).toElement();
	if (!t.isNull()) {
		d->replaceId = t.attribute(QStringLiteral("id"));
	}
	return true;
}

/*!
	Error object used to deny a request.
*/
Stanza::Error HttpAuthRequest::denyError(Stanza::Error::Auth, Stanza::Error::NotAuthorized);

/*!
	Constructs request of resource URL \a u, made by method \a m, with transaction id \a i.
*/
HttpAuthRequest::HttpAuthRequest(const QString &m, const QString &u, const QString &i) : method_(m), url_(u), id_(i), hasId_(true)
{
}

/*!
        Constructs request of resource URL \a u, made by method \a m, without transaction id.
*/
HttpAuthRequest::HttpAuthRequest(const QString &m, const QString &u) : method_(m), url_(u), hasId_(false)
{
}

/*!
	Constructs request object by reading XML <confirm/> element \a e.
*/
HttpAuthRequest::HttpAuthRequest(const QDomElement &e)
{
	fromXml(e);
}

/*!
	Returns true is object is empty (not valid).
*/
bool HttpAuthRequest::isEmpty() const
{
	return method_.isEmpty() && url_.isEmpty();
}

/*!
	Sets request method.
*/
void HttpAuthRequest::setMethod(const QString& m)
{
	method_ = m;
}

/*!
	Sets requested URL.
*/
void HttpAuthRequest::setUrl(const QString& u)
{
	url_ = u;
}

/*!
	Sets transaction identifier.
*/
void HttpAuthRequest::setId(const QString& i)
{
	id_ = i;
	hasId_ = true;
}

/*!
	Returns request method.
*/
QString HttpAuthRequest::method() const
{
	return method_;
}

/*!
	Returns requested URL.
*/
QString HttpAuthRequest::url() const
{
	return url_;
}

/*!
	Returns transaction identifier.
	Empty QString may mean both empty id or no id. Use hasId() to tell the difference.
*/
QString HttpAuthRequest::id() const
{
	return id_;
}

/*!
	Returns true if the request contains transaction id.
*/
bool HttpAuthRequest::hasId() const
{
	return hasId_;
}

/*!
	Returns XML element representing the request.
	If object is empty, this function returns empty element.
*/
QDomElement HttpAuthRequest::toXml(QDomDocument &doc) const
{
	QDomElement e;
	if(isEmpty())
		return e;

	e = doc.createElementNS(QStringLiteral("http://jabber.org/protocol/http-auth"), QStringLiteral("confirm"));
	e.setAttribute(QStringLiteral("xmlns"), QStringLiteral("http://jabber.org/protocol/http-auth"));

	if(hasId_)
		e.setAttribute(QStringLiteral("id"), id_);
	e.setAttribute(QStringLiteral("method"), method_);
	e.setAttribute(QStringLiteral("url"), url_);

	return e;
}

/*!
	Reads request data from XML element \a e.
*/
bool HttpAuthRequest::fromXml(const QDomElement &e)
{
	if(e.tagName() != QLatin1String("confirm"))
		return false;

	hasId_ = e.hasAttribute(QStringLiteral("id"));
	if(hasId_)
		id_ = e.attribute(QStringLiteral("id"));

	method_ = e.attribute(QStringLiteral("method"));
	url_ = e.attribute(QStringLiteral("url"));

	return true;
}

//---------------------------------------------------------------------------
// Subscription
//---------------------------------------------------------------------------
Subscription::Subscription(SubType type)
{
	value = type;
}

int Subscription::type() const
{
	return value;
}

QString Subscription::toString() const
{
	switch(value) {
		case Remove:
			return QStringLiteral("remove");
		case Both:
			return QStringLiteral("both");
		case From:
			return QStringLiteral("from");
		case To:
			return QStringLiteral("to");
		case None:
		default:
			return QStringLiteral("none");
	}
}

bool Subscription::fromString(const QString &s)
{
	if(s == QLatin1String("remove"))
		value = Remove;
	else if(s == QLatin1String("both"))
		value = Both;
	else if(s == QLatin1String("from"))
		value = From;
	else if(s == QLatin1String("to"))
		value = To;
	else if(s == QLatin1String("none"))
		value = None;
	else
		return false;

	return true;
}


//---------------------------------------------------------------------------
// Status
//---------------------------------------------------------------------------
/**
 * Default constructor.
 */
CapsSpec::CapsSpec() :
    hashAlgo_(CapsSpec::invalidAlgo)
{
}


/**
 * \brief Basic constructor.
 * @param node the node
 * @param ven the version
 * @param ext the list of extensions (separated by spaces)
 */
CapsSpec::CapsSpec(const QString& node, QCryptographicHash::Algorithm hashAlgo, const QString& ver)
	: node_(node)
	, ver_(ver)
    , hashAlgo_(hashAlgo)
{}

CapsSpec::CapsSpec(const DiscoItem &disco, QCryptographicHash::Algorithm hashAlgo) :
    node_(disco.node().section('#', 0, 0)),
    ver_(disco.capsHash(hashAlgo)),
    hashAlgo_(hashAlgo)
{}

/**
 * @brief Checks for validity
 * @return true on valid
 */
bool CapsSpec::isValid() const
{
	return !node_.isEmpty() && !ver_.isEmpty() && (hashAlgo_ != CapsSpec::invalidAlgo);
}


/**
 * \brief Returns the node of the capabilities specification.
 */
const QString& CapsSpec::node() const
{
	return node_;
}


/**
 * \brief Returns the version of the capabilities specification.
 */
const QString& CapsSpec::version() const
{
	return ver_;
}

QCryptographicHash::Algorithm CapsSpec::hashAlgorithm() const
{
	return hashAlgo_;
}

QDomElement CapsSpec::toXml(QDomDocument *doc) const
{
	QDomElement c = doc->createElement(QStringLiteral("c"));
	c.setAttribute(QStringLiteral("xmlns"), NS_CAPS);
	QString algo = cryptoMap().key(hashAlgo_);
	c.setAttribute(QStringLiteral("hash"),algo);
	c.setAttribute(QStringLiteral("node"),node_);
	c.setAttribute(QStringLiteral("ver"),ver_);
	return c;
}

CapsSpec CapsSpec::fromXml(const QDomElement &e)
{
	QString node = e.attribute(QStringLiteral("node"));
	QString ver = e.attribute(QStringLiteral("ver"));
	QString hashAlgo = e.attribute(QStringLiteral("hash"));
	QString ext = e.attribute(QStringLiteral("ext")); // deprecated. let it be here till 2018
	CryptoMap &cm = cryptoMap();
	CapsSpec cs;
	if (!node.isEmpty() && !ver.isEmpty()) {
		QCryptographicHash::Algorithm algo = CapsSpec::invalidAlgo;
		CryptoMap::ConstIterator it;
		if (!hashAlgo.isEmpty() && (it = cm.constFind(hashAlgo)) != cm.constEnd()) {
			algo = it.value();
		}
		cs = CapsSpec(node, algo, ver);
		if (!ext.isEmpty()) {
			cs.ext_ = ext.split(QStringLiteral(" "), QString::SkipEmptyParts);
		}
	}
	return cs;
}

CapsSpec::CryptoMap &CapsSpec::cryptoMap()
{
	static CryptoMap cm;
	if (cm.isEmpty()) {
		cm.insert(QStringLiteral("md5"),     QCryptographicHash::Md5);
		cm.insert(QStringLiteral("sha-1"),   QCryptographicHash::Sha1);
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
		cm.insert(QStringLiteral("sha-224"), QCryptographicHash::Sha224);
		cm.insert(QStringLiteral("sha-256"), QCryptographicHash::Sha256);
		cm.insert(QStringLiteral("sha-384"), QCryptographicHash::Sha384);
		cm.insert(QStringLiteral("sha-512"), QCryptographicHash::Sha512);
#endif
	}
	return cm;
}

/**
 * \brief Flattens the caps specification into the set of 'simple'
 * specifications.
 * A 'simple' specification is a specification with exactly one extension,
 * or with the version number as the extension.
 *
 * Example: A caps specification with node=http://psi-im.org, version=0.10,
 * and ext='achat vchat' would be expanded into the following list of specs:
 *	node=http://psi-im.org, ver=0.10, ext=0.10
 *	node=http://psi-im.org, ver=0.10, ext=achat
 *	node=http://psi-im.org, ver=0.10, ext=vchat
 */
QString CapsSpec::flatten() const
{
	if (isValid())
		return node_ + QLatin1String("#") + ver_;
	return QString();
}

void CapsSpec::resetVersion()
{
	ver_.clear();
}

bool CapsSpec::operator==(const CapsSpec& s) const
{
	return (node() == s.node() && version() == s.version() && hashAlgorithm() == s.hashAlgorithm());
}

bool CapsSpec::operator!=(const CapsSpec& s) const
{
	return !((*this) == s);
}

bool CapsSpec::operator<(const CapsSpec& s) const
{
	return (node() != s.node() ? node() < s.node() :
			(version() != s.version() ? version() < s.version() :
			 hashAlgorithm() < s.hashAlgorithm()));
}


class StatusPrivate : public QSharedData
{
public:
	StatusPrivate() :
		hasPhotoHash(false),
		isMUC(false),
		hasMUCItem(false),
		hasMUCDestroy(false),
		mucHistoryMaxChars(-1),
		mucHistoryMaxStanzas(-1),
		mucHistorySeconds(-1),
		ecode(-1)
	{}

	int priority;
	QString show, status, key;
	QDateTime timeStamp;
	bool isAvailable;
	bool isInvisible;
	QString photoHash;
	bool hasPhotoHash;

	QString xsigned;
	// gabber song extension
	QString songTitle;
	CapsSpec caps;
	QList<BoBData> bobDataList;

	// MUC
	bool isMUC, hasMUCItem, hasMUCDestroy;
	MUCItem mucItem;
	MUCDestroy mucDestroy;
	QList<int> mucStatuses;
	QString mucPassword;
	int mucHistoryMaxChars, mucHistoryMaxStanzas, mucHistorySeconds;
	QDateTime mucHistorySince;

	int ecode;
	QString estr;
};


Status::Status(const QString &show, const QString &status, int priority, bool available)
{
	d = new StatusPrivate;

	d->isAvailable = available;
	d->show = show;
	d->status = status;
	d->priority = priority;
	d->timeStamp = QDateTime::currentDateTime();
	d->isInvisible = false;
}

Status::Status(Type type, const QString& status, int priority)
{
	d = new StatusPrivate;

	d->status = status;
	d->priority = priority;
	d->timeStamp = QDateTime::currentDateTime();
	setType(type);
}

Status::Status(const Status &other) :
	d(other.d)
{
}

Status &Status::operator=(const Status &other)
{
	d = other.d;
	return *this;
}

Status::~Status()
{
}

bool Status::hasError() const
{
	return (d->ecode != -1);
}

void Status::setError(int code, const QString &str)
{
	d->ecode = code;
	d->estr = str;
}

void Status::setIsAvailable(bool available)
{
	d->isAvailable = available;
}

void Status::setIsInvisible(bool invisible)
{
	d->isInvisible = invisible;
}

void Status::setPriority(int x)
{
	d->priority = x;
}

void Status::setType(Status::Type _type)
{
	bool available = true;
	bool invisible = false;
	QString show;
	switch(_type) {
		case Away:    show = QStringLiteral("away"); break;
		case FFC:     show = QStringLiteral("chat"); break;
		case XA:      show = QStringLiteral("xa"); break;
		case DND:     show = QStringLiteral("dnd"); break;
		case Offline: available = false; break;
		case Invisible: invisible = true; break;
		default: break;
	}
	setShow(show);
	setIsAvailable(available);
	setIsInvisible(invisible);
}

Status::Type Status::txt2type(const QString& stat)
{
      if (stat == QLatin1String("offline"))
              return XMPP::Status::Offline;
      else if (stat == QLatin1String("online"))
              return XMPP::Status::Online;
      else if (stat == QLatin1String("away"))
              return XMPP::Status::Away;
      else if (stat == QLatin1String("xa"))
              return XMPP::Status::XA;
      else if (stat == QLatin1String("dnd"))
              return XMPP::Status::DND;
      else if (stat == QLatin1String("invisible"))
              return XMPP::Status::Invisible;
      else if (stat == QLatin1String("chat"))
              return XMPP::Status::FFC;
      else
              return XMPP::Status::Away;
}

void Status::setType(QString stat)
{
      setType(txt2type(stat));
}

void Status::setShow(const QString & _show)
{
	d->show = _show;
}

void Status::setStatus(const QString & _status)
{
	d->status = _status;
}

void Status::setTimeStamp(const QDateTime & _timestamp)
{
	d->timeStamp = _timestamp;
}

void Status::setKeyID(const QString &key)
{
	d->key = key;
}

void Status::setXSigned(const QString &s)
{
	d->xsigned = s;
}

void Status::setSongTitle(const QString & _songtitle)
{
	d->songTitle = _songtitle;
}

void Status::setCaps(const CapsSpec & caps)
{
	d->caps = caps;
}

void Status::setMUC()
{
	d->isMUC = true;
}

void Status::setMUCItem(const MUCItem& i)
{
	d->hasMUCItem = true;
	d->mucItem = i;
}

void Status::setMUCDestroy(const MUCDestroy& i)
{
	d->hasMUCDestroy = true;
	d->mucDestroy = i;
}

void Status::setMUCHistory(int maxchars, int maxstanzas, int seconds, const QDateTime &since)
{
	d->mucHistoryMaxChars = maxchars;
	d->mucHistoryMaxStanzas = maxstanzas;
	d->mucHistorySeconds = seconds;
	d->mucHistorySince = since;
}


const QString& Status::photoHash() const
{
	return d->photoHash;
}

void Status::setPhotoHash(const QString& h)
{
	d->photoHash = h;
	d->hasPhotoHash = true;
}

bool Status::hasPhotoHash() const
{
	return d->hasPhotoHash;
}

void Status::addBoBData(const BoBData &bob)
{
	d->bobDataList.append(bob);
}

QList<BoBData> Status::bobDataList() const
{
	return d->bobDataList;
}

bool Status::isAvailable() const
{
	return d->isAvailable;
}

bool Status::isAway() const
{
	return (d->show == QLatin1String("away") || d->show == QLatin1String("xa") || d->show == QLatin1String("dnd"));
}

bool Status::isInvisible() const
{
	return d->isInvisible;
}

int Status::priority() const
{
	return d->priority;
}

Status::Type Status::type() const
{
	Status::Type type = Status::Online;
	if (!isAvailable()) {
		type = Status::Offline;
	}
	else if (isInvisible()) {
		type = Status::Invisible;
	}
	else {
		QString s = show();
		if (s == QLatin1String("away"))
			type = Status::Away;
		else if (s == QLatin1String("xa"))
		  	type = Status::XA;
		else if (s == QLatin1String("dnd"))
		 	type = Status::DND;
		else if (s == QLatin1String("chat"))
			type = Status::FFC;
	}
	return type;
}

QString Status::typeString() const
{
	QString stat;
	switch(type()) {
		case XMPP::Status::Offline: stat = QStringLiteral("offline"); break;
		case XMPP::Status::Online: stat = QStringLiteral("online"); break;
		case XMPP::Status::Away: stat = QStringLiteral("away"); break;
		case XMPP::Status::XA: stat = QStringLiteral("xa"); break;
		case XMPP::Status::DND: stat = QStringLiteral("dnd"); break;
		case XMPP::Status::Invisible: stat = QStringLiteral("invisible"); break;
		case XMPP::Status::FFC: stat = QStringLiteral("chat"); break;
		default: stat = QStringLiteral("away");
	}
	return stat;
}

const QString & Status::show() const
{
	return d->show;
}
const QString & Status::status() const
{
	return d->status;
}

QDateTime Status::timeStamp() const
{
	return d->timeStamp;
}

const QString & Status::keyID() const
{
	return d->key;
}

const QString & Status::xsigned() const
{
	return d->xsigned;
}

const QString & Status::songTitle() const
{
	return d->songTitle;
}

const CapsSpec & Status::caps() const
{
	return d->caps;
}

bool Status::isMUC() const
{
	return d->isMUC || !d->mucPassword.isEmpty() || hasMUCHistory();
}

bool Status::hasMUCItem() const
{
	return d->hasMUCItem;
}

const MUCItem& Status::mucItem() const
{
	return d->mucItem;
}

bool Status::hasMUCDestroy() const
{
	return d->hasMUCDestroy;
}

const MUCDestroy& Status::mucDestroy() const
{
	return d->mucDestroy;
}

const QList<int>& Status::getMUCStatuses() const
{
	return d->mucStatuses;
}

void Status::addMUCStatus(int i)
{
	d->mucStatuses += i;
}

const QString& Status::mucPassword() const
{
	return d->mucPassword;
}

bool Status::hasMUCHistory() const
{
	return d->mucHistoryMaxChars >= 0 || d->mucHistoryMaxStanzas >= 0 || d->mucHistorySeconds >= 0 || !d->mucHistorySince.isNull();
}

int Status::mucHistoryMaxChars() const
{
	return d->mucHistoryMaxChars;
}

int Status::mucHistoryMaxStanzas() const
{
	return d->mucHistoryMaxStanzas;
}

int Status::mucHistorySeconds() const
{
	return d->mucHistorySeconds;
}

const QDateTime & Status::mucHistorySince() const
{
	return d->mucHistorySince;
}

void Status::setMUCPassword(const QString& i)
{
	d->mucPassword = i;
}

int Status::errorCode() const
{
	return d->ecode;
}

const QString & Status::errorString() const
{
	return d->estr;
}


//---------------------------------------------------------------------------
// Resource
//---------------------------------------------------------------------------
Resource::Resource(const QString &name, const Status &stat)
{
	v_name = name;
	v_status = stat;
}

Resource::~Resource()
{
}

const QString & Resource::name() const
{
	return v_name;
}

int Resource::priority() const
{
	return v_status.priority();
}

const Status & Resource::status() const
{
	return v_status;
}

void Resource::setName(const QString & _name)
{
	v_name = _name;
}

void Resource::setStatus(const Status & _status)
{
	v_status = _status;
}


//---------------------------------------------------------------------------
// ResourceList
//---------------------------------------------------------------------------
ResourceList::ResourceList()
:QList<Resource>()
{
}

ResourceList::~ResourceList()
{
}

ResourceList::Iterator ResourceList::find(const QString & _find)
{
	for(ResourceList::Iterator it = begin(); it != end(); ++it) {
		if((*it).name() == _find)
			return it;
	}

	return end();
}

ResourceList::Iterator ResourceList::priority()
{
	ResourceList::Iterator highest = end();

	for(ResourceList::Iterator it = begin(); it != end(); ++it) {
		if(highest == end() || (*it).priority() > (*highest).priority())
			highest = it;
	}

	return highest;
}

ResourceList::ConstIterator ResourceList::find(const QString & _find) const
{
	for(ResourceList::ConstIterator it = begin(); it != end(); ++it) {
		if((*it).name() == _find)
			return it;
	}

	return end();
}

ResourceList::ConstIterator ResourceList::priority() const
{
	ResourceList::ConstIterator highest = end();

	for(ResourceList::ConstIterator it = begin(); it != end(); ++it) {
		if(highest == end() || (*it).priority() > (*highest).priority())
			highest = it;
	}

	return highest;
}


//---------------------------------------------------------------------------
// RosterItem
//---------------------------------------------------------------------------
RosterItem::RosterItem(const Jid &_jid)
{
	v_jid = _jid;
	v_push = false;
}

RosterItem::~RosterItem()
{
}

const Jid & RosterItem::jid() const
{
	return v_jid;
}

const QString & RosterItem::name() const
{
	return v_name;
}

const QStringList & RosterItem::groups() const
{
	return v_groups;
}

const Subscription & RosterItem::subscription() const
{
	return v_subscription;
}

const QString & RosterItem::ask() const
{
	return v_ask;
}

bool RosterItem::isPush() const
{
	return v_push;
}

bool RosterItem::inGroup(const QString &g) const
{
	for(QStringList::ConstIterator it = v_groups.begin(); it != v_groups.end(); ++it) {
		if(*it == g)
			return true;
	}
	return false;
}

void RosterItem::setJid(const Jid &_jid)
{
	v_jid = _jid;
}

void RosterItem::setName(const QString &_name)
{
	v_name = _name;
}

void RosterItem::setGroups(const QStringList &_groups)
{
	v_groups = _groups;
}

void RosterItem::setSubscription(const Subscription &type)
{
	v_subscription = type;
}

void RosterItem::setAsk(const QString &_ask)
{
	v_ask = _ask;
}

void RosterItem::setIsPush(bool b)
{
	v_push = b;
}

bool RosterItem::addGroup(const QString &g)
{
	if(inGroup(g))
		return false;

	v_groups += g;
	return true;
}

bool RosterItem::removeGroup(const QString &g)
{
	for(QStringList::Iterator it = v_groups.begin(); it != v_groups.end(); ++it) {
		if(*it == g) {
			v_groups.erase(it);
			return true;
		}
	}

	return false;
}

QDomElement RosterItem::toXml(QDomDocument *doc) const
{
	QDomElement item = doc->createElement(QStringLiteral("item"));
	item.setAttribute(QStringLiteral("jid"), v_jid.full());
	item.setAttribute(QStringLiteral("name"), v_name);
	item.setAttribute(QStringLiteral("subscription"), v_subscription.toString());
	if(!v_ask.isEmpty())
		item.setAttribute(QStringLiteral("ask"), v_ask);
	for(QStringList::ConstIterator it = v_groups.begin(); it != v_groups.end(); ++it)
		item.appendChild(textTag(doc, QStringLiteral("group"), *it));

	return item;
}

bool RosterItem::fromXml(const QDomElement &item)
{
	if(item.tagName() != QLatin1String("item"))
		return false;
	Jid j(item.attribute(QStringLiteral("jid")));
	if(!j.isValid())
		return false;
	QString na = item.attribute(QStringLiteral("name"));
	Subscription s;
	if(!s.fromString(item.attribute(QStringLiteral("subscription"))) )
		return false;
	QStringList g;
	for(QDomNode n = item.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement i = n.toElement();
		if(i.isNull())
			continue;
		if(i.tagName() == QLatin1String("group"))
			g += tagContent(i);
	}
	QString a = item.attribute(QStringLiteral("ask"));

	v_jid = j;
	v_name = na;
	v_subscription = s;
	v_groups = g;
	v_ask = a;

	return true;
}


//---------------------------------------------------------------------------
// Roster
//---------------------------------------------------------------------------
Roster::Roster()
:QList<RosterItem>()
{
}

Roster::~Roster()
{
}

Roster::Iterator Roster::find(const Jid &j)
{
	for(Roster::Iterator it = begin(); it != end(); ++it) {
		if((*it).jid().compare(j))
			return it;
	}

	return end();
}

Roster::ConstIterator Roster::find(const Jid &j) const
{
	for(Roster::ConstIterator it = begin(); it != end(); ++it) {
		if((*it).jid().compare(j))
			return it;
	}

	return end();
}


//---------------------------------------------------------------------------
// FormField
//---------------------------------------------------------------------------
FormField::FormField(const QString &type, const QString &value)
{
	v_type = misc;
	if(!type.isEmpty()) {
		int x = tagNameToType(type);
		if(x != -1)
			v_type = x;
	}
	v_value = value;
}

FormField::~FormField()
{
}

int FormField::type() const
{
	return v_type;
}

QString FormField::realName() const
{
	return typeToTagName(v_type);
}

QString FormField::fieldName() const
{
	switch(v_type) {
		case username:  return QObject::tr("Username");
		case nick:      return QObject::tr("Nickname");
		case password:  return QObject::tr("Password");
		case name:      return QObject::tr("Name");
		case first:     return QObject::tr("First Name");
		case last:      return QObject::tr("Last Name");
		case email:     return QObject::tr("E-mail");
		case address:   return QObject::tr("Address");
		case city:      return QObject::tr("City");
		case state:     return QObject::tr("State");
		case zip:       return QObject::tr("Zipcode");
		case phone:     return QObject::tr("Phone");
		case url:       return QObject::tr("URL");
		case date:      return QObject::tr("Date");
		case misc:      return QObject::tr("Misc");
		default:        return QLatin1String("");
	};
}

bool FormField::isSecret() const
{
	return (type() == password);
}

const QString & FormField::value() const
{
	return v_value;
}

void FormField::setType(int x)
{
	v_type = x;
}

bool FormField::setType(const QString &in)
{
	int x = tagNameToType(in);
	if(x == -1)
		return false;

	v_type = x;
	return true;
}

void FormField::setValue(const QString &in)
{
	v_value = in;
}

int FormField::tagNameToType(const QString &in) const
{
	if(!in.compare(QLatin1String("username"))) return username;
	if(!in.compare(QLatin1String("nick")))     return nick;
	if(!in.compare(QLatin1String("password"))) return password;
	if(!in.compare(QLatin1String("name")))     return name;
	if(!in.compare(QLatin1String("first")))    return first;
	if(!in.compare(QLatin1String("last")))     return last;
	if(!in.compare(QLatin1String("email")))    return email;
	if(!in.compare(QLatin1String("address")))  return address;
	if(!in.compare(QLatin1String("city")))     return city;
	if(!in.compare(QLatin1String("state")))    return state;
	if(!in.compare(QLatin1String("zip")))      return zip;
	if(!in.compare(QLatin1String("phone")))    return phone;
	if(!in.compare(QLatin1String("url")))      return url;
	if(!in.compare(QLatin1String("date")))     return date;
	if(!in.compare(QLatin1String("misc")))     return misc;

	return -1;
}

QString FormField::typeToTagName(int type) const
{
	switch(type) {
		case username:  return QStringLiteral("username");
		case nick:      return QStringLiteral("nick");
		case password:  return QStringLiteral("password");
		case name:      return QStringLiteral("name");
		case first:     return QStringLiteral("first");
		case last:      return QStringLiteral("last");
		case email:     return QStringLiteral("email");
		case address:   return QStringLiteral("address");
		case city:      return QStringLiteral("city");
		case state:     return QStringLiteral("state");
		case zip:       return QStringLiteral("zipcode");
		case phone:     return QStringLiteral("phone");
		case url:       return QStringLiteral("url");
		case date:      return QStringLiteral("date");
		case misc:      return QStringLiteral("misc");
		default:        return QLatin1String("");
	};
}


//---------------------------------------------------------------------------
// Form
//---------------------------------------------------------------------------
Form::Form(const Jid &j)
:QList<FormField>()
{
	setJid(j);
}

Form::~Form()
{
}

Jid Form::jid() const
{
	return v_jid;
}

QString Form::instructions() const
{
	return v_instructions;
}

QString Form::key() const
{
	return v_key;
}

void Form::setJid(const Jid &j)
{
	v_jid = j;
}

void Form::setInstructions(const QString &s)
{
	v_instructions = s;
}

void Form::setKey(const QString &s)
{
	v_key = s;
}


//---------------------------------------------------------------------------
// SearchResult
//---------------------------------------------------------------------------
SearchResult::SearchResult(const Jid &jid)
{
	setJid(jid);
}

SearchResult::~SearchResult()
{
}

const Jid & SearchResult::jid() const
{
	return v_jid;
}

const QString & SearchResult::nick() const
{
	return v_nick;
}

const QString & SearchResult::first() const
{
	return v_first;
}

const QString & SearchResult::last() const
{
	return v_last;
}

const QString & SearchResult::email() const
{
	return v_email;
}

void SearchResult::setJid(const Jid &jid)
{
	v_jid = jid;
}

void SearchResult::setNick(const QString &nick)
{
	v_nick = nick;
}

void SearchResult::setFirst(const QString &first)
{
	v_first = first;
}

void SearchResult::setLast(const QString &last)
{
	v_last = last;
}

void SearchResult::setEmail(const QString &email)
{
	v_email = email;
}

PubSubItem::PubSubItem()
{
}

PubSubItem::PubSubItem(const QString& id, const QDomElement& payload) : id_(id), payload_(payload)
{
}

const QString& PubSubItem::id() const
{
	return id_;
}

const QDomElement& PubSubItem::payload() const
{
	return payload_;
}


PubSubRetraction::PubSubRetraction()
{
}

PubSubRetraction::PubSubRetraction(const QString& id) : id_(id)
{
}

const QString& PubSubRetraction::id() const
{
	return id_;
}



// =========================================
//            CaptchaChallenge
// =========================================
class CaptchaChallengePrivate : public QSharedData
{
public:
	CaptchaChallengePrivate() :
	    state(CaptchaChallenge::New) {}

	CaptchaChallenge::State state;
	Jid arbiter;
	Jid offendedJid;
	XData form;
	QDateTime dt;
	QString explanation;
	UrlList urls;
};

CaptchaChallenge::CaptchaChallenge() :
    d(new CaptchaChallengePrivate)
{}

CaptchaChallenge::CaptchaChallenge(const CaptchaChallenge &other) :
    d(other.d)
{}

CaptchaChallenge::CaptchaChallenge(const Message &m) :
    d(new CaptchaChallengePrivate)
{
	if (m.spooled()) {
		if (m.timeStamp().secsTo(QDateTime::currentDateTime()) < Timeout) {
			return;
		}
		d->dt = m.timeStamp();
	} else {
		d->dt = QDateTime::currentDateTime();
	}

	if (m.getForm().registrarType() != QLatin1String("urn:xmpp:captcha") || m.getForm().type() != XData::Data_Form)
		return;

	if (m.id().isEmpty() || m.getForm().getField(QStringLiteral("challenge")).value().value(0) !=m.id())
		return;

	if (m.getForm().getField(QStringLiteral("from")).value().value(0).isEmpty())
		return;

	d->form = m.getForm();
	d->explanation = m.body();
	d->urls = m.urlList();
	d->arbiter = m.from();
	d->offendedJid = Jid(m.getForm().getField(QStringLiteral("from")).value().value(0));
}

CaptchaChallenge::~CaptchaChallenge()
{

}

CaptchaChallenge &CaptchaChallenge::operator=(const CaptchaChallenge &from)
{
	d = from.d;
	return *this;
}

const XData &CaptchaChallenge::form() const
{
	return d->form;
}

QString CaptchaChallenge::explanation() const
{
	return d->explanation;
}

const UrlList &CaptchaChallenge::urls() const
{
	return d->urls;
}

CaptchaChallenge::State CaptchaChallenge::state() const
{
	return d->state;
}

CaptchaChallenge::Result CaptchaChallenge::validateResponse(const XData &xd)
{
	d->state = Fail;
	return Unavailable; // TODO implement response validation
}

bool CaptchaChallenge::isValid() const
{
	return d->dt.isValid() &&
	        d->dt.secsTo(QDateTime::currentDateTime()) < Timeout &&
	        d->form.fields().count() > 0;
}

const Jid &CaptchaChallenge::offendedJid() const
{
	return d->offendedJid;
}

const Jid &CaptchaChallenge::arbiter() const
{
	return d->arbiter;
}

}
