
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

#include <klineedit.h>
#include <klocale.h>
#include <kurllabel.h>

#include "tasks.h"

#include "dlgjabbervcard.h"

/*
 *  Constructs a dlgJabberVCard which is a child of 'parent', with the
 *  name 'name'
 *
 */
dlgJabberVCard::dlgJabberVCard (QWidget * parent, const char *name, Jabber::JT_VCard * vCard):dlgVCard (parent, name)
{

	if (vCard != NULL)
	  {
		  // populate all fields from the vCard
		  assignVCard (vCard);
	  }

	connect (btnClose, SIGNAL (clicked ()), this, SLOT (slotClose ()));
	connect (btnSaveNickname, SIGNAL (clicked ()), this, SLOT (slotSaveNickname ()));

	setReadOnly (true);

}

/*
 *  Destroys the object and frees any allocated resources
 */
dlgJabberVCard::~dlgJabberVCard ()
{
	// no need to delete child widgets, Qt does it all for us
}

/*
 * Assign new vCard data to this dialog
 */
void dlgJabberVCard::assignVCard (Jabber::JT_VCard * vCard)
{

	leJID->setText (vCard->jid ().userHost ());
	leNickname->setText (vCard->vcard ().field[Jabber::vNickname]);
	leName->setText (vCard->vcard ().field[Jabber::vFullname]);
	leBirthday->setText (vCard->vcard ().field[Jabber::vBday]);
	urlEmail->setText (vCard->vcard ().field[Jabber::vEmail]);
	leEmail->setText (vCard->vcard ().field[Jabber::vEmail]);
	urlEmail->setURL (vCard->vcard ().field[Jabber::vEmail]);
	urlHomepage->setText (vCard->vcard ().field[Jabber::vHomepage]);
	leHomepage->setText (vCard->vcard ().field[Jabber::vHomepage]);
	urlHomepage->setURL (vCard->vcard ().field[Jabber::vHomepage]);
	teAddress->setText (vCard->vcard ().field[Jabber::vStreet]);
	leCity->setText (vCard->vcard ().field[Jabber::vCity]);
	leState->setText (vCard->vcard ().field[Jabber::vState]);
	leZIP->setText (vCard->vcard ().field[Jabber::vPcode]);
	leCountry->setText (vCard->vcard ().field[Jabber::vCountry]);
	lePhone->setText (vCard->vcard ().field[Jabber::vPhone]);

}

/*
 * Close the dialog window
 */
void dlgJabberVCard::slotClose ()
{

	delete this;

}

/*
 * Save the nickname
 */
void dlgJabberVCard::slotSaveNickname ()
{
	if (mIsReadOnly == true)
		emit updateNickname (leNickname->text ());

	else
	{
		doc = QDomDocument ();
		QDomElement element = doc.createElement ("vcard");

		element.setAttribute ("version", "2.0");
		element.setAttribute ("xmlns", "vcard-temp");
		element.setAttribute ("prodid", "-//HandGen//NONSGML vGen v1.0//EN");

		element.appendChild (textTag ("country", leCountry->text ()));
		element.appendChild (textTag ("pcode", leZIP->text ()));
		element.appendChild (textTag ("region", leState->text ()));
		element.appendChild (textTag ("locality", leCity->text ()));
		element.appendChild (textTag ("street", teAddress->text ()));
		element.appendChild (textTag ("voice", lePhone->text ()));
		element.appendChild (textTag ("url", leHomepage->text ()));
		element.appendChild (textTag ("bday", leBirthday->text ()));
		element.appendChild (textTag ("email", leEmail->text ()));
		element.appendChild (textTag ("nickname", leNickname->text ()));
		element.appendChild (textTag ("fn", leName->text ()));

		emit saveAsXML (element);
	}

	delete this;

}

void dlgJabberVCard::setReadOnly (bool b)
{

	leJID->setReadOnly (b);
	leNickname->setReadOnly (b);
	leName->setReadOnly (b);
	leBirthday->setReadOnly (b);
	leGender->setReadOnly (b);

	if (b == false)
	  {
		  urlEmail->hide ();
		  urlHomepage->hide ();
		  leEmail->show ();
		  leHomepage->show ();
		  leEmail->setText (urlEmail->text ());
		  leHomepage->setText (urlHomepage->text ());
	  }
	else
	  {
		  urlEmail->show ();
		  urlHomepage->show ();
		  leEmail->hide ();
		  leHomepage->hide ();
	  }

	teAddress->setReadOnly (b);
	leCity->setReadOnly (b);
	leState->setReadOnly (b);
	leZIP->setReadOnly (b);
	leCountry->setReadOnly (b);
	lePhone->setReadOnly (b);

	if (b == false)
		btnSaveNickname->setText (i18n ("Save vCard"));
	else
		btnSaveNickname->setText (i18n ("Save Nickname"));

	mIsReadOnly = b;

}

QDomElement dlgJabberVCard::textTag (const QString & name, const QString & content)
{

	QDomElement tag = doc.createElement (name);
	QDomText text = doc.createTextNode (content);

	tag.appendChild (text);

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
