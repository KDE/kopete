/*
    kopeteonlinestatusmanager.cpp

    Copyright (c) 2004 by Olivier Goffart  <ogoffart@kde.fr>
    Copyright (c) 2003 by Will Stephenson <wstephenson@kde.org>

    Kopete    (c) 2003-2008 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteonlinestatusaction.h"

#include <kopeteonlinestatus.h>

namespace Kopete {

class OnlineStatusAction::Private
{
public:
	Private( const OnlineStatus& t_status )
	 : status(t_status)
	{}

	OnlineStatus status;
};

OnlineStatusAction::OnlineStatusAction( const OnlineStatus& status, const QString &text, const QIcon &pix, QObject *parent )
	: KAction( KIcon(pix), text, parent) , d( new Private(status) )
{
	setShortcut( KShortcut() );

	connect(this, SIGNAL(triggered(bool)), this, SLOT(slotActivated()));

	connect(parent,SIGNAL(destroyed()),this,SLOT(deleteLater()));
}

OnlineStatusAction::~OnlineStatusAction()
{
	delete d;
}

void OnlineStatusAction::slotActivated()
{
	emit activated(d->status);
}


} //END namespace Kopete

#include "kopeteonlinestatusaction.moc"

// vim: set noet ts=4 sts=4 sw=4:

