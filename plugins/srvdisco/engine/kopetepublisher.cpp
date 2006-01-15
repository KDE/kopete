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

#define DEBUG_PREFIX "KopetePublisher"

#include <qvariant.h>
#include <qtimer.h>
#include "kopetepublisher.h"
#include "debug.h"

namespace SrvDisco {

KopetePublisher::KopetePublisher(PublicService* service, Scope::Ptr scope)
 : Publisher(service,scope)
{
	debug() << "KopetePublisher created for " << service << " at " << scope->id() <<	 endl;
}


KopetePublisher::~KopetePublisher()
{
	debug() << "KopetePublisher for " <<  m_scope->id()  << " destructor" << endl;
}

void KopetePublisher::start()
{
	QTimer::singleShot(0,this,SLOT(result()));
}

void KopetePublisher::result()
{
	bool b = true; // get this from engine's properties?
	emit finished(b); 
}

void KopetePublisher::stop()
{
	debug() << "KopetePublisher:Stopped" << endl;
	Publisher::stop();
}

}

#include "kopetepublisher.moc"
