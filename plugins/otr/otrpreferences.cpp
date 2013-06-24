/*************************************************************************
 * Copyright <2007 - 2013>  <Michael Zanetti> <mzanetti@kde.org>         *
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

#include "otrpreferences.h"
#include "kopete_otr.h"
#include "otrlconfinterface.h"
#include "otrlchatinterface.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qmap.h>
#include <qcombobox.h>
#include <qstringlist.h>
#include <qtablewidget.h>
#include <qradiobutton.h>
#include <qtabwidget.h>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kurlrequester.h>
#include <kmessagebox.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kpassivepopup.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>

#include <kopeteaccountmanager.h>
#include <kopeteaccount.h>
#include <kopeteprotocol.h>

/**
  * @author Michael Zanetti
  */

K_PLUGIN_FACTORY(OTRPreferencesFactory, registerPlugin<OTRPreferences>();)
K_EXPORT_PLUGIN(OTRPreferencesFactory ( "kcm_kopete_otr" ))

OTRPreferences::OTRPreferences(QWidget *parent, const QVariantList &args)
		: KCModule(OTRPreferencesFactory::componentData(), parent, args)
{
	QVBoxLayout *layout = new QVBoxLayout( this ) ;
	QWidget *widget = new QWidget(this);
	preferencesDialog = new Ui::OTRPrefsUI();
	preferencesDialog->setupUi( widget );
	layout->addWidget( widget );
	
	addConfig( KopeteOtrKcfg::self(), widget );
	KopeteOtrKcfg::self()->readConfig();

	otrlConfInterface = new OtrlConfInterface( widget );

	connect( preferencesDialog->btGenFingerprint, SIGNAL(clicked()), SLOT(generateFingerprint()));
	connect( preferencesDialog->cbKeys, SIGNAL(activated(int)), SLOT(showPrivFingerprint(int)));
	connect( preferencesDialog->btVerify, SIGNAL(clicked()), SLOT(verifyFingerprint()));
	connect( preferencesDialog->twSettings, SIGNAL(currentChanged(QWidget*)), SLOT(fillFingerprints()));
	connect( preferencesDialog->tbFingerprints, SIGNAL(currentCellChanged(int,int,int,int)), SLOT(updateButtons(int,int,int,int)));
	connect( preferencesDialog->btForget, SIGNAL(clicked()), SLOT(forgetFingerprint()) );
	connect( OtrlChatInterface::self(), SIGNAL(goneSecure(Kopete::ChatSession*,int)), this, SLOT(fillFingerprints()) );

	int index = 0;
	int accountnr = 0;
	QList<Kopete::Account*> accounts = Kopete::AccountManager::self()->accounts();
	if( !accounts.isEmpty() ){
		for( int i = 0; i < accounts.size(); i++){
			Kopete::Account *account = accounts[i];
				if (  account->protocol()->pluginId() != "IRCProtocol" ){
					preferencesDialog->cbKeys->insertItem(index, account->accountId() + " (" + account->protocol()->displayName() + ')');
					privKeys.insert(index++, accountnr);
				}
				accountnr++;
		}
	}
	showPrivFingerprint( preferencesDialog->cbKeys->currentIndex() );
	
	preferencesDialog->tbFingerprints->setColumnWidth( 0, 200 );
	preferencesDialog->tbFingerprints->setColumnWidth( 1, 80 );
	preferencesDialog->tbFingerprints->setColumnWidth( 2, 60 );
	preferencesDialog->tbFingerprints->setColumnWidth( 3, 400 );
	preferencesDialog->tbFingerprints->setColumnWidth( 4, 200 );

}

OTRPreferences::~OTRPreferences(){
       delete preferencesDialog;
}

void OTRPreferences::generateFingerprint()
{
	QList<Kopete::Account*> accounts = Kopete::AccountManager::self()->accounts();
 
	if( (accounts.isEmpty())){
		return;
	}

	Kopete::Account *account = accounts.at( preferencesDialog->cbKeys->currentIndex() );

	if ((otrlConfInterface->hasPrivFingerprint( account->accountId(), account->protocol()->displayName() ) ) && (KMessageBox::questionYesNo(this, i18n("Selected account already has a key. Do you want to create a new one?"), i18n("Overwrite key?")) !=3)) return;
	
	otrlConfInterface->generateNewPrivKey( account->accountId(), account->protocol()->displayName() );
	showPrivFingerprint( preferencesDialog->cbKeys->currentIndex() );	
}

void OTRPreferences::showPrivFingerprint( int accountnr )
{
	QList<Kopete::Account*> accounts = Kopete::AccountManager::self()->accounts();
	if( !accounts.isEmpty() ){
		Kopete::Account *account = accounts.at(privKeys[accountnr]);
		preferencesDialog->tlFingerprint->setText( otrlConfInterface->getPrivFingerprint( account->accountId(), account->protocol()->displayName() ) );
	}
}

void OTRPreferences::fillFingerprints(){
	preferencesDialog->tbFingerprints->setRowCount(0);
	const QList<QStringList> list = otrlConfInterface->readAllFingerprints();
	QList<QStringList>::ConstIterator it;
	int j = 0;

//	preferencesDialog->tbFingerprints->setSortingEnabled(false);
	for( it = list.constBegin(); it != list.constEnd(); ++it ){
		preferencesDialog->tbFingerprints->setRowCount( preferencesDialog->tbFingerprints->rowCount() +1 );
 		preferencesDialog->tbFingerprints->setItem(j, 0,new QTableWidgetItem(OtrlChatInterface::self()->formatContact((*it)[j*5])));
		for( int i = 1; i < 5; i++ ){ 	
			preferencesDialog->tbFingerprints->setItem(j, i, new QTableWidgetItem((*it)[j*5 + i]) );
			preferencesDialog->tbFingerprints->item(j,i)->setTextAlignment(Qt::AlignLeft);
		}
		j++;
	}
//	preferencesDialog->tbFingerprints->setSortingEnabled(true);
	updateButtons( preferencesDialog->tbFingerprints->currentRow(), preferencesDialog->tbFingerprints->currentColumn(), 0, 0 );
}

void OTRPreferences::verifyFingerprint(){

	int doVerify = KMessageBox::questionYesNo( 
		this, 
		QString(
			i18n("Please contact %1 via another secure way and verify that the following fingerprint is correct:",preferencesDialog->tbFingerprints->item( preferencesDialog->tbFingerprints->currentRow(), 0 )->text()) + 
			"\n\n" +
			preferencesDialog->tbFingerprints->item( preferencesDialog->tbFingerprints->currentRow(), 3 )->text() + 
			"\n\n" + 
			i18n("Are you sure you want to trust this fingerprint?")), 
		i18n("Verify fingerprint")  ) ;


	if( doVerify == KMessageBox::Yes ){
		otrlConfInterface->verifyFingerprint( preferencesDialog->tbFingerprints->item( preferencesDialog->tbFingerprints->currentRow(), 3 )->text(), true );
	} else {
		otrlConfInterface->verifyFingerprint( preferencesDialog->tbFingerprints->item( preferencesDialog->tbFingerprints->currentRow(), 3 )->text(), false );
	}
	fillFingerprints();
}

void OTRPreferences::updateButtons( int row, int col, int prevRow, int prevCol ){

	Q_UNUSED(col)
	Q_UNUSED(prevRow)
	Q_UNUSED(prevCol)

	if( row != -1 ){
		preferencesDialog->btVerify->setEnabled( true );
		if( !otrlConfInterface->isEncrypted( preferencesDialog->tbFingerprints->item( row, 3 )->text() ) ){
			preferencesDialog->btForget->setEnabled( true );
		} else {
			preferencesDialog->btForget->setEnabled( false );			
		}
	} else {
		preferencesDialog->btForget->setEnabled( false );
		preferencesDialog->btVerify->setEnabled( false );
	}
}

void OTRPreferences::forgetFingerprint(){
	if( !otrlConfInterface->isEncrypted( preferencesDialog->tbFingerprints->item( preferencesDialog->tbFingerprints->currentRow(), 3 )->text() ) ){
		otrlConfInterface->forgetFingerprint( preferencesDialog->tbFingerprints->item( preferencesDialog->tbFingerprints->currentRow(), 3 )->text() );
		fillFingerprints();
	} else {
		updateButtons( preferencesDialog->tbFingerprints->currentRow(), preferencesDialog->tbFingerprints->currentColumn(), 0, 0 );
	}
}


#include "otrpreferences.moc"

// vim: set noet ts=4 sts=4 sw=4:
