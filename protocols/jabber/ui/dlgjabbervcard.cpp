/***************************************************************************
                          dlgjabbervcard.cpp  -  description
                             -------------------
    begin                : Thu Aug 08 2002
    copyright            : (C) 2002 by Till Gerken
    email                : till@tantalo.net
    
    Rewritten version of the original dialog
   
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

 
#include <klocale.h>
#include <klineedit.h>
#include <qtextedit.h>
#include <kurllabel.h>
#include <qpushbutton.h>
#include "dlgjabbervcard.h"
#include "dlgvcard.h"
#include "jabcommon.h"
#include "jabtasks.h"


/* 
 *  Constructs a dlgJabberVCard which is a child of 'parent', with the 
 *  name 'name'
 *
 */
dlgJabberVCard::dlgJabberVCard( QWidget* parent,  const char* name, JT_VCard *vCard )
    : dlgVCard( parent, name )
{

	if(vCard != NULL)
	{
		// populate all fields from the vCard
		assignVCard(vCard);
	}
	
	connect(btnClose, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(btnSaveNickname, SIGNAL(clicked()), this, SLOT(slotSaveNickname()));

}

/*  
 *  Destroys the object and frees any allocated resources
 */
dlgJabberVCard::~dlgJabberVCard()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 * Assign new vCard data to this dialog
 */
void dlgJabberVCard::assignVCard(JT_VCard *vCard)
{

	leJID->setText(vCard->jid);
	leNickname->setText(vCard->vcard.field[vNickname]);
	leName->setText(vCard->vcard.field[vFullname]);
	leBirthday->setText(vCard->vcard.field[vBday]);
	urlEmail->setText(vCard->vcard.field[vEmail]);
	urlEmail->setURL(vCard->vcard.field[vEmail]);
	urlHomepage->setText(vCard->vcard.field[vHomepage]);
	urlHomepage->setURL(vCard->vcard.field[vHomepage]);
	teAddress->setText(vCard->vcard.field[vStreet]);
	leCity->setText(vCard->vcard.field[vCity]);
	leState->setText(vCard->vcard.field[vState]);
	leZIP->setText(vCard->vcard.field[vPcode]);
	leCountry->setText(vCard->vcard.field[vCountry]);
	lePhone->setText(vCard->vcard.field[vPhone]);

}

/*
 * Close the dialog window
 */
void dlgJabberVCard::slotClose()
{

	delete this;

}

/*
 * Save the nickname
 */
void dlgJabberVCard::slotSaveNickname()
{
	emit updateNickname(leNickname->text());
}

#include "dlgjabbervcard.moc"
/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:

