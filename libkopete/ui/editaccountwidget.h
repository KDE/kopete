/*
    editaccountwidget.h - Kopete Account Widget

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

#ifndef EDITACCOUNTWIDGET_H
#define EDITACCOUNTWIDGET_H

class KopeteAccount;

/**
 * @author Olivier Goffart <ogoffart@tiscalinet.be>
 */

class EditAccountWidget
{
	public:
		EditAccountWidget(KopeteAccount *);

		virtual bool validateData()=0;
		virtual KopeteAccount *apply()=0;

	protected :
		KopeteAccount *m_account;
};
#endif


