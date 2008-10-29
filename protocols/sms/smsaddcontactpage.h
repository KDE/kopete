/*
  smsaddcontactpage.h  -  SMS Plugin

  Copyright (c) 2003      by Richard Lärkäng        <nouseforaname@home.se>

  *************************************************************************
  *                                                                       *
  * This program is free software; you can redistribute it and/or modify  *
  * it under the terms of the GNU General Public License as published by  *
  * the Free Software Foundation; either version 2 of the License, or     *
  * (at your option) any later version.                                   *
  *                                                                       *
  *************************************************************************
*/

#ifndef SMSADDCONTACTPAGE_H
#define SMSADDCONTACTPAGE_H

#include <qwidget.h>
#include <addcontactpage.h>
#include <qlabel.h>

class smsAddUI;

class SMSAddContactPage : public AddContactPage
{
   Q_OBJECT
public:
	SMSAddContactPage(QWidget *parent=0);
	~SMSAddContactPage();
	smsAddUI *smsdata;
	virtual bool validateData();
	virtual bool apply( Kopete::Account*, Kopete::MetaContact* );
};


#endif



/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

