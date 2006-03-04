/*
    oscartestbase.cpp - OSCAR Testlib base

    Copyright (c) 2006 by Brian Smith <linuxfood@linuxfood.net>

    Kopete    (c) 2002-2006 by the Kopete developers  <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include "oscartestbase.h"
#include <QFile>
#include <QByteArray>
#include <QDir>
#include "buffer.h"

OscarTestBase::OscarTestBase()
    : m_data( 0 )
{
}


OscarTestBase::~OscarTestBase()
{
	delete m_data;
}

void OscarTestBase::setPath( const QString& path )
{
	m_dataDir = path;
}

bool OscarTestBase::loadFile(const QString& file)
{
	if ( ! QFile::exists(m_dataDir + QDir::separator() + file) )
		return false;

	QFile datFile(m_dataDir + QDir::separator() + file);
	if (! datFile.open(QIODevice::ReadOnly))
		return false;
	if ( m_data != NULL )
	{
		delete m_data;
		m_data = NULL; //Safety
	}
	m_data = new Buffer(datFile.readAll());
	datFile.close();
	if (m_data->length() == 0)
		return false; //unless it's an empty file, we must have failed
	return true;
}

#include "oscartestbase.moc"
