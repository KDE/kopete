/*
 * message.cpp - message handling classes
 * Copyright (C) 2001, 2002  Justin Karneges
 *                           Akito Nozaki
 *                           Hideaki Omuro
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

//! \class Jabber::Url message.h
//! \brief Url - Basic url container
//!
//! Very basic url container. This container will hold the url and the
//! description. You may choose to use UrlList to handel multiple Url Object.
//!
//! The next example will show you how to send multiple url and description to your friends!
//! \code
//! #include <message.h>
//!
//! ...
//! client = new Jabber::Client;
//! ...
//!
//! Jabber::Message m(Jabber::Jid("anpluto@orbit.tenpura.org"));
//! m.setBody("Here's some cool links!");
//!
//! // Create the Url object with url and description in the constructor!
//! Jabber::Url url("http://psi.affinix.com/","Psi's official page!");
//! m.urlAdd(url);
//!
//! // Set the url and the description manually.
//! url.setUrl("http://www.kde.org/");
//! url.setDesc("The best desktop environment!!");
//! m.urlAdd(url);
//!
//! // Send message with 2 url attached
//! client->sendMessage(m);
//!
//! ...
//! \endcode
//!
//! Please refer to Message and Client for more info on sending messages.
//! \sa Jabber::Client Jabber::Message Jabber::UrlList

//! \class Jabber::UrlList message.h
//! \brief hold one or more urls
//!
//! UrlList is a QValueList<Url> (please refer to trolltechs Qt document for more info).
//!
//! The next example will show you how to send multiple url using UrlList.
//! \code
//! #include <message.h>
//!
//! ...
//! client = new Jabber::Client;
//! ...
//!
//! Jabber::Message m(Jabber::Jid("anpluto@orbit.tenpura.org"));
//! Jabber::UrlList list;
//! // Set urls
//! list += Jabber::Url("http://psi.affinix.com","Psi's offical webpage!");
//! list += Jabber::Url("http://www.trolltech.com", "Qt's offical webpage!");
//! list += Jabber::Url("http://www.kde.com", "Best Qt based desktop!");
//!
//! // Create the message with multiple url.
//! m.setUrlList(list);
//! m.setBody("here's some links");
//!
//! // Display the content inside the List
//! for(Jabber::UrlList::Iterator it = list.begin(); it != list.end(); ++it)
//! 	printf("URL: %s {%s}\n", (*it).url().latin1(), (*it).desc().latin1());
//!
//! // Send message with 3 url attached
//! client->sendMessage(m);
//! ...
//! \endcode
//! \sa Jabber::Url Jabber::Message Jabber::Client

//! \class Jabber::Message message.h
//! \brief handles all incomming/outgoing messages.
//!
//! Message member is a very handy tool that will help you process the
//! message information faster. This function will do all the XML processing
//! to convert it into Message object format.
//!
//! The next example will show you how to recive and send the message.
//! \code
//! #include <message.h>
//!
//! ...
//! client = new Jabber::Client;
//! ...
//!
//! ...
//! // Setup where the message is going to go when received
//! connect(client, SIGNAL(messageReceived(const Message &)), SLOT(clientMessageReceived(const Message &)));
//! ...
//!
//! ...
//! clientMessageReceived(const Jabber::Message &m)
//! {
//! 	// Once the message has been recieved you can print the information out.
//! 	printf("message received!\n");
//! 	printf("From: %s\n", m.from().full().latin1());
//! 	printf("Subject: %s\n", m.subject().latin1());
//! 	printf("Body: %s\n", m.body().latin1());
//! 	for(Jabber::UrlList::Iterator it = m.urlList().begin(); it != m.urlList().end(); ++it)
//!     	printf("URL: %s {%s}\n", (*it).url().latin1(), (*it).desc().latin1());
//!
//! 	// Well for example... lets forward this message to 2 people =)
//! 	// Its probably easier if i just set the "to" to my jid but i'll do it the hard way.
//! 	Jabber::Message fwd(Jid("anpluto@orbit.tenpura.org"));
//! 	// Set the subject
//! 	fwd.setSubject(m.subject());
//! 	// Since we want to know who it came from
//! 	fwd.setBody("From: m.from().full\n\n" + m.body());
//!     // Set the url list
//! 	fwd.setUrlList(m.urlList());
//!
//! 	// Ok, finally. Lets send it
//! 	client->sendMessage(fwd);
//!
//! 	// OK, lets send it off to one more person
//! 	fwd.setTo(Jid("justin@orbit.tenpura.org"));
//! 	client->sendMessage(fwd);
//! }
//! \endcode
//!
//! Pretty long example but this should show everything you need to
//! recieve/send message to your friends.
//!
//! Don't use this exact code to test. I don't want millions of message
//! forwarded to me... =P
//!
//! \sa Jabber::Client Jabber::Url Jabber::UrlList
#include"xmpp_message.h"

#include<qdatetime.h>
#include<qdom.h>

#include"xmpp_xmlcommon.h"

#include<qtextstream.h>
static QByteArray nodeToArray(const QDomNode &e)
{
	QString out;
	QTextStream ts(&out, IO_WriteOnly);
	e.save(ts, 1);
	QCString xmlToEnc = out.utf8();
	//printf("{%s}\n", out.data());
	int len = xmlToEnc.length();
	QByteArray b(len);
	memcpy(b.data(), xmlToEnc.data(), len);
	return b;
}

using namespace Jabber;

//----------------------------------------------------------------------------
// Url
//----------------------------------------------------------------------------

//! \if _hide_doc_
class Url::UrlPrivate
{
public:
	UrlPrivate() {}

	QString url;
	QString desc;
};
//! \endif

//! \if _hide_doc_
class Message::MessagePrivate
{
public:
	MessagePrivate() {}

	Jid to, from;

	QString type;
	QString subject;
	QString bodyPlain, bodyRich;
	QString encrypted;
	UrlList urlList;
	QDateTime timeStamp;
	bool spooled, wasEncrypted;
	int errorCode;
	QString errorString;
	QString thread;

	QString invite;
	QString xencrypted;

	QDomElement out;
	bool flag;
};
//! \endif

//! \brief Construct Url object with a given URL and Description.
//!
//! This function will construct a Url object.
//! \param QString - url (default: empty string)
//! \param QString - description of url (default: empty string)
//! \sa setUrl() setDesc()
Url::Url(const QString &url, const QString &desc)
{
	d = new UrlPrivate;
	d->url = url;
	d->desc = desc;
}

//! \brief Construct Url object.
//!
//! Overloaded constructor which will constructs a exact copy of the Url object that was passed to the constructor.
//! \param Url - Url Object
Url::Url(const Url &from)
{
	d = 0;
	*this = from;
}

//! \brief operator overloader needed for d pointer (Internel).
Url & Url::operator=(const Url &from)
{
	delete d;
	d = new UrlPrivate;
	*d = *from.d;

	return *this;
}

//! \brief destroy Url object.
Url::~Url()
{
	delete d;
	d = 0;
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

//! \brief Constructs UrlList
UrlList::UrlList()
: QValueList<Url>()
{
}


//----------------------------------------------------------------------------
// Message
//----------------------------------------------------------------------------
//! \brief Constructs Message with given Jid information.
//!
//! This function will construct a Message container.
//! \param to - specify reciver (default: empty string)
Message::Message(const Jid &to)
{
	d = new MessagePrivate;
	d->to = to;
	d->flag = false;
	d->spooled = false;
	d->wasEncrypted = false;
	d->errorCode = -1;
}

//! \brief Constructs a copy of Message object
//!
//! Overloaded constructor which will constructs a exact copy of the Message
//! object that was passed to the constructor.
//! \param from - Message object you want to copy
Message::Message(const Message &from)
{
	d = 0;
	*this = from;
}

//! \brief Required for internel use.
Message & Message::operator=(const Message &from)
{
	delete d;
	d = new MessagePrivate;
	*d = *from.d;

	return *this;
}

//! \brief Destroy Message object.
Message::~Message()
{
	delete d;
	d = 0;
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

//! \brief Return body information.
//!
//! This function will return a plain text or the Richtext version if it
//! it exists.
//! \param rich - Returns richtext if true and plain text if false. (default: false)
//! \note Richtext is in Qt's richtext format and not in xhtml.
QString Message::body(bool rich) const
{
	if(rich)
		return d->bodyRich;
	else
		return d->bodyPlain;
}

//! \brief Return subject information.
QString Message::subject() const
{
	return d->subject;
}

//! \brief Return type information
QString Message::type() const
{
	return d->type;
}

QString Message::encrypted() const
{
	return d->encrypted;
}

//! \brief Return time information
QDateTime Message::timeStamp() const
{
	return d->timeStamp;
}

//! \brief Return list of urls attached to message.
UrlList Message::urlList() const
{
	return d->urlList;
}

//! \brief Return spooled information.
//!
//!Returns false if the object has not finish reciving a message
//! and true if the message has been recieved.
bool Message::spooled() const
{
	return d->spooled;
}

bool Message::wasEncrypted() const
{
	return d->wasEncrypted;
}

//! \brief Return error string
QString Message::errorString() const
{
	return d->errorString;
}

void Message::setError(int code, const QString &str)
{
	d->errorCode = code;
	d->errorString = str;
}

QString Message::invite() const
{
	return d->invite;
}

QString Message::xencrypted() const
{
	return d->xencrypted;
}

//! \brief Set receivers information
//!
//! \param to - Receivers Jabber id
void Message::setTo(const Jid &to)
{
	d->to = to;
	d->flag = false;
}

void Message::setFrom(const Jid &from)
{
	d->from = from;
	d->flag = false;
}

//! \brief Set subject
//!
//! \param subject - Subject information
void Message::setSubject(const QString &subject)
{
	d->subject = subject;
	d->flag = false;
}

//! \brief Set body
//!
//! \param body - body information
//! \param rich - set richtext if true and set plaintext if false.
//! \note Richtext support will be implemented in the future... Sorry.
void Message::setBody(const QString &body, bool)
{
	d->bodyPlain = body;
	d->flag = false;
}

//! \brief Set Type of message
//!
//! \param type - type of message your going to send
void Message::setType(const QString &type)
{
	d->type = type;
	d->flag = false;
}

void Message::setTimeStamp(const QDateTime &ts)
{
	d->timeStamp = ts;
}

void Message::setEncrypted(const QString &s)
{
	d->encrypted = s;
}

QByteArray Message::generateEncryptablePayload(QDomDocument *doc)
{
	QDomElement pay = doc->createElement("payload");
	QDomElement e = toXml(doc);
	pay.setAttribute("xmlns", "http://jabber.org/protocol/e2e#payload");
	QDomElement id = doc->createElement("id");
	pay.appendChild(id);
	for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling())
		pay.appendChild(n.cloneNode());

	// convert to bytes
	return nodeToArray(pay);
}

bool Message::applyDecryptedPayload(const QByteArray &a, QDomDocument *doc)
{
	QDomDocument tmp;
	if(!tmp.setContent(a))
		return false;
	QDomElement pay = doc->importNode(tmp.documentElement(), true).toElement();
	if(pay.tagName() != "payload" || pay.attribute("xmlns") != "http://jabber.org/protocol/e2e#payload")
		return false;
	// use just our header
	QDomElement me = toXml(doc).cloneNode(false).toElement();
	// apply payload
	QString id = QString::null;
	for(QDomNode n = pay.firstChild(); !n.isNull(); n = n.nextSibling()) {
		if(n.isElement()) {
			// skip id
			if(n.toElement().tagName() == "id") {
				id = tagContent(n.toElement());
				continue;
			}
		}
		me.appendChild(n.cloneNode());
	}
	Message m;
	if(!m.fromXml(me))
		return false;
	*this = m;
	d->wasEncrypted = true;
	return true;
}

//! \brief Add Url to the url list.
//!
//! \param url - url to append
void Message::urlAdd(const Url &url)
{
	d->urlList += url;
	d->flag = false;
}

//! \brief clear out the url list.
void Message::urlsClear()
{
	d->urlList.clear();
	d->flag = false;
}

//! \brief Set urls to send
//!
//! \param urlList - list of urls to send
void Message::setUrlList(const UrlList &urlList)
{
	d->urlList = urlList;
	d->flag = false;
}

void Message::setSpooled(bool b)
{
	d->spooled = b;
}

void Message::setWasEncrypted(bool b)
{
	d->wasEncrypted = b;
}

void Message::setInvite(const QString &i)
{
	d->invite = i;
}

void Message::setXEncrypted(const QString &s)
{
	d->xencrypted = s;
}

//! \brief Set XML dom element
//!
//! \param out - xml information
void Message::setAsXml(const QDomElement &out)
{
	d->out = out;
	d->flag = true;
}

QString Message::thread() const
{
	return d->thread;
}

void Message::setThread(const QString &thread)
{
	d->thread = thread;
}

//! \brief convert QDomDocument to QDomElement
//!
//! Create message xml ready to be sent out.
//! \param doc -
QDomElement Message::toXml(QDomDocument *doc)
{
	if(d->flag)
		return d->out;

	QDomElement message = doc->createElement("message");
	if(!d->to.isEmpty())
		message.setAttribute("to", d->to.full());
	if(!d->from.isEmpty())
		message.setAttribute("from", d->from.full());
	if(!d->type.isEmpty())
		message.setAttribute("type", d->type);

	if(!d->thread.isEmpty())
		message.appendChild(textTag(doc, "thread", d->thread));

	if(!d->subject.isEmpty())
		message.appendChild(textTag(doc, "subject", d->subject));

	if(!d->bodyPlain.isEmpty())
		message.appendChild(textTag(doc, "body", d->bodyPlain));

	if(d->errorCode >= 0) {
		QDomElement e = textTag(doc, "error", d->errorString);
		e.setAttribute("code", QString::number(d->errorCode));
		message.appendChild(e);
	}

	for(QValueList<Url>::Iterator it = d->urlList.begin(); it != d->urlList.end(); ++it) {
		QDomElement x = doc->createElement("x");
		x.setAttribute("xmlns", "jabber:x:oob");
		x.appendChild(textTag(doc, "url", (*it).url()));
		if(!(*it).desc().isEmpty())
			x.appendChild(textTag(doc, "desc", (*it).desc()));
		message.appendChild(x);

	}

	if(!d->invite.isEmpty()) {
		QDomElement e = doc->createElement("x");
		e.setAttribute("xmlns", "jabber:x:conference");
		e.setAttribute("jid", d->invite);
		message.appendChild(e);
	}

	if(!d->xencrypted.isEmpty()) {
		QDomElement e = textTag(doc, "x", d->xencrypted);
		e.setAttribute("xmlns", "jabber:x:encrypted");
		message.appendChild(e);
	}

	if(!d->encrypted.isEmpty()) {
		QDomElement e = textTag(doc, "x", d->encrypted);
		e.setAttribute("xmlns", "http://jabber.org/protocol/e2e");
		message.appendChild(e);
	}

	d->out = message;
	return d->out;
}

//! \brief Conver XML encoded message into Message object.
//!
//! This function will return false if the XML chunk is not a message chunk.
//! \param e - XML encoded chunk
//! \param timeZoneOffset - If you need to set any time zone offset.
bool Message::fromXml(const QDomElement &e, int timeZoneOffset)
{
	if(e.tagName() != "message")
		return false;

	QDomElement tag;
	bool found;
	d->spooled = false;
	d->timeStamp = QDateTime::currentDateTime();
	QString errorString;

	d->to = e.attribute("to");
	d->from = e.attribute("from");
	d->type = e.attribute("type");

	d->bodyPlain = "";
	d->bodyRich = "";
	tag = findSubTag(e, "body", &found);
	if(found)
		d->bodyPlain = tagContent(tag);

	tag = findSubTag(e, "thread", &found);
	if (found)
		d->thread = tagContent(tag);

	tag = findSubTag(e, "subject", &found);
	if(found)
		d->subject = tagContent(tag);

	tag = findSubTag(e, "error", &found);
	if(found)
		d->errorString = tagContent(tag);

	d->urlList.clear();
	for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement i = n.toElement();
		if(i.isNull())
			continue;

		if(i.tagName() == "x" && i.attribute("xmlns") == "jabber:x:delay") {
			if(i.hasAttribute("stamp")) {
				if(stamp2TS(i.attribute("stamp"), &d->timeStamp)) {
					d->timeStamp = d->timeStamp.addSecs(timeZoneOffset * 3600);
					d->spooled = true;
				}
			}
		}

		if(i.tagName() == "x" && i.attribute("xmlns") == "jabber:x:oob") {
			QDomElement tag;
			bool found;
			Url u;

			tag = findSubTag(i, "url", &found);
			if(found)
				u.setUrl(tagContent(tag));
			tag = findSubTag(i, "desc", &found);
			if(found)
				u.setDesc(tagContent(tag));

			d->urlList += u;
		}

		if(i.tagName() == "x" && i.attribute("xmlns") == "jabber:x:conference") {
			d->invite = i.attribute("jid");
		}

		if(i.tagName() == "x" && i.attribute("xmlns") == "jabber:x:encrypted") {
			d->xencrypted = tagContent(i);
		}

		if(i.tagName() == "x" && i.attribute("xmlns") == "http://jabber.org/protocol/e2e")
			d->encrypted = tagContent(i);
	}

	return true;
}
