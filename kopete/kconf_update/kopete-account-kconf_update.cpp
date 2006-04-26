/*
    kconf_update app for migrating kopete 0.6.x accounts to 0.7. Code is
    not up to my normal standards, but it does the job, and since it's
    supposed to run exactly once on each system that's good enough for me :)

    Copyright (c) 2003      by Martijn Klingens <klingens@kde.org>

    Kopete    (c) 2002-2003 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <qmap.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <QStringList>

static QTextStream qcin ( stdin,  QIODevice::ReadOnly );
static QTextStream qcout( stdout, QIODevice::WriteOnly );
static QTextStream qcerr( stderr, QIODevice::WriteOnly );

// Group cache. Yes, I know global vars are ugly :)
bool needFlush = false;
QString accountId;
QString password;
QString autoConnect;
QString protocol;
QMap<QString, QString> pluginData;

// Global vars to hold separate IRC vars until we have read all of them
QString ircNick;
QString ircServer;
QString ircPort;

/*
 * Function for (en/de)crypting strings for config file, taken from KMail
 * Author: Stefan Taferner <taferner@alpin.or.at>
 */
QString cryptStr(const QString &aStr)
{
	QString result;
	for (unsigned int i = 0; i < aStr.length(); i++)
		result += (aStr[i].unicode() < 0x20) ? aStr[i] :
			QChar(0x1001F - aStr[i].unicode());
	return result;
}

void parseGroup( const QString &group, const QString &rawLine )
{
	// Groups that are converted can almost certainly be removed entirely

	if ( group == "MSN" || group == "ICQ" || group == "Oscar" || group == "Gadu" || group == "Jabber" || group == "IRC" )
	{
		accountId = "EMPTY";
		autoConnect = "true";

		if ( group == "Oscar" )
			protocol = "AIMProtocol";
		else
			protocol = group + "Protocol";

		password.clear();
		pluginData.clear();

		needFlush = true;

		qcout << "# DELETEGROUP [" << group << "]" << endl;
	}
	else
	{
		// Groups we don't convert. Output the raw line instead.
		qcout << rawLine << endl;
	}
}

void parseKey( const QString &group, const QString &key, const QString &value, const QString &rawLine )
{
	//qcerr << "*** group='" << group << "'" << endl;
	if ( group == "MSN" )
	{
		if ( key == "UserID" )
			accountId = value;
		else if ( key == "Password" )
			password = value;
		else if ( key == "AutoConnect" )
			autoConnect = value;
		else if ( key == "Nick" )
			pluginData[ "displayName" ] = value;

		// All other keys are ignored for MSN, as these apply to stuff that's
		// now in libkopete (and the main app) instead.
	}
	else if ( group == "ICQ" )
	{
		if ( key == "UIN" )
			accountId = value;
		else if ( key == "Password" )
			password = value;
		else if ( key == "AutoConnect" )
			autoConnect = value;
		else if ( key == "Nick" )
			pluginData[ "NickName" ] = value;
		else if ( key == "Server" )
			pluginData[ key ] = value;
		else if ( key == "Port" )
			pluginData[ key ] = value;
	}
	else if ( group == "Oscar" )
	{
		if ( key == "ScreenName" )
			accountId = value;
		else if ( key == "Password" )
			password = value;
		else if ( key == "Server" )
			pluginData[ key ] = value;
		else if ( key == "Port" )
			pluginData[ key ] = value;
 	}
	else if ( group == "Jabber" )
	{
		if ( key == "UserID" )
			accountId = value;
		else if ( key == "Password" )
			password = value;
		if ( key == "Server" ||
			 key == "Port" || key == "UseSSL" || key == "Resource" )
			pluginData[ key ] = value;
	}
	else if ( group == "Gadu" )
	{
		if ( key == "UIN" )
			accountId = value;
		else if ( key == "Password" )
			password = value;
		else if ( key == "Nick" )
			pluginData[ "displayName" ] = value;
	}
	else if ( group == "IRC" )
	{
		if ( key == "Nickname" )
			ircNick = value;
		if ( key == "Server" )
			ircServer = value;
		if ( key == "Port" )
			ircPort = value;
		if ( accountId == "EMPTY" &&
			 !ircNick.isEmpty( ) && !ircServer.isEmpty() &&
			 !ircPort.isEmpty() )
		{
			accountId = QString::fromLatin1( "%1@%2:%3" ).arg( ircNick, ircServer, ircPort );
		}
	}
	/*
		fixme: insert all other plugins here - martijn
	*/
	else if ( key == "Modules" )
	{
		QString newValue = value;
		newValue.replace ( ".plugin", ".desktop" );
		qcout << "Plugins=" << newValue;
	}
	else
	{
		// groups we don't convert. output the raw line instead.
		qcout << rawLine << endl;
	}
}

void flushData( const QString &group )
{

	qcout << "[Account_" << protocol << "_" << accountId << "]" << endl;
	qcout << "Protocol=" << protocol << endl;

	if( group == "Jabber" )
		qcout << "AccountId=" << accountId << "@" << pluginData["Server"] << endl;
	else
		qcout << "AccountId=" << accountId << endl;

	qcout << "Password=" << cryptStr( password ) << endl;
	qcout << "AutoConnect=" << autoConnect << endl;

	QMap<QString, QString>::ConstIterator it;
	for ( it = pluginData.begin(); it != pluginData.end(); ++it )
		qcout << "PluginData_" << protocol << "_" << it.key() << "=" << it.value() << endl;

}

int main()
{
	qcin.setCodec(QTextCodec::codecForName("UTF-8"));
	qcout.setCodec(QTextCodec::codecForName("UTF-8"));

	QString curGroup;

	QRegExp groupRegExp( "^\\[(.*)\\]" );
	QRegExp keyRegExp( "^([a-zA-Z0-9:, _-]*)\\s*=\\s*(.*)\\s*" );
	QRegExp commentRegExp( "^(#.*)?$" );

	while ( !qcin.atEnd() )
	{
		QString line = qcin.readLine();

		if ( commentRegExp.exactMatch( line ) )
		{
			// We found a comment, leave unchanged
			qcout << line << endl;
		}
		else if ( groupRegExp.exactMatch( line ) )
		{
			// We found the start of a group, parse it
			if ( needFlush )
			{
				// ... but we were already working on a group, so finish what
				// we were doing - flush existing group first
				flushData ( curGroup );
				needFlush = false;
			}

			curGroup = groupRegExp.capturedTexts()[ 1 ];
			parseGroup( curGroup, line );
		}
		else if ( keyRegExp.exactMatch( line ) )
		{
			// We found the a key line
			parseKey( curGroup, keyRegExp.capturedTexts()[ 1 ], keyRegExp.capturedTexts()[ 2 ], line );
		}
		else
		{
			qcerr << "** Unknown input line: " << line << endl;
		}
	}

	if ( needFlush )
		flushData ( curGroup );

	return 0;
}

// vim: set noet ts=4 sts=4 sw=4:

