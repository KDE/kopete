/*
    addcontactpage.h - Kopete's Add Contact GUI

    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef ADDCONTACTPAGE_H
#define ADDCONTACTPAGE_H

#include <qwidget.h>
#include <kopeteprotocol.h>

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 *
 */

class AddContactPage : public QWidget
{
Q_OBJECT

public:
	AddContactPage(QWidget *parent=0, const char *name=0);
	virtual ~AddContactPage();
	//KopeteProtocol *protocol;

	/**
	 * Plugin should reimplement this methot.
	 * return true if the content of the page are valid
	 */
	virtual bool validateData()=0;

	/**
	 * add the contact the the specified meta contact, with the given account
	 * return false if the contact has not been added
	 */
	virtual bool apply(KopeteAccount * , KopeteMetaContact *) = 0;

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

