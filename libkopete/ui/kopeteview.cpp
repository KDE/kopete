/*
    kopeteview.cpp - View Abstract Class

    Copyright (c) 2003 by Jason Keirstead
    Copyright (c) 2003 by Olivier Goffart   <ogoffart@tiscalinet.be>
    Kopete    (c) 2002-2003 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteview.h"

KopeteView::KopeteView( KopeteMessageManager *manager )
{
	m_manager = manager;
}

KopeteMessageManager *KopeteView::msgManager()
{
	return m_manager;
}

const KopeteMessageManager *KopeteView::msgManager() const
{
	return m_manager;
}

KopeteMessage::MessageType KopeteView::viewType()
{
	return m_type;
}

void KopeteView::clear()
{
	//Do nothing
}

