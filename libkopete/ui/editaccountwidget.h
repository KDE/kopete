/*
    editaccountwidget.h - Kopete Identity Widget

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

#ifndef EDITIDENTITYWIDGET_H
#define EDITIDENTITYWIDGET_H

class KopeteIdentity;

/**
 * @author Olivier Goffart <ogoffart@tiscalinet.be>
 */

class EditIdentityWidget
{
	public:
		EditIdentityWidget(KopeteIdentity *);

		virtual bool validateData()=0;
		virtual KopeteIdentity *apply()=0;

	protected :
		KopeteIdentity *m_identity;
};
#endif


