/*
    addcontactpage.h - Kopete's Add Contact GUI

    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#ifndef ADDCONTACTPAGE_H
#define ADDCONTACTPAGE_H

#include <QtGui/QWidget>

#include <kopeteprotocol.h>
#include <kopete_export.h>

/**
 * @author Duncan Mac-Vicar P. <duncan@kde.org>
 * @todo   i want to be able to have a assync apply. 
 *     (in the case of jabber, i need to translate the legacy id to a JID)
 *     this could also be useful in the case of MLSN to check if no error
 *     (and also jabber)
 */
class KOPETE_EXPORT AddContactPage : public QWidget
{
Q_OBJECT

public:
	AddContactPage(QWidget *parent=0);
	virtual ~AddContactPage();
	//Kopete::Protocol *protocol;

	/**
	 * Plugin should reimplement this methode.
	 * return true if the content of the page are valid
	 *
	 * This method is called in the add account wizzard when the user press the next button
	 * and this page is showed. when it return false, it does not go to the nextpage.
	 * You should popup a dialog to explain WHY the page has not been validate
	 */
	virtual bool validateData()=0;

	/**
	 * add the contact to the specified meta contact, with the given account
	 * return false if the contact has not been added
	 */
	virtual bool apply(Kopete::Account * , Kopete::MetaContact *) = 0;

signals:
	/**
	 * New incarnation of validateData, emit it every time you think the current data is valid/invalid
	 */
	void dataValid( AddContactPage *, bool);
};

#endif
// vim: set noet ts=4 sts=4 sw=4:
