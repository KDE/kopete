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

#include <qpushbutton.h>
#include <qtextedit.h>
#include <qdom.h>
 
#include <klineedit.h>
#include <kurllabel.h>

#include "dlgjabbervcard.h"
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

	setReadOnly(true);
	
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
	leEmail->setText(vCard->vcard.field[vEmail]);
	urlEmail->setURL(vCard->vcard.field[vEmail]);
	urlHomepage->setText(vCard->vcard.field[vHomepage]);
	leHomepage->setText(vCard->vcard.field[vHomepage]);
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
	if (mIsReadOnly == true)
		emit updateNickname(leNickname->text());
	else { /* sigh. */
		doc = QDomDocument();
		QDomElement element = doc.createElement("vCard");
		element.setAttribute("version", "3.0");
		element.setAttribute("xmlns", "vcard-temp");
		element.setAttribute("prodid","-//HandGen//NONSGML vGen v1.0//EN");
		
		if (!leCountry->text().isNull())
			element.appendChild(textTag("COUNTRY", leCountry->text()));

		if (!leZIP->text().isNull())
			element.appendChild(textTag("PCODE", leZIP->text()));

		if (!leState->text().isNull())
			element.appendChild(textTag("REGION", leState->text()));

		if (!leCity->text().isNull())
			element.appendChild(textTag("LOCALITY", leCity->text()));

		if (!teAddress->text().isNull())
			element.appendChild(textTag("STREET", teAddress->text()));

		if (!lePhone->text().isNull())
			element.appendChild(textTag("VOICE", lePhone->text()));

		if (!leHomepage->text().isNull())
			element.appendChild(textTag("URL", leHomepage->text()));

		if (!leBirthday->text().isNull())
			element.appendChild(textTag("BDAY", leBirthday->text()));

		if (!leEmail->text().isNull())
			element.appendChild(textTag("EMAIL", leEmail->text()));

		if (!leNickname->text().isNull())
			element.appendChild(textTag("NICKNAME", leNickname->text()));

		if (!leName->text().isNull())
			element.appendChild(textTag("FN", leName->text()));
		
		emit saveAsXML(element);
	}
}

void dlgJabberVCard::setReadOnly(bool b)
{ /* I can't believe I'm giving in on coding style. */
	leJID->setReadOnly(b);
	leNickname->setReadOnly(b);
	leName->setReadOnly(b);
	leBirthday->setReadOnly(b);
	if (b == false) {
		urlEmail->hide();
		urlHomepage->hide();
		leEmail->show();
		leHomepage->show();
		leEmail->setText(urlEmail->text());
		leHomepage->setText(urlHomepage->text());
	}
	else {
		urlEmail->show();
		urlHomepage->show();
		leEmail->hide();
		leHomepage->hide();
	}
	teAddress->setReadOnly(b);
	leCity->setReadOnly(b);
	leState->setReadOnly(b);
	leZIP->setReadOnly(b);
	leCountry->setReadOnly(b);
	lePhone->setReadOnly(b);
	if (b == false) /* Abuse of the worst kind; kill me now. */
		btnSaveNickname->setText("Save vCard");
	else
		btnSaveNickname->setText("Save Nickname");
	mIsReadOnly = b;
}

QDomElement dlgJabberVCard::textTag(const QString &name, const QString &content) {
	QDomElement tag = doc.createElement(name);
	QDomText text = doc.createTextNode(content);
	tag.appendChild(text);
	return tag;
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

