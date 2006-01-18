/**
 * Copyright (C)  2006  Jakub Stachowski <qbast@go2.pl>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "srvdiscofilter.h"
#include <kopetemessageevent.h>
#include <kopetemetacontact.h>
#include <kurl.h>
#include <kdebug.h>


using namespace SrvDisco;

SrvDiscoFilterFactory::SrvDiscoFilterFactory( )
{}

SrvDiscoFilterFactory::~SrvDiscoFilterFactory()
{}

MessageHandler *SrvDiscoFilterFactory::create( ChatSession */*manager*/, Message::MessageDirection direction )
{
	if ( direction != Message::Inbound )
		return 0;
	SrvDiscoFilter *handler = new SrvDiscoFilter;
	kdDebug() << "Created SrvDiscoFilter" << endl;
	emit created(handler);
	return handler;
}

int SrvDiscoFilterFactory::filterPosition( ChatSession */*manager*/, Message::MessageDirection direction )
{
	if ( direction != Message::Inbound )
		return StageDoNotCreate;
	return InStageToSent;
}

SrvDiscoFilter::SrvDiscoFilter()
{}

SrvDiscoFilter::~SrvDiscoFilter()
{}

void SrvDiscoFilter::handleMessage( MessageEvent *event )
{
	QRegExp rx("^#SRVDISCO:(ANNOUNCE:|RELEASE:)([^/]+)/([^/]+)/([^/]*)/(TCP|UDP)/([^/]+)/([\\d]+)/([^/]*)/");
	kdDebug() << "Got message: " << event->message().plainBody() << endl;
	if (rx.search(event->message().plainBody())==-1) MessageHandler::handleMessage( event );
	else {
		ServiceDef d;
		kdDebug() << "HIT!!!" << endl;
		d.name = KURL::decode_string(rx.cap(2)); 
		d.type=ServiceType(rx.cap(3),rx.cap(4),((rx.cap(5)=="TCP") ? ServiceType::TCP : ServiceType::UDP));
		d.hostName=rx.cap(6);
		d.port=rx.cap(7).toUShort();
		Kopete::MetaContact* m=event->message().from()->metaContact();
		d.contact = m->metaContactId();
		if (rx.cap(1)==QString("ANNOUNCE:")) emit newService(m,d);
		else emit deleteService(m,d);
		event->discard();		
	}
}

#include "srvdiscofilter.moc"

// vim: set noet ts=4 sts=4 sw=4:
