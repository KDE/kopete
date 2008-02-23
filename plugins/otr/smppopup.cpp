/***************************************************************************
 *   Copyright (C) 2007 by Michael Zanetti 
 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/**
  * @author Michael Zanetti
  */

#include "qlayout.h"

#include "kurl.h"
#include "krun.h"

#include "kopetecontact.h"

#include "smppopup.h"
#include "verifydialog.h"

SMPPopup::SMPPopup( QWidget *parent, ConnContext *context, Kopete::ChatSession *session, bool initiate  ):KDialog( parent ){
	this->context = context;
	this->session = session;
	this->initiate = initiate;

	QWidget *widget = new QWidget( this );
	ui.setupUi( widget );
	setMainWidget( widget );
	setCaption( i18n( "Enter authentication secret" ) );
	setButtons( KDialog::None );

	ui.lMessage->setText( i18n( "Please enter the secret passphrase to authenticate %1", OtrlChatInterface::self()->formatContact( session->members().first()->contactId() ) ) );

	ui.lIcon->setPixmap( KIcon( "application-pgp-signature" ).pixmap( 48, 48 ) );

	connect( ui.pbOk, SIGNAL( clicked(bool) ), this, SLOT( respondSMP() ) );
	connect( ui.pbCancel, SIGNAL( clicked(bool) ), this, SLOT( cancelSMP() ) );
	connect( ui.pbHelp, SIGNAL( clicked(bool) ), this, SLOT( openHelp() ) );
	connect( ui.pbManualAuth, SIGNAL( clicked(bool) ), this, SLOT( manualAuth() ) );
}

SMPPopup::~SMPPopup(){
}

void SMPPopup::cancelSMP()
{
	OtrlChatInterface::self()->abortSMP( context, session );
	this->close();
}

void SMPPopup::respondSMP()
{
	OtrlChatInterface::self()->respondSMP( context, session, ui.lePassphrase->text(), initiate );
	this->close();
}

void SMPPopup::openHelp()
{
	new KRun(KUrl("http://www.cypherpunks.ca/otr/help/authenticate.php?lang=en"), 0, 0, false, true);
}

void SMPPopup::manualAuth(){
	VerifyDialog *vfDialog = new VerifyDialog(this, session);
	vfDialog->show();
	this->close();
}
#include "smppopup.moc"