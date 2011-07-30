
/***************************************************************************
                          dlgqqvcard.cpp  -  vCard dialog
                             -------------------
    begin                : Thu Aug 08 2002
    copyright            : (C) 2002-2003 by Till Gerken <till@tantalo.net>
                           (C) 2005      by Michaël Larouche <larouche@kde.org>
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
#include <kdebug.h>
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
	setButtonGuiItem( KDialog::User1, KGuiItem(i18n("&Save User Info")) );
	setButtonGuiItem( KDialog::User2, KGuiItem(i18n("Fetch vCard")) );
	setDefaultButton( KDialog::Close );

	m_account = account;
	m_contact = contact;

	QWidget* w = new QWidget(this);
	m_mainWidget = new Ui::QQVCard;
	m_mainWidget->setupUi(w);
	setMainWidget(w);
	m_mainWidget->lblStatus->setText( i18n("WARNING: This vCard may be out-of-date.") );

	connect (this, SIGNAL (user1Clicked()), this, SLOT (slotSaveVCard()));
	connect (this, SIGNAL(user2Clicked()), this, SLOT (slotGetVCard()));

	assignContactProperties();

	show ();
	raise ();

	if( m_account->isConnected() )
		slotGetVCard();
	else
	{
		setEnabled(false);
		setReadOnly(true);
	}
}

/*
 *  Destroys the object and frees any allocated resources
 */
dlgQQVCard::~dlgQQVCard ()
{
	// no need to delete child widgets, Qt does it all for us
	delete m_mainWidget;
}

/*
 * Activated when the close button gets pressed. Deletes the dialog.
 */
void dlgQQVCard::slotClose()
{
	kDebug(14140) << "Deleting dialog.";
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
	m_mainWidget->leAge->setEnabled (state);
	m_mainWidget->cbGender->setEnabled (state);
	m_mainWidget->teSignature->setEnabled (state);


	// Contact tab
	m_mainWidget->leStreet->setEnabled (state);
	m_mainWidget->leCity->setEnabled (state);
	m_mainWidget->leState->setEnabled (state);
	m_mainWidget->leZipcode->setEnabled (state);
	m_mainWidget->leCountry->setEnabled (state);

	m_mainWidget->lePhoneHome->setEnabled (state);
	m_mainWidget->lePhoneQQ->setEnabled (state);
	m_mainWidget->lePhoneCell->setEnabled (state);

	// About tab
	m_mainWidget->leOccupation->setEnabled (state);
	m_mainWidget->leGraduate->setEnabled (state);
	m_mainWidget->leEmail->setEnabled (state);
	m_mainWidget->leHomepage->setEnabled (state);
	m_mainWidget->cbZodiac->setEnabled (state);
	m_mainWidget->cbHoroscope->setEnabled (state);
	m_mainWidget->teIntroduction->setEnabled (state);

	// save button
	enableButton(User1, state);
	enableButton(User2, state);
}

/*
 * Saves a vCard to the contact properties
 */
void dlgQQVCard::slotSaveVCard()
{
	setEnabled(false);
	m_mainWidget->lblStatus->setText( i18n("Saving vCard to server...") );
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
	m_mainWidget->lblStatus->setText( i18n("Fetching contact vCard...") );

	setReadOnly(true);
	setEnabled(false);

	QObject::connect( m_contact, SIGNAL(gotVCard()), this, SLOT(slotGotVCard()) ); 
	m_account->getVCard( m_contact );
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
		path = filePath.toLocalFile();

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
	Q_UNUSED(url);
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
