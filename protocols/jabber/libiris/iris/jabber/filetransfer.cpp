/*
 * filetransfer.cpp - File Transfer
 * Copyright (C) 2004  Justin Karneges
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

#include"filetransfer.h"

#include<qtimer.h>
#include<qptrlist.h>
#include<qguardedptr.h>
#include"xmpp_xmlcommon.h"
#include"s5b.h"

#define SENDBUFSIZE 65536

using namespace XMPP;

// firstChildElement
//
// Get an element's first child element
static QDomElement firstChildElement(const QDomElement &e)
{
	for(QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling()) {
		if(n.isElement())
			return n.toElement();
	}
	return QDomElement();
}

//----------------------------------------------------------------------------
// FileTransfer
//----------------------------------------------------------------------------
class FileTransfer::Private
{
public:
	FileTransferManager *m;
	JT_FT *ft;
	Jid peer;
	QString fname;
	int size;
	int sent;
	bool rangeSupported;
	int rangeOffset, rangeLength;
	QString streamType;
	bool needStream;
	QString id, iq_id;
	S5BConnection *c;
};

FileTransfer::FileTransfer(FileTransferManager *m, QObject *parent)
:QObject(parent)
{
	d = new Private;
	d->m = m;
	d->ft = 0;
	d->c = 0;
	reset();
}

FileTransfer::~FileTransfer()
{
	printf("~FileTransfer: %p\n", this);
	reset();
	delete d;
}

void FileTransfer::reset()
{
	printf("FileTransfer: reset(), %p\n", this);
	d->m->unlink(this);

	delete d->ft;
	d->ft = 0;

	delete d->c;
	d->c = 0;

	d->needStream = false;
	d->sent = 0;
}

void FileTransfer::sendFile(const Jid &to, const QString &fname, int size)
{
	d->peer = to;
	d->fname = fname;
	d->size = size;
	d->id = d->m->link(this);

	d->ft = new JT_FT(d->m->client()->rootTask());
	connect(d->ft, SIGNAL(finished()), SLOT(ft_finished()));
	QStringList list;
	list += "http://jabber.org/protocol/bytestreams";
	d->ft->request(to, d->id, fname, size, list);
	printf("iq-set begin\n");
	d->ft->go(true);
}

int FileTransfer::dataSizeNeeded() const
{
	int pending = d->c->bytesToWrite();
	int left = d->size - (d->sent + pending);
	int size = SENDBUFSIZE - pending;
	if(size > left)
		size = left;
	return size;
}

void FileTransfer::writeFileData(const QByteArray &a)
{
	int pending = d->c->bytesToWrite();
	int left = d->size - (d->sent + pending);
	if(left <= 0)
		return;

	QByteArray block;
	if((int)a.size() > left) {
		QByteArray b = a.copy();
		block = ByteStream::takeArray(&b, left);
	}
	else
		block = a;
	printf("writing %d bytes\n", block.size());
	d->c->write(block);
}

Jid FileTransfer::peer() const
{
	return d->peer;
}

QString FileTransfer::fileName() const
{
	return d->fname;
}

uint FileTransfer::fileSize() const
{
	return d->size;
}

bool FileTransfer::rangeSupported() const
{
	return d->rangeSupported;
}

void FileTransfer::accept(int offset, int length)
{
	d->rangeOffset = offset;
	d->rangeLength = length;
	d->streamType = "http://jabber.org/protocol/bytestreams";
	d->m->con_accept(this);
}

void FileTransfer::reject()
{
	// TODO
}

void FileTransfer::ft_finished()
{
	printf("iq-set finished\n");
	JT_FT *ft = d->ft;
	d->ft = 0;

	if(ft->success()) {
		QGuardedPtr<QObject> self = this;
		accepted();
		if(!self)
			return;

		d->rangeOffset = ft->rangeOffset();
		d->rangeLength = ft->rangeLength();
		d->streamType = ft->streamType();
		d->c = d->m->client()->s5bManager()->createConnection();
		connect(d->c, SIGNAL(connected()), SLOT(s5b_connected()));
		connect(d->c, SIGNAL(bytesWritten(int)), SLOT(s5b_bytesWritten(int)));
		connect(d->c, SIGNAL(error(int)), SLOT(s5b_error(int)));
		d->c->connectToJid(d->peer, d->id);
	}
	else {
		reset();
		if(ft->statusCode() == 403)
			error(ErrReject);
		else
			error(ErrNeg);
	}
}

void FileTransfer::takeConnection(S5BConnection *c)
{
	d->c = c;
	connect(d->c, SIGNAL(connected()), SLOT(s5b_connected()));
	connect(d->c, SIGNAL(readyRead()), SLOT(s5b_readyRead()));
	connect(d->c, SIGNAL(error(int)), SLOT(s5b_error(int)));
	d->c->accept();
}

void FileTransfer::s5b_connected()
{
	QGuardedPtr<QObject> self = this;
	connected();
	if(!self)
		return;
}

void FileTransfer::s5b_readyRead()
{
	QByteArray a = d->c->read();
	d->sent += a.size();
	if(d->sent == d->size) {
		printf("about to reset\n");
		reset();
		printf("done resetting\n");
	}
	readyRead(a);
}

void FileTransfer::s5b_bytesWritten(int x)
{
	d->sent += x;
	if(d->sent == d->size) {
		printf("about to reset\n");
		reset();
		printf("done resetting\n");
	}
	printf("byteswritten\n");
	bytesWritten(x);
	printf("done bytes written\n");
}

void FileTransfer::s5b_error(int x)
{
	reset();
	if(x == S5BConnection::ErrRefused || x == S5BConnection::ErrConnect)
		error(ErrConnect);
	else
		error(ErrStream);
}

void FileTransfer::man_waitForAccept(const FTRequest &req)
{
	d->peer = req.from;
	d->id = req.id;
	d->iq_id = req.iq_id;
	d->fname = req.fname;
	d->size = req.size;
	d->rangeSupported = req.rangeSupported;

	// TODO: do something with req.streamTypes
}

//----------------------------------------------------------------------------
// FileTransferManager
//----------------------------------------------------------------------------
class FileTransferManager::Private
{
public:
	Client *client;
	QPtrList<FileTransfer> list, incoming;
	JT_PushFT *pft;
};

FileTransferManager::FileTransferManager(Client *client)
:QObject(client)
{
	d = new Private;
	d->client = client;

	d->pft = new JT_PushFT(d->client->rootTask());
	connect(d->pft, SIGNAL(incoming(const FTRequest &)), SLOT(pft_incoming(const FTRequest &)));
}

FileTransferManager::~FileTransferManager()
{
	d->incoming.setAutoDelete(true);
	d->incoming.clear();
	delete d->pft;
	delete d;
}

Client *FileTransferManager::client() const
{
	return d->client;
}

FileTransfer *FileTransferManager::createTransfer()
{
	FileTransfer *ft = new FileTransfer(this);
	return ft;
}

FileTransfer *FileTransferManager::takeIncoming()
{
	if(d->incoming.isEmpty())
		return 0;

	FileTransfer *ft = d->incoming.getFirst();
	d->incoming.removeRef(ft);

	// move to active list
	d->list.append(ft);
	return ft;
}

void FileTransferManager::pft_incoming(const FTRequest &req)
{
	printf("incoming file request from [%s] (%s)\n", req.from.full().latin1(), req.id.latin1());

	// TODO: ensure we aren't using this sid

	FileTransfer *ft = new FileTransfer(this);
	ft->man_waitForAccept(req);
	d->incoming.append(ft);
	incomingReady();
}

void FileTransferManager::s5b_incomingReady(S5BConnection *c)
{
	QPtrListIterator<FileTransfer> it(d->list);
	FileTransfer *ft = 0;
	for(FileTransfer *i; (i = it.current()); ++it) {
		if(i->d->needStream && i->d->peer.compare(c->peer()) && i->d->id == c->sid()) {
			ft = i;
			break;
		}
	}
	if(!ft) {
		c->close();
		c->deleteLater();
		return;
	}
	ft->takeConnection(c);
}

QString FileTransferManager::link(FileTransfer *ft)
{
	d->list.append(ft);
	return d->client->s5bManager()->genUniqueSID(ft->d->peer);
}

void FileTransferManager::con_accept(FileTransfer *ft)
{
	ft->d->needStream = true;
	d->pft->respondSuccess(ft->d->peer, ft->d->iq_id, ft->d->rangeOffset, ft->d->rangeLength, ft->d->streamType);
}

void FileTransferManager::unlink(FileTransfer *ft)
{
	d->list.removeRef(ft);
}

//----------------------------------------------------------------------------
// JT_FT
//----------------------------------------------------------------------------
class JT_FT::Private
{
public:
	QDomElement iq;
	Jid to;
	int rangeOffset, rangeLength;
	QString streamType;
	QStringList streamTypes;
};

JT_FT::JT_FT(Task *parent)
:Task(parent)
{
	d = new Private;
}

JT_FT::~JT_FT()
{
	delete d;
}

void JT_FT::request(const Jid &to, const QString &_id, const QString &fname, int size, const QStringList &streamTypes)
{
	QDomElement iq;
	d->to = to;
	iq = createIQ(doc(), "set", to.full(), id());
	QDomElement si = doc()->createElement("si");
	si.setAttribute("xmlns", "http://jabber.org/protocol/si");
	si.setAttribute("id", _id);
	si.setAttribute("profile", "http://jabber.org/protocol/si/profile/file-transfer");

	QDomElement file = doc()->createElement("file");
	file.setAttribute("xmlns", "http://jabber.org/protocol/si/profile/file-transfer");
	file.setAttribute("name", fname);
	file.setAttribute("size", size);
	QDomElement range = doc()->createElement("range");
	file.appendChild(range);
	si.appendChild(file);

	QDomElement feature = doc()->createElement("feature");
	feature.setAttribute("xmlns", "http://jabber.org/protocol/feature-neg");
	QDomElement x = doc()->createElement("x");
	x.setAttribute("xmlns", "jabber:x:data");
	x.setAttribute("type", "form");

	QDomElement field = doc()->createElement("field");
	field.setAttribute("var", "stream-method");
	field.setAttribute("type", "list-single");
	for(QStringList::ConstIterator it = streamTypes.begin(); it != streamTypes.end(); ++it) {
		QDomElement option = doc()->createElement("option");
		QDomElement value = doc()->createElement("value");
		value.appendChild(doc()->createTextNode(*it));
		option.appendChild(value);
		field.appendChild(option);
	}

	x.appendChild(field);
	feature.appendChild(x);

	si.appendChild(feature);
	iq.appendChild(si);

	d->streamTypes = streamTypes;
	d->iq = iq;
}

int JT_FT::rangeOffset() const
{
	return d->rangeOffset;
}

int JT_FT::rangeLength() const
{
	return d->rangeLength;
}

QString JT_FT::streamType() const
{
	return d->streamType;
}

void JT_FT::onGo()
{
	send(d->iq);
}

bool JT_FT::take(const QDomElement &x)
{
	if(!iqVerify(x, d->to, id()))
		return false;

	if(x.attribute("type") == "result") {
		QDomElement si = firstChildElement(x);
		if(si.attribute("xmlns") != "http://jabber.org/protocol/si" || si.tagName() != "si")
			return true; // ignore

		QString id = si.attribute("id");

		int range_offset = 0;
		int range_length = -1;
		QDomElement file = si.elementsByTagName("file").item(0).toElement();
		if(!file.isNull()) {
			QDomElement range = file.elementsByTagName("range").item(0).toElement();
			if(!range.isNull()) {
				int x;
				x = range.attribute("offset").toInt();
				if(x < 0)
					return true;
				range_offset = x;
				x = range.attribute("length").toInt();
				if(x < 0)
					return true;
				range_length = x;
			}
		}

		QString streamtype;
		QDomElement feature = si.elementsByTagName("feature").item(0).toElement();
		if(!feature.isNull() && feature.attribute("xmlns") == "http://jabber.org/protocol/feature-neg") {
			QDomElement x = feature.elementsByTagName("x").item(0).toElement();
			if(!x.isNull() && x.attribute("type") == "submit") {
				QDomElement field = x.elementsByTagName("field").item(0).toElement();
				if(!field.isNull() && field.attribute("var") == "stream-method") {
					QDomElement value = field.elementsByTagName("value").item(0).toElement();
					if(!value.isNull())
						streamtype = value.text();
				}
			}
		}

		// must be one of the offered streamtypes
		bool found = false;
		for(QStringList::ConstIterator it = d->streamTypes.begin(); it != d->streamTypes.end(); ++it) {
			if((*it) == streamtype) {
				found = true;
				break;
			}
		}
		if(!found)
			return true;

		d->rangeOffset = range_offset;
		d->rangeLength = range_length;
		d->streamType = streamtype;
		setSuccess();
	}
	else {
		setError(x);
	}

	return true;
}

//----------------------------------------------------------------------------
// JT_PushFT
//----------------------------------------------------------------------------
JT_PushFT::JT_PushFT(Task *parent)
:Task(parent)
{
}

JT_PushFT::~JT_PushFT()
{
}

void JT_PushFT::respondSuccess(const Jid &to, const QString &id, int rangeOffset, int rangeLength, const QString &streamType)
{
	QDomElement iq = createIQ(doc(), "result", to.full(), id);
	QDomElement si = doc()->createElement("si");
	si.setAttribute("xmlns", "http://jabber.org/protocol/si");

	if(rangeOffset != 0 || rangeLength != -1) {
		QDomElement file = doc()->createElement("file");
		file.setAttribute("xmlns", "http://jabber.org/protocol/si/profile/file-transfer");
		QDomElement range = doc()->createElement("range");
		if(rangeOffset != 0)
			range.setAttribute("offset", QString::number(rangeOffset));
		if(rangeLength != -1)
			range.setAttribute("length", QString::number(rangeLength));
		file.appendChild(range);
		si.appendChild(file);
	}

	QDomElement feature = doc()->createElement("feature");
	feature.setAttribute("xmlns", "http://jabber.org/protocol/feature-neg");
	QDomElement x = doc()->createElement("x");
	x.setAttribute("xmlns", "jabber:x:data");
	x.setAttribute("type", "submit");

	QDomElement field = doc()->createElement("field");
	field.setAttribute("var", "stream-method");
	QDomElement value = doc()->createElement("value");
	value.appendChild(doc()->createTextNode(streamType));
	field.appendChild(value);

	x.appendChild(field);
	feature.appendChild(x);

	si.appendChild(feature);
	iq.appendChild(si);
	send(iq);
}

void JT_PushFT::respondError(const Jid &to, const QString &id, int code, const QString &str)
{
	QDomElement iq = createIQ(doc(), "error", to.full(), id);
	QDomElement err = textTag(doc(), "error", str);
	err.setAttribute("code", QString::number(code));
	iq.appendChild(err);
	send(iq);
}

bool JT_PushFT::take(const QDomElement &e)
{
	// must be an iq-set tag
	if(e.tagName() != "iq")
		return false;
	if(e.attribute("type") != "set")
		return false;

	QDomElement si = firstChildElement(e);
	if(si.attribute("xmlns") != "http://jabber.org/protocol/si" || si.tagName() != "si")
		return false;
	if(si.attribute("profile") != "http://jabber.org/protocol/si/profile/file-transfer")
		return false;

	Jid from(e.attribute("from"));
	QString id = si.attribute("id");

	QDomElement file = si.elementsByTagName("file").item(0).toElement();
	if(file.isNull())
		return true;

	QString fname = file.attribute("name");
	int size = file.attribute("size").toInt();
	if(size < 1)
		return true;

	bool rangeSupported = false;
	QDomElement range = file.elementsByTagName("range").item(0).toElement();
	if(!range.isNull())
		rangeSupported = true;

	QStringList streamTypes;
	QDomElement feature = si.elementsByTagName("feature").item(0).toElement();
	if(!feature.isNull() && feature.attribute("xmlns") == "http://jabber.org/protocol/feature-neg") {
		QDomElement x = feature.elementsByTagName("x").item(0).toElement();
		if(!x.isNull() /*&& x.attribute("type") == "form"*/) {
			QDomElement field = x.elementsByTagName("field").item(0).toElement();
			if(!field.isNull() && field.attribute("var") == "stream-method" && field.attribute("type") == "list-single") {
				QDomNodeList nl = field.elementsByTagName("option");
				for(uint n = 0; n < nl.count(); ++n) {
					QDomElement e = nl.item(n).toElement();
					QDomElement value = e.elementsByTagName("value").item(0).toElement();
					if(!value.isNull())
						streamTypes += value.text();
				}
			}
		}
	}

	if(streamTypes.isEmpty())
		return true;

	FTRequest r;
	r.from = from;
	r.iq_id = e.attribute("id");
	r.id = id;
	r.fname = fname;
	r.size = size;
	r.rangeSupported = rangeSupported;
	r.streamTypes = streamTypes;

	incoming(r);
	return true;
}
