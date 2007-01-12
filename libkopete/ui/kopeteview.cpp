/*
    kopeteview.cpp - View Abstract Class

    Copyright (c) 2003 by Jason Keirstead
    Copyright (c) 2003 by Olivier Goffart   <ogoffart@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteview.h"

KopeteView::KopeteView( Kopete::ChatSession *manager, Kopete::ViewPlugin *plugin )
    : m_manager(manager), m_plugin(plugin)
{
}

Kopete::ChatSession *KopeteView::msgManager() const
{
	return m_manager;
}

void KopeteView::clear()
{
	//Do nothing
}

void KopeteView::appendMessages(QList<Kopete::Message> msgs)
{
	QList<Kopete::Message>::iterator it;
	for ( it = msgs.begin(); it != msgs.end(); ++it )
	{
		appendMessage(*it);
	}

}

Kopete::ViewPlugin *KopeteView::plugin()
{
	return m_plugin;
}

KopeteView::~KopeteView( )
{
}
