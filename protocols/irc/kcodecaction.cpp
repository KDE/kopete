/*
    kcodecaction.cpp

    Copyright (c) 2003 by Jason Keirstead        <jason@keirstead.org>
    Kopete    (c) 2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/
#include <qstringlist.h>
#include <qtextcodec.h>

#include "kcodecaction.h"

KCodecAction::KCodecAction( const QString &text, const KShortcut &cut,
		QObject *parent, const char *name ) : KSelectAction( text, "", cut, parent, name )
{
	QObject::connect( this, SIGNAL( activated( int ) ),
		this, SLOT(slotActivated( int )) );

	QTextCodec *codec;
	QStringList items;

	for (uint i = 0; ( codec = QTextCodec::codecForIndex(i) ); ++i)
	{
        	items.append( codec->name() );
		codecMap.insert( i, codec );
	}

	setItems( items );
}

void KCodecAction::slotActivated( int index )
{
	emit activated( codecMap[ index ] );
}

void KCodecAction::setCodec( const QTextCodec *codec )
{
	for( QIntDictIterator<QTextCodec> it( codecMap ); it.current(); ++it )
	{
		if( it.current() == codec )
			setCurrentItem( it.currentKey() );
	}
}

#include "kcodecaction.moc"
