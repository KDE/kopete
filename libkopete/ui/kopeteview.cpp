/*
    kopeteview.cpp - View Abstract Class

    Copyright (c) 2003 by Jason Keirstead
    Copyright (c) 2003 by Olivier Goffart   <ogoffart@tiscalinet.be>
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

KopeteView::KopeteView( Kopete::ChatSession *manager )
{
	m_manager = manager;
}

Kopete::ChatSession *KopeteView::msgManager() const
{
	return m_manager;
}

Kopete::Message::ViewType KopeteView::viewType()
{
	return m_type;
}

void KopeteView::clear()
{
	//Do nothing
}

void KopeteView::appendMessages(QValueList<Kopete::Message> msgs)
{
	QValueList<Kopete::Message>::iterator it;
    for ( it = msgs.begin(); it != msgs.end(); ++it )
	{
		appendMessage(*it);
	}

}

