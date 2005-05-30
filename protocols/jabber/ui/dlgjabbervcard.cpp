
/***************************************************************************
                          dlgjabbervcard.cpp  -  vCard dialog
                             -------------------
    begin                : Thu Aug 08 2002
    copyright            : (C) 2002-2003 by Till Gerken <till@tantalo.net>
    email                : kopete-devel@kde.org

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

#include "dlgjabbervcard.h"

#include <qtextedit.h>

#include <qapplication.h>
#include <qwidgetstack.h>
#include <kdebug.h>
#include <kpushbutton.h>
#include <klineedit.h>
#include <klocale.h>
#include <kurllabel.h>
#include <kdialogbase.h>
#include <kmessagebox.h>
#include <krun.h>

#include "im.h"
#include "xmpp.h"
#include "xmpp_tasks.h"

#include "jabberprotocol.h"
#include "jabbercontact.h"
#include "jabberaccount.h"
#include "jabbercontactpool.h"
#include "jabberbasecontact.h"
#include "dlgvcard.h"

/*
 *  Constructs a dlgJabberVCard which is a child of 'parent', with the
 *  name 'name'
 *
 */
dlgJabberVCard::dlgJabberVCard (JabberAccount *account, const QString &jid, QWidget * parent, const char *name)
	: KDialogBase (parent, name, false, i18n("Jabber vCard"), Close | User1, Close, false, i18n("&Save User Info"))
{

	m_account = account;
	m_jid = jid;

	m_mainWidget = new dlgVCard(this);
	setMainWidget(m_mainWidget);

	connect (this, SIGNAL (user1Clicked()), this, SLOT (slotSaveVCard ()));
	connect (m_mainWidget->btnSaveNick, SIGNAL (clicked ()), this, SLOT (slotSaveNickname ()));
	connect (m_mainWidget->urlHomeEmail, SIGNAL (leftClickedURL(const QString &)), this, SLOT (slotOpenURL (const QString &)));
	connect (m_mainWidget->urlWorkEmail, SIGNAL (leftClickedURL(const QString &)), this, SLOT (slotOpenURL (const QString &)));
	connect (m_mainWidget->urlHomepage, SIGNAL (leftClickedURL(const QString &)), this, SLOT (slotOpenURL (const QString &)));

	if(m_account->myself()->contactId() == jid)
		setReadOnly (false);
	else
		setReadOnly (true);

	XMPP::JT_VCard *task = new XMPP::JT_VCard (m_account->client()->rootTask ());
	// signal to ourselves when the vCard data arrived
	QObject::connect (task, SIGNAL (finished ()), this, SLOT (slotGotVCard ()));
	task->get (m_jid);
	task->go (true);

}

/*
 *  Destroys the object and frees any allocated resources
 */
dlgJabberVCard::~dlgJabberVCard ()
{
	// no need to delete child widgets, Qt does it all for us
}

/*
 * Activated when the close button gets pressed. Deletes the dialog.
 */
void dlgJabberVCard::slotClose()
{
	kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "Deleting dialog." << endl;
	delayedDestruct();
}

/*
 * Activated when a vCard arrived from the server.
 */
void dlgJabberVCard::slotGotVCard()
{
	XMPP::JT_VCard * vCard = (XMPP::JT_VCard *) sender ();

	if ( !vCard->success() && ( m_account->myself()->contactId () != m_jid ) )
	{
		// unsuccessful, or incomplete
		KMessageBox::error (this, i18n ("Unable to retrieve vCard for %1").arg (vCard->jid ().userHost ()));
		return;
	}

	kdDebug (JABBER_DEBUG_GLOBAL) << k_funcinfo << "Got vCard for user " << vCard->jid ().userHost () << ", displaying." << endl;

	// update the contact first
	JabberContact *contact = dynamic_cast<JabberContact *> ( m_account->contactPool()->findExactMatch ( XMPP::Jid ( m_jid ) ) );
	if ( contact )
	{
		contact->setPropertiesFromVCard ( vCard->vcard () );
	}

	assignVCard(vCard->vcard());

	show ();
	raise ();

}

/*
 * Assign new vCard data to this dialog
 */
void dlgJabberVCard::assignVCard (const XMPP::VCard &vCard)
{

	// general tab
	m_mainWidget->leNick->setText (vCard.nickName());
	m_mainWidget->leName->setText (vCard.fullName());
	m_mainWidget->leJID->setText (vCard.jid());
	m_mainWidget->leBirthday->setText (vCard.bdayStr());
	m_mainWidget->leTimezone->setText (vCard.timezone());
	m_mainWidget->leHomepage->setText (vCard.url());
	m_mainWidget->urlHomepage->setText (vCard.url());
	m_mainWidget->urlHomepage->setURL (vCard.url());
	m_mainWidget->urlHomepage->setUseCursor ( !vCard.url().isEmpty () );

	// addresses
	for(XMPP::VCard::AddressList::const_iterator it = vCard.addressList().begin(); it != vCard.addressList().end(); ++it)
	{
		XMPP::VCard::Address address = (*it);

		if(address.work)
		{
			m_mainWidget->leWorkStreet->setText (address.street);
			m_mainWidget->leWorkExtAddr->setText (address.extaddr);
			m_mainWidget->leWorkPOBox->setText (address.pobox);
			m_mainWidget->leWorkCity->setText (address.locality);
			m_mainWidget->leWorkPostalCode->setText (address.pcode);
			m_mainWidget->leWorkCountry->setText (address.country);
		}
		else
		if(address.home)
		{
			m_mainWidget->leHomeStreet->setText (address.street);
			m_mainWidget->leHomeExtAddr->setText (address.extaddr);
			m_mainWidget->leHomePOBox->setText (address.pobox);
			m_mainWidget->leHomeCity->setText (address.locality);
			m_mainWidget->leHomePostalCode->setText (address.pcode);
			m_mainWidget->leHomeCountry->setText (address.country);
		}
	}

	// email
	m_mainWidget->urlWorkEmail->setUseCursor ( false );
	m_mainWidget->urlHomeEmail->setUseCursor ( false );
	for(XMPP::VCard::EmailList::const_iterator it = vCard.emailList().begin(); it != vCard.emailList().end(); it++)
	{
		XMPP::VCard::Email email = (*it);

		if(email.work)
		{
			m_mainWidget->leWorkEmail->setText (email.userid);
			m_mainWidget->urlWorkEmail->setText (email.userid);
			m_mainWidget->urlWorkEmail->setURL ("mailto:" + email.userid);
			m_mainWidget->urlWorkEmail->setUseCursor ( !email.userid.stripWhiteSpace().isEmpty () );
		}
		else
		if(email.home)
		{
			m_mainWidget->leHomeEmail->setText (email.userid);
			m_mainWidget->urlHomeEmail->setText (email.userid);
			m_mainWidget->urlHomeEmail->setURL ("mailto:" + email.userid);
			m_mainWidget->urlHomeEmail->setUseCursor ( !email.userid.stripWhiteSpace().isEmpty () );
		}
	}

	// work information tab
	m_mainWidget->leCompany->setText (vCard.org().name);
	m_mainWidget->leDepartment->setText (vCard.org().unit.join(","));
	m_mainWidget->lePosition->setText (vCard.title());
	m_mainWidget->leRole->setText (vCard.role());

	// phone numbers tab
	for(XMPP::VCard::PhoneList::const_iterator it = vCard.phoneList().begin(); it != vCard.phoneList().end(); it++)
	{
		XMPP::VCard::Phone phone = (*it);

		if(phone.work)
		{
			m_mainWidget->lePhoneWork->setText (phone.number);
		}
		else
		if(phone.fax)
		{
			m_mainWidget->lePhoneFax->setText (phone.number);
		}
		else
		if(phone.cell)
		{
			m_mainWidget->lePhoneCell->setText (phone.number);
		}
		else
		if(phone.home)
		{
			m_mainWidget->lePhoneHome->setText (phone.number);
		}

	}

	// about tab
	m_mainWidget->teAbout->setText (vCard.desc());

}

/*
 * Save the nickname
 */
void dlgJabberVCard::slotSaveNickname ()
{

	JabberBaseContact *jc = m_account->contactPool()->findExactMatch ( XMPP::Jid ( m_jid ) );

	if(!jc)
	{
		kdDebug(JABBER_DEBUG_GLOBAL) << k_funcinfo << "WARNING: Trying to save new nickname for non-existant contact " << m_jid << endl;
	}
	else
	{
		jc->setProperty ( jc->protocol()->propNickName, m_mainWidget->leNick->text() );
	}

}

void dlgJabberVCard::setReadOnly (bool state)
{

	// general tab
	m_mainWidget->leNick->setReadOnly (false);
	m_mainWidget->leName->setReadOnly (state);
	m_mainWidget->leJID->setReadOnly (state);
	m_mainWidget->leBirthday->setReadOnly (state);
	m_mainWidget->leTimezone->setReadOnly (state);
	m_mainWidget->wsHomepage->raiseWidget(state ? 0 : 1);

	// home address tab
	m_mainWidget->leHomeStreet->setReadOnly (state);
	m_mainWidget->leHomeExtAddr->setReadOnly (state);
	m_mainWidget->leHomePOBox->setReadOnly (state);
	m_mainWidget->leHomeCity->setReadOnly (state);
	m_mainWidget->leHomePostalCode->setReadOnly (state);
	m_mainWidget->leHomeCountry->setReadOnly (state);
	m_mainWidget->wsHomeEmail->raiseWidget(state ? 0 : 1);

	// work address tab
	m_mainWidget->leWorkStreet->setReadOnly (state);
	m_mainWidget->leWorkExtAddr->setReadOnly (state);
	m_mainWidget->leWorkPOBox->setReadOnly (state);
	m_mainWidget->leWorkCity->setReadOnly (state);
	m_mainWidget->leWorkPostalCode->setReadOnly (state);
	m_mainWidget->leWorkCountry->setReadOnly (state);
	m_mainWidget->wsWorkEmail->raiseWidget(state ? 0 : 1);

	// work information tab
	m_mainWidget->leCompany->setReadOnly (state);
	m_mainWidget->leDepartment->setReadOnly (state);
	m_mainWidget->lePosition->setReadOnly (state);
	m_mainWidget->leRole->setReadOnly (state);

	// phone numbers tab
	m_mainWidget->lePhoneHome->setReadOnly (state);
	m_mainWidget->lePhoneWork->setReadOnly (state);
	m_mainWidget->lePhoneFax->setReadOnly (state);
	m_mainWidget->lePhoneCell->setReadOnly (state);

	// about tab
	m_mainWidget->teAbout->setReadOnly (state);

	// save button
	enableButton(User1, !state);

}

/*
 * Saves a vCard to the server.
 */
void dlgJabberVCard::slotSaveVCard()
{
	XMPP::VCard vCard;
	XMPP::VCard::AddressList addressList;
	XMPP::VCard::EmailList emailList;
	XMPP::VCard::PhoneList phoneList;

	if ( !m_account->isConnected () )
	{
		m_account->errorConnectFirst ();
		return;
	}

	// general tab
	vCard.setNickName (m_mainWidget->leNick->text());
	vCard.setFullName (m_mainWidget->leName->text());
	vCard.setJid (m_mainWidget->leJID->text());
	vCard.setBdayStr (m_mainWidget->leBirthday->text());
	vCard.setTimezone (m_mainWidget->leTimezone->text());
	vCard.setUrl (m_mainWidget->leHomepage->text());

	// home address tab
	XMPP::VCard::Address homeAddress;

	homeAddress.home = true;
	homeAddress.street = m_mainWidget->leHomeStreet->text();
	homeAddress.extaddr = m_mainWidget->leHomeExtAddr->text();
	homeAddress.pobox = m_mainWidget->leHomePOBox->text();
	homeAddress.locality = m_mainWidget->leHomeCity->text();
	homeAddress.pcode = m_mainWidget->leHomePostalCode->text();
	homeAddress.country = m_mainWidget->leHomeCountry->text();

	// work address tab
	XMPP::VCard::Address workAddress;

	workAddress.work = true;
	workAddress.street = m_mainWidget->leWorkStreet->text();
	workAddress.extaddr = m_mainWidget->leWorkExtAddr->text();
	workAddress.pobox = m_mainWidget->leWorkPOBox->text();
	workAddress.locality = m_mainWidget->leWorkCity->text();
	workAddress.pcode = m_mainWidget->leWorkPostalCode->text();
	workAddress.country = m_mainWidget->leWorkCountry->text();

	addressList.append(homeAddress);
	addressList.append(workAddress);

	vCard.setAddressList(addressList);

	// home email
	XMPP::VCard::Email homeEmail;

	homeEmail.home = true;
	homeEmail.userid = m_mainWidget->leHomeEmail->text();

	// work email
	XMPP::VCard::Email workEmail;

	workEmail.home = true;
	workEmail.userid = m_mainWidget->leWorkEmail->text();

	emailList.append(homeEmail);
	emailList.append(workEmail);

	vCard.setEmailList(emailList);

	// work information tab
	XMPP::VCard::Org org;
	org.name = m_mainWidget->leCompany->text();
	org.unit = QStringList::split(",", m_mainWidget->leDepartment->text());
	vCard.setOrg(org);
	vCard.setTitle (m_mainWidget->lePosition->text());
	vCard.setRole (m_mainWidget->leRole->text());

	// phone numbers tab
	XMPP::VCard::Phone phoneHome;
	phoneHome.home = true;
	phoneHome.number = m_mainWidget->lePhoneHome->text();

	XMPP::VCard::Phone phoneWork;
	phoneWork.work = true;
	phoneWork.number = m_mainWidget->lePhoneWork->text();

	XMPP::VCard::Phone phoneFax;
	phoneFax.fax = true;
	phoneFax.number = m_mainWidget->lePhoneFax->text();

	XMPP::VCard::Phone phoneCell;
	phoneCell.cell = true;
	phoneCell.number = m_mainWidget->lePhoneCell->text();

	phoneList.append(phoneHome);
	phoneList.append(phoneWork);
	phoneList.append(phoneFax);
	phoneList.append(phoneCell);

	vCard.setPhoneList(phoneList);

	// about tab
	vCard.setDesc(m_mainWidget->teAbout->text());

	vCard.setVersion("3.0");
	vCard.setProdId("Kopete");

	XMPP::JT_VCard *task = new XMPP::JT_VCard (m_account->client()->rootTask ());
	// signal to ourselves when the vCard data arrived
	QObject::connect (task, SIGNAL (finished ()), this, SLOT (slotSentVCard ()));
	task->set (vCard);
	task->go (true);

}

void dlgJabberVCard::slotSentVCard()
{
	XMPP::JT_VCard * vCard = (XMPP::JT_VCard *) sender ();

	if (!vCard->success())
	{
		// unsuccessful, or incomplete
		KMessageBox::error (this, i18n("Unable to store vCard for %1").arg (vCard->jid ().userHost ()));
		return;
	}

}

void dlgJabberVCard::slotOpenURL(const QString &url)
{

	if ( !url.isEmpty () || (url == QString::fromLatin1("mailto:") ) )
		new KRun(KURL( url ) );

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
