/*
    editaccountwidget.cpp - Kopete Account Widget

    Copyright (c) 2003 by Olivier Goffart  <ogoffart@tiscalinet.be>

    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "editaccountwidget.h"
#include "kopeteaccount.h"

EditAccountWidget::EditAccountWidget(KopeteAccount *i)
{
	m_account = i;
}

