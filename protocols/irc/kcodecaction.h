/*
   kcodecaction.h

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
#ifndef KCODECACTION_H
#define KCODECACTION_H

#include <kdeversion.h>
#include <qintdict.h>

#if KDE_IS_VERSION( 3, 1, 90 )
	#include <kactionclasses.h>
#else
	#include <kaction.h>
#endif

class KCodecAction : public KSelectAction
{
	Q_OBJECT
	public:
		KCodecAction( const QString &text, const KShortcut &cut = KShortcut(),
			QObject *parent = 0, const char *name = 0 );

		void setCodec( const QTextCodec *codec );

	signals:
		void activated( const QTextCodec * );

	private slots:
		void slotActivated( int );

	private:
		QIntDict<QTextCodec> codecMap;
};

#endif
