
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KOPETE_JAVASCRIPTFILE_H
#define KOPETE_JAVASCRIPTFILE_H

#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QStringList>

class JavaScriptFile
	: public QObject
{
	Q_OBJECT
public:
	JavaScriptFile(QObject *parent = 0);

	QString script( bool reload = false );

	QString id;
	QString name;
	QString description;
	QString author;
	QString version;
	QStringList accounts;
	QString fileName;
	QMap<QString,QString> functions;
	bool immutable;

private:
	QString m_script;
};

#endif

