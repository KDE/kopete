/*
 * xmlfilter.h - stream XML parsing
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

#ifndef JABBER_XMLFILTER_H
#define JABBER_XMLFILTER_H

#include<qobject.h>
#include<qxml.h>
#include<qdom.h>
#include<qstring.h>


namespace Jabber
{
	class XmlHandler;

	class XmlFilter : public QObject
	{
		Q_OBJECT
	public:
		XmlFilter();
		~XmlFilter();

		void reset();
		void begin();

		void putIncomingXmlData(const QByteArray &);

	signals:
		void packetReady(const QDomElement &);
		void handshake(bool, const QString &);

	private slots:
		void handler_packetReady(const QDomElement &);
		void handler_handshake(bool, const QString &);

	private:
		QDomDocument *doc;
		QXmlInputSource *src;
		QXmlSimpleReader *reader;
		XmlHandler *handler;
		bool first_time;
	};

	class XmlHandler : public QObject, public QXmlDefaultHandler
	{
		Q_OBJECT
	public:
		XmlHandler(QDomDocument *);

		// Xml functions (reimplemented)
		bool startDocument();
		bool startElement(const QString &, const QString &, const QString &, const QXmlAttributes &);
		bool endElement(const QString &, const QString &, const QString &);
		bool characters(const QString &);

	signals:
		void packetReady(const QDomElement &);
		void handshake(bool, const QString &);

	private:
		QString indent, characterData;
		int depth;
		QDomDocument *doc;
		QDomElement chunk, current;
	};
}

#endif
