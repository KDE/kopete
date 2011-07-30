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

#include <kdebug.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmenu.h>
#include <ktoolbar.h>
#include <kauthorized.h>
#include <kicon.h>
#include <kemoticons.h>

#include "emoticonselector.h"
#include "kopeteemoticons.h"

class KopeteEmoticonAction::KopeteEmoticonActionPrivate
{
public:
	KopeteEmoticonActionPrivate()
	{
		m_popup = new KMenu(0L);
		emoticonSelector = new EmoticonSelector( m_popup );
		emoticonSelector->setObjectName( QLatin1String("KopeteEmoticonActionPrivate::emoticonSelector") );
//FIXME do it the kde4 way
//		m_popup->insertItem( static_cast<QObject*>(emoticonSelector) );
		// TODO: Maybe connect to kopeteprefs and redo list only on config changes
		QWidgetAction *act = new QWidgetAction(m_popup);
		act->setDefaultWidget(emoticonSelector);
		m_popup->addAction(act);
		connect( m_popup, SIGNAL(aboutToShow()), emoticonSelector, SLOT(prepareList()) );
	}

	~KopeteEmoticonActionPrivate()
	{
		delete m_popup;
		m_popup = 0;
	}

	KMenu *m_popup;
	EmoticonSelector *emoticonSelector;
};

KopeteEmoticonAction::KopeteEmoticonAction( QObject* parent )
  : KActionMenu( i18n( "Add Smiley" ), parent )
{
	d = new KopeteEmoticonActionPrivate;

	// Try to load the icon for our current emoticon theme, when it fails
	// fall back to our own default
	QString icon;
	QHash<QString, QStringList> emoticonsMap = Kopete::Emoticons::self()->theme().emoticonsMap();
	for( QHash<QString, QStringList>::const_iterator it = emoticonsMap.constBegin();
		it != emoticonsMap.constEnd(); ++it )
	{
		if( ( *it ).contains( ":)" ) || ( *it ).contains( ":-)" ) )
		{
			icon = it.key();
			break;
		}
	}


	setMenu( d->m_popup );

	if ( icon.isNull() )
		setIcon( KIcon("emoticon") );
	else
		setIcon( KIcon( icon ) );

	setShortcutConfigurable( false );
	connect( d->emoticonSelector, SIGNAL(itemSelected(QString)),
		this, SIGNAL(activated(QString)) );
}

KopeteEmoticonAction::~KopeteEmoticonAction()
{
//	kDebug(14010) << "KopeteEmoticonAction::~KopeteEmoticonAction()";
	delete d;
	d = 0;
}

#include "kopeteemoticonaction.moc"

// vim: set noet ts=4 sts=4 sw=4:

