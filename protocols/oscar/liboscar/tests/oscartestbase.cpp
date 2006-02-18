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
#include "buffer.h"
#include <QString>
#include <QFile>
#include <QByteArray>
#include <QDir>

OscarTestBase::OscarTestBase(const QString& path, QObject *parent)
: QObject(parent)
{
	m_dataDir = path;
	m_data = NULL;
}

OscarTestBase::~OscarTestBase()
{
	if ( m_data != NULL )
	{
		delete m_data;
	}
}

bool OscarTestBase::loadFile(const QString& file)
{
	if ( ! QFile::exists(m_dataDir + QDir::separator() + file) )
		return false;

	QFile datFile(m_dataDir + QDir::separator() + file);
	if ( m_data == NULL )
	{
		m_data = new Buffer(datFile.readAll());
	}
	else
	{
		delete m_data;
		m_data = NULL; //Safety
		m_data = new Buffer(datFile.readAll());
	}
	return true;
}

#include "oscartestbase.moc"
