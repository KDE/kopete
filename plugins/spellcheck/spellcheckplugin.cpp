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
#include "spellcheckpreferences.h"

K_EXPORT_COMPONENT_FACTORY( kopete_spellcheck, KGenericFactory<SpellCheckPlugin> );

SpellCheckPlugin::SpellCheckPlugin( QObject *parent, const char *name, const QStringList &) : KopetePlugin( parent, name )
{
	if ( !pluginStatic_ )
		pluginStatic_ = this;

	m_actionCollection = 0L;
	mSpell = 0L;
	mSingleSpell = 0L;
	mView = 0L;
	words = 0L;
	singleSpellCheckerReady = false;
	mBound = QRegExp( QString::fromLatin1("[\\s\\W]") );
	mPrefs = new SpellCheckPreferences( QString::fromLatin1("spellcheck"), this );

	connect( mPrefs, SIGNAL( saved() ), this, SLOT( slotPrefsSaved() ) );
	connect( KopeteViewManager::viewManager(), SIGNAL( viewCreated( KopeteView * ) ), this, SLOT( slotBindToView( KopeteView * ) ) );

	slotPrefsSaved();
}

SpellCheckPlugin::~SpellCheckPlugin()
{
	pluginStatic_ = 0L;
	delete words;
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

void SpellCheckPlugin::slotPrefsSaved()
{
	if( mPrefs->autoCheckEnabled() )
	{
		if( !mSingleSpell )
		{
  			mSingleSpell = new KSpell(0L, i18n("Spellcheck - Kopete"), this, SLOT( slotSingleSpellCheckerReady( KSpell * )), mPrefs->spellConfig() );
			connect( mSingleSpell, SIGNAL( misspelling( const QString&, const QStringList&, unsigned int ) ), this, SLOT( slotSingleCorrection( const QString&, const QStringList&, unsigned int ) ) );
		}
	}
	else
	{
		if( mSingleSpell )
		{
			mSingleSpell->cleanUp();
			delete mSingleSpell;
			singleSpellCheckerReady = false;
			slotUpdateTextEdit();
		}
	}
}

KActionCollection *SpellCheckPlugin::customChatActions( KopeteMessageManager *manager )
{
	kdDebug() << k_funcinfo << endl;

	delete m_actionCollection;

	mView = KopeteViewManager::viewManager()->view( manager, false);

	m_actionCollection = new KActionCollection(this);

	KAction *spellCheck = new  KAction( i18n( "Check S&pelling" ), QString::fromLatin1( "spellcheck" ),
		mPrefs->shortCut(), this, SLOT( slotCheckSpelling() ), m_actionCollection, "checkSpelling" );

	m_actionCollection->insert ( spellCheck );

	return m_actionCollection;
}

void SpellCheckPlugin::slotBindToView( KopeteView *view )
{
	QTextEdit *t = static_cast<QTextEdit*>( view->editWidget() );
	t->installEventFilter( this );
	t->viewport()->installEventFilter( this );
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
			QChar typedChar = QChar( event->ascii() );
			if( singleSpellCheckerReady && !typedChar.isLetterOrNumber() )
			{
				delete words;
				words = new QStringList( QStringList::split( mBound, t->text() ) );
				if( words->count() > 0 )
					mSingleSpell->checkList( words, false );
			}
			slotUpdateTextEdit();
			break;
		}
		case QEvent::ContextMenu:
		{
			if ( singleSpellCheckerReady )
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
						if( reps.count() > 0 )
						{
							int listPos = 0;
							for ( QStringList::Iterator it = reps.begin(); it != reps.end(); ++it ) {
								p.insertItem( *it, listPos );
								listPos++;
							}
						}
						else
						{
							p.insertItem( QString::fromLatin1("No Suggestions"), -2 );
						}

						int id = p.exec( w->mapToGlobal( event->pos() ) );

						if( id > -1 )
						{
							int parIdx = 1, txtIdx = 1;
							t->getCursorPosition(&parIdx, &txtIdx);

							QString txtContents = t->text();
							QString newContents = txtContents.left(firstSpace) + mReplacements[word][id] +
								txtContents.right( txtContents.length() - lastSpace );

							t->setText( newContents );
							if( txtIdx > lastSpace )
								txtIdx += newContents.length() - txtContents.length();

							t->setCursorPosition(parIdx, txtIdx);

							if( !txtContents.contains( QRegExp(QString::fromLatin1("\\b(%1)\\b").arg(word)) ) )
								mReplacements.remove( word );

							slotUpdateTextEdit();
						}

						return true;
					}
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
		mSpell = new KSpell(0L, i18n("Spellcheck - Kopete"), this, SLOT( slotSpellCheckerReady( KSpell * )), mPrefs->spellConfig() );
		connect( mSpell, SIGNAL( misspelling( const QString&, const QStringList&, unsigned int ) ), this, SLOT( slotMisspelling( const QString&, const QStringList&, unsigned int ) ) );
		connect( mSpell, SIGNAL( corrected( const QString&, const QString&, unsigned int ) ), this, SLOT( slotCorrection( const QString&, const QString&, unsigned int ) ) );
		connect( mSpell, SIGNAL( done( const QString & ) ), this, SLOT( slotSpellDone( const QString &) ) );
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

void SpellCheckPlugin::slotSingleCorrection( const QString &originalword, const QStringList &suggestions, unsigned int )
{
	if( !mReplacements.contains( originalword ) )
	{
		kdDebug() << k_funcinfo << originalword << "IS MISSPELLED!" << endl;
		mReplacements[originalword] = suggestions;
	}
	slotUpdateTextEdit();
}

void SpellCheckPlugin::slotUpdateTextEdit()
{
	KopeteView *activeView = KopeteViewManager::viewManager()->activeView();
	if( activeView )
	{
		QTextEdit *t = static_cast<QTextEdit*>( activeView->editWidget() );

		QString plainTextContents = t->text();

		int parIdx = 1, txtIdx = 1;
		t->getCursorPosition(&parIdx, &txtIdx);

		if( singleSpellCheckerReady )
		{
			QString highlightColor;
			if( t->paletteForegroundColor().red() < 250 )
				highlightColor = QString::fromLatin1("red");
			else
				highlightColor = QString::fromLatin1("blue");

			QStringList words = QStringList::split( mBound, plainTextContents );
			if( words.count() > 0 )
			{
				for( QStringList::Iterator it = words.begin(); it != words.end(); ++it )
				{
					if( mReplacements.contains(*it) )
					{
						plainTextContents.replace( QRegExp( QString::fromLatin1("\\b(%1)\\b").arg( *it ) ),
							QString::fromLatin1("<font color=\"" + highlightColor + "\">") + *it +
							QString::fromLatin1("</font>") );
					}
				}
			}
		}

		t->setTextFormat( QTextEdit::RichText );
		t->setText( plainTextContents );
		t->setTextFormat( QTextEdit::PlainText );

		t->setCursorPosition( parIdx, txtIdx );;
	}
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
}



