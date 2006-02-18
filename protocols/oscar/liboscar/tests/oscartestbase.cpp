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

OscarTestBase::OscarTestBase(QString &path, QObject *parent)
: QObject(parent)
{
	m_dataDir = path;
	m_data = Buffer();
}

bool OscarTestBase::loadFile(QString &file)
{
	if ( ! QFile::exists(m_dataDir + QDir::separator() + file) )
		return false;

	QFile datFile(m_dataDir + QDir::separator() + file);
	m_data.addString(datFile.readAll());
	return true;
}
