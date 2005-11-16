 /*
    kopetechatwindowstyle.cpp - A Chat Window Style.

    Copyright (c) 2005      by Michaël Larouche     <michael.larouche@kdemail.net>

    Kopete    (c) 2002-2005 by the Kopete developers <kopete-devel@kde.org>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#include "kopetechatwindowstyle.h"

// Qt includes
#include <qfile.h>
#include <qtextstream.h>

// KDE includes
#include <kdebug.h>


class ChatWindowStyle::Private
{
public:
	QString stylePath;
	StyleVariants variantsList;
	QString baseHref;
	QString currentVariantPath;

	QString headerHtml;
	QString footerHtml;
	QString incomingHtml;
	QString outgoingHtml;
	QString statusHtml;
};

ChatWindowStyle::ChatWindowStyle(const QString &stylePath, int styleBuildMode)
	: d(new Private)
{
	// TODO: Add a private method init or create() to do not duplicate code.
	d->stylePath = stylePath;
	d->baseHref = stylePath + QString::fromUtf8("/Contents/Resources/");
	
	readStyleFiles();
	if(styleBuildMode & StyleBuildNormal)
	{
		// TODO: List all available variants
	}
}

ChatWindowStyle::ChatWindowStyle(const QString &stylePath, const QString &variantPath, int styleBuildMode)
	: d(new Private)
{
	d->stylePath = stylePath;
	d->baseHref = stylePath + QString::fromUtf8("/Contents/Resources/");
	d->currentVariantPath = variantPath;

	readStyleFiles();
	if(styleBuildMode & StyleBuildNormal)
	{
		// TODO: List all available variants
	}
}

ChatWindowStyle::~ChatWindowStyle()
{
	delete d;
}

ChatWindowStyle::StyleVariants ChatWindowStyle::getVariants() const
{
	return d->variantsList;
}

QString ChatWindowStyle::getStylePath() const
{
	return d->stylePath;
}

QString ChatWindowStyle::getStyleBaseHref() const
{
	return d->baseHref;
}

QString ChatWindowStyle::getHeaderHtml() const
{
	return d->headerHtml;
}

QString ChatWindowStyle::getFooterHtml() const
{
	return d->footerHtml;
}

QString ChatWindowStyle::getIncomingHtml() const
{
	return d->incomingHtml;
}

QString ChatWindowStyle::getOutgoingHtml() const
{
	return d->outgoingHtml;
}

QString ChatWindowStyle::getStatusHtml() const
{
	return d->statusHtml;
}

void ChatWindowStyle::readStyleFiles()
{
	QString headerFile = d->baseHref + QString("Header.html");
	QString footerFile = d->baseHref + QString("Footer.html");
	QString incomingFile = d->baseHref + QString("Incoming/Content.html");
	QString outgoingFile = d->baseHref + QString("Outgoing/Content.html");
	QString statusFile = d->baseHref + QString("Status.html");

	QFile fileAccess;
	// First load header file.
	if( QFile::exists(headerFile) )
	{
		fileAccess.setName(headerFile);
		fileAccess.open(QIODevice::ReadOnly);
		QTextStream headerStream(&fileAccess);
		headerStream.setEncoding(QTextStream::UnicodeUTF8);
		d->headerHtml = headerStream.read();
		kdDebug(14000) << k_funcinfo << d->headerHtml << endl;
		fileAccess.close();
	}
	// Load Footer file
	if( QFile::exists(footerFile) )
	{
		fileAccess.setName(footerFile);
		fileAccess.open(QIODevice::ReadOnly);
		QTextStream headerStream(&fileAccess);
		headerStream.setEncoding(QTextStream::UnicodeUTF8);
		d->footerHtml = headerStream.read();
		kdDebug(14000) << k_funcinfo << d->footerHtml << endl;
		fileAccess.close();
	}
	// Load incoming file
	if( QFile::exists(incomingFile) )
	{
		fileAccess.setName(incomingFile);
		fileAccess.open(QIODevice::ReadOnly);
		QTextStream headerStream(&fileAccess);
		headerStream.setEncoding(QTextStream::UnicodeUTF8);
		d->incomingHtml = headerStream.read();
		kdDebug(14000) << k_funcinfo << d->incomingHtml << endl;
		fileAccess.close();
	}
	// Load outgoing file
	if( QFile::exists(outgoingFile) )
	{
		fileAccess.setName(outgoingFile);
		fileAccess.open(QIODevice::ReadOnly);
		QTextStream headerStream(&fileAccess);
		headerStream.setEncoding(QTextStream::UnicodeUTF8);
		d->outgoingHtml = headerStream.read();
		kdDebug(14000) << k_funcinfo << d->outgoingHtml << endl;
		fileAccess.close();
	}
	// Load status file
	if( QFile::exists(statusFile) )
	{
		fileAccess.setName(statusFile);
		fileAccess.open(QIODevice::ReadOnly);
		QTextStream headerStream(&fileAccess);
		headerStream.setEncoding(QTextStream::UnicodeUTF8);
		d->statusHtml = headerStream.read();
		kdDebug(14000) << k_funcinfo << d->statusHtml << endl;
		fileAccess.close();
	}
}
