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

#include "kopetecontact.h"

#include "verifydialog.h"
#include "otrlchatinterface.h"

VerifyDialog::VerifyDialog( QWidget *parent, Kopete::ChatSession *session ):KDialog( parent ){

	this->session = session;

	QWidget *widget = new QWidget( this );
	ui.setupUi( widget );
	setMainWidget( widget );
	setCaption( i18n( "Manual Authentication" ) );
	setButtons( KDialog::None );

	ui.tlContact->setText(i18n("Verify fingerprint for %1.",OtrlChatInterface::self()->formatContact(session->members().first()->contactId())));
	ui.tlFingerprint->setText(i18n("The received fingerprint is:\n\n%1.\n\nContact %2 via another secure channel and verify that this fingerprint is correct.",OtrlChatInterface::self()->findActiveFingerprint(session),OtrlChatInterface::self()->formatContact(session->members().first()->contactId())));
	ui.tlVerified->setText(i18n("verified that this is in fact the correct fingerprint for %1.",OtrlChatInterface::self()->formatContact(session->members().first()->contactId())));
	ui.cbVerify->addItem(i18n("I have not"));
	ui.cbVerify->addItem(i18n("I have"));
	if( OtrlChatInterface::self()->isVerified(session)){
		ui.cbVerify->setCurrentIndex(1);
	} else {
		ui.cbVerify->setCurrentIndex(0);
	}

	connect( ui.cbVerify, SIGNAL( currentIndexChanged( int ) ), this, SLOT( cbChanged( int ) ) );
	connect (ui.pbOk, SIGNAL( clicked( bool ) ), this, SLOT( delayedDestruct() ) );

}

VerifyDialog::~VerifyDialog(){
}

void VerifyDialog::cbChanged( int index ){
	if( index == 0 ){
		OtrlChatInterface::self()->setTrust(session, false);
	} else {
		OtrlChatInterface::self()->setTrust(session, true);
	}
	OtrlChatInterface::self()->emitGoneSecure( session, OtrlChatInterface::self()->privState( session ) );
}

#include "verifydialog.moc"
