
/***************************************************************************
                          dlgqqvcard.cpp  -  vCard dialog
                             -------------------
    begin                : Thu Aug 08 2002
    copyright            : (C) 2002-2003 by Till Gerken <till@tantalo.net>
                           (C) 2005      by Michaël Larouche <michael.larouche@kdemail.net>
						   (C) 2006		 by Hui Jin <angela.jin@gmail.com>
    email                : kopete-devel@kde.org

    Rewritten version from dlgjabbervcard.cpp

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dlgqqvcard.h"

#include <QPixmap>
// KDE includes

// Kopete includes
#include "qqprotocol.h"
#include "qqcontact.h"
#include "qqaccount.h"
#include "ui_qqvcard.h"

/*
 *  Constructs a dlgQQVCard which is a child of 'parent', with the
 *  name 'name'
 *
 */
dlgQQVCard::dlgQQVCard (QQAccount *account, QQContact *contact, QWidget * parent)
	: KDialog(parent)
{

	setCaption( i18n("QQ vCard") );
	setButtons( KDialog::Close | KDialog::User1 | KDialog::User2 );
	setButtonGuiItem( KDialog::User1, i18n("&Save User Info") );
	setButtonGuiItem( KDialog::User2, i18n("Fetch vCard") );
	setDefaultButton( KDialog::Close );

	m_account = account;
	m_contact = contact;

	QWidget* w = new QWidget(this);
	m_mainWidget = new Ui::QQVCard;
	m_mainWidget->setupUi(w);
	setMainWidget(w);

	connect (this, SIGNAL (user1Clicked()), this, SLOT (slotSaveVCard ()));
	connect (this, SIGNAL( user2Clicked()), this, SLOT (slotGetVCard ()));

	/*
	connect (m_mainWidget->urlHomeEmail, SIGNAL (leftClickedURL(const QString &)), this, SLOT (slotOpenURL (const QString &)));
	connect (m_mainWidget->urlHomepage, SIGNAL (leftClickedURL(const QString &)), this, SLOT (slotOpenURL (const QString &)));
	*/

	assignContactProperties();

	show ();
	raise ();

	slotGetVCard();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dlgQQVCard::~dlgQQVCard ()
{
	// no need to delete child widgets, Qt does it all for us
}

/*
 * Activated when the close button gets pressed. Deletes the dialog.
 */
void dlgQQVCard::slotClose()
{
	kDebug(14140) << k_funcinfo << "Deleting dialog." << endl;
	deleteLater();
}

/*
 * Assign the contact properties to this dialog
 */
void dlgQQVCard::assignContactProperties ()
{
	QQProtocol* proto = static_cast<QQProtocol*>(m_account->protocol());
	// general tab
	m_mainWidget->leNick->setText (m_contact->property(proto->propNickName).value().toString());
	m_mainWidget->leName->setText (m_contact->property(proto->propFullName).value().toString());
	// Guess the JID from the Kopete::Contact if the propJid is empty.
	m_mainWidget->leQQId->setText (m_contact->contactId());

	QString homepage = m_contact->property(proto->propHomepage).value().toString();
	m_mainWidget->leHomepage->setText (homepage);
	/*
	m_mainWidget->urlHomepage->setText (homepage);
	m_mainWidget->urlHomepage->setUrl (homepage);
	m_mainWidget->urlHomepage->setUseCursor ( !homepage.isEmpty () );
	*/

	// Set photo
	/*
	m_photoPath = m_contact->property(m_account->protocol()->propPhoto).value().toString();
	if( !m_photoPath.isEmpty() )
	{
		m_mainWidget->lblPhoto->setPixmap( QPixmap(m_photoPath) );
	}
	*/

	// addresses
	m_mainWidget->leStreet->setText (m_contact->property(proto->propStreet).value().toString());
	m_mainWidget->leCity->setText (m_contact->property(proto->propCity).value().toString());
	m_mainWidget->leZipcode->setText (m_contact->property(proto->propZipcode).value().toString());
	m_mainWidget->leCountry->setText (m_contact->property(proto->propCountry).value().toString());

	// email
	/*
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
	*/

	// phone numbers tab

	if(m_account->myself() == m_contact)
		setReadOnly (false);
	else
		setReadOnly (true);
}

void dlgQQVCard::setReadOnly (bool state)
{
	// general tab
	m_mainWidget->leNick->setReadOnly (state);
	m_mainWidget->leName->setReadOnly (state);
	m_mainWidget->leQQId->setReadOnly (state);

	// home address tab
	m_mainWidget->leStreet->setReadOnly (state);
	m_mainWidget->leCity->setReadOnly (state);
	m_mainWidget->leZipcode->setReadOnly (state);
	m_mainWidget->leCountry->setReadOnly (state);

	// phone numbers tab
	m_mainWidget->lePhoneHome->setReadOnly (state);
	m_mainWidget->lePhoneCell->setReadOnly (state);
	m_mainWidget->lePhoneQQ->setReadOnly (state);

	// about tab
	//m_mainWidget->teAbout->setReadOnly (state);

	// save button
	enableButton(User1, !state);
}

void dlgQQVCard::setEnabled(bool state)
{
	// general tab
	m_mainWidget->leNick->setEnabled (state);
	m_mainWidget->leName->setEnabled (state);

	// home address tab
	m_mainWidget->leStreet->setEnabled (state);
	m_mainWidget->leCity->setEnabled (state);
	m_mainWidget->leZipcode->setEnabled (state);
	m_mainWidget->leCountry->setEnabled (state);

	// phone numbers tab
	m_mainWidget->lePhoneHome->setEnabled (state);
	m_mainWidget->lePhoneQQ->setEnabled (state);
	m_mainWidget->lePhoneCell->setEnabled (state);

	// save button
	enableButton(User1, state);
	enableButton(User2, state);
}

/*
 * Saves a vCard to the contact properties
 */
void dlgQQVCard::slotSaveVCard()
{
/*
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
	vCard.setDesc( m_mainWidget->teAbout->text() );

	// Set contact photo as a binary value (if he has set a photo)
	if( !m_photoPath.isEmpty() )
	{
		QString photoPath = m_photoPath;
		QImage image( photoPath );
		QByteArray ba;
		QBuffer buffer( &ba );
		buffer.open( IO_WriteOnly );
		image.save( &buffer, "PNG" );
		vCard.setPhoto( ba );
	}

	vCard.setVersion("3.0");
	vCard.setProdId("Kopete");

	XMPP::JT_VCard *task = new XMPP::JT_VCard( m_account->client()->rootTask() );
	// signal to ourselves when the vCard data arrived
	QObject::connect(task, SIGNAL(finished()), this, SLOT(slotVCardSaved()));
	task->set(vCard);
	task->go(true);
*/
}

void dlgQQVCard::slotVCardSaved()
{
/*
	if( vCard->success() )
	{
		m_mainWidget->lblStatus->setText( i18n("vCard save sucessful.") );
		m_contact->setPropertiesFromVCard( vCard->vcard() );
	}
	else
	{
		m_mainWidget->lblStatus->setText( i18n("Error: Unable to save vCard.") );
	}

	setEnabled(true);
*/
}

void dlgQQVCard::slotGetVCard()
{
	//m_mainWidget->lblStatus->setText( i18n("Fetching contact vCard...") );
/*

	setReadOnly(true);
	setEnabled(false);

	XMPP::JT_VCard *task = new XMPP::JT_VCard ( m_account->client()->rootTask() );
	// signal to ourselves when the vCard data arrived
	QObject::connect( task, SIGNAL ( finished () ), this, SLOT ( slotGotVCard () ) );
	task->get ( m_contact->rosterItem().jid().full() );
	task->go ( true );	
*/
}

void dlgQQVCard::slotGotVCard()
{
/*
	XMPP::JT_VCard * vCard = (XMPP::JT_VCard *) sender ();
	
	if( vCard->success() )
	{
		m_contact->setPropertiesFromVCard( vCard->vcard() );
		setEnabled( true );

		assignContactProperties();		

		m_mainWidget->lblStatus->setText( i18n("vCard fetching Done.") );
	}
	else
	{
		m_mainWidget->lblStatus->setText( i18n("Error: vCard could not be fetched correctly. Check connectivity with the QQ server.") );
		//it is maybe possible to anyway edit our own vCard (if it is new
		if(m_account->myself() == m_contact)
			setEnabled( true );
	}
*/
}

void dlgQQVCard::slotSelectPhoto()
{
/*
	QString path;
	bool remoteFile = false;
	KUrl filePath = KFileDialog::getImageOpenUrl( KUrl(), this, i18n( "QQ Photo" ) );
	if( filePath.isEmpty() )
		return;

	if( !filePath.isLocalFile() ) 
	{
		if( !KIO::NetAccess::download( filePath, path, this ) ) 
		{
			KMessageBox::queuedMessageBox( this, KMessageBox::Sorry, i18n( "Downloading of QQ contact photo failed." ) );
			return;
		}
		remoteFile = true;
	}
	else 
		path = filePath.path();

	QImage img( path );
	img = KPixmapRegionSelectorDialog::getSelectedImage( QPixmap::fromImage(img), 96, 96, this );

	if( !img.isNull() ) 
	{
		if(img.width() > 96 || img.height() > 96)
		{
			// Scale and crop the picture.
			img = img.scaled( 96, 96, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation );
			// crop image if not square
			if(img.width() < img.height()) 
				img = img.copy((img.width()-img.height())/2, 0, 96, 96);
			else if (img.width() > img.height())
				img = img.copy(0, (img.height()-img.width())/2, 96, 96);

		}
		else if (img.width() < 32 || img.height() < 32)
		{
			// Scale and crop the picture.
			img = img.scaled( 32, 32, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
			// crop image if not square
			if(img.width() < img.height())
				img = img.copy((img.width()-img.height())/2, 0, 32, 32);
			else if (img.width() > img.height())
				img = img.copy(0, (img.height()-img.width())/2, 32, 32);
	
		}
		else if (img.width() != img.height())
		{
			if(img.width() < img.height())
				img = img.copy((img.width()-img.height())/2, 0, img.height(), img.height());
			else if (img.width() > img.height())
				img = img.copy(0, (img.height()-img.width())/2, img.height(), img.height());
		}

		m_photoPath = KStandardDirs::locateLocal("appdata", "qqphotos/" + m_contact->rosterItem().jid().full().toLower().replace(QRegExp("[./~]"),"-")  +".png");
		if( img.save(m_photoPath, "PNG") )
		{
			m_mainWidget->lblPhoto->setPixmap( QPixmap::fromImage(img) );
		}
		else
		{
			m_photoPath.clear();
		}
	}
	else
	{
		KMessageBox::queuedMessageBox( this, KMessageBox::Sorry, i18n( "<qt>An error occurred when trying to change the photo.<br>"
			"Make sure that you have selected a correct image file</qt>" ) );
	}
	if( remoteFile )
		KIO::NetAccess::removeTempFile( path );
*/
}

void dlgQQVCard::slotClearPhoto()
{
	//m_mainWidget->lblPhoto->setPixmap( QPixmap() );
	m_photoPath.clear();
}

void dlgQQVCard::slotOpenURL(const QString &url)
{
/*
	if ( !url.isEmpty () || (url == QString::fromLatin1("mailto:") ) )
		new KRun( KUrl( url ), this );
	*/
}

#include "dlgqqvcard.moc"

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: t
 * End:
 */
// vim: set noet ts=4 sts=4 sw=4:
