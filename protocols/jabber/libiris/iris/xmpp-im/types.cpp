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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include"im.h"
#include "protocol.h"
#include<qmap.h>
#include<qapplication.h>

#define NS_XML     "http://www.w3.org/XML/1998/namespace"

static QDomElement textTag(QDomDocument *doc, const QString &name, const QString &content)
{
	QDomElement tag = doc->createElement(name);
	QDomText text = doc->createTextNode(content);
	tag.appendChild(text);

	return tag;
}

static QString tagContent(const QDomElement &e)
{
	// look for some tag content
	for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomText i = n.toText();
		if(i.isNull())
			continue;
		return i.data();
	}

	return "";
}

static QDateTime stamp2TS(const QString &ts)
{
	if(ts.length() != 17)
		return QDateTime();

	int year  = ts.mid(0,4).toInt();
	int month = ts.mid(4,2).toInt();
	int day   = ts.mid(6,2).toInt();

	int hour  = ts.mid(9,2).toInt();
	int min   = ts.mid(12,2).toInt();
	int sec   = ts.mid(15,2).toInt();

	QDate xd;
	xd.setYMD(year, month, day);
	if(!xd.isValid())
		return QDateTime();

	QTime xt;
	xt.setHMS(hour, min, sec);
	if(!xt.isValid())
		return QDateTime();

	return QDateTime(xd, xt);
}

/*static QString TS2stamp(const QDateTime &d)
{
	QString str;

	str.sprintf("%04d%02d%02dT%02d:%02d:%02d",
		d.date().year(),
		d.date().month(),
		d.date().day(),
		d.time().hour(),
		d.time().minute(),
		d.time().second());

	return str;
}*/

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
// Message
//----------------------------------------------------------------------------
class Message::Private
{
public:
	Jid to, from;
	QString id, type, lang;

	StringMap subject, body, xHTMLBody;

	QString thread;
	Stanza::Error error;

	// extensions
	QDateTime timeStamp;
	UrlList urlList;
	QValueList<MsgEvent> eventList;
	QString eventId;
	QString xencrypted, invite;

	bool spooled, wasEncrypted;
};

//! \brief Constructs Message with given Jid information.
//!
//! This function will construct a Message container.
//! \param to - specify reciver (default: empty string)
Message::Message(const Jid &to)
{
	d = new Private;
	d->to = to;
	d->spooled = false;
	d->wasEncrypted = false;
	/*d->flag = false;
	d->spooled = false;
	d->wasEncrypted = false;
	d->errorCode = -1;*/
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
	return d->subject[lang];
}

//! \brief Return body information.
//!
//! This function will return a plain text or the Richtext version if it
//! it exists.
//! \param rich - Returns richtext if true and plain text if false. (default: false)
//! \note Richtext is in Qt's richtext format and not in xhtml.
QString Message::body(const QString &lang) const
{
	return d->body[lang];
}

QString Message::xHTMLBody(const QString &lang) const
{
	return d->xHTMLBody[lang];
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
}

void Message::setXHTMLBody(const QString &s, const QString &lang, const QString &attr)
{
	//ugly but needed if s is not a node but a list of leaf

	QString content = "<body xmlns='" + QString(NS_XHTML) + "' "+attr+" >\n" + s +"\n</body>";
	d->xHTMLBody[lang] = content;
}

void Message::setThread(const QString &s)
{
	d->thread = s;
}

void Message::setError(const Stanza::Error &err)
{
	d->error = err;
}

QDateTime Message::timeStamp() const
{
	return d->timeStamp;
}

void Message::setTimeStamp(const QDateTime &ts)
{
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

QString Message::xencrypted() const
{
	return d->xencrypted;
}

void Message::setXEncrypted(const QString &s)
{
	d->xencrypted = s;
}

QString Message::invite() const
{
	return d->invite;
}

void Message::setInvite(const QString &s)
{
	d->invite = s;
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
	for(it = d->subject.begin(); it != d->subject.end(); ++it) {
		const QString &str = it.data();
		if(!str.isEmpty()) {
			QDomElement e = s.createTextElement(s.baseNS(), "subject", str);
			if(!it.key().isEmpty())
				e.setAttributeNS(NS_XML, "xml:lang", it.key());
			s.appendChild(e);
		}
	}
	for(it = d->body.begin(); it != d->body.end(); ++it) {
		const QString &str = it.data();
		if(!str.isEmpty()) {
			QDomElement e = s.createTextElement(s.baseNS(), "body", str);
			if(!it.key().isEmpty())
				e.setAttributeNS(NS_XML, "xml:lang", it.key());
			s.appendChild(e);
		}
	}
	if ( !d->xHTMLBody.isEmpty()) {
		QDomElement parent = s.createElement(s.xhtmlImNS(), "html");
		for(it = d->xHTMLBody.begin(); it != d->xHTMLBody.end(); ++it) {
			const QString &str = it.data();
			if(!str.isEmpty()) {
				QDomElement child = s.createXHTMLElement(str);
				if(!it.key().isEmpty())
					child.setAttributeNS(NS_XML, "xml:lang", it.key());
				parent.appendChild(child);
			}
		}
		s.appendChild(parent);
	}
	if(d->type == "error")
		s.setError(d->error);

	// timestamp
	/*if(!d->timeStamp.isNull()) {
		QDomElement e = s.createElement("jabber:x:delay", "x");
		e.setAttribute("stamp", TS2stamp(d->timeStamp));
		s.appendChild(e);
	}*/

	// urls
	for(QValueList<Url>::ConstIterator uit = d->urlList.begin(); uit != d->urlList.end(); ++uit) {
		QDomElement x = s.createElement("jabber:x:oob", "x");
		x.appendChild(s.createTextElement("jabber:x:oob", "url", (*uit).url()));
		if(!(*uit).desc().isEmpty())
			x.appendChild(s.createTextElement("jabber:x:oob", "desc", (*uit).desc()));
		s.appendChild(x);
	}

	// events
	if (!d->eventList.isEmpty()) {
		QDomElement x = s.createElement("jabber:x:event", "x");

		if (d->body.isEmpty()) {
			if (d->eventId.isEmpty())
				x.appendChild(s.createElement("jabber:x:event","id"));
			else
				x.appendChild(s.createTextElement("jabber:x:event","id",d->eventId));
		}
		else if (d->type=="chat" || d->type=="groupchat")
			s.appendChild(  s.createElement(NS_CHATSTATES , "active" ) );

		bool need_x_event=false;
		for(QValueList<MsgEvent>::ConstIterator ev = d->eventList.begin(); ev != d->eventList.end(); ++ev) {
			switch (*ev) {
				case OfflineEvent:
					x.appendChild(s.createElement("jabber:x:event", "offline"));
					need_x_event=true;
					break;
				case DeliveredEvent:
					x.appendChild(s.createElement("jabber:x:event", "delivered"));
					need_x_event=true;
					break;
				case DisplayedEvent:
					x.appendChild(s.createElement("jabber:x:event", "displayed"));
					need_x_event=true;
					break;
				case ComposingEvent: 
					x.appendChild(s.createElement("jabber:x:event", "composing"));
					need_x_event=true;
					if (d->body.isEmpty() && (d->type=="chat" || d->type=="groupchat") )
						s.appendChild(  s.createElement(NS_CHATSTATES , "composing" ) ); 
					break;
				case CancelEvent:
					need_x_event=true;
					if (d->body.isEmpty() && (d->type=="chat" || d->type=="groupchat") )
						s.appendChild(  s.createElement(NS_CHATSTATES , "paused" ) ); 
					break;
				case InactiveEvent:
					if (d->body.isEmpty() && (d->type=="chat" || d->type=="groupchat") )
						s.appendChild(  s.createElement(NS_CHATSTATES , "inactive" ) ); 
					break;
				case GoneEvent:
					if (d->body.isEmpty() && (d->type=="chat" || d->type=="groupchat") )
						s.appendChild(  s.createElement(NS_CHATSTATES , "gone" ) ); 
					break;
			}
		}
		if(need_x_event)  //we don't need to have the (empty) x:event element if this is only <gone> or <inactive>
			s.appendChild(x);
	}
		

	// xencrypted
	if(!d->xencrypted.isEmpty())
		s.appendChild(s.createTextElement("jabber:x:encrypted", "x", d->xencrypted));

	// invite
	if(!d->invite.isEmpty()) {
		QDomElement e = s.createElement("jabber:x:conference", "x");
		e.setAttribute("jid", d->invite);
		s.appendChild(e);
	}

	return s;
}

bool Message::fromStanza(const Stanza &s, int timeZoneOffset)
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
	d->thread = QString();
	d->eventList.clear();

	QDomElement root = s.element();

	QDomNodeList nl = root.childNodes();
	uint n;
	for(n = 0; n < nl.count(); ++n) {
		QDomNode i = nl.item(n);
		if(i.isElement()) {
			QDomElement e = i.toElement();
			if(e.namespaceURI() == s.baseNS()) {
				if(e.tagName() == "subject") {
					QString lang = e.attributeNS(NS_XML, "lang", "");
					d->subject[lang] = e.text();
				}
				else if(e.tagName() == "body") {
					QString lang = e.attributeNS(NS_XML, "lang", "");
					d->body[lang] = e.text();
				}
				else if(e.tagName() == "thread")
					d->thread = e.text();
			}
			else if (e.namespaceURI() == s.xhtmlImNS()) {
				 if (e.tagName() == "html") {
					QDomNodeList htmlNL= e.childNodes();
					for (unsigned int x = 0; x < htmlNL.count(); x++) {
						QDomElement i = htmlNL.item(x).toElement();

						if (i.tagName() == "body") {
							QDomDocument RichText;
							QString lang = i.attributeNS(NS_XML, "lang", "");
							RichText.appendChild(i);
							d-> xHTMLBody[lang] = RichText.toString();
						}
					}
				}
			}
			else if (e.namespaceURI() == NS_CHATSTATES)
			{
				if(e.tagName() == "active")
				{
					//like in JEP-0022  we let the client know that we can receive ComposingEvent
					// (we can do that according to  §4.6  of the JEP-0085)
					d->eventList += ComposingEvent;
					d->eventList += InactiveEvent;
					d->eventList += GoneEvent;
				}
				else if (e.tagName() == "composing")
				{
					d->eventList += ComposingEvent;
				}
				else if (e.tagName() == "paused")
				{
					d->eventList += CancelEvent;
				}
				else if (e.tagName() == "inactive")
				{
					d->eventList += InactiveEvent;
				}
				else if (e.tagName() == "gone")
				{
					d->eventList += GoneEvent;
				}
			}
			else {
				//printf("extension element: [%s]\n", e.tagName().latin1());
			}
		}
	}

	if(s.type() == "error")
		d->error = s.error();

	// timestamp
	QDomElement t = root.elementsByTagNameNS("jabber:x:delay", "x").item(0).toElement();
	if(!t.isNull()) {
		d->timeStamp = stamp2TS(t.attribute("stamp"));
		d->timeStamp = d->timeStamp.addSecs(timeZoneOffset * 3600);
		d->spooled = true;
	}
	else {
		d->timeStamp = QDateTime::currentDateTime();
		d->spooled = false;
	}

	// urls
	d->urlList.clear();
	nl = root.elementsByTagNameNS("jabber:x:oob", "x");
	for(n = 0; n < nl.count(); ++n) {
		QDomElement t = nl.item(n).toElement();
		Url u;
		u.setUrl(t.elementsByTagName("url").item(0).toElement().text());
		u.setDesc(t.elementsByTagName("desc").item(0).toElement().text());
		d->urlList += u;
	}
	
    // events
	nl = root.elementsByTagNameNS("jabber:x:event", "x");
	if (nl.count()) {
		nl = nl.item(0).childNodes();
		for(n = 0; n < nl.count(); ++n) {
			QString evtag = nl.item(n).toElement().tagName();
			if (evtag == "id") {
				d->eventId =  nl.item(n).toElement().text();
			}
			else if (evtag == "displayed")
				d->eventList += DisplayedEvent;
			else if (evtag == "composing")
				d->eventList += ComposingEvent;
			else if (evtag == "delivered")
				d->eventList += DeliveredEvent;
			else if (evtag == "offline")
				d->eventList += OfflineEvent;
		}
		if (d->eventList.isEmpty())
			d->eventList += CancelEvent;
	}

	// xencrypted
	t = root.elementsByTagNameNS("jabber:x:encrypted", "x").item(0).toElement();
	if(!t.isNull())
		d->xencrypted = t.text();
	else
		d->xencrypted = QString();

	// invite
	t = root.elementsByTagNameNS("jabber:x:conference", "x").item(0).toElement();
	if(!t.isNull())
		d->invite = t.attribute("jid");
	else
		d->invite = QString();

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
			return "remove";
		case Both:
			return "both";
		case From:
			return "from";
		case To:
			return "to";
		case None:
		default:
			return "none";
	}
}

bool Subscription::fromString(const QString &s)
{
	if(s == "remove")
		value = Remove;
	else if(s == "both")
		value = Both;
	else if(s == "from")
		value = From;
	else if(s == "to")
		value = To;
	else if(s == "none")
		value = None;
	else
		return false;

	return true;
}


//---------------------------------------------------------------------------
// Status
//---------------------------------------------------------------------------
Status::Status(const QString &show, const QString &status, int priority, bool available)
{
	v_isAvailable = available;
	v_show = show;
	v_status = status;
	v_priority = priority;
	v_timeStamp = QDateTime::currentDateTime();
	v_isInvisible = false;
	ecode = -1;
}

Status::~Status()
{
}

bool Status::hasError() const
{
	return (ecode != -1);
}

void Status::setError(int code, const QString &str)
{
	ecode = code;
	estr = str;
}

void Status::setIsAvailable(bool available)
{
	v_isAvailable = available;
}

void Status::setIsInvisible(bool invisible)
{
	v_isInvisible = invisible;
}

void Status::setPriority(int x)
{
	v_priority = x;
}

void Status::setShow(const QString & _show)
{
	v_show = _show;
}

void Status::setStatus(const QString & _status)
{
	v_status = _status;
}

void Status::setTimeStamp(const QDateTime & _timestamp)
{
	v_timeStamp = _timestamp;
}

void Status::setKeyID(const QString &key)
{
	v_key = key;
}

void Status::setXSigned(const QString &s)
{
	v_xsigned = s;
}

void Status::setSongTitle(const QString & _songtitle)
{
	v_songTitle = _songtitle;
}

void Status::setCapsNode(const QString & _capsNode)
{
	v_capsNode = _capsNode;
}

void Status::setCapsVersion(const QString & _capsVersion)
{
	v_capsVersion = _capsVersion;
}

void Status::setCapsExt(const QString & _capsExt)
{
	v_capsExt = _capsExt;
}

bool Status::isAvailable() const
{
	return v_isAvailable;
}

bool Status::isAway() const
{
	if(v_show == "away" || v_show == "xa" || v_show == "dnd")
		return true;

	return false;
}

bool Status::isInvisible() const
{
	return v_isInvisible;
}

int Status::priority() const
{
	return v_priority;
}

const QString & Status::show() const
{
	return v_show;
}
const QString & Status::status() const
{
	return v_status;
}

QDateTime Status::timeStamp() const
{
	return v_timeStamp;
}

const QString & Status::keyID() const
{
	return v_key;
}

const QString & Status::xsigned() const
{
	return v_xsigned;
}

const QString & Status::songTitle() const
{
	return v_songTitle;
}

const QString & Status::capsNode() const
{
	return v_capsNode;
}

const QString & Status::capsVersion() const
{
	return v_capsVersion;
}

const QString & Status::capsExt() const
{
	return v_capsExt;
}

int Status::errorCode() const
{
	return ecode;
}

const QString & Status::errorString() const
{
	return estr;
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
:QValueList<Resource>()
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
			v_groups.remove(it);
			return true;
		}
	}

	return false;
}

QDomElement RosterItem::toXml(QDomDocument *doc) const
{
	QDomElement item = doc->createElement("item");
	item.setAttribute("jid", v_jid.full());
	item.setAttribute("name", v_name);
	item.setAttribute("subscription", v_subscription.toString());
	if(!v_ask.isEmpty())
		item.setAttribute("ask", v_ask);
	for(QStringList::ConstIterator it = v_groups.begin(); it != v_groups.end(); ++it)
		item.appendChild(textTag(doc, "group", *it));

	return item;
}

bool RosterItem::fromXml(const QDomElement &item)
{
	if(item.tagName() != "item")
		return false;
	Jid j(item.attribute("jid"));
	if(!j.isValid())
		return false;
	QString na = item.attribute("name");
	Subscription s;
	if(!s.fromString(item.attribute("subscription")) )
		return false;
	QStringList g;
	for(QDomNode n = item.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement i = n.toElement();
		if(i.isNull())
			continue;
		if(i.tagName() == "group")
			g += tagContent(i);
	}
	QString a = item.attribute("ask");

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
:QValueList<RosterItem>()
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
		default:        return "";
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
	if(!in.compare("username")) return username;
	if(!in.compare("nick"))     return nick;
	if(!in.compare("password")) return password;
	if(!in.compare("name"))     return name;
	if(!in.compare("first"))    return first;
	if(!in.compare("last"))     return last;
	if(!in.compare("email"))    return email;
	if(!in.compare("address"))  return address;
	if(!in.compare("city"))     return city;
	if(!in.compare("state"))    return state;
	if(!in.compare("zip"))      return zip;
	if(!in.compare("phone"))    return phone;
	if(!in.compare("url"))      return url;
	if(!in.compare("date"))     return date;
	if(!in.compare("misc"))     return misc;

	return -1;
}

QString FormField::typeToTagName(int type) const
{
	switch(type) {
		case username:  return "username";
		case nick:      return "nick";
		case password:  return "password";
		case name:      return "name";
		case first:     return "first";
		case last:      return "last";
		case email:     return "email";
		case address:   return "address";
		case city:      return "city";
		case state:     return "state";
		case zip:       return "zipcode";
		case phone:     return "phone";
		case url:       return "url";
		case date:      return "date";
		case misc:      return "misc";
		default:        return "";
	};
}


//---------------------------------------------------------------------------
// Form
//---------------------------------------------------------------------------
Form::Form(const Jid &j)
:QValueList<FormField>()
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

//---------------------------------------------------------------------------
// Features
//---------------------------------------------------------------------------

Features::Features()
{
}

Features::Features(const QStringList &l)
{
	setList(l);
}

Features::Features(const QString &str)
{
	QStringList l;
	l << str;

	setList(l);
}

Features::~Features()
{
}

QStringList Features::list() const
{
	return _list;
}

void Features::setList(const QStringList &l)
{
	_list = l;
}

bool Features::test(const QStringList &ns) const
{
	QStringList::ConstIterator it = ns.begin();
	for ( ; it != ns.end(); ++it)
		if ( _list.find( *it ) != _list.end() )
			return true;

	return false;
}

#define FID_REGISTER "jabber:iq:register"
bool Features::canRegister() const
{
	QStringList ns;
	ns << FID_REGISTER;

	return test(ns);
}

#define FID_SEARCH "jabber:iq:search"
bool Features::canSearch() const
{
	QStringList ns;
	ns << FID_SEARCH;

	return test(ns);
}

#define FID_XHTML  "http://jabber.org/protocol/xhtml-im"
bool Features::canXHTML() const
{
	QStringList ns;

	ns << FID_XHTML;

	return test(ns);
}

#define FID_GROUPCHAT "jabber:iq:conference"
bool Features::canGroupchat() const
{
	QStringList ns;
	ns << "http://jabber.org/protocol/muc";
	ns << FID_GROUPCHAT;

	return test(ns);
}

#define FID_VOICE "http://www.google.com/xmpp/protocol/voice/v1"
bool Features::canVoice() const
{
	QStringList ns;
	ns << FID_VOICE;

	return test(ns);
}

#define FID_GATEWAY "jabber:iq:gateway"
bool Features::isGateway() const
{
	QStringList ns;
	ns << FID_GATEWAY;

	return test(ns);
}

#define FID_DISCO "http://jabber.org/protocol/disco"
bool Features::canDisco() const
{
	QStringList ns;
	ns << FID_DISCO;
	ns << "http://jabber.org/protocol/disco#info";
	ns << "http://jabber.org/protocol/disco#items";

	return test(ns);
}

#define FID_VCARD "vcard-temp"
bool Features::haveVCard() const
{
	QStringList ns;
	ns << FID_VCARD;

	return test(ns);
}

// custom Psi acitons
#define FID_ADD "psi:add"

class Features::FeatureName : public QObject
{
	Q_OBJECT
public:
	FeatureName()
	: QObject(qApp)
	{
		id2s[FID_Invalid]	= tr("ERROR: Incorrect usage of Features class");
		id2s[FID_None]		= tr("None");
		id2s[FID_Register]	= tr("Register");
		id2s[FID_Search]	= tr("Search");
		id2s[FID_Groupchat]	= tr("Groupchat");
		id2s[FID_Gateway]	= tr("Gateway");
		id2s[FID_Disco]		= tr("Service Discovery");
		id2s[FID_VCard]		= tr("VCard");

		// custom Psi actions
		id2s[FID_Add]		= tr("Add to roster");

		// compute reverse map
		//QMap<QString, long>::Iterator it = id2s.begin();
		//for ( ; it != id2s.end(); ++it)
		//	s2id[it.data()] = it.key();

		id2f[FID_Register]	= FID_REGISTER;
		id2f[FID_Search]	= FID_SEARCH;
		id2f[FID_Groupchat]	= FID_GROUPCHAT;
		id2f[FID_Gateway]	= FID_GATEWAY;
		id2f[FID_Disco]		= FID_DISCO;
		id2f[FID_VCard]		= FID_VCARD;

		// custom Psi actions
		id2f[FID_Add]		= FID_ADD;
	}

	//QMap<QString, long> s2id;
	QMap<long, QString> id2s;
	QMap<long, QString> id2f;
};

static Features::FeatureName *featureName = 0;

long Features::id() const
{
	if ( _list.count() > 1 )
		return FID_Invalid;
	else if ( canRegister() )
		return FID_Register;
	else if ( canSearch() )
		return FID_Search;
	else if ( canGroupchat() )
		return FID_Groupchat;
	else if ( isGateway() )
		return FID_Gateway;
	else if ( canDisco() )
		return FID_Disco;
	else if ( haveVCard() )
		return FID_VCard;
	else if ( test(FID_ADD) )
		return FID_Add;

	return FID_None;
}

long Features::id(const QString &feature)
{
	Features f(feature);
	return f.id();
}

QString Features::feature(long id)
{
	if ( !featureName )
		featureName = new FeatureName();

	return featureName->id2f[id];
}

QString Features::name(long id)
{
	if ( !featureName )
		featureName = new FeatureName();

	return featureName->id2s[id];
}

QString Features::name() const
{
	return name(id());
}

QString Features::name(const QString &feature)
{
	Features f(feature);
	return f.name(f.id());
}

//---------------------------------------------------------------------------
// DiscoItem
//---------------------------------------------------------------------------
class DiscoItem::Private
{
public:
	Private()
	{
		action = None;
	}

	Jid jid;
	QString name;
	QString node;
	Action action;

	Features features;
	Identities identities;
};

DiscoItem::DiscoItem()
{
	d = new Private;
}

DiscoItem::DiscoItem(const DiscoItem &from)
{
	d = new Private;
	*this = from;
}

DiscoItem & DiscoItem::operator= (const DiscoItem &from)
{
	d->jid = from.d->jid;
	d->name = from.d->name;
	d->node = from.d->node;
	d->action = from.d->action;
	d->features = from.d->features;
	d->identities = from.d->identities;

	return *this;
}

DiscoItem::~DiscoItem()
{
	delete d;
}

AgentItem DiscoItem::toAgentItem() const
{
	AgentItem ai;

	ai.setJid( jid() );
	ai.setName( name() );

	Identity id;
	if ( !identities().isEmpty() )
		id = identities().first();

	ai.setCategory( id.category );
	ai.setType( id.type );

	ai.setFeatures( d->features );

	return ai;
}

void DiscoItem::fromAgentItem(const AgentItem &ai)
{
	setJid( ai.jid() );
	setName( ai.name() );

	Identity id;
	id.category = ai.category();
	id.type = ai.type();
	id.name = ai.name();

	Identities idList;
	idList << id;

	setIdentities( idList );

	setFeatures( ai.features() );
}

const Jid &DiscoItem::jid() const
{
	return d->jid;
}

void DiscoItem::setJid(const Jid &j)
{
	d->jid = j;
}

const QString &DiscoItem::name() const
{
	return d->name;
}

void DiscoItem::setName(const QString &n)
{
	d->name = n;
}

const QString &DiscoItem::node() const
{
	return d->node;
}

void DiscoItem::setNode(const QString &n)
{
	d->node = n;
}

DiscoItem::Action DiscoItem::action() const
{
	return d->action;
}

void DiscoItem::setAction(Action a)
{
	d->action = a;
}

const Features &DiscoItem::features() const
{
	return d->features;
}

void DiscoItem::setFeatures(const Features &f)
{
	d->features = f;
}

const DiscoItem::Identities &DiscoItem::identities() const
{
	return d->identities;
}

void DiscoItem::setIdentities(const Identities &i)
{
	d->identities = i;

	if ( name().isEmpty() && i.count() )
		setName( i.first().name );
}


DiscoItem::Action DiscoItem::string2action(QString s)
{
	Action a;

	if ( s == "update" )
		a = Update;
	else if ( s == "remove" )
		a = Remove;
	else
		a = None;

	return a;
}

QString DiscoItem::action2string(Action a)
{
	QString s;

	if ( a == Update )
		s = "update";
	else if ( a == Remove )
		s = "remove";
	else
		s = QString::null;

	return s;
}

}

#include"types.moc"
