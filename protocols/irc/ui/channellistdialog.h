 /*
    channellist.h - IRC Channel Search Widget

    Copyright (c) 2004      by Jason Keirstead <jason@keirstead.org>

    Kopete    (c) 2004      by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef CHANNELLISTDIALOG_H
#define CHANNELLISTDIALOG_H

#include "channellist.h"

#include "kdialogbase.h"

class ChannelListDialog
	: public KDialogBase
{
	Q_OBJECT

	public:
		ChannelListDialog(KIRC::Engine *engine, const QString &caption, QObject *target, const char* slotJoinChan);

		void clear();

		void search();

	private slots:
		void slotChannelDoubleClicked( const QString & );

	private:
		KIRC::Engine *m_engine;
		ChannelList *m_list;
};

#endif
