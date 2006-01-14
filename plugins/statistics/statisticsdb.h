/*
    statisticsdb.h

    Copyright (c) 2003-2004 by Marc Cramdal        <marc.cramdal@gmail.com>


    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef _STATISTICSDB_H_H
#define _STATISTICSDB_H_H 1

typedef struct sqlite3;

class StatisticsDB
{
	
public:
	StatisticsDB();
	~StatisticsDB();
	 //sql helper methods
          QStringList query( const QString& statement, QStringList* const names = 0, bool debug = false );
          QString escapeString( QString string );
private:
	sqlite3 *m_db;
};

#endif

