/*
    kopeteemoticonaction.cpp

    KAction to show the emoticon selector

    Copyright (c) 2002      by Stefan Gehn            <metz AT gehn.net>
    Copyright (c) 2003      by Martijn Klingens       <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopeteemoticonaction.h"

#include <math.h>

#include <qwhatsthis.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kpopupmenu.h>
#include <ktoolbar.h>
#include <ktoolbarbutton.h>

#include "emoticonselector.h"
#include "kopeteemoticons.h"

class KopeteEmoticonAction::KopeteEmoticonActionPrivate
{
public:
	KopeteEmoticonActionPrivate()
	{
		m_delayed = true;
		m_stickyMenu = true;
		m_popup = new KPopupMenu(0L,"KopeteEmoticonActionPrivate::m_popup");
		emoticonSelector = new EmoticonSelector( m_popup, "KopeteEmoticonActionPrivate::emoticonSelector");
		m_popup->insertItem( emoticonSelector );
		// TODO: Maybe connect to kopeteprefs and redo list only on config changes
		connect( m_popup, SIGNAL( aboutToShow() ), emoticonSelector, SLOT( prepareList() ) );
	}

	~KopeteEmoticonActionPrivate()
	{
		delete m_popup;
		m_popup = 0;
	}

	KPopupMenu *m_popup;
	EmoticonSelector *emoticonSelector;
	bool m_delayed;
	bool m_stickyMenu;
};

KopeteEmoticonAction::KopeteEmoticonAction( QObject* parent, const char* name )
  : KAction( i18n( "Add Smiley" ), 0, parent, name )
{
	d = new KopeteEmoticonActionPrivate;

	// Try to load the icon for our current emoticon theme, when it fails
	// fall back to our own default
	QString icon;
	QMap<QString, QStringList> emoticonsMap = Kopete::Emoticons::self()->emoticonAndPicList();
	for( QMap<QString, QStringList>::const_iterator it = emoticonsMap.constBegin();
		it != emoticonsMap.constEnd(); ++it )
	{
		if( ( *it ).contains( ":)" ) || ( *it ).contains( ":-)" ) )
		{
			icon = it.key();
			break;
		}
	}

	if ( icon.isNull() )
		setIcon( "emoticon" );
	else
		setIconSet( QIconSet( icon ) );

	setShortcutConfigurable( false );
	connect( d->emoticonSelector, SIGNAL( ItemSelected( const QString & ) ),
		this, SIGNAL( activated( const QString & ) ) );
}

KopeteEmoticonAction::~KopeteEmoticonAction()
{
	unplugAll();
//	kdDebug(14010) << "KopeteEmoticonAction::~KopeteEmoticonAction()" << endl;
	delete d;
	d = 0;
}

void KopeteEmoticonAction::popup( const QPoint& global )
{
	popupMenu()->popup( global );
}

KPopupMenu* KopeteEmoticonAction::popupMenu() const
{
	return d->m_popup;
}

bool KopeteEmoticonAction::delayed() const
{
	return d->m_delayed;
}

void KopeteEmoticonAction::setDelayed(bool _delayed)
{
	d->m_delayed = _delayed;
}

bool KopeteEmoticonAction::stickyMenu() const
{
	return d->m_stickyMenu;
}

void KopeteEmoticonAction::setStickyMenu(bool sticky)
{
	d->m_stickyMenu = sticky;
}

int KopeteEmoticonAction::plug( QWidget* widget, int index )
{
	if (kapp && !kapp->authorizeKAction(name()))
		return -1;

//	kdDebug(14010) << "KopeteEmoticonAction::plug( " << widget << ", " << index << " )" << endl;

	// KDE4/Qt TODO: Use qobject_cast instead.
	if ( widget->inherits("QPopupMenu") )
	{
		QPopupMenu* menu = static_cast<QPopupMenu*>( widget );
		int id;
		if ( hasIcon() )
			id = menu->insertItem( iconSet(KIcon::Small), text(), d->m_popup, -1, index );
		else
			id = menu->insertItem( text(), d->m_popup, -1, index );

		if ( !isEnabled() )
			menu->setItemEnabled( id, false );

		addContainer( menu, id );
		connect( menu, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

		if ( m_parentCollection )
			m_parentCollection->connectHighlight( menu, this );

		return containerCount() - 1;
	}
	// KDE4/Qt TODO: Use qobject_cast instead.
	else if ( widget->inherits( "KToolBar" ) )
	{
		KToolBar *bar = static_cast<KToolBar *>( widget );

		int id_ = KAction::getToolButtonID();

		if ( icon().isEmpty() && !iconSet(KIcon::Small).isNull() )
		{
			bar->insertButton(
				iconSet(KIcon::Small).pixmap(), id_, SIGNAL(clicked()), this,
				SLOT(slotActivated()), isEnabled(), plainText(),
				index );
		}
		else
		{
			KInstance *instance;

			if ( m_parentCollection )
			instance = m_parentCollection->instance();
			else
			instance = KGlobal::instance();

			bar->insertButton( icon(), id_, SIGNAL( clicked() ), this,
									SLOT( slotActivated() ), isEnabled(), plainText(),
									index, instance );
		}

		addContainer( bar, id_ );

		if (!whatsThis().isEmpty())
			QWhatsThis::add( bar->getButton(id_), whatsThis() );

		connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

		if (delayed())
			bar->setDelayedPopup(id_, popupMenu(), stickyMenu());
		else
			bar->getButton(id_)->setPopup(popupMenu(), stickyMenu());

		if ( m_parentCollection )
			m_parentCollection->connectHighlight(bar, this);

		return containerCount() - 1;
	}
	// KDE4/Qt TODO: Use qobject_cast instead.
	else if ( widget->inherits( "QMenuBar" ) )
	{
		QMenuBar *bar = static_cast<QMenuBar *>( widget );

		int id;

		id = bar->insertItem( text(), popupMenu(), -1, index );

		if ( !isEnabled() )
			bar->setItemEnabled( id, false );

		addContainer( bar, id );
		connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

		return containerCount() - 1;
	}

	return -1;
}

#include "kopeteemoticonaction.moc"

// vim: set noet ts=4 sts=4 sw=4:

