/*
    statisticsdb.cpp

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

#include "statisticsdb.h"

#include <QByteArray>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>

#include <kdebug.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kdeversion.h>

#include <time.h>

StatisticsDB::StatisticsDB()
{
	QString path = KStandardDirs::locateLocal ( "appdata", "kopete_statistics-0.1.db" );
	kDebug ( 14315 ) << "DB path:" << path;
	m_db = QSqlDatabase::addDatabase ( "QSQLITE", "kopete-statistics" );
	m_db.setDatabaseName(path);
	if (! m_db.open()){
		kError ( 14315 ) << "Unable to open database" << path;
		return;
	}

	// With synchronous mode turned off, SQLite does not call fsync (for every query).
	// System call fsync only ask kernel to flush buffers to disk and wait until complete.
	// It is expensive and not all filesystems implement it. Also it does not guarantee data lose.
	// If Kopete crash database will not be corrupted even if fsync will not called (but kernel must not crash).
	// Synchronous mode is turned on by default and degrades performance about 50 times.
	// On bigger database it cause big slowdown and changing status from online to offline can take ages...
	// When synchronous mode is off it dramatically speed up writes to database.
	// Synchronous mode has more disadvantages, so turn it off.
	query ( "PRAGMA synchronous = OFF" );

	has_transaction = m_db.driver()->hasFeature(QSqlDriver::Transactions);

	// Creates the tables if they do not exist.
	QStringList result = query ( "SELECT name FROM sqlite_master WHERE type='table'" );

	if ( !result.contains ( "contactstatus" ) )
	{
		kDebug ( 14315 ) << "Database empty";
		query ( QString ( "CREATE TABLE contactstatus "
		                  "(id INTEGER PRIMARY KEY,"
		                  "metacontactid TEXT,"
		                  "status TEXT,"
		                  "datetimebegin INTEGER,"
		                  "datetimeend INTEGER"
		                  ");" ) );
	}

	if ( !result.contains ( "commonstats" ) )
	{
		// To store things like the contact answer time etc.
		query ( QString ( "CREATE TABLE commonstats"
		                  " (id INTEGER PRIMARY KEY,"
		                  "metacontactid TEXT,"
		                  "statname TEXT," // for instance, answertime, lastmessage, messagelength ...
		                  "statvalue1 TEXT,"
		                  "statvalue2 TEXT"
		                  ");" ) );
	}

	if ( !result.contains ( "statsgroup" ) )
	{
		query ( QString ( "CREATE TABLE statsgroup"
		                  "(id INTEGER PRIMARY KEY,"
		                  "datetimebegin INTEGER,"
		                  "datetimeend INTEGER,"
		                  "caption TEXT);" ) );
	}

}

StatisticsDB::~StatisticsDB()
{
	m_db.close();
}

/**
 * Executes a SQL query on the already opened database
 * @param statement SQL program to execute. Only one SQL statement is allowed.
 * @param debug     Set to true for verbose debug output.
 * @retval names    Will contain all column names, set to NULL if not used.
 * @return          The queried data, or QStringList() on error.
 */
QStringList StatisticsDB::query ( const QString& statement, QStringList* const names, bool debug )
{

	if ( debug )
		kDebug ( 14315 ) << "query-start: " << statement;

	clock_t start = clock();

	QStringList values;
	QSqlQuery query ( m_db );

	// prepare query
	if ( !query.prepare ( statement ) )
	{
		kError ( 14315 ) << "error" << query.lastError().text() << "on query:" << statement;
		return QStringList();
	}

	// do query
	if ( !query.exec() )
	{
		kError ( 14315 ) << "error" << query.lastError().text() << "on query:" << statement;
		return QStringList();
	}
	
	int columns = query.record().count();
	while (query.next()) {
		for ( int i = 0; i < columns; i++ ){
			values << query.value(i).toString();
			if ( names )
				*names << query.record().fieldName(i);
		}
	}

	if ( debug )
	{
		clock_t finish = clock();
		const double duration = ( double ) ( finish - start ) / CLOCKS_PER_SEC;
		kDebug ( 14315 ) << "SQL-query (" << duration << "s): " << statement;
	}

	return values;
}

bool StatisticsDB::transaction()
{
	if (!has_transaction)
		return true;

	bool ret = m_db.transaction();
	if (!ret)
		kError ( 14315 ) << "failed to open transaction";

	return ret;
}

bool StatisticsDB::commit()
{
	if (!has_transaction)
		return true;

	bool ret = m_db.commit();
	if (!ret)
		kError ( 14315 ) << "failed to commit transaction:" << m_db.lastError().text();

	return ret;
}
