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
#include <ktempfile.h>
#include <krun.h>
#include <kurl.h>
#include <kio/netaccess.h>
#include <kurlrequesterdlg.h>
#include <kdirwatch.h>
#include <kgenericfactory.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <ktrader.h>
#include <klibloader.h>
#include <qfile.h>
#include <qlayout.h>
#include <qptrlist.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qsignal.h>

#include "kopeteaccountmanager.h"
#include "kopeteaccount.h"
#include "kopeteuiglobal.h"

#include "javascriptpreferences.h"
#include "javascriptprefsbase.h"

typedef KGenericFactory<JavaScriptPreferences> JavaScriptPreferencesFactory;

class ScriptItem : public QCheckListItem
{
	public:
		ScriptItem( QListView *parent, Script *m_script, QObject *reciever, const char* slot ) :
			QCheckListItem( parent, m_script->name, QCheckListItem::CheckBox ),
			script( m_script )
		{
			setText( 1, script->description );
			setText( 2, script->author );
			setEnabled( !script->immutable );

			sig.connect( reciever, slot );
			sig.setValue( (int)this );
			lockSig = false;
		}

		void stateChange( bool )
		{
			kdDebug() << k_funcinfo << endl;
			if( !lockSig )
				sig.activate();
		}

		void setOn( bool b, bool lock = false )
		{
			lockSig = lock;
			QCheckListItem::setOn( b );
			lockSig = false;
		}

		Script *script;

	private:
		QSignal sig;
		bool lockSig;

};

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

	new QListViewItem( preferencesDialog->accountList, i18n("All Accounts") );
        tempFile = 0L;

	load();

	connect(config, SIGNAL(changed()), this, SLOT(slotEmitChanged()) );
	connect(preferencesDialog->scriptList, SIGNAL(selectionChanged()), this, SLOT(slotUpdateButtons()) );
	connect(preferencesDialog->accountList, SIGNAL(selectionChanged()), this, SLOT(slotUpdateScriptList()) );
	connect(preferencesDialog->addScript, SIGNAL(clicked()), this, SLOT(slotAddScript()) );
	connect(preferencesDialog->editScript, SIGNAL(clicked()), this, SLOT(slotEditScript()) );
	connect(preferencesDialog->eventType, SIGNAL(activated(int)), this, SLOT(slotUpdateScriptList()) );
	connect(preferencesDialog->accountList, SIGNAL(selectionChanged()), this, SLOT(slotUpdateScriptList()) );
	connect(KDirWatch::self(), SIGNAL( dirty( const QString & ) ), this, SLOT( slotFileDirty( const QString & ) ) );

	QPtrList<KopeteAccount> accounts = KopeteAccountManager::manager()->accounts();
	for( KopeteAccount *a = accounts.first(); a; a = accounts.next() )
	{
		new AccountItem( preferencesDialog->accountList, a );
	}

	slotUpdateButtons();
}

void JavaScriptPreferences::slotEmitChanged()
{
	emit KCModule::changed(true);
}

void JavaScriptPreferences::slotUpdateButtons()
{
	QListViewItem *selectedItem = preferencesDialog->scriptList->selectedItem();
	if( selectedItem )
	{
		Script *s = config->script( selectedItem->text(0) );
		if( s )
		{
			preferencesDialog->editScript->setEnabled( !s->immutable );
			preferencesDialog->removeScript->setEnabled( !s->immutable );
		}
	}
	else
	{
		preferencesDialog->editScript->setEnabled( false );
		preferencesDialog->removeScript->setEnabled( false );
	}
}

void JavaScriptPreferences::slotUpdateScriptList()
{
	AccountItem *item = dynamic_cast<AccountItem*>( preferencesDialog->accountList->selectedItem() );
	int type = preferencesDialog->eventType->currentItem();
	QStringList scripts = config->scriptNamesFor( item ? item->account : 0L, type );

	for( ScriptItem *i = static_cast<ScriptItem*>( preferencesDialog->scriptList->firstChild() );
		i; i = static_cast<ScriptItem*>( i->nextSibling() ) )
	{
		i->setOn( !scripts.grep( i->text(0) ).isEmpty(), true );
	}
}

void JavaScriptPreferences::slotAddScript()
{
	KURL script = KURLRequesterDlg::getURL( QString::null, preferencesDialog, i18n("Select Script") );
	QString fileName;

	if( KIO::NetAccess::download( script, fileName, Kopete::UI::Global::mainWidget() ) )
	{
		QFile f( fileName );
		f.open( IO_ReadOnly );
		QTextStream stream( &f );
		QString contents;
		stream >> contents;

		if( script.isLocalFile() )
			f.close();
		else
			f.remove();

		Script *scriptPtr = config->addScript( script.path(), QString::null, QString::null, contents );

		new ScriptItem( preferencesDialog->scriptList, scriptPtr,
			this, SLOT(slotEnableScript(const QVariant &)) );;
	}
	else
	{
		kdError() << k_funcinfo << "Could not download URL " << script << endl;
	}
}

void JavaScriptPreferences::slotEditScript()
{
	if( tempFile )
	{
		tempFile->unlink();
		delete tempFile;
	}

	currentScript = static_cast<ScriptItem*>( preferencesDialog->scriptList->selectedItem() )->script;
	tempFile = new KTempFile;
	*tempFile->textStream() << currentScript->script;
	tempFile->close();
	KDirWatch::self()->addFile( tempFile->name() );
	KRun::runURL( tempFile->name(), QString::fromLatin1("text/javascript"), false );
}

void JavaScriptPreferences::slotEnableScript( const QVariant &scriptItem )
{
	kdDebug() << k_funcinfo << endl;
	ScriptItem *i = (ScriptItem*)scriptItem.toInt();

	QListViewItem *accountItem = preferencesDialog->accountList->selectedItem();
	if( accountItem )
	{
		AccountItem *a = dynamic_cast<AccountItem*>( accountItem );
		int type = preferencesDialog->eventType->currentItem();

		config->setScriptEnabled( a ? a->account : 0L, type, i->text(0), i->isOn() );
	}
}

void JavaScriptPreferences::slotFileDirty( const QString &file )
{
	if( tempFile && file == tempFile->name() )
	{
		*tempFile->textStream() >> currentScript->script;
		emit KCModule::changed(true);
	}
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

	config->apply();

	emit KCModule::changed(false);
}


#include "javascriptpreferences.moc"

