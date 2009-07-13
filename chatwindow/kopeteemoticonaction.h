/*
    kopeteemoticonaction.h

    KAction to show the emoticon selector

    Copyright (c) 2002      by Stefan Gehn            <metz@gehn.net>
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

#ifndef _KOPETEEMOTICONACTION_H_
#define _KOPETEEMOTICONACTION_H_

#include <KActionMenu>

#include <kopete_export.h>

class KOPETECHATWINDOW_SHARED_EXPORT KopeteEmoticonAction : public KActionMenu
{
	Q_OBJECT

public:
	KopeteEmoticonAction( QObject *parent );
	virtual ~KopeteEmoticonAction();
	
signals:
	void activated( const QString &item );

private:
	class KopeteEmoticonActionPrivate;
	KopeteEmoticonActionPrivate *d;

};

#endif

// vim: set noet ts=4 sts=4 sw=4:

