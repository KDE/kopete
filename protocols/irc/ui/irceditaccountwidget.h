/*
   irceditaccountwidget.h - MSN Account Widget

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



#ifndef IRCEDITACCOUNTWIDEGET_H
#define IRCEDITACCOUNTWIDEGET_H

#include "editaccountwidget.h"
#include "irceditaccount.h"

class IRCProtocol;
class IRCAccount;

class IRCEditAccountWidget : public IRCEditAccountBase, public EditAccountWidget
{
	Q_OBJECT

	public:
		IRCEditAccountWidget(IRCProtocol *proto, IRCAccount *, QWidget *parent=0, const char *name=0);
		~IRCEditAccountWidget();

		virtual bool validateData();
		virtual KopeteAccount *apply();

	private:
		IRCProtocol *mProtocol;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

