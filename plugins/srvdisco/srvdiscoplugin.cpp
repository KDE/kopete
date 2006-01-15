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

#include <kdebug.h>

#include "srvdiscoplugin.h"
#include "kopetecontact.h"
#include "kopetechatsessionmanager.h"
#include "srvdiscofilter.h"
#include "kopeteglobal.h"


K_EXPORT_COMPONENT_FACTORY( kopete_srvdisco, SrvDiscoPluginFactory( "kopete_srvdisco" )  )

SrvDiscoPlugin::SrvDiscoPlugin(QObject *parent, const char *name, const QStringList &/*args*/)
 : Kopete::Plugin(SrvDiscoPluginFactory::instance(), parent, name)
{
	kdDebug() << "SrvDisco plugin loading" << endl;
	impl = new SrvDiscoIfaceImpl(/*"srvdisco"*/QString().toAscii());
	filter = new SrvDiscoFilterFactory();
	connect(filter,SIGNAL(created(SrvDiscoFilter*)),impl,SLOT(newHandler(SrvDiscoFilter*)));
}


SrvDiscoPlugin::~SrvDiscoPlugin()
{
	delete impl;
	delete filter;
}


#include "srvdiscoplugin.moc"


