/*
    statisticsdb.cpp

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

#include <qfile.h>

#include "sqlite/sqlite3.h"

#include <kgenericfactory.h>
#include <kaboutdata.h>
#include <kaction.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kdeversion.h>

#include "statisticsdb.h"

#include <unistd.h>
#include <time.h>

StatisticsDB::StatisticsDB()
{
	QCString path = (::locateLocal("appdata", "kopete_statistics-0.1.db")).latin1();
	kdDebug() << "statistics: DB path:" << path << endl;

	// Open database file and check for correctness
	bool failOpen = true;
	QFile file( path );
	if ( file.open( IO_ReadOnly ) ) 
	{
		QString format;
		file.readLine( format, 50 );
		if ( !format.startsWith( "SQLite format 3" ) ) 
		{
			kdWarning() << "[statistics] Database versions incompatible. Removing and rebuilding database.\n";
		}
		else if ( sqlite3_open( path, &m_db ) != SQLITE_OK ) 
		{
			kdWarning() << "[statistics] Database file corrupt. Removing and rebuilding database.\n";
			sqlite3_close( m_db );
		}
		else
			failOpen = false;
	}
	
	if ( failOpen ) 
	{
	// Remove old db file; create new
	QFile::remove( path );
	sqlite3_open( path, &m_db );
	}

	kdDebug() << "[Statistics] Contructor"<< endl;

	// Creates the tables if they do not exist.
	QStringList result = query("SELECT name FROM sqlite_master WHERE type='table'");
	
	if (!result.contains("contacts"))
	{
		query(QString("CREATE TABLE contacts "
			"(id INTEGER PRIMARY KEY,"
			"statisticid TEXT,"
			"contactid TEXT"
			");"));
	}

	if (!result.contains("contactstatus"))
	{
		kdDebug() << "[Statistics] Database empty"<< endl;
		query(QString("CREATE TABLE contactstatus "
			"(id INTEGER PRIMARY KEY,"
			"metacontactid TEXT,"
			"status TEXT,"
			"datetimebegin INTEGER,"
			"datetimeend INTEGER"
			");"));
	}
	
	if (!result.contains("commonstats"))
	{
		// To store things like the contact answer time etc.
		query(QString("CREATE TABLE commonstats"
			" (id INTEGER PRIMARY KEY,"
			"metacontactid TEXT,"
			"statname TEXT," // for instance, answertime, lastmessage, messagelength ...
			"statvalue1 TEXT,"
			"statvalue2 TEXT"
			");"));
	}

	/// @fixme This is not used anywhere
	if (!result.contains("statsgroup"))
	{
		query(QString("CREATE TABLE statsgroup"
			"(id INTEGER PRIMARY KEY,"
			"datetimebegin INTEGER,"
			"datetimeend INTEGER,"
			"caption TEXT);"));
	}
 
}

StatisticsDB::~StatisticsDB()
{
	sqlite3_close(m_db);
} 

 /**
  * Executes a SQL query on the already opened database
  * @param statement SQL program to execute. Only one SQL statement is allowed.
  * @param debug     Set to true for verbose debug output.
  * @retval names    Will contain all column names, set to NULL if not used.
  * @return          The queried data, or QStringList() on error.
  */
 QStringList StatisticsDB::query( const QString& statement, QStringList* const names, bool debug )
 {
 
     if ( debug )
         kdDebug() << "query-start: " << statement << endl;
 
     clock_t start = clock();
 
     if ( !m_db )
     {
         kdError() << k_funcinfo << "[CollectionDB] SQLite pointer == NULL.\n";
         return QStringList();
     }
 
     int error;
     QStringList values;
     const char* tail;
     sqlite3_stmt* stmt;
 
     //compile SQL program to virtual machine
     error = sqlite3_prepare( m_db, statement.utf8(), statement.length(), &stmt, &tail );
 
     if ( error != SQLITE_OK )
     {
         kdError() << k_funcinfo << "[CollectionDB] sqlite3_compile error:" << endl;
         kdError() << sqlite3_errmsg( m_db ) << endl;
         kdError() << "on query: " << statement << endl;
 
         return QStringList();
     }
 
     int busyCnt = 0;
     int number = sqlite3_column_count( stmt );
     //execute virtual machine by iterating over rows
     while ( true )
     {
         error = sqlite3_step( stmt );
 
         if ( error == SQLITE_BUSY )
         {
             if ( busyCnt++ > 20 ) {
                 kdError() << "[CollectionDB] Busy-counter has reached maximum. Aborting this sql statement!\n";
                 break;
             }
             ::usleep( 100000 ); // Sleep 100 msec
             kdDebug() << "[CollectionDB] sqlite3_step: BUSY counter: " << busyCnt << endl;
         }
         if ( error == SQLITE_MISUSE )
             kdDebug() << "[CollectionDB] sqlite3_step: MISUSE" << endl;
         if ( error == SQLITE_DONE || error == SQLITE_ERROR )
             break;
 
         //iterate over columns
         for ( int i = 0; i < number; i++ )
         {
             values << QString::fromUtf8( (const char*) sqlite3_column_text( stmt, i ) );
             if ( names ) *names << QString( sqlite3_column_name( stmt, i ) );
         }
     }
     //deallocate vm ressources
     sqlite3_finalize( stmt );
 
     if ( error != SQLITE_DONE )
     {
         kdError() << k_funcinfo << "sqlite_step error.\n";
         kdError() << sqlite3_errmsg( m_db ) << endl;
         kdError() << "on query: " << statement << endl;
 
         return QStringList();
     }
 
     if ( debug )
     {
         clock_t finish = clock();
         const double duration = (double) (finish - start) / CLOCKS_PER_SEC;
         kdDebug() << "[CollectionDB] SQL-query (" << duration << "s): " << statement << endl;
     }
 
 
    return values;
}
