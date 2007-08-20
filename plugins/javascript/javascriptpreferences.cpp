/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "javascriptpreferences.h"

#include "javascriptconfig.h"
#include "javascriptfile.h"
//#include "javascriptnamedialog.h"

#include "kopeteaccountmanager.h"
#include "kopeteaccount.h"
#include "kopeteuiglobal.h"

#include <kdialog.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktemporaryfile.h>

#include <qdir.h>

#ifdef __GNUC__
#warning CLEAN ME
#endif
#include <krun.h>
#include <kurl.h>
#include <kurlrequester.h>
#include <kio/netaccess.h>
#include <kurlrequesterdialog.h>
#include <kdirwatch.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <klibloader.h>

#include <qfile.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qtimer.h>
#include <qlineedit.h>

struct JavaScriptPreferences::Private {
	JavaScriptConfig *config;
	KTemporaryFile *tempFile;
	JavaScriptFile *currentScript;
	KDialog *addScriptDialog;
//	JavaScriptDialog *nameDialog;
//	pid_t editProcess;
};

typedef KGenericFactory<JavaScriptPreferences> JavaScriptPreferencesFactory;
/*
class ScriptItem
	: public QCheckListItem
{
public:
	ScriptItem( QListView *parent, Script *m_script, QObject *receiver, const char* slot ) :
		QCheckListItem( parent, m_script->name, QCheckListItem::CheckBox ),
		script( m_script )
	{
		setText( 1, script->version );
		setText( 2, script->description );
		setText( 3, script->author );
		setEnabled( !script->immutable );

//		sig.connect( receiver, slot );
//		sig.setValue( (int)this );
		lockSig = false;
	}

	void stateChange( bool b )
	{
		kDebug() ;
		QCheckListItem::stateChange( b );
//		if( !lockSig )
//			sig.activate();
	}

	void setOn( bool b, bool lock = false )
	{
		lockSig = lock;
		QCheckListItem::setOn( b );
		lockSig = false;
	}

	JavaScriptFile *script;

private:
//	QSignal sig;
	bool lockSig;
};

class AccountItem
	: public QListViewItem
{
public:
	AccountItem( QListView *parent, Kopete::Account *a ) :
	QListViewItem( parent, a->accountId() )
	{
		this->setPixmap( 0, a->accountIcon() );
		account = a;
	}

	Kopete::Account *account;
};
*/
K_EXPORT_COMPONENT_FACTORY( kcm_kopete_javascript, JavaScriptPreferencesFactory( "kcm_kopete_javascript" ) )

JavaScriptPreferences::JavaScriptPreferences( QWidget *parent, const QStringList &args )
	: KCModule( JavaScriptPreferencesFactory::componentData(), parent, args )
	, KNewStuff( "kopete/javascript", "http://kopete.kde.org/knewstuff/javascript/providers.xml", parent )
	, d(new JavaScriptPreferences::Private)
{
	d->config = JavaScriptConfig::instance();

//	( new QVBoxLayout( this ) )->setAutoAdd( true );
//	preferencesDialog = new JavaScriptPrefsBase( this );
	setupUi(this);

//	new QListViewItem( preferencesDialog->accountList, i18n("All Accounts") );
        d->tempFile = 0L;

	load();

	connect(d->config, SIGNAL(changed()),
		this, SLOT(slotEmitChanged()) );
	connect(this, SIGNAL( installPackage( const QString &, bool & ) ),
		d->config, SLOT( installPackage( const QString &, bool &) ) );
/*
	connect(scriptList, SIGNAL(selectionChanged()),
		this, SLOT(slotUpdateButtons()) );
	connect(accountList, SIGNAL(selectionChanged()),
		this, SLOT(slotUpdateScriptList()) );
	connect(addScript, SIGNAL(clicked()),
		this, SLOT( slotAddScript() ) );
	connect(downloadScript, SIGNAL(clicked()),
		this, SLOT( slotDownloadScript() ) );
	connect(editScript, SIGNAL(clicked()),
		this, SLOT(slotEditScript()) );
	connect(accountList, SIGNAL(selectionChanged()),
		this, SLOT(slotUpdateScriptList()) );
	connect(KDirWatch::self(), SIGNAL( dirty( const QString & ) ),
		this, SLOT( slotFileDirty( const QString & ) ) );

	foreach(Kopete::Account *account, Kopete::AccountManager::self()->accounts())
	{
//		new AccountItem( accountList, account );
	}

	foreach( JavaScriptFile *script, d->config->allScripts() )
	{
//		new ScriptItem( preferencesDialog->scriptList, script,
//			this, SLOT( slotEnableScript(const QVariant &) ) );
	}
*/
	slotUpdateButtons();
}

JavaScriptPreferences::~JavaScriptPreferences()
{
	delete d;
}

void JavaScriptPreferences::slotEmitChanged()
{
	emit KCModule::changed(true);
}

void JavaScriptPreferences::slotUpdateButtons()
{
/*
	ScriptItem *selectedItem = static_cast<ScriptItem*>( preferencesDialog->scriptList->selectedItem() );
	if( selectedItem )
	{
		JavaScriptFile *s = config->script( selectedItem->script->id );
		if( s )
		{
			editScript->setEnabled( !s->immutable );
			removeScript->setEnabled( !s->immutable );
			configureScript->setEnabled( s->functions.contains("Configure") );
		}
		else
		{
			editScript->setEnabled( false );
			removeScript->setEnabled( false );
			configureScript->setEnabled( false );
		}
	}
	else
	{
		editScript->setEnabled( false );
		removeScript->setEnabled( false );
		configureScript->setEnabled( false );
	}
*/
}

void JavaScriptPreferences::slotUpdateScriptList()
{
/*
	AccountItem *item = dynamic_cast<AccountItem*>( accountList->selectedItem() );
	QString act("GLOBAL_SCRIPT");
	if( item )
		act = item->account->accountId();

	for( ScriptItem *i = static_cast<ScriptItem*>( scriptList->firstChild() );
		i; i = static_cast<ScriptItem*>( i->nextSibling() ) )
	{
		i->setOn( i->script->accounts.contains( act ) );
	}
*/
}

void JavaScriptPreferences::slotAddScript()
{
/*
	addScriptDialog = new KDialog( this, "javascriptdialog", false, "Add Script",
                  KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true );

	d->nameDialog = new JavaScriptDialog( addScriptDialog->makeVBoxMainWidget() );
	d->nameDialog->show();

	connect( addScriptDialog, SIGNAL( okClicked() ), this, SLOT( slotAddComplete() ) );
	connect( addScriptDialog, SIGNAL( finished() ), this, SLOT( slotAddDone() ) );

	addScriptDialog->show();
*/
}

void JavaScriptPreferences::slotAddComplete()
{
/*
	QMap<QString,QString> functions;
	QString none = QLatin1String("None");

	if( d->nameDialog->initFunction->currentText() != none )
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
		KUrl url( nameDialog->scriptPath->url() );

		QString id = QString::number( time( NULL ) );
		QString localScriptsDir( locateLocal("data", QString::fromLatin1("kopete/scripts")) );
		QDir d;
		if( !d.exists( localScriptsDir ) )
			d.mkdir( localScriptsDir );

		QString scriptDir = localScriptsDir + '/' +  id;
		if( !d.exists( scriptDir ) )
			d.mkdir( scriptDir );

		QFile f( scriptDir + '/' + url.fileName() );
		if( f.open( QIODevice::WriteOnly ) )
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
*/
}

void JavaScriptPreferences::slotAddDone()
{
	d->addScriptDialog->delayedDestruct();
}

void JavaScriptPreferences::slotEditScript()
{
/*
	ScriptItem *selItem = static_cast<ScriptItem*>( preferencesDialog->scriptList->selectedItem() );
	if( selItem )
	{
		QString localScriptsDir( locateLocal("data", QString::fromLatin1("kopete/scripts")) );
		preferencesDialog->scriptList->setEnabled(false);
		preferencesDialog->editScript->setEnabled(false);
		editProcess = KRun::runURL( localScriptsDir + '/' + selItem->script->id + '/' + selItem->script->fileName,
			"application/javascript", false, false );
		slotWaitForEdit();
	}
*/
}

void JavaScriptPreferences::slotWaitForEdit()
{
/*
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
*/
}

void JavaScriptPreferences::slotEditDone()
{
	QObject *runner = sender();
	runner->deleteLater();

	scriptList->setEnabled( true );
/*
	ScriptItem *selItem = static_cast<ScriptItem*>( preferencesDialog->scriptList->selectedItem() );
	if( selItem )
	{
		selItem->script->script(true);
		emit KCModule::changed(true);
	}
*/
}

void JavaScriptPreferences::slotEnableScript( const QVariant &scriptItem )
{
	kDebug() ;
/*
	ScriptItem *i = (ScriptItem*)scriptItem.toInt();

	QListViewItem *accountItem = preferencesDialog->accountList->selectedItem();
	if( accountItem )
	{
		AccountItem *a = dynamic_cast<AccountItem*>( accountItem );
		config->setScriptEnabled( a ? a->account : 0L, i->script->id, i->isOn() );
		emit KCModule::changed(true);
	}
*/
}

void JavaScriptPreferences::slotFileDirty( const QString &file )
{
/*
	if( d->tempFile && file == d->tempFile->name() )
	{
		*d->tempFile->textStream() >> d->currentScript->fileName;
		emit KCModule::changed(true);
	}
*/
}

// reload configuration reading it from kopeterc
void JavaScriptPreferences::load()
{
	writeEnabled->setChecked( d->config->writeEnabled() );
	treeEnabled->setChecked( d->config->treeEnabled() );
	factoryEnabled->setChecked( d->config->factoryEnabled() );
	signalsEnabled->setChecked( d->config->signalsEnabled() );
}

// save list to kopeterc and creates map out of it
void JavaScriptPreferences::save()
{
	d->config->setWriteEnabled( writeEnabled->isChecked() );
	d->config->setTreeEnabled( treeEnabled->isChecked() );
	d->config->setFactoryEnabled( factoryEnabled->isChecked() );
	d->config->setSignalsEnabled( signalsEnabled->isChecked() );

	d->config->apply();

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

