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
#include <qregexp.h>
#include <kpopupmenu.h>

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
	mSingleSpell = 0L;
	mView = 0L;
	spellCheckComplete = false;
	mBound = QRegExp( QString::fromLatin1("[\\s\\W]") );

	connect( KopeteViewManager::viewManager(), SIGNAL( viewCreated( KopeteView * ) ), this, SLOT( slotBindToView( KopeteView * ) ) );
}

SpellCheckPlugin::~SpellCheckPlugin()
{
	pluginStatic_ = 0L;
	if( mSingleSpell )
	{
		mSingleSpell->cleanUp();
		delete mSingleSpell;
	}
}

SpellCheckPlugin* SpellCheckPlugin::plugin()
{
	return pluginStatic_ ;
}

SpellCheckPlugin* SpellCheckPlugin::pluginStatic_ = 0L;

KActionCollection *SpellCheckPlugin::customChatActions( KopeteMessageManager *manager )
{
	kdDebug() << k_funcinfo << endl;

	delete m_actionCollection;

	mView = KopeteViewManager::viewManager()->view( manager, false);

	m_actionCollection = new KActionCollection(this);

	KAction *spellCheck = new  KAction( i18n( "Check S&pelling" ), QString::fromLatin1( "spellcheck" ),
		QKeySequence(CTRL+ ALT + Key_S), this, SLOT( slotCheckSpelling() ), m_actionCollection, "checkSpelling" );

	m_actionCollection->insert ( spellCheck );

	return m_actionCollection;
}

void SpellCheckPlugin::slotBindToView( KopeteView *view )
{
	QTextEdit *t = static_cast<QTextEdit*>( view->editWidget() );
	t->installEventFilter( this );
	t->viewport()->installEventFilter( this );

	if( !mSingleSpell )
	{
		mSingleSpell = new KSpell(0L, i18n("Spellcheck - Kopete"), this, SLOT( slotSingleSpellCheckerReady( KSpell * )));
		connect( mSingleSpell, SIGNAL( corrected( const QString&, const QString&, unsigned int ) ), this, SLOT( slotSingleCorrection( const QString&, const QString&, unsigned int ) ) );
	}
}

bool SpellCheckPlugin::eventFilter(QObject *o, QEvent *e)
{
	QTextEdit *t;
	if( o->inherits("QTextEdit") )
		t = static_cast<QTextEdit*>( o );
	else
		t = static_cast<QTextEdit*>( o->parent() );

	QWidget *w = static_cast<QWidget*>( o );

	switch( e->type() )
	{
		case QEvent::KeyPress:
		{
			QKeyEvent *event = (QKeyEvent*) e;
			if( singleSpellCheckerReady && QChar( event->ascii() ).isSpace() )
			{
				QString txtContents = t->text();
				QString lastWord = txtContents.section( mBound, -1, -1 );
				mSingleSpell->checkWord( lastWord );
			}
			break;
		}
		case QEvent::ContextMenu:
		{
			QContextMenuEvent *event = (QContextMenuEvent*) e;

			KPopupMenu p;
			int para = 0, charPos, firstSpace, lastSpace;

			charPos = t->charAt( event->pos(), &para );
			QString paraText = t->text( para );
			QString word = QString::null;

			if( !paraText.at(charPos).isSpace() )
			{
				firstSpace = paraText.findRev( mBound, charPos ) + 1;
				lastSpace = paraText.find( mBound, charPos );
				if( lastSpace == -1 )
					lastSpace = paraText.length();
				word = paraText.mid( firstSpace, lastSpace - firstSpace );

				if( !word.isNull() && mReplacements.contains( word ) )
				{
					p.insertTitle( i18n("Suggestions") );
					QStringList reps = mReplacements[word];
					int listPos = 0;
					for ( QStringList::Iterator it = reps.begin(); it != reps.end(); ++it ) {
						p.insertItem( *it, listPos );
						listPos++;
					}

					int id = p.exec( w->mapToGlobal( event->pos() ) );

					if( id > -1 )
					{
						QString txtContents = t->text();
						t->setText( txtContents.left(firstSpace) + mReplacements[word][id] +
							txtContents.right( txtContents.length() - lastSpace ) );
						slotUpdateTextEdit();
					}

					return true;
				}
			}

			break;
		}
		default:
			break;
	}

	return false;
}

void SpellCheckPlugin::slotCheckSpelling()
{
	kdDebug() << k_funcinfo << endl;

	mBuffer = mView->currentMessage();

	if( !mSpell )
	{
		mSpell = new KSpell(0L, i18n("Spellcheck - Kopete"), this, SLOT( slotSpellCheckerReady( KSpell * )));
		connect( mSpell, SIGNAL( misspelling( const QString&, const QStringList&, unsigned int ) ), this, SLOT( slotMisspelling( const QString&, const QStringList&, unsigned int ) ) );
		connect( mSpell, SIGNAL( corrected( const QString&, const QString&, unsigned int ) ), this, SLOT( slotCorrection( const QString&, const QString&, unsigned int ) ) );
		connect( mSpell, SIGNAL( done( const QString & ) ), this, SLOT( slotSpellDone() ) );
	}
}

void SpellCheckPlugin::slotSingleSpellCheckerReady( KSpell * )
{
	singleSpellCheckerReady = true;
}

void SpellCheckPlugin::slotSpellCheckerReady( KSpell * )
{
	mSpell->check( mBuffer.plainBody() );
}

void SpellCheckPlugin::slotSingleCorrection( const QString &originalword, const QString &, unsigned int )
{
	if( mSingleSpell->suggestions().count() > 0 )
	{
		mReplacements[originalword] = mSingleSpell->suggestions();
		slotUpdateTextEdit();
	}
}

void SpellCheckPlugin::slotUpdateTextEdit()
{
	QTextEdit *t = static_cast<QTextEdit*>( KopeteViewManager::viewManager()->activeView()->editWidget() );

	QString highlightColor;
	if( t->paletteForegroundColor().red() < 250 )
		highlightColor = QString::fromLatin1("red");
	else
		highlightColor = QString::fromLatin1("blue");

	QString plainTextContents = t->text();

	ReplacementMap::Iterator it;
	for ( it = mReplacements.begin(); it != mReplacements.end(); ++it )
	{
		plainTextContents.replace( QRegExp( QString::fromLatin1("\\b(%1)\\b").arg( it.key() ) ),
			QString::fromLatin1("<font color=\"" + highlightColor + "\">") + it.key() +
			QString::fromLatin1("</font>") );
	}

	t->setTextFormat( QTextEdit::RichText );
	t->setText( plainTextContents );
	t->setTextFormat( QTextEdit::PlainText );

	t->setCursorPosition( 0, t->text().length() + 1 );;
}

void SpellCheckPlugin::slotCorrection( const QString &originalword, const QString &newword, unsigned int pos )
{
	QString mBuff = mBuffer.plainBody();
	mBuff.replace( pos,originalword.length(), newword );
	mBuffer.setBody( mBuff );

	mView->setCurrentMessage( mBuffer );
}

void SpellCheckPlugin::slotMisspelling( const QString &originalword, const QStringList &, unsigned int pos )
{
	QTextEdit *t = dynamic_cast<QTextEdit*>( mView->editWidget() );
	t->setSelection( 0, pos, 0, pos + originalword.length() );
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
	spellCheckComplete = true;
}



