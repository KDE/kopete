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
#include <qkeysequence.h>

#include <kdebug.h>
#include <kspell.h>
#include <klocale.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kmainwindow.h>
#include <kmessagebox.h>
#include <kgenericfactory.h>

#include "kopeteviewmanager.h"
#include "kopetemessagemanagerfactory.h"
#include "spellcheckplugin.h"

K_EXPORT_COMPONENT_FACTORY( kopete_spellcheck, KGenericFactory<SpellCheckPlugin> );

SpellCheckPlugin::SpellCheckPlugin( QObject *parent, const char *name, const QStringList &) : KopetePlugin( parent, name )
{
	if ( !pluginStatic_ )
		pluginStatic_ = this;

	m_actionCollection = 0L;
	mSpell = 0L;

	//connect( KopeteMessageManagerFactory::factory(), SIGNAL( aboutToSend( KopeteMessage & ) ), SLOT( slotOutgoingMessage( KopeteMessage & ) ) );
}

SpellCheckPlugin::~SpellCheckPlugin()
{
	pluginStatic_ = 0L;
	slotViewDestroyed();
}

SpellCheckPlugin* SpellCheckPlugin::plugin()
{
	return pluginStatic_ ;
}

SpellCheckPlugin* SpellCheckPlugin::pluginStatic_ = 0L;

KActionCollection *SpellCheckPlugin::customChatActions( KopeteMessageManager * )
{
	return customToolbarActions();
}

KActionCollection *SpellCheckPlugin::customToolbarActions()
{
	kdDebug() << k_funcinfo << endl;

	delete m_actionCollection;

	m_actionCollection = new KActionCollection(this);

	KAction *spellCheck = new  KAction( i18n( "Check S&pelling" ), QString::fromLatin1( "spellcheck" ),
		QKeySequence(CTRL+ ALT + Key_S), this, SLOT( slotCheckSpelling() ), m_actionCollection, "checkSpelling" );

	m_actionCollection->insert ( spellCheck );

	return m_actionCollection;
}

void SpellCheckPlugin::slotCheckSpelling()
{
	kdDebug() << k_funcinfo << endl;

	mView = KopeteViewManager::viewManager()->activeView();
	if( mView )
	{
		connect( dynamic_cast<QObject*>(mView), SIGNAL( destroyed() ), this, SLOT( slotViewDestroyed() ) );

		delete mSpell;

		mSpell = new KSpell(dynamic_cast<QWidget*>(mView), i18n("Spellcheck - Kopete"), this, SLOT( slotSpellCheckerReady( KSpell * )));
		connect( mSpell, SIGNAL( corrected( const QString&, const QString&, unsigned int ) ), this, SLOT( slotCorrection( const QString&, const QString&, unsigned int ) ) );
		connect( mSpell, SIGNAL( death() ), this, SLOT( slotSpellDone() ) );
	}
	else
	{
		kdDebug() << k_funcinfo << "Error, no active view" << endl;
	}
}


void SpellCheckPlugin::slotSpellCheckerReady( KSpell * )
{
	KopeteMessage m = mView->currentMessage();
	QString msgText = m.plainBody();

	if( !msgText.isEmpty() && !msgText.isNull() )
	{
		mBuffer = KopeteMessage(
					m.timestamp(),
					m.from(),
					m.to(),
					msgText,
					m.subject(),
					m.direction(),
					KopeteMessage::PlainText, m.type() );

		mBuffer.setBg( m.bg() );
		mBuffer.setFg( m.fg() );
		mBuffer.setFont( m.font() );

		mSpell->check( msgText );
	}
}

void SpellCheckPlugin::slotCorrection( const QString &originalword, const QString &newword, unsigned int pos )
{
	QString mBuff = mBuffer.plainBody();
	mBuff.replace( pos,originalword.length(), newword );
	mBuffer.setBody( mBuff );
	mView->setCurrentMessage( mBuffer );
}

void SpellCheckPlugin::slotSpellDone()
{
	if( mSpell )
	{
		KSpell::spellStatus status = mSpell->status();
		delete mSpell;
		mSpell = 0;

		if (status == KSpell::Error)
			KMessageBox::sorry(0L, i18n("ISpell/Aspell could not be started. Please make sure you have ISpell or Aspell properly configured and in your PATH."));
		else if (status == KSpell::Crashed)
			KMessageBox::sorry(0L, i18n("ISpell/Aspell seems to have crashed."));
	}
}

void SpellCheckPlugin::slotViewDestroyed()
{
	if( mSpell )
		mSpell->cleanUp();
}
