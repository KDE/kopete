/*
 * xmlfilter.cpp - stream XML parsing
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

#include"xmpp_xmlfilter.h"

using namespace Jabber;

//! \class Jabber::XmlFilter xmlfilter.h
//! \brief XML to Dom converter
//!
//! This function will parse through the XML string and create
//! a QT Dom Element.

//! \fn void Jabber::XmlFilter::packetReady(const QDomElement &)
//! \brief sends the converted Dom file
//!
//! This function will send you the QDomElement.

//! \fn void Jabber::XmlFilter::handshake(bool,const QString &)
//! \brief Tells you initialization information.
//!
//! Tells you if the initialization was successfull or not.
//! \param bool - tells you if the initilization was successfull or not.
//! \param QString - tells you the status of the bool.

//----------------------------------------------------------------------------
// XmlFilter
//----------------------------------------------------------------------------
//! \brief Creat XmlFilter Object
XmlFilter::XmlFilter()
{
	doc = 0;
	src = 0;
	reader = 0;
	handler = 0;
}

//! \brief destroy XmlFilter Object
XmlFilter::~XmlFilter()
{
	reset();
}

//! \brief reset XmlFilter's variables
void XmlFilter::reset()
{
	delete reader;
	delete src;
	delete handler;
	delete doc;

	doc = 0;
	src = 0;
	reader = 0;
	handler = 0;
}

//! \brief Initializes XML filtering.
void XmlFilter::begin()
{
	reset();

	// start an XML document
	doc = new QDomDocument;

	// setup the input source
	src = new QXmlInputSource;
	first_time = true;

	// setup the reader and handler
	reader = new QXmlSimpleReader;
	handler = new XmlHandler(doc);
	connect(handler, SIGNAL(packetReady(const QDomElement &)), SLOT(handler_packetReady(const QDomElement &)));
	connect(handler, SIGNAL(handshake(bool, const QString &)), SLOT(handler_handshake(bool, const QString &)));
	reader->setContentHandler(handler);
}

//! \brief Put Xml data here
//!
//! Prepare the object for Xml to Dom conversion
//!
//! \param QByteArray - data to be decoded to Dom
void XmlFilter::putIncomingXmlData(const QByteArray &buf)
{
	if(!doc)
		return;

	// crunch the new data (*chomp, chomp!*)
	src->setData(buf);
	if(first_time) {
		reader->parse(src, true);
		first_time = false;
	}
	else
		reader->parseContinue();
}

void XmlFilter::handler_packetReady(const QDomElement &e)
{
	packetReady(e);
}

void XmlFilter::handler_handshake(bool b, const QString &s)
{
	handshake(b, s);
}


//----------------------------------------------------------------------------
// XmlHandler
//----------------------------------------------------------------------------

//! \brief Create XmlHandler
//!
//! \param QDomDocument - doc to initialize.
XmlHandler::XmlHandler(QDomDocument *_doc)
{
	doc = _doc;
}

//! \brief Tells the object to reset the Document.
bool XmlHandler::startDocument()
{
	depth = 0;
	return TRUE;
}

bool XmlHandler::startElement(const QString &ns, const QString &, const QString &name, const QXmlAttributes &attributes)
{
	if(depth >= 1) {
		QDomElement tag = doc->createElement(name);
		for(int n = 0; n < attributes.length(); ++n)
			tag.setAttribute(attributes.qName(n), attributes.value(n));

		if(depth == 1) {
			current = tag;
			chunk = tag;
		}
		else {
			current.appendChild(tag);
			current = tag;
		}

		// add namespace attribute only if it's different from parents
		bool ok = true;
		QDomElement par = current.parentNode().toElement();
		while(!par.isNull()) {
			if(par.attribute("xmlns") == ns) {
				ok = false;
				break;
			}
			par = par.parentNode().toElement();
		}
		// stream:stream is considered a parent also
		if(ns == "jabber:client")
			ok = false;
		if(ok)
			tag.setAttribute("xmlns", ns);
	}
	else {
		// stream tag?
		if(name == "stream:stream") {
			// get the id
			QString id;
			for(int n = 0; n < attributes.length(); ++n) {
				if(attributes.qName(n) == "id") {
					id = attributes.value(n);
					break;
				}
			}

			handshake(true, id);
		}
		else
			handshake(false, "");
	}

	++depth;

	return true;
}

bool XmlHandler::endElement(const QString &, const QString &, const QString &)
{
	--depth;

	if(depth >= 1) {
		// done with a section?  export the chunk
		if(depth == 1) {
			packetReady(chunk);

			// nuke
			chunk = QDomNode().toElement();
			current = QDomNode().toElement();
		}
		else
			current = current.parentNode().toElement();
	}

	return true;
}

bool XmlHandler::characters(const QString &str)
{
	if(depth >= 1) {
		QString content = str;//str.stripWhiteSpace();
		if(content.isEmpty())
			return true;

		if(!current.isNull()) {
			QDomText text = doc->createTextNode(content);
			current.appendChild(text);
		}
	}

	return true;
}

#include "xmpp_xmlfilter.moc"
