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
#include <qtextedit.h>

#include <kdebug.h>
#include <kspell.h>
#include <klocale.h>
#include <kaction.h>
#include <kmessagebox.h>
#include <kgenericfactory.h>

#include "kopeteviewmanager.h"
#include "kopetemessagemanagerfactory.h"
#include "spellcheckplugin.h"
#include "spellcheckpreferences.h"
#include "singlespellinstance.h"

K_EXPORT_COMPONENT_FACTORY( kopete_spellcheck, KGenericFactory<SpellCheckPlugin> );

SpellCheckPlugin::SpellCheckPlugin( QObject *parent, const char *name, const QStringList &) : KopetePlugin( parent, name )
{
	if ( !pluginStatic_ )
		pluginStatic_ = this;

	m_actionCollection = 0L;
	mSpell = 0L;
	mPrefs = new SpellCheckPreferences( QString::fromLatin1("spellcheck"), this );

	//Connect to the viewCreated signal to install our event filter
	connect( KopeteViewManager::viewManager(), SIGNAL( viewCreated( KopeteView * ) ), this, SLOT( slotBindToView( KopeteView * ) ) );
}

SpellCheckPlugin::~SpellCheckPlugin()
{
	delete mPrefs;
	pluginStatic_ = 0L;
}

SpellCheckPlugin* SpellCheckPlugin::plugin()
{
	return pluginStatic_ ;
}

SpellCheckPlugin* SpellCheckPlugin::pluginStatic_ = 0L;

KActionCollection *SpellCheckPlugin::customChatActions( KopeteMessageManager * )
{
	kdDebug() << k_funcinfo << endl;

	delete m_actionCollection;

	m_actionCollection = new KActionCollection(this);

	KAction *spellCheck = new  KAction( i18n( "Check S&pelling" ), QString::fromLatin1( "spellcheck" ),
		mPrefs->shortCut(), this, SLOT( slotCheckSpelling() ), m_actionCollection, "checkSpelling" );

	m_actionCollection->insert ( spellCheck );

	return m_actionCollection;
}

void SpellCheckPlugin::slotBindToView( KopeteView *view )
{
	//Install event filter on the edit widget
	if( mPrefs->autoCheckEnabled() && view->editWidget()->inherits("QTextEdit") )
	{
		kdDebug() << k_funcinfo << "New View, creating a single speller instance" << endl;
		new SingleSpellInstance( view, mPrefs );
	}
}

void SpellCheckPlugin::slotCheckSpelling()
{
	kdDebug() << k_funcinfo << endl;

	KopeteView *activeView = KopeteViewManager::viewManager()->activeView();
	mBuffer = activeView->currentMessage();

	if( !mSpell )
	{
		mSpell = new KSpell(0L, i18n("Spellcheck - Kopete"), this, SLOT( slotSpellCheckerReady( KSpell * )), mPrefs->spellConfig() );
		connect( mSpell, SIGNAL( misspelling( const QString&, const QStringList&, unsigned int ) ), this, SLOT( slotMisspelling( const QString&, const QStringList&, unsigned int ) ) );
		connect( mSpell, SIGNAL( corrected( const QString&, const QString&, unsigned int ) ), this, SLOT( slotCorrection( const QString&, const QString&, unsigned int ) ) );
		connect( mSpell, SIGNAL( done( const QString & ) ), this, SLOT( slotSpellDone( const QString &) ) );
	}
}

void SpellCheckPlugin::slotSpellCheckerReady( KSpell * )
{
	mSpell->check( mBuffer.plainBody() );
}

void SpellCheckPlugin::slotCorrection( const QString &originalword, const QString &newword, unsigned int pos )
{
	KopeteView *activeView = KopeteViewManager::viewManager()->activeView();

	QString mBuff = mBuffer.plainBody();
	mBuff.replace( pos,originalword.length(), newword );
	mBuffer.setBody( mBuff );

	activeView->setCurrentMessage( mBuffer );
}

void SpellCheckPlugin::slotMisspelling( const QString &originalword, const QStringList &, unsigned int pos )
{
	KopeteView *activeView = KopeteViewManager::viewManager()->activeView();
	if( activeView->editWidget()->inherits("QTextEdit") )
	{
		//Select the misspelled text
		QTextEdit *t = dynamic_cast<QTextEdit*>( activeView->editWidget() );
		t->setSelection( 0, pos, 0, pos + originalword.length() );
	}
}

void SpellCheckPlugin::slotSpellDone( const QString & )
{
	kdDebug() << k_funcinfo << endl;
	mSpell->cleanUp();

	KSpell::spellStatus status = mSpell->status();

	if (status == KSpell::Error)
		KMessageBox::sorry(0L, i18n("ISpell/Aspell could not be started. Please make sure you have ISpell or Aspell properly configured and in your PATH."));
	else if (status == KSpell::Crashed)
		KMessageBox::sorry(0L, i18n("ISpell/Aspell seems to have crashed."));

	delete mSpell;
	mSpell = 0L;
}




