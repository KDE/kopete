/*
   irceditaccountwidget.h - MSN Identity Widget

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



#ifndef IRCEDITIDENTITYWIDEGET_H
#define IRCEDITIDENTITYWIDEGET_H

#include "editaccountwidget.h"
#include "irceditaccount.h"

class IRCProtocol;
class IRCIdentity;

class IRCEditIdentityWidget : public IRCEditIdentityBase, public EditIdentityWidget
{
	Q_OBJECT

	public:
		IRCEditIdentityWidget(const IRCProtocol *proto, IRCIdentity *, QWidget *parent=0, const char *name=0);
		~IRCEditIdentityWidget();

		virtual bool validateData();
		virtual KopeteIdentity *apply();

	private:
		const IRCProtocol *mProtocol;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

