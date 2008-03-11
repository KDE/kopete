/*************************************************************************
 * Copyright <2007>  <Michael Zanetti> <michael_zanetti@gmx.net>         *
 *                                                                       *
 * This program is free software; you can redistribute it and/or         *
 * modify it under the terms of the GNU General Public License as        *
 * published by the Free Software Foundation; either version 2 of        *
 * the License or (at your option) version 3 or any later version        *
 * accepted by the membership of KDE e.V. (or its successor approved     *
 * by the membership of KDE e.V.), which shall act as a proxy            *
 * defined in Section 14 of version 3 of the license.                    *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/ 

/**
  * @author Michael Zanetti
  */

#include "smppopup.h"
#include "verifydialog.h"

#include <qlayout.h>

#include <kurl.h>
#include <krun.h>
#include <kdebug.h>

#include <kopetecontact.h>

SMPPopup::SMPPopup( QWidget *parent, ConnContext *context, Kopete::ChatSession *session, bool initiate  ):KDialog( parent ){
	this->context = context;
	this->session = session;
	this->initiate = initiate;

	ui.setupUi( mainWidget() );

	setCaption( i18n( "Enter authentication secret" ) );
	setButtons( KDialog::Help | KDialog::Ok | KDialog::Cancel | KDialog::User1 );
	setButtonText( KDialog::User1, i18nc( "@button", "Manual Authentication" ) );

	setHelp("plugins-otr-auth");

	ui.lMessage->setText( i18n( "Please enter the secret passphrase to authenticate %1", OtrlChatInterface::self()->formatContact( session->members().first()->contactId() ) ) );

	ui.lIcon->setPixmap( KIcon( "application-pgp-signature" ).pixmap( 48, 48 ) );

	connect( this, SIGNAL( okClicked() ), this, SLOT( respondSMP() ) );
	connect( this, SIGNAL( cancelClicked() ), this, SLOT( cancelSMP() ) );
	connect( this, SIGNAL( user1Clicked() ), this, SLOT( manualAuth() ) );
}

SMPPopup::~SMPPopup(){
}

void SMPPopup::cancelSMP()
{
	OtrlChatInterface::self()->abortSMP( context, session );
}

void SMPPopup::respondSMP()
{
	OtrlChatInterface::self()->respondSMP( context, session, ui.lePassphrase->text(), initiate );
}

void SMPPopup::manualAuth(){
	VerifyDialog *vfDialog = new VerifyDialog(this, session);
	vfDialog->show();
	this->close();
}

// Overriding closeEvent to prevent cancelling SMP on pressuing "Manual Auth"
void SMPPopup::closeEvent( QCloseEvent *event )
{
	QDialog::closeEvent( event );
}

#include "smppopup.moc"
