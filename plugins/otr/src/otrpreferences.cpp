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

#include <qlayout.h>
#include <qlabel.h>
#include <qmap.h>
//#include <q3ptrlist.h>
#include <qcombobox.h>
#include <qstringlist.h>
//#include <q3table.h>
//#include <q3paintdevicemetrics.h>
//#include <q3vbox.h>
#include <qradiobutton.h>
#include <qtabwidget.h>
//Added by qt3to4:
//#include <Q3VBoxLayout>
//#include <Q3ValueList>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kurlrequester.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kapplication.h>
//#include <kanimwidget.h>
#include <kpassivepopup.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>

#include <kopeteaccountmanager.h>
#include <kopeteaccount.h>
#include <kopeteprotocol.h>

//#include "ui_otrprefs.h"
#include "otrpreferences.h"
#include "otrplugin.h"
#include "kopete_otr.h"
#include "otrlconfinterface.h"

/**
  * @author Michael Zanetti
  */

K_PLUGIN_FACTORY(OTRPreferencesFactory, registerPlugin<OTRPreferences>();)
K_EXPORT_PLUGIN(OTRPreferencesFactory ( "kcm_kopete_otr" ))

OTRPreferences::OTRPreferences(QWidget *parent, const QVariantList &args)
		: KCModule(OTRPreferencesFactory::componentData(), parent, args)
{
//	( new Q3VBoxLayout( this ) )->setAutoAdd( true );
	QWidget *widget = new QWidget(this);
	preferencesDialog = new Ui::OTRPrefsUI();
	preferencesDialog->setupUi( widget );
	
	addConfig( KopeteOtrKcfg::self(), widget );
	KopeteOtrKcfg::self()->readConfig();
	load();


	otrlConfInterface = new OtrlConfInterface( widget );

	connect( preferencesDialog->btGenFingerprint, SIGNAL(clicked()), SLOT(generateFingerprint()));
	connect( preferencesDialog->cbKeys, SIGNAL(activated(int)), SLOT(showPrivFingerprint(int)));
	connect( preferencesDialog->btVerify, SIGNAL(clicked()), SLOT(verifyFingerprint()));
	connect( preferencesDialog->twSettings, SIGNAL(currentChanged(QWidget *)), SLOT(fillFingerprints()));
	connect( preferencesDialog->tbFingerprints, SIGNAL(currentChanged(int, int)), SLOT(updateButtons(int, int)));
	connect( preferencesDialog->btForget, SIGNAL( clicked() ), SLOT( forgetFingerprint() ) );

	int index = 0;
	int accountnr = 0;
//	Q3PtrList<Kopete::Account> accounts = Kopete::AccountManager::self()->accounts();
//	if( !accounts.isEmpty() ){
//		for ( Q3PtrListIterator<Kopete::Account> it( accounts );
//			Kopete::Account *account = it.current();
//			++it ){
//				if (  account->protocol()->pluginId() != "IRCProtocol" ){
//					preferencesDialog->cbKeys->insertItem(account->accountId() + " (" + account->protocol()->displayName() + ")");
//					privKeys.insert(index++, accountnr);
//				}
//				accountnr++;
//		}
//	}
	showPrivFingerprint( preferencesDialog->cbKeys->currentIndex() );
	
	preferencesDialog->tbFingerprints->setColumnWidth( 0, 200 );
	preferencesDialog->tbFingerprints->setColumnWidth( 1, 80 );
	preferencesDialog->tbFingerprints->setColumnWidth( 2, 60 );
	preferencesDialog->tbFingerprints->setColumnWidth( 3, 400 );
	preferencesDialog->tbFingerprints->setColumnWidth( 4, 200 );

}

OTRPreferences::~OTRPreferences(){
}

void OTRPreferences::generateFingerprint()
{
//	Q3PtrList<Kopete::Account> accounts = Kopete::AccountManager::self()->accounts();

//	if( (accounts.isEmpty())){
//		return;
//	}

//	Kopete::Account *account = accounts.at( preferencesDialog->cbKeys->currentItem() );

//	if ((otrlConfInterface->hasPrivFingerprint( account->accountId(), account->protocol()->displayName() ) ) && (KMessageBox::questionYesNo(this, i18n("Selected account already has a key. Do you want to create a new one?"), i18n("Overwrite key?")) !=3)) return;
	
//	otrlConfInterface->generateNewPrivKey( account->accountId(), account->protocol()->displayName() );
	showPrivFingerprint( preferencesDialog->cbKeys->currentIndex() );	
}

void OTRPreferences::showPrivFingerprint( int accountnr )
{
//	Q3PtrList<Kopete::Account> accounts = Kopete::AccountManager::self()->accounts();
//	if( !accounts.isEmpty() ){
//		Kopete::Account *account = accounts.at(privKeys[accountnr]);
//		preferencesDialog->tlFingerprint->setText( otrlConfInterface->getPrivFingerprint( account->accountId(), account->protocol()->displayName() ) );
//	}
}

void OTRPreferences::fillFingerprints(){
	//Q3Table *fingerprintsTable = preferencesDialog->tbFingerprints;
	preferencesDialog->tbFingerprints->setRowCount(0);
//	Q3ValueList<QString[5]> list = otrlConfInterface->readAllFingerprints();
//	Q3ValueList<QString[5]>::iterator it;
	int j = 0;
//	for( it = list.begin(); it != list.end(); ++it ){
//		preferencesDialog->tbFingerprints->setNumRows( preferencesDialog->tbFingerprints->numRows() +1 );
 //		(*it)[0] = OtrlChatInterface::self()->formatContact((*it)[0]);
//		for( int i = 0; i < 5; i++ ){ 	
//			//preferencesDialog->tbFingerprints->setText(j, i, (*it)[i] );
//			preferencesDialog->tbFingerprints->setItem(j,i, new QAlignTableItem(preferencesDialog->tbFingerprints, Q3TableItem::Never,(*it)[i],Qt::AlignLeft));
 //		}
//		j++;
//	}
	updateButtons( preferencesDialog->tbFingerprints->currentRow(), preferencesDialog->tbFingerprints->currentColumn() );
}

void OTRPreferences::verifyFingerprint(){

	int doVerify = KMessageBox::questionYesNo( 
		this, 
		i18n("Please contact %1 via another secure way and verify that the following Fingerprint is correct:").arg(preferencesDialog->tbFingerprints->takeItem( preferencesDialog->tbFingerprints->currentRow(), 0 )->text()) + "\n\n" + preferencesDialog->tbFingerprints->takeItem( preferencesDialog->tbFingerprints->currentRow(), 3 )->text() + "\n\n" + i18n("Are you sure you want to trust this fingerprint?"), i18n("Verify fingerprint")  );
	

	if( doVerify == KMessageBox::Yes ){
		otrlConfInterface->verifyFingerprint( preferencesDialog->tbFingerprints->takeItem( preferencesDialog->tbFingerprints->currentRow(), 3 )->text(), true );
	} else {
		otrlConfInterface->verifyFingerprint( preferencesDialog->tbFingerprints->takeItem( preferencesDialog->tbFingerprints->currentRow(), 3 )->text(), false );
	}
	fillFingerprints();
}

void OTRPreferences::updateButtons( int row, int col ){
	if( row != -1 ){
		if( !otrlConfInterface->isEncrypted( preferencesDialog->tbFingerprints->takeItem( row, 3 )->text() ) ){
			preferencesDialog->btForget->setEnabled( true );
		} else {
		preferencesDialog->btForget->setEnabled( false );			
		}
	} else {
		preferencesDialog->btForget->setEnabled( false );
	}
}

void OTRPreferences::forgetFingerprint(){
	if( !otrlConfInterface->isEncrypted( preferencesDialog->tbFingerprints->takeItem( preferencesDialog->tbFingerprints->currentRow(), 3 )->text() ) ){
		otrlConfInterface->forgetFingerprint( preferencesDialog->tbFingerprints->takeItem( preferencesDialog->tbFingerprints->currentRow(), 3 )->text() );
		fillFingerprints();
	} else {
		updateButtons( preferencesDialog->tbFingerprints->currentRow(), preferencesDialog->tbFingerprints->currentColumn() );
	}
}

//QAlignTableItem :: QAlignTableItem( Q3Table *table, EditType editType, const QString& text, int alignment )
//	 : Q3TableItem( table, editType, text ) {
//	align = alignment;
//}


#include "otrpreferences.moc"

// vim: set noet ts=4 sts=4 sw=4:
