/*
    linkaddressbookui.h

    This code was shamelessly stolen from kopete's add new contact wizard, used in
    Konversation, and then reappropriated by Kopete.

    Copyright (c) 2004 by John Tapsell           <john@geola.co.uk>
    Copyright (c) 2003-2005 by Will Stephenson   <will@stevello.free-online.co.uk>
    Copyright (c) 2002 by Nick Betcher           <nbetcher@kde.org>
    Copyright (c) 2002 by Duncan Mac-Vicar Prett <duncan@kde.org>

    Kopete    (c) 2002-2004 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef LINKADDRESSBOOKUI_H
#define LINKADDRESSBOOKUI_H

#include <kdialogbase.h>

class LinkAddressbookUI_Base;

namespace KABC {
		class AddressBook;
		class Addressee;
}

namespace Kopete {
		class MetaContact;
}

class LinkAddressbookUI : public KDialogBase
{
	Q_OBJECT

public:
		LinkAddressbookUI( Kopete::MetaContact * mc,
											 QWidget *parent = 0, const char *name  = 0 );
	~LinkAddressbookUI();
	KABC::Addressee addressee();
	
private:
	KABC::AddressBook * m_addressBook;
	KABC::Addressee m_addressee;
	Kopete::MetaContact * m_mc;
	LinkAddressbookUI_Base* m_mainWidget;

protected slots:
	virtual void accept();
	virtual void reject();
	void slotAddAddresseeClicked();
	void slotAddresseeListClicked( QListViewItem *addressee );
	/**
	 * Utility function, populates the addressee list
	 */
	void slotLoadAddressees();

};

#endif

// vim: set noet ts=4 sts=4 sw=4:

