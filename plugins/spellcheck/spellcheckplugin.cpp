/*
    spellcheckplugin.cpp

    Kopete Spell Checking plugin

    Copyright (c) 2003 by Jason Keirstead   <jason@keirstead.org>

    Kopete    (c) 2002 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include <qapplication.h>
#include <qtextedit.h>

#include <kdebug.h>
#include <kspell.h>
#undef KDE_NO_COMPAT
#include <kaction.h>
#include <kmessagebox.h>
#include <kgenericfactory.h>

#include "kopeteview.h"
#include "kopetemessagemanagerfactory.h"
#include "spellcheckplugin.h"
#include "spellcheckpreferences.h"
#include "singlespellinstance.h"

typedef KGenericFactory<SpellCheckPlugin> SpellCheckPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kopete_spellcheck, SpellCheckPluginFactory( "kopete_spellcheck" )  );

SpellCheckPlugin* SpellCheckPlugin::pluginStatic_ = 0L;

SpellCheckPlugin::SpellCheckPlugin( QObject *parent, const char *name, const QStringList & )
: KopetePlugin( SpellCheckPluginFactory::instance(), parent, name )
{
	if ( !pluginStatic_ )
		pluginStatic_ = this;

	m_actionCollection = 0L;
	m_currentKMM=0L;
	mSpell = 0L;
	mPrefs = new SpellCheckPreferences( QString::fromLatin1( "spellcheck" ), this );

	m_spellCheckerReady = false;
	m_manualCheckInProgress = false;

	connect( mPrefs, SIGNAL( saved() ), this, SLOT( slotPrefsSaved() ) );

	// Connect to the viewCreated signal to install our event filter
	connect( KopeteMessageManagerFactory::factory(), SIGNAL( viewCreated( KopeteView * ) ), this, SLOT( slotBindToView( KopeteView * ) ) );
}

SpellCheckPlugin::~SpellCheckPlugin()
{
	slotPrefsSaved();

	singleSpellers.setAutoDelete( true );
	singleSpellers.clear();

	pluginStatic_ = 0L;
}

SpellCheckPlugin* SpellCheckPlugin::plugin()
{
	return pluginStatic_ ;
}

void SpellCheckPlugin::slotPrefsSaved()
{
	// Destroy the spelling instance so that new prefs will be reloaded
	if( mSpell )
	{
		mSpell->cleanUp();
		delete mSpell;
		mSpell = 0L;
	}
}

KSpell *SpellCheckPlugin::speller()
{
	if( !mSpell )
	{
		mSpell = new KSpell( 0L, i18n( "Spellcheck" ), this, SLOT( slotSpellCheckerReady( KSpell * ) ), mPrefs->spellConfig() );
		connect( mSpell, SIGNAL( misspelling( const QString&, const QStringList&, unsigned int ) ),
			this, SLOT( slotMisspelling( const QString&, const QStringList&, unsigned int ) ) );
		connect( mSpell, SIGNAL( corrected( const QString&, const QString&, unsigned int ) ),
			this, SLOT( slotCorrection( const QString&, const QString&, unsigned int ) ) );
		connect( mSpell, SIGNAL( done( const QString & ) ), this, SLOT( slotSpellDone( const QString &) ) );

		if( !singleSpellers.isEmpty() )
		{
			for ( SingleSpellInstance *it = singleSpellers.first(); it; it = singleSpellers.next() )
				connect( mSpell, SIGNAL( misspelling( const QString&, const QStringList&, unsigned int ) ),
					it, SLOT( misspelling( const QString&, const QStringList&, unsigned int ) ) );
		}
	}

	// If the spell checker is not ready, return 0L. The caller can try later
	// whether initialization succeeded by calling speller() again
	if ( m_spellCheckerReady )
		return mSpell;
	else
		return 0L;
}

KActionCollection *SpellCheckPlugin::customChatActions( KopeteMessageManager *kmm )
{
	kdDebug() << k_funcinfo << endl;

	m_currentKMM=kmm;

	delete m_actionCollection;

	m_actionCollection = new KActionCollection(this);

	KAction *spellCheck = new  KAction( i18n( "Check S&pelling" ), QString::fromLatin1( "spellcheck" ),
		mPrefs->shortCut(), this, SLOT( slotCheckSpelling() ), m_actionCollection, "checkSpelling" );

	m_actionCollection->insert( spellCheck );

	return m_actionCollection;
}

void SpellCheckPlugin::slotBindToView( KopeteView *view )
{
	// Install event filter on the edit widget
	if ( mPrefs->autoCheckEnabled() && view->editWidget()->inherits( "QTextEdit" ) )
	{
		kdDebug() << k_funcinfo << "New View, creating a single speller instance" << endl;

		SingleSpellInstance *spellInstance = new SingleSpellInstance( this, view );
		singleSpellers.append( spellInstance );
		KSpell *spell = speller();
		if ( spell )
		{
			connect( speller(), SIGNAL( misspelling( const QString &, const QStringList &, unsigned int ) ),
				spellInstance, SLOT( misspelling( const QString &, const QStringList &, unsigned int ) ) );
		}
	}
}

void SpellCheckPlugin::slotCheckSpelling()
{
	kdDebug() << k_funcinfo << endl;

	if ( m_spellCheckerReady )
	{
		delete mSpell;
		mSpell = 0L;
		m_manualCheckInProgress = true;
		KopeteView *activeView = m_currentKMM->view();
		mBuffer = activeView->currentMessage();

		// Return here, the actual spell check is done when
		// slotSpellCheckerReady() is called.
	}
}

void SpellCheckPlugin::slotSpellCheckerReady( KSpell * )
{
	m_spellCheckerReady = true;

	// If a user requested a manual check a new speller is created.
	// Call check() here
	if ( m_manualCheckInProgress )
		mSpell->check( mBuffer.plainBody() );

	// FIXME: Bind to all existing views here. This is needed because
	//        1. slotBindToView() may have been called while spellcheck was initializing.
	//           in this case nothing is done yet and the connect should be done here instead.
	//        2. A user may decide to load spell check later, while existing chats are
	//           already open
	// - Martijn
}

void SpellCheckPlugin::slotCorrection( const QString &originalword, const QString &newword, unsigned int pos )
{
	if( m_manualCheckInProgress )
	{
		//FIXME
		KopeteView *activeView = 0L;//m_currentKMM->view();
		KopeteMessageManagerFactory::factory()->activeView(activeView);

		QString mBuff = mBuffer.plainBody();
		mBuff.replace( pos,originalword.length(), newword );
		mBuffer.setBody( mBuff );

		activeView->setCurrentMessage( mBuffer );
	}
}

void SpellCheckPlugin::slotMisspelling( const QString &originalword, const QStringList &, unsigned int pos )
{
	if( m_manualCheckInProgress )
	{
		//FIXME
		KopeteView *activeView = 0L;//m_currentKMM->view();
		KopeteMessageManagerFactory::factory()->activeView(activeView);

		if( QTextEdit *t = dynamic_cast<QTextEdit*>( activeView->editWidget() ) )
		{
			//Select the misspelled text
			t->setSelection( 0, pos, 0, pos + originalword.length() );
		}
	}
}

void SpellCheckPlugin::slotSpellDone( const QString & )
{
	m_manualCheckInProgress = false;

	KSpell::spellStatus status = mSpell->status();

	if (status == KSpell::Error)
		KMessageBox::sorry(0L, i18n("ISpell/Aspell could not be started. Please make sure you have ISpell or Aspell properly configured and in your PATH."));
	else if (status == KSpell::Crashed)
		KMessageBox::sorry(0L, i18n("ISpell/Aspell seems to have crashed."));

}

#include "spellcheckplugin.moc"

// vim: set noet ts=4 sts=4 sw=4:

