/*
 * message.h - message handling classes
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/**
 ** TODO: Richtext support
 **/

#ifndef JABBER_MESSAGE_H
#define JABBER_MESSAGE_H

#include<qstring.h>
#include<qvaluelist.h>
#include"xmpp_jid.h"


class QDateTime;
class QDomElement;
class QDomDocument;

namespace Jabber
{
	class Url
	{
	public:
		Url(const QString &url="", const QString &desc="");
		Url(const Url &);
		~Url();

		Url & operator=(const Url &);

		QString url() const;
		QString desc() const;

		void setUrl(const QString &);
		void setDesc(const QString &);

	private:
		//! if _hide_doc_
		class UrlPrivate;
		//! endif
		UrlPrivate *d;
	};

	class UrlList : public QValueList<Url>
	{
	public:
		UrlList();
	};

	class Message
	{
	public:
		Message(const Jid &to="");
		Message(const Message &from);
		Message & operator=(const Message &from);
		~Message();

		Jid to() const;
		Jid from() const;
		QString body(bool rich=false) const;
		QString subject() const;
		QString type() const;
		QDateTime timeStamp() const;
		QString encrypted() const;
		UrlList urlList() const;
		bool spooled() const;
		bool wasEncrypted() const;
		QString errorString() const;
		QString invite() const;
		QString xencrypted() const;
		QString thread() const;

		void setTo(const Jid &);
		void setFrom(const Jid &);
		void setSubject(const QString &);
		void setBody(const QString &, bool rich=false);
		void setType(const QString &);
		void setTimeStamp(const QDateTime &);
		void setEncrypted(const QString &);
		void urlAdd(const Url &);
		void urlsClear();
		void setUrlList(const UrlList &);
		void setSpooled(bool);
		void setWasEncrypted(bool);
		void setInvite(const QString &);
		void setXEncrypted(const QString &);
		void setThread(const QString &);
		void setError(int, const QString &);
		void setAsXml(const QDomElement &);

		QByteArray generateEncryptablePayload(QDomDocument *doc);
		bool applyDecryptedPayload(const QByteArray &, QDomDocument *);

		QDomElement toXml(QDomDocument *);
		bool fromXml(const QDomElement &, int timeZoneOffset=0);

	private:
	//! \if _hide_doc_
		class MessagePrivate;
	//! \endif
		MessagePrivate *d;
	};
}

#endif //JABBER_MESSAGE_H
