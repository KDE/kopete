/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <klistview.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <ktrader.h>
#include <klibloader.h>
#include <qlayout.h>
#include <qptrlist.h>
#include <qcheckbox.h>
#include <qcombobox.h>

#include <ktextedit.h>

#include <ktexteditor/highlightinginterface.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include "kopeteaccountmanager.h"
#include "kopeteaccount.h"

#include "javascriptpreferences.h"
#include "javascriptprefsbase.h"
#include "javascriptconfig.h"

typedef KGenericFactory<JavaScriptPreferences> JavaScriptPreferencesFactory;

class AccountItem : public QListViewItem
{
	public:
		AccountItem( QListView *parent, KopeteAccount *a ) :
		QListViewItem( parent, a->accountId() )
		{
			this->setPixmap( 0, a->accountIcon() );
			account = a;
		}

		KopeteAccount *account;
};

K_EXPORT_COMPONENT_FACTORY( kcm_kopete_javascript, JavaScriptPreferencesFactory( "kcm_kopete_javascript" ) )

JavaScriptPreferences::JavaScriptPreferences( QWidget *parent, const char *, const QStringList &args )
	: KCModule( JavaScriptPreferencesFactory::instance(), parent, args )
{
	config = JavaScriptConfig::instance();

	( new QVBoxLayout( this ) )->setAutoAdd( true );
	preferencesDialog = new JavaScriptPrefsBase( this );

	(new QHBoxLayout( preferencesDialog->scriptFrame ))->setAutoAdd( true );
	editDocument = new KTextEdit( preferencesDialog->scriptFrame );
	/*KTrader::OfferList offers = KTrader::self()->query( "KTextEditor/Document" );
	KService::Ptr service = *offers.begin();
	KLibFactory *factory = KLibLoader::self()->factory( service->library().latin1() );
	editDocument = static_cast<KTextEditor::Document *>(
		factory->create( preferencesDialog->scriptFrame, 0, "KTextEditor::Document" )
	);

	editDocument->createView( preferencesDialog->scriptFrame, 0 )->setSizePolicy(
		QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding)
	);

	updateHighlight();
	*/

	new QListViewItem( preferencesDialog->accountList, i18n("All Accounts") );

	load();

	connect(preferencesDialog, SIGNAL(destroyed()), editDocument, SLOT(deleteLater()) );
	connect(preferencesDialog->eventType, SIGNAL(activated(int)), this, SLOT(slotUpdateScript()) );
	connect(preferencesDialog->accountList, SIGNAL(selectionChanged()), this, SLOT(slotUpdateScript()) );
	connect(editDocument, SIGNAL(textChanged()), this, SLOT( slotTextChanged()) );
}
/*
void JavaScriptPreferences::updateHighlight()
{
	KTextEditor::HighlightingInterface *hi = KTextEditor::highlightingInterface( editDocument );
	int count = hi->hlModeCount();

	for( int i=0; i < count; ++i )
	{
		if( hi->hlModeName(i) == QString::fromLatin1("JavaScript") )
		{
			hi->setHlMode(i);
			break;
		}
	}
}
*/
void JavaScriptPreferences::slotTextChanged()
{
	emit KCModule::changed(true);
}

void JavaScriptPreferences::slotUpdateScript()
{
	QListViewItem *selectedItem = preferencesDialog->accountList->selectedItem();
	AccountItem *accountItem = dynamic_cast<AccountItem*>( selectedItem );

	editDocument->setText(
		config->script( accountItem ? accountItem->account : 0, preferencesDialog->eventType->currentItem() )
	);
}

// reload configuration reading it from kopeterc
void JavaScriptPreferences::load()
{
	preferencesDialog->writeEnabled->setChecked( config->writeEnabled() );
	preferencesDialog->treeEnabled->setChecked( config->treeEnabled() );
	preferencesDialog->factoryEnabled->setChecked( config->factoryEnabled() );
	preferencesDialog->signalsEnabled->setChecked( config->signalsEnabled() );
}

// save list to kopeterc and creates map out of it
void JavaScriptPreferences::save()
{
	config->setWriteEnabled( preferencesDialog->writeEnabled->isChecked() );
	config->setTreeEnabled( preferencesDialog->treeEnabled->isChecked() );
	config->setFactoryEnabled( preferencesDialog->factoryEnabled->isChecked() );
	config->setSignalsEnabled( preferencesDialog->signalsEnabled->isChecked() );

	QListViewItem *selectedItem = preferencesDialog->accountList->selectedItem();
	AccountItem *accountItem = dynamic_cast<AccountItem*>( selectedItem );

	config->setScript( accountItem ? accountItem->account : 0, preferencesDialog->eventType->currentItem(),
		editDocument->text() );

	emit KCModule::changed(false);
}


#include "javascriptpreferences.moc"

