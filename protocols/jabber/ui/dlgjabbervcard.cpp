
/***************************************************************************
                          dlgjabbervcard.cpp  -  vCard dialog
                             -------------------
    begin                : Thu Aug 08 2002
    copyright            : (C) 2002-2003 by Till Gerken <till@tantalo.net>
                           (C) 2005      by Michaël Larouche <larouche@kde.org>
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

#include <qregexp.h>
#include <qbuffer.h>

#include <qapplication.h>

// KDE includes
#include <kdebug.h>
#include <kpushbutton.h>
#include <klineedit.h>
#include <klocale.h>
#include <kurllabel.h>
#include <kmessagebox.h>
#include <krun.h>
#include <kio/netaccess.h>
#include <kfiledialog.h>
#include <kstandarddirs.h>

// libiris(XMPP backend) includes
#include "im.h"
#include "xmpp.h"
#include "xmpp_tasks.h"

// Kopete includes
#include "jabberprotocol.h"
#include "jabbercontact.h"
#include "jabberaccount.h"
#include "jabbercontactpool.h"
#include "jabberbasecontact.h"
#include "jabberclient.h"
#include "ui_dlgvcard.h"
#include "avatardialog.h"

/*
 *  Constructs a dlgJabberVCard which is a child of 'parent', with the
 *  name 'name'
 *
 */
dlgJabberVCard::dlgJabberVCard (JabberAccount *account, JabberBaseContact *contact, QWidget * parent)
	: KDialog(parent)
{

	setCaption( i18n("Jabber vCard") );
	setButtons( KDialog::Close | KDialog::User1 | KDialog::User2 );
	setButtonGuiItem( KDialog::User1, KGuiItem( i18n("&Save User Info") ) );
	setButtonGuiItem( KDialog::User2, KGuiItem( i18n("Fetch vCard") ) );
	setDefaultButton( KDialog::Close );

	m_account = account;
	m_contact = contact;

	QWidget* w = new QWidget(this);
	m_mainWidget = new Ui::dlgVCard;
	m_mainWidget->setupUi(w);
	setMainWidget(w);

	connect (this, SIGNAL (user1Clicked()), this, SLOT (slotSaveVCard()));
	connect (this, SIGNAL(user2Clicked()), this, SLOT (slotGetVCard()));

	connect (m_mainWidget->btnSelectPhoto, SIGNAL (clicked()), this, SLOT (slotSelectPhoto()));
	connect (m_mainWidget->btnClearPhoto, SIGNAL (clicked()), this, SLOT (slotClearPhoto()));
	connect (m_mainWidget->urlHomeEmail, SIGNAL (leftClickedUrl(QString)), this, SLOT (slotOpenURL(QString)));
	connect (m_mainWidget->urlWorkEmail, SIGNAL (leftClickedUrl(QString)), this, SLOT (slotOpenURL(QString)));
	connect (m_mainWidget->urlHomepage, SIGNAL (leftClickedUrl(QString)), this, SLOT (slotOpenURL(QString)));

	assignContactProperties();

	show ();
	raise ();

	slotGetVCard();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dlgJabberVCard::~dlgJabberVCard ()
{
	// no need to delete child widgets, Qt does it all for us
	delete m_mainWidget;
}

/*
 * Activated when the close button gets pressed. Deletes the dialog.
 */
void dlgJabberVCard::slotClose()
{
	kDebug(JABBER_DEBUG_GLOBAL) << "Deleting dialog.";
	deleteLater();
}

/*
 * Assign the contact properties to this dialog
 */
void dlgJabberVCard::assignContactProperties ()
{
	// general tab
	m_mainWidget->leNick->setText (m_contact->property(m_account->protocol()->propNickName).value().toString());
	m_mainWidget->leName->setText (m_contact->property(m_account->protocol()->propFullName).value().toString());
	// Guess the JID from the Kopete::Contact if the propJid is empty.
	if( m_contact->property( m_account->protocol()->propJid ).value().toString().isEmpty() )
		m_mainWidget->leJID->setText (m_contact->rosterItem().jid().full());
	else
		m_mainWidget->leJID->setText (m_contact->property(m_account->protocol()->propJid).value().toString());
	m_mainWidget->leBirthday->setText (m_contact->property(m_account->protocol()->propBirthday).value().toString());
	m_mainWidget->leTimezone->setText (m_contact->property(m_account->protocol()->propTimezone).value().toString());

	QString homepage = m_contact->property(m_account->protocol()->propHomepage).value().toString();
	m_mainWidget->leHomepage->setText (homepage);
	m_mainWidget->urlHomepage->setText (homepage);
	m_mainWidget->urlHomepage->setUrl (homepage);
	m_mainWidget->urlHomepage->setUseCursor ( !homepage.isEmpty () );

	// Set photo
	m_photoPath = m_contact->property(m_account->protocol()->propPhoto).value().toString();
	if( !m_photoPath.isEmpty() )
	{
		m_mainWidget->lblPhoto->setPixmap( QPixmap(m_photoPath) );
	}

	// addresses
	m_mainWidget->leWorkStreet->setText (m_contact->property(m_account->protocol()->propWorkStreet).value().toString());
	m_mainWidget->leWorkExtAddr->setText (m_contact->property(m_account->protocol()->propWorkExtAddr).value().toString());
	m_mainWidget->leWorkPOBox->setText (m_contact->property(m_account->protocol()->propWorkPOBox).value().toString());
	m_mainWidget->leWorkCity->setText (m_contact->property(m_account->protocol()->propWorkCity).value().toString());
	m_mainWidget->leWorkPostalCode->setText (m_contact->property(m_account->protocol()->propWorkPostalCode).value().toString());
	m_mainWidget->leWorkCountry->setText (m_contact->property(m_account->protocol()->propWorkCountry).value().toString());

	m_mainWidget->leHomeStreet->setText (m_contact->property(m_account->protocol()->propHomeStreet).value().toString());
	m_mainWidget->leHomeExtAddr->setText (m_contact->property(m_account->protocol()->propHomeExtAddr).value().toString());
	m_mainWidget->leHomePOBox->setText (m_contact->property(m_account->protocol()->propHomePOBox).value().toString());
	m_mainWidget->leHomeCity->setText (m_contact->property(m_account->protocol()->propHomeCity).value().toString());
	m_mainWidget->leHomePostalCode->setText (m_contact->property(m_account->protocol()->propHomePostalCode).value().toString());
	m_mainWidget->leHomeCountry->setText (m_contact->property(m_account->protocol()->propHomeCountry).value().toString());

	// email
	m_mainWidget->urlWorkEmail->setUseCursor ( false );
	m_mainWidget->urlHomeEmail->setUseCursor ( false );

	QString workEmail = m_contact->property(m_account->protocol()->propWorkEmailAddress).value().toString();
	QString homeEmail = m_contact->property(m_account->protocol()->propEmailAddress).value().toString();
	m_mainWidget->leWorkEmail->setText (workEmail);
	m_mainWidget->urlWorkEmail->setText (workEmail);
	m_mainWidget->urlWorkEmail->setUrl ("mailto:" + workEmail);
	bool enableMail=!workEmail.trimmed().isEmpty ();
	m_mainWidget->urlWorkEmail->setUseCursor ( enableMail );
	m_mainWidget->urlWorkEmail->setEnabled ( enableMail ); 
		
	m_mainWidget->leHomeEmail->setText (homeEmail);
	m_mainWidget->urlHomeEmail->setText (homeEmail);
	enableMail=!homeEmail.trimmed().isEmpty ();
	m_mainWidget->urlHomeEmail->setUrl ("mailto:" + homeEmail);
	m_mainWidget->urlHomeEmail->setUseCursor ( enableMail );
	m_mainWidget->urlHomeEmail->setEnabled ( enableMail );

	// work information tab
	m_mainWidget->leCompany->setText (m_contact->property(m_account->protocol()->propCompanyName).value().toString());
	m_mainWidget->leDepartment->setText (m_contact->property(m_account->protocol()->propCompanyDepartement).value().toString());
	m_mainWidget->lePosition->setText (m_contact->property(m_account->protocol()->propCompanyPosition).value().toString());
	m_mainWidget->leRole->setText (m_contact->property(m_account->protocol()->propCompanyRole).value().toString());

	// phone numbers tab
	m_mainWidget->lePhoneFax->setText(m_contact->property(m_account->protocol()->propPhoneFax).value().toString());
	m_mainWidget->lePhoneWork->setText(m_contact->property(m_account->protocol()->propWorkPhone).value().toString());
	m_mainWidget->lePhoneCell->setText(m_contact->property(m_account->protocol()->propPrivateMobilePhone).value().toString());
	m_mainWidget->lePhoneHome->setText(m_contact->property(m_account->protocol()->propPrivatePhone).value().toString());

	// about tab
	m_mainWidget->teAbout->setText (m_contact->property(m_account->protocol()->propAbout).value().toString());

	if(m_account->myself() == m_contact)
		setReadOnly (false);
	else
		setReadOnly (true);
}

void dlgJabberVCard::setReadOnly (bool state)
{
	// general tab
	m_mainWidget->leNick->setReadOnly (state);
	m_mainWidget->leName->setReadOnly (state);
	m_mainWidget->leJID->setReadOnly (state);
	m_mainWidget->leBirthday->setReadOnly (state);
	m_mainWidget->leTimezone->setReadOnly (state);
	m_mainWidget->wsHomepage->setCurrentIndex(state ? 0 : 1);
	// Disable photo buttons when read only
	m_mainWidget->btnSelectPhoto->setEnabled(!state);
	m_mainWidget->btnClearPhoto->setEnabled(!state);

	// home address tab
	m_mainWidget->leHomeStreet->setReadOnly (state);
	m_mainWidget->leHomeExtAddr->setReadOnly (state);
	m_mainWidget->leHomePOBox->setReadOnly (state);
	m_mainWidget->leHomeCity->setReadOnly (state);
	m_mainWidget->leHomePostalCode->setReadOnly (state);
	m_mainWidget->leHomeCountry->setReadOnly (state);
	m_mainWidget->wsHomeEmail->setCurrentIndex(state ? 0 : 1);

	// work address tab
	m_mainWidget->leWorkStreet->setReadOnly (state);
	m_mainWidget->leWorkExtAddr->setReadOnly (state);
	m_mainWidget->leWorkPOBox->setReadOnly (state);
	m_mainWidget->leWorkCity->setReadOnly (state);
	m_mainWidget->leWorkPostalCode->setReadOnly (state);
	m_mainWidget->leWorkCountry->setReadOnly (state);
	m_mainWidget->wsWorkEmail->setCurrentIndex(state ? 0 : 1);

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

void dlgJabberVCard::setEnabled(bool state)
{
	// general tab
	m_mainWidget->leNick->setEnabled (state);
	m_mainWidget->leName->setEnabled (state);
	m_mainWidget->leJID->setEnabled (state);
	m_mainWidget->leBirthday->setEnabled (state);
	m_mainWidget->leTimezone->setEnabled (state);
	m_mainWidget->wsHomepage->setCurrentIndex(state ? 1 : 0);
	// Disable photo buttons when read only
	m_mainWidget->btnSelectPhoto->setEnabled(state);
	m_mainWidget->btnClearPhoto->setEnabled(state);

	// home address tab
	m_mainWidget->leHomeStreet->setEnabled (state);
	m_mainWidget->leHomeExtAddr->setEnabled (state);
	m_mainWidget->leHomePOBox->setEnabled (state);
	m_mainWidget->leHomeCity->setEnabled (state);
	m_mainWidget->leHomePostalCode->setEnabled (state);
	m_mainWidget->leHomeCountry->setEnabled (state);
	m_mainWidget->wsHomeEmail->setCurrentIndex(state ? 0 : 1);

	// work address tab
	m_mainWidget->leWorkStreet->setEnabled (state);
	m_mainWidget->leWorkExtAddr->setEnabled (state);
	m_mainWidget->leWorkPOBox->setEnabled (state);
	m_mainWidget->leWorkCity->setEnabled (state);
	m_mainWidget->leWorkPostalCode->setEnabled (state);
	m_mainWidget->leWorkCountry->setEnabled (state);
	m_mainWidget->wsWorkEmail->setCurrentIndex(state ? 0 : 1);

	// work information tab
	m_mainWidget->leCompany->setEnabled (state);
	m_mainWidget->leDepartment->setEnabled (state);
	m_mainWidget->lePosition->setEnabled (state);
	m_mainWidget->leRole->setEnabled (state);

	// phone numbers tab
	m_mainWidget->lePhoneHome->setEnabled (state);
	m_mainWidget->lePhoneWork->setEnabled (state);
	m_mainWidget->lePhoneFax->setEnabled (state);
	m_mainWidget->lePhoneCell->setEnabled (state);

	// about tab
	m_mainWidget->teAbout->setEnabled (state);

	// save button
	enableButton(User1, state);
	enableButton(User2, state);
}

/*
 * Saves a vCard to the contact properties
 */
void dlgJabberVCard::slotSaveVCard()
{
	setEnabled(false);
	m_mainWidget->lblStatus->setText( i18n("Saving vCard to server...") );

	XMPP::VCard vCard;
	XMPP::VCard::AddressList addressList;
	XMPP::VCard::EmailList emailList;
	XMPP::VCard::PhoneList phoneList;

	// General information
	vCard.setNickName( m_mainWidget->leNick->text() );
	vCard.setFullName( m_mainWidget->leName->text() );
	vCard.setJid( m_mainWidget->leJID->text() );
	vCard.setBdayStr( m_mainWidget->leBirthday->text() );
	vCard.setTimezone( m_mainWidget->leTimezone->text() );
	vCard.setUrl( m_mainWidget->leHomepage->text() );

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

	workEmail.work = true;
	workEmail.userid = m_mainWidget->leWorkEmail->text();

	emailList.append(homeEmail);
	emailList.append(workEmail);

	vCard.setEmailList(emailList);

	// work information tab
	XMPP::VCard::Org org;
	org.name = m_mainWidget->leCompany->text();
	org.unit = m_mainWidget->leDepartment->text().split(',');
	vCard.setOrg(org);
	vCard.setTitle( m_mainWidget->lePosition->text() );
	vCard.setRole( m_mainWidget->leRole->text() );

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
	vCard.setDesc( m_mainWidget->teAbout->toPlainText() );

	// Set contact photo as a binary value (if he has set a photo)
	if( !m_photoPath.isEmpty() )
	{
		QString photoPath = m_photoPath;
		QImage image( photoPath );
		QByteArray ba;
		QBuffer buffer( &ba );
		buffer.open( QIODevice::WriteOnly );
		image.save( &buffer, "PNG" );
		vCard.setPhoto( ba );
	}

	vCard.setVersion("3.0");
	vCard.setProdId("Kopete");

	XMPP::JT_VCard *task = new XMPP::JT_VCard( m_account->client()->rootTask() );
	// signal to ourselves when the vCard data arrived
	QObject::connect(task, SIGNAL(finished()), this, SLOT(slotVCardSaved()));
	task->set(m_contact->rosterItem().jid(), vCard);
	task->go(true);
}

void dlgJabberVCard::slotVCardSaved()
{
	XMPP::JT_VCard *vCard = (XMPP::JT_VCard*)sender();
	
	if( vCard->success() )
	{
		m_mainWidget->lblStatus->setText( i18n("vCard successfully saved.") );
		m_contact->setPropertiesFromVCard( vCard->vcard() );
	}
	else
	{
		m_mainWidget->lblStatus->setText( i18n("Error: Unable to save vCard.") );
	}

	setEnabled(true);
}

void dlgJabberVCard::slotGetVCard()
{
	m_mainWidget->lblStatus->setText( i18n("Fetching contact vCard...") );

	setReadOnly(true);
	setEnabled(false);

	XMPP::JT_VCard *task = new XMPP::JT_VCard ( m_account->client()->rootTask() );
	// signal to ourselves when the vCard data arrived
	QObject::connect( task, SIGNAL (finished()), this, SLOT (slotGotVCard()) );
	task->get ( m_contact->rosterItem().jid().full() );
	task->go ( true );	
}

void dlgJabberVCard::slotGotVCard()
{
	XMPP::JT_VCard * vCard = (XMPP::JT_VCard *) sender ();
	
	if( vCard->success() )
	{
		m_contact->setPropertiesFromVCard( vCard->vcard() );
		setEnabled( true );

		assignContactProperties();		

		m_mainWidget->lblStatus->setText( i18n("vCard successfully retrieved.") );
	}
	else
	{
		// XMPP::Task::ErrDisc + 1: error code set when a valid vCard reply is sent,
		// but an empty vCard was sent
		if ( vCard->statusCode() == ( XMPP::Task::ErrDisc + 1 ) )
			m_mainWidget->lblStatus->setText( i18n("No vCard available.") );
		else
			m_mainWidget->lblStatus->setText( i18n("Error: vCard could not be fetched correctly.\nCheck connectivity with the Jabber server.") );
		//it is maybe possible to anyway edit our own vCard (if it is new
		if(m_account->myself() == m_contact)
			setEnabled( true );
	}
}

void dlgJabberVCard::slotSelectPhoto()
{
	bool ok = false;
	QString path = Kopete::UI::AvatarDialog::getAvatar(this, m_photoPath, &ok);
	if ( !ok )
	{
		return;
	}

	QPixmap pix( path );

	if( !pix.isNull() ) 
	{
		m_photoPath = path;
		m_mainWidget->lblPhoto->setPixmap( pix );
	}
	else
	{
		KMessageBox::queuedMessageBox( this, KMessageBox::Sorry, i18n( "<qt>An error occurred when trying to change the photo.<br />"
			"Make sure that you have selected a valid image file</qt>" ) );
		m_photoPath.clear();
	}
}

void dlgJabberVCard::slotClearPhoto()
{
	m_mainWidget->lblPhoto->setPixmap( QPixmap() );
	m_photoPath.clear();
}

void dlgJabberVCard::slotOpenURL(const QString &url)
{
	if ( !url.isEmpty () || (url == QString::fromLatin1("mailto:") ) )
		new KRun( KUrl( url ), this );
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
