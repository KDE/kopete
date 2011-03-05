/*
    statisticsdb.h

    Copyright (c) 2003-2004 by Marc Cramdal        <marc.cramdal@gmail.com>

    Copyright (c) 2007      by the Kopete Developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef _STATISTICSDB_H
#define _STATISTICSDB_H

#include <QSqlDatabase>

class StatisticsDB
{

	public:
		StatisticsDB();
		~StatisticsDB();
		//sql helper methods
		QStringList query ( const QString& statement, QStringList* const names = 0, bool debug = false );
		QString escapeString ( QString string );

		bool transaction ();
		bool commit ();
	private:
		QSqlDatabase m_db;
		bool has_transaction;
};

#endif

