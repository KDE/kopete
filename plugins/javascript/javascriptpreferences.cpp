/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <sys/wait.h>

#include <qtimer.h>
#include <qvbox.h>
#include <qdir.h>
#include <qlineedit.h>

#include <klistview.h>
#include <klocale.h>
#include <ktempfile.h>
#include <krun.h>
#include <kurl.h>
#include <kurlrequester.h>
#include <kdialogbase.h>
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

#include "javascriptnamedialog.h"
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
			setText( 1, script->version );
			setText( 2, script->description );
			setText( 3, script->author );
			setEnabled( !script->immutable );

			sig.connect( reciever, slot );
			sig.setValue( (int)this );
			lockSig = false;
		}

		void stateChange( bool b )
		{
			kdDebug() << k_funcinfo << endl;
			QCheckListItem::stateChange( b );
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
	: KCModule( JavaScriptPreferencesFactory::instance(), parent, args ),
	KNewStuff( "kopete/javascript", "http://kopete.kde.org/knewstuff/javascript/providers.xml", parent )
{
	config = JavaScriptConfig::instance();

	( new QVBoxLayout( this ) )->setAutoAdd( true );
	preferencesDialog = new JavaScriptPrefsBase( this );

	new QListViewItem( preferencesDialog->accountList, i18n("All Accounts") );
        tempFile = 0L;

	load();

	connect(config, SIGNAL(changed()), this, SLOT(slotEmitChanged()) );
	connect(this, SIGNAL( installPackage( const QString &, bool & ) ),
		config, SLOT( installPackage( const QString &, bool &) ) );

	connect(preferencesDialog->scriptList, SIGNAL(selectionChanged()), this, SLOT(slotUpdateButtons()) );
	connect(preferencesDialog->accountList, SIGNAL(selectionChanged()), this, SLOT(slotUpdateScriptList()) );
	connect(preferencesDialog->addScript, SIGNAL(clicked()), this, SLOT( slotAddScript() ) );
	connect(preferencesDialog->downloadScript, SIGNAL(clicked()), this, SLOT( slotDownloadScript() ) );
	connect(preferencesDialog->editScript, SIGNAL(clicked()), this, SLOT(slotEditScript()) );
	connect(preferencesDialog->accountList, SIGNAL(selectionChanged()), this, SLOT(slotUpdateScriptList()) );
	connect(KDirWatch::self(), SIGNAL( dirty( const QString & ) ), this, SLOT( slotFileDirty( const QString & ) ) );

	QPtrList<KopeteAccount> accounts = KopeteAccountManager::manager()->accounts();
	for( KopeteAccount *a = accounts.first(); a; a = accounts.next() )
	{
		new AccountItem( preferencesDialog->accountList, a );
	}

	QValueList<Script*> scripts = config->allScripts();
	for( QValueList<Script*>::iterator it = scripts.begin(); it != scripts.end(); ++it )
	{
		new ScriptItem( preferencesDialog->scriptList, *it, this, SLOT( slotEnableScript(const QVariant &) ) );
	}

	slotUpdateButtons();
}

void JavaScriptPreferences::slotEmitChanged()
{
	emit KCModule::changed(true);
}

void JavaScriptPreferences::slotUpdateButtons()
{
	ScriptItem *selectedItem = static_cast<ScriptItem*>( preferencesDialog->scriptList->selectedItem() );
	if( selectedItem )
	{
		Script *s = config->script( selectedItem->script->id );
		if( s )
		{
			preferencesDialog->editScript->setEnabled( !s->immutable );
			preferencesDialog->removeScript->setEnabled( !s->immutable );
			preferencesDialog->configureScript->setEnabled( s->functions.contains("Configure") );
		}
		else
		{
			preferencesDialog->editScript->setEnabled( false );
			preferencesDialog->removeScript->setEnabled( false );
			preferencesDialog->configureScript->setEnabled( false );
		}
	}
	else
	{
		preferencesDialog->editScript->setEnabled( false );
		preferencesDialog->removeScript->setEnabled( false );
		preferencesDialog->configureScript->setEnabled( false );
	}
}

void JavaScriptPreferences::slotUpdateScriptList()
{
	AccountItem *item = dynamic_cast<AccountItem*>( preferencesDialog->accountList->selectedItem() );
	QString act("GLOBAL_SCRIPT");
	if( item )
		act = item->account->accountId();

	for( ScriptItem *i = static_cast<ScriptItem*>( preferencesDialog->scriptList->firstChild() );
		i; i = static_cast<ScriptItem*>( i->nextSibling() ) )
	{
		i->setOn( i->script->accounts.contains( act ) );
	}
}

void JavaScriptPreferences::slotAddScript()
{
	addScriptDialog = new KDialogBase( this, "javascriptdialog", false, "Add Script",
                  KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true );

	nameDialog = new JavaScriptDialog( addScriptDialog->makeVBoxMainWidget() );
	nameDialog->show();

	connect( addScriptDialog, SIGNAL( okClicked() ), this, SLOT( slotAddComplete() ) );
	connect( addScriptDialog, SIGNAL( finished() ), this, SLOT( slotAddDone() ) );

	addScriptDialog->show();
}

void JavaScriptPreferences::slotAddComplete()
{
	QMap<QString,QString> functions;
	QString none = QString::fromLatin1("None");

	if( nameDialog->initFunction->currentText() != none )
		functions.insert( QString::fromLatin1("Init"), nameDialog->initFunction->currentText() );

	if( nameDialog->configureFunction->currentText() != none )
		functions.insert( QString::fromLatin1("Configure"), nameDialog->configureFunction->currentText() );

	if( nameDialog->incoming->currentText() != none )
		functions.insert( QString::fromLatin1("Incoming"), nameDialog->incoming->currentText() );

	if( nameDialog->outgoing->currentText() != none )
		functions.insert( QString::fromLatin1("Outgoing"), nameDialog->outgoing->currentText() );

	if( nameDialog->display->currentText() != none )
		functions.insert( QString::fromLatin1("Display"), nameDialog->display->currentText() );

	if( nameDialog->accountStatus->currentText() != none )
		functions.insert( QString::fromLatin1("AccountStatus"), nameDialog->accountStatus->currentText() );

	if( nameDialog->contactStatus->currentText() != none )
		functions.insert( QString::fromLatin1("ContactStatus"), nameDialog->contactStatus->currentText() );

	if( nameDialog->contactAdded->currentText() != none )
		functions.insert( QString::fromLatin1("ContactAdded"), nameDialog->contactAdded->currentText() );

	if( nameDialog->contactRemoved->currentText() != none )
		functions.insert( QString::fromLatin1("ContactRemoved"), nameDialog->contactRemoved->currentText() );

	QString scriptName = nameDialog->scriptName->text();
	if( !scriptName.isEmpty() )
	{
		KURL url( nameDialog->scriptPath->url() );

		QString id = QString::number( time( NULL ) );
		QString localScriptsDir( locateLocal("data", QString::fromLatin1("kopete/scripts")) );
		QDir d;
		if( !d.exists( localScriptsDir ) )
			d.mkdir( localScriptsDir );

		QString scriptDir = localScriptsDir + "/" +  id;
		if( !d.exists( scriptDir ) )
			d.mkdir( scriptDir );

		QFile f( scriptDir + "/" + url.fileName() );
		if( f.open( IO_WriteOnly ) )
		{
			QTextStream stream( &f );
			stream << fileContents( nameDialog->scriptPath->url() );
			f.close();

			Script *s = config->addScript( url.fileName(), scriptName,
				nameDialog->scriptDescription->text(), nameDialog->scriptAuthor->text(),
				nameDialog->scriptVersion->text(), functions, id );

			new ScriptItem( preferencesDialog->scriptList, s, this, SLOT( slotEnableScript(const QVariant &) ) );

			slotUpdateScriptList();

			emit KCModule::changed(true);

		}
		else
		{
			KMessageBox::sorry( Kopete::UI::Global::mainWidget(),
			i18n("Unable to open %1 for writing, check permissions to this location.").arg( f.name() ) );
		}
	}
}

void JavaScriptPreferences::slotAddDone()
{
	addScriptDialog->delayedDestruct();
}

void JavaScriptPreferences::slotEditScript()
{
	ScriptItem *selItem = static_cast<ScriptItem*>( preferencesDialog->scriptList->selectedItem() );
	if( selItem )
	{
		QString localScriptsDir( locateLocal("data", QString::fromLatin1("kopete/scripts")) );
		preferencesDialog->scriptList->setEnabled(false);
		preferencesDialog->editScript->setEnabled(false);
		editProcess = KRun::runURL( localScriptsDir + "/" + selItem->script->id + "/" + selItem->script->fileName,
			"text/javascript", false, false );
		slotWaitForEdit();
	}
}

void JavaScriptPreferences::slotWaitForEdit()
{
	int status;
	if( waitpid( editProcess, &status, WNOHANG ) < 1 )
	{
		preferencesDialog->scriptList->setEnabled(true);
		preferencesDialog->editScript->setEnabled(true);
	}
	else
	{
		QTimer::singleShot( 100, this, SLOT( slotWaitForEdit() ) );
	}
}

void JavaScriptPreferences::slotEditDone()
{
	QObject *runner = const_cast<QObject*>( sender() );
	runner->deleteLater();

	preferencesDialog->scriptList->setEnabled( true );
	ScriptItem *selItem = static_cast<ScriptItem*>( preferencesDialog->scriptList->selectedItem() );
	if( selItem )
	{
		selItem->script->script(true);
		emit KCModule::changed(true);
	}
}

void JavaScriptPreferences::slotEnableScript( const QVariant &scriptItem )
{
	kdDebug() << k_funcinfo << endl;
	ScriptItem *i = (ScriptItem*)scriptItem.toInt();

	QListViewItem *accountItem = preferencesDialog->accountList->selectedItem();
	if( accountItem )
	{
		AccountItem *a = dynamic_cast<AccountItem*>( accountItem );
		config->setScriptEnabled( a ? a->account : 0L, i->script->id, i->isOn() );
		emit KCModule::changed(true);
	}
}

void JavaScriptPreferences::slotFileDirty( const QString &file )
{
	if( tempFile && file == tempFile->name() )
	{
		*tempFile->textStream() >> currentScript->fileName;
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

void JavaScriptPreferences::slotDownloadScript()
{
	 //Uses KNewStuff to download script
	 this->download();
}

bool JavaScriptPreferences::install( const QString &fileName )
{
	bool retVal = false;
	emit installPackage( fileName, retVal );
	emit KCModule::changed(true);
	return retVal;
}

bool JavaScriptPreferences::createUploadFile( const QString & )
{
	//TODO: Do this!
	return false;
}


#include "javascriptpreferences.moc"

